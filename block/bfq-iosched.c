/*
 * Budget Fair Queueing (BFQ) disk scheduler.
 *
 * Based on ideas and code from CFQ:
 * Copyright (C) 2003 Jens Axboe <axboe@kernel.dk>
 *
 * Copyright (C) 2008 Fabio Checconi <fabio@gandalf.sssup.it>
 *		      Paolo Valente <paolo.valente@unimore.it>
 *
 * Copyright (C) 2010 Paolo Valente <paolo.valente@unimore.it>
 *
 * Licensed under the GPL-2 as detailed in the accompanying COPYING.BFQ
 * file.
 *
 * BFQ is a proportional-share storage-I/O scheduling algorithm based on
 * the slice-by-slice service scheme of CFQ. But BFQ assigns budgets,
 * measured in number of sectors, to processes instead of time slices. The
 * device is not granted to the in-service process for a given time slice,
 * but until it has exhausted its assigned budget. This change from the time
 * to the service domain allows BFQ to distribute the device throughput
 * among processes as desired, without any distortion due to ZBR, workload
 * fluctuations or other factors. BFQ uses an ad hoc internal scheduler,
 * called B-WF2Q+, to schedule processes according to their budgets. More
 * precisely, BFQ schedules queues associated to processes. Thanks to the
 * accurate policy of B-WF2Q+, BFQ can afford to assign high budgets to
 * I/O-bound processes issuing sequential requests (to boost the
 * throughput), and yet guarantee a low latency to interactive and soft
 * real-time applications.
 *
 * BFQ is described in [1], where also a reference to the initial, more
 * theoretical paper on BFQ can be found. The interested reader can find
 * in the latter paper full details on the main algorithm, as well as
 * formulas of the guarantees and formal proofs of all the properties.
 * With respect to the version of BFQ presented in these papers, this
 * implementation adds a few more heuristics, such as the one that
 * guarantees a low latency to soft real-time applications, and a
 * hierarchical extension based on H-WF2Q+.
 *
 * B-WF2Q+ is based on WF2Q+, that is described in [2], together with
 * H-WF2Q+, while the augmented tree used to implement B-WF2Q+ with O(log N)
 * complexity derives from the one introduced with EEVDF in [3].
 *
 * [1] P. Valente and M. Andreolini, ``Improving Application Responsiveness
 *     with the BFQ Disk I/O Scheduler'',
 *     Proceedings of the 5th Annual International Systems and Storage
 *     Conference (SYSTOR '12), June 2012.
 *
 * http://algogroup.unimo.it/people/paolo/disk_sched/bf1-v1-suite-results.pdf
 *
 * [2] Jon C.R. Bennett and H. Zhang, ``Hierarchical Packet Fair Queueing
 *     Algorithms,'' IEEE/ACM Transactions on Networking, 5(5):675-689,
 *     Oct 1997.
 *
 * http://www.cs.cmu.edu/~hzhang/papers/TON-97-Oct.ps.gz
 *
 * [3] I. Stoica and H. Abdel-Wahab, ``Earliest Eligible Virtual Deadline
 *     First: A Flexible and Accurate Mechanism for Proportional Share
 *     Resource Allocation,'' technical report.
 *
 * http://www.cs.berkeley.edu/~istoica/papers/eevdf-tr-95.pdf
 */
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/blkdev.h>
#include <linux/cgroup.h>
#include <linux/elevator.h>
#include <linux/jiffies.h>
#include <linux/rbtree.h>
#include <linux/ioprio.h>
#include "bfq.h"
#include "blk.h"

/* Expiration time of sync (0) and async (1) requests, in jiffies. */
static const int bfq_fifo_expire[2] = { HZ / 4, HZ / 8 };

/* Maximum backwards seek, in KiB. */
static const int bfq_back_max = 16 * 1024;

/* Penalty of a backwards seek, in number of sectors. */
static const int bfq_back_penalty = 2;

/* Idling period duration, in jiffies. */
static int bfq_slice_idle = HZ / 125;

/* Default maximum budget values, in sectors and number of requests. */
static const int bfq_default_max_budget = 16 * 1024;
static const int bfq_max_budget_async_rq = 4;

/*
 * Async to sync throughput distribution is controlled as follows:
 * when an async request is served, the entity is charged the number
 * of sectors of the request, multiplied by the factor below
 */
static const int bfq_async_charge_factor = 10;

/* Default timeout values, in jiffies, approximating CFQ defaults. */
static const int bfq_timeout_sync = HZ / 8;
static int bfq_timeout_async = HZ / 25;

struct kmem_cache *bfq_pool;

/* Below this threshold (in ms), we consider thinktime immediate. */
#define BFQ_MIN_TT		2

/* hw_tag detection: parallel requests threshold and min samples needed. */
#define BFQ_HW_QUEUE_THRESHOLD	4
#define BFQ_HW_QUEUE_SAMPLES	32

#define BFQQ_SEEK_THR	 (sector_t)(8 * 1024)
#define BFQQ_SEEKY(bfqq) ((bfqq)->seek_mean > BFQQ_SEEK_THR)

/* Min samples used for peak rate estimation (for autotuning). */
#define BFQ_PEAK_RATE_SAMPLES	32

/* Shift used for peak rate fixed precision calculations. */
#define BFQ_RATE_SHIFT		16

/*
 * By default, BFQ computes the duration of the weight raising for
 * interactive applications automatically, using the following formula:
 * duration = (R / r) * T, where r is the peak rate of the device, and
 * R and T are two reference parameters.
 * In particular, R is the peak rate of the reference device (see below),
 * and T is a reference time: given the systems that are likely to be
 * installed on the reference device according to its speed class, T is
 * about the maximum time needed, under BFQ and while reading two files in
 * parallel, to load typical large applications on these systems.
 * In practice, the slower/faster the device at hand is, the more/less it
 * takes to load applications with respect to the reference device.
 * Accordingly, the longer/shorter BFQ grants weight raising to interactive
 * applications.
 *
 * BFQ uses four different reference pairs (R, T), depending on:
 * . whether the device is rotational or non-rotational;
 * . whether the device is slow, such as old or portable HDDs, as well as
 *   SD cards, or fast, such as newer HDDs and SSDs.
 *
 * The device's speed class is dynamically (re)detected in
 * bfq_update_peak_rate() every time the estimated peak rate is updated.
 *
 * In the following definitions, R_slow[0]/R_fast[0] and T_slow[0]/T_fast[0]
 * are the reference values for a slow/fast rotational device, whereas
 * R_slow[1]/R_fast[1] and T_slow[1]/T_fast[1] are the reference values for
 * a slow/fast non-rotational device. Finally, device_speed_thresh are the
 * thresholds used to switch between speed classes.
 * Both the reference peak rates and the thresholds are measured in
 * sectors/usec, left-shifted by BFQ_RATE_SHIFT.
 */
static int R_slow[2] = {1536, 10752};
static int R_fast[2] = {17415, 34791};
/*
 * To improve readability, a conversion function is used to initialize the
 * following arrays, which entails that they can be initialized only in a
 * function.
 */
static int T_slow[2];
static int T_fast[2];
static int device_speed_thresh[2];

#define BFQ_SERVICE_TREE_INIT	((struct bfq_service_tree)		\
				{ RB_ROOT, RB_ROOT, NULL, NULL, 0, 0 })

#define RQ_BIC(rq)		((struct bfq_io_cq *) (rq)->elv.priv[0])
#define RQ_BFQQ(rq)		((rq)->elv.priv[1])

static inline void bfq_schedule_dispatch(struct bfq_data *bfqd);

#include "bfq-ioc.c"
#include "bfq-sched.c"
#include "bfq-cgroup.c"

#define bfq_class_idle(bfqq)	((bfqq)->entity.ioprio_class ==\
				 IOPRIO_CLASS_IDLE)
#define bfq_class_rt(bfqq)	((bfqq)->entity.ioprio_class ==\
				 IOPRIO_CLASS_RT)

#define bfq_sample_valid(samples)	((samples) > 80)

/*
 * The following macro groups conditions that need to be evaluated when
 * checking if existing queues and groups form a symmetric scenario
 * and therefore idling can be reduced or disabled for some of the
 * queues. See the comment to the function bfq_bfqq_must_not_expire()
 * for further details.
 */
#ifdef CONFIG_CGROUP_BFQIO
#define symmetric_scenario	  (!bfqd->active_numerous_groups && \
				   !bfq_differentiated_weights(bfqd))
#else
#define symmetric_scenario	  (!bfq_differentiated_weights(bfqd))
#endif

/*
 * We regard a request as SYNC, if either it's a read or has the SYNC bit
 * set (in which case it could also be a direct WRITE).
 */
static inline int bfq_bio_sync(struct bio *bio)
{
	if (bio_data_dir(bio) == READ || (bio->bi_rw & REQ_SYNC))
		return 1;

	return 0;
}

/*
 * Scheduler run of queue, if there are requests pending and no one in the
 * driver that will restart queueing.
 */
static inline void bfq_schedule_dispatch(struct bfq_data *bfqd)
{
	if (bfqd->queued != 0) {
		bfq_log(bfqd, "schedule dispatch");
		kblockd_schedule_work(bfqd->queue, &bfqd->unplug_work);
	}
}

/*
 * Lifted from AS - choose which of rq1 and rq2 that is best served now.
 * We choose the request that is closesr to the head right now.  Distance
 * behind the head is penalized and only allowed to a certain extent.
 */
static struct request *bfq_choose_req(struct bfq_data *bfqd,
				      struct request *rq1,
				      struct request *rq2,
				      sector_t last)
{
	sector_t s1, s2, d1 = 0, d2 = 0;
	unsigned long back_max;
#define BFQ_RQ1_WRAP	0x01 /* request 1 wraps */
#define BFQ_RQ2_WRAP	0x02 /* request 2 wraps */
	unsigned wrap = 0; /* bit mask: requests behind the disk head? */

	if (rq1 == NULL || rq1 == rq2)
		return rq2;
	if (rq2 == NULL)
		return rq1;

	if (rq_is_sync(rq1) && !rq_is_sync(rq2))
		return rq1;
	else if (rq_is_sync(rq2) && !rq_is_sync(rq1))
		return rq2;
	if ((rq1->cmd_flags & REQ_META) && !(rq2->cmd_flags & REQ_META))
		return rq1;
	else if ((rq2->cmd_flags & REQ_META) && !(rq1->cmd_flags & REQ_META))
		return rq2;

	s1 = blk_rq_pos(rq1);
	s2 = blk_rq_pos(rq2);

	/*
	 * By definition, 1KiB is 2 sectors.
	 */
	back_max = bfqd->bfq_back_max * 2;

	/*
	 * Strict one way elevator _except_ in the case where we allow
	 * short backward seeks which are biased as twice the cost of a
	 * similar forward seek.
	 */
	if (s1 >= last)
		d1 = s1 - last;
	else if (s1 + back_max >= last)
		d1 = (last - s1) * bfqd->bfq_back_penalty;
	else
		wrap |= BFQ_RQ1_WRAP;

	if (s2 >= last)
		d2 = s2 - last;
	else if (s2 + back_max >= last)
		d2 = (last - s2) * bfqd->bfq_back_penalty;
	else
		wrap |= BFQ_RQ2_WRAP;

	/* Found required data */

	/*
	 * By doing switch() on the bit mask "wrap" we avoid having to
	 * check two variables for all permutations: --> faster!
	 */
	switch (wrap) {
	case 0: /* common case for CFQ: rq1 and rq2 not wrapped */
		if (d1 < d2)
			return rq1;
		else if (d2 < d1)
			return rq2;
		else {
			if (s1 >= s2)
				return rq1;
			else
				return rq2;
		}

	case BFQ_RQ2_WRAP:
		return rq1;
	case BFQ_RQ1_WRAP:
		return rq2;
	case (BFQ_RQ1_WRAP|BFQ_RQ2_WRAP): /* both rqs wrapped */
	default:
		/*
		 * Since both rqs are wrapped,
		 * start with the one that's further behind head
		 * (--> only *one* back seek required),
		 * since back seek takes more time than forward.
		 */
		if (s1 <= s2)
			return rq1;
		else
			return rq2;
	}
}

static struct bfq_queue *
bfq_rq_pos_tree_lookup(struct bfq_data *bfqd, struct rb_root *root,
		     sector_t sector, struct rb_node **ret_parent,
		     struct rb_node ***rb_link)
{
	struct rb_node **p, *parent;
	struct bfq_queue *bfqq = NULL;

	parent = NULL;
	p = &root->rb_node;
	while (*p) {
		struct rb_node **n;

		parent = *p;
		bfqq = rb_entry(parent, struct bfq_queue, pos_node);

		/*
		 * Sort strictly based on sector. Smallest to the left,
		 * largest to the right.
		 */
		if (sector > blk_rq_pos(bfqq->next_rq))
			n = &(*p)->rb_right;
		else if (sector < blk_rq_pos(bfqq->next_rq))
			n = &(*p)->rb_left;
		else
			break;
		p = n;
		bfqq = NULL;
	}

	*ret_parent = parent;
	if (rb_link)
		*rb_link = p;

	bfq_log(bfqd, "rq_pos_tree_lookup %llu: returning %d",
		(long long unsigned)sector,
		bfqq != NULL ? bfqq->pid : 0);

	return bfqq;
}

static void bfq_rq_pos_tree_add(struct bfq_data *bfqd, struct bfq_queue *bfqq)
{
	struct rb_node **p, *parent;
	struct bfq_queue *__bfqq;

	if (bfqq->pos_root != NULL) {
		rb_erase(&bfqq->pos_node, bfqq->pos_root);
		bfqq->pos_root = NULL;
	}

	if (bfq_class_idle(bfqq))
		return;
	if (!bfqq->next_rq)
		return;

	bfqq->pos_root = &bfqd->rq_pos_tree;
	__bfqq = bfq_rq_pos_tree_lookup(bfqd, bfqq->pos_root,
			blk_rq_pos(bfqq->next_rq), &parent, &p);
	if (__bfqq == NULL) {
		rb_link_node(&bfqq->pos_node, parent, p);
		rb_insert_color(&bfqq->pos_node, bfqq->pos_root);
	} else
		bfqq->pos_root = NULL;
}

/*
 * Tell whether there are active queues or groups with differentiated weights.
 */
static inline bool bfq_differentiated_weights(struct bfq_data *bfqd)
{
	/*
	 * For weights to differ, at least one of the trees must contain
	 * at least two nodes.
	 */
	return (!RB_EMPTY_ROOT(&bfqd->queue_weights_tree) &&
		(bfqd->queue_weights_tree.rb_node->rb_left ||
		 bfqd->queue_weights_tree.rb_node->rb_right)
#ifdef CONFIG_CGROUP_BFQIO
	       ) ||
	       (!RB_EMPTY_ROOT(&bfqd->group_weights_tree) &&
		(bfqd->group_weights_tree.rb_node->rb_left ||
		 bfqd->group_weights_tree.rb_node->rb_right)
#endif
	       );
}

/*
 * If the weight-counter tree passed as input contains no counter for
 * the weight of the input entity, then add that counter; otherwise just
 * increment the existing counter.
 *
 * Note that weight-counter trees contain few nodes in mostly symmetric
 * scenarios. For example, if all queues have the same weight, then the
 * weight-counter tree for the queues may contain at most one node.
 * This holds even if low_latency is on, because weight-raised queues
 * are not inserted in the tree.
 * In most scenarios, the rate at which nodes are created/destroyed
 * should be low too.
 */
static void bfq_weights_tree_add(struct bfq_data *bfqd,
				 struct bfq_entity *entity,
				 struct rb_root *root)
{
	struct rb_node **new = &(root->rb_node), *parent = NULL;

	/*
	 * Do not insert if the entity is already associated with a
	 * counter, which happens if:
	 *   1) the entity is associated with a queue,
	 *   2) a request arrival has caused the queue to become both
	 *      non-weight-raised, and hence change its weight, and
	 *      backlogged; in this respect, each of the two events
	 *      causes an invocation of this function,
	 *   3) this is the invocation of this function caused by the
	 *      second event. This second invocation is actually useless,
	 *      and we handle this fact by exiting immediately. More
	 *      efficient or clearer solutions might possibly be adopted.
	 */
	if (entity->weight_counter)
		return;

	while (*new) {
		struct bfq_weight_counter *__counter = container_of(*new,
						struct bfq_weight_counter,
						weights_node);
		parent = *new;

		if (entity->weight == __counter->weight) {
			entity->weight_counter = __counter;
			goto inc_counter;
		}
		if (entity->weight < __counter->weight)
			new = &((*new)->rb_left);
		else
			new = &((*new)->rb_right);
	}

	entity->weight_counter = kzalloc(sizeof(struct bfq_weight_counter),
					 GFP_ATOMIC);
	entity->weight_counter->weight = entity->weight;
	rb_link_node(&entity->weight_counter->weights_node, parent, new);
	rb_insert_color(&entity->weight_counter->weights_node, root);

inc_counter:
	entity->weight_counter->num_active++;
}

/*
 * Decrement the weight counter associated with the entity, and, if the
 * counter reaches 0, remove the counter from the tree.
 * See the comments to the function bfq_weights_tree_add() for considerations
 * about overhead.
 */
static void bfq_weights_tree_remove(struct bfq_data *bfqd,
				    struct bfq_entity *entity,
				    struct rb_root *root)
{
	if (!entity->weight_counter)
		return;

	BUG_ON(RB_EMPTY_ROOT(root));
	BUG_ON(entity->weight_counter->weight != entity->weight);

	BUG_ON(!entity->weight_counter->num_active);
	entity->weight_counter->num_active--;
	if (entity->weight_counter->num_active > 0)
		goto reset_entity_pointer;

	rb_erase(&entity->weight_counter->weights_node, root);
	kfree(entity->weight_counter);

reset_entity_pointer:
	entity->weight_counter = NULL;
}

static struct request *bfq_find_next_rq(struct bfq_data *bfqd,
					struct bfq_queue *bfqq,
					struct request *last)
{
	struct rb_node *rbnext = rb_next(&last->rb_node);
	struct rb_node *rbprev = rb_prev(&last->rb_node);
	struct request *next = NULL, *prev = NULL;

	BUG_ON(RB_EMPTY_NODE(&last->rb_node));

	if (rbprev != NULL)
		prev = rb_entry_rq(rbprev);

	if (rbnext != NULL)
		next = rb_entry_rq(rbnext);
	else {
		rbnext = rb_first(&bfqq->sort_list);
		if (rbnext && rbnext != &last->rb_node)
			next = rb_entry_rq(rbnext);
	}

	return bfq_choose_req(bfqd, next, prev, blk_rq_pos(last));
}

/* see the definition of bfq_async_charge_factor for details */
static inline unsigned long bfq_serv_to_charge(struct request *rq,
					       struct bfq_queue *bfqq)
{
	return blk_rq_sectors(rq) *
		(1 + ((!bfq_bfqq_sync(bfqq)) * (bfqq->wr_coeff == 1) *
		bfq_async_charge_factor));
}

/**
 * bfq_updated_next_req - update the queue after a new next_rq selection.
 * @bfqd: the device data the queue belongs to.
 * @bfqq: the queue to update.
 *
 * If the first request of a queue changes we make sure that the queue
 * has enough budget to serve at least its first request (if the
 * request has grown).  We do this because if the queue has not enough
 * budget for its first request, it has to go through two dispatch
 * rounds to actually get it dispatched.
 */
static void bfq_updated_next_req(struct bfq_data *bfqd,
				 struct bfq_queue *bfqq)
{
	struct bfq_entity *entity = &bfqq->entity;
	struct bfq_service_tree *st = bfq_entity_service_tree(entity);
	struct request *next_rq = bfqq->next_rq;
	unsigned long new_budget;

	if (next_rq == NULL)
		return;

	if (bfqq == bfqd->in_service_queue)
		/*
		 * In order not to break guarantees, budgets cannot be
		 * changed after an entity has been selected.
		 */
		return;

	BUG_ON(entity->tree != &st->active);
	BUG_ON(entity == entity->sched_data->in_service_entity);

	new_budget = max_t(unsigned long, bfqq->max_budget,
			   bfq_serv_to_charge(next_rq, bfqq));
	if (entity->budget != new_budget) {
		entity->budget = new_budget;
		bfq_log_bfqq(bfqd, bfqq, "updated next rq: new budget %lu",
					 new_budget);
		bfq_activate_bfqq(bfqd, bfqq);
	}
}

static inline unsigned int bfq_wr_duration(struct bfq_data *bfqd)
{
	u64 dur;

	if (bfqd->bfq_wr_max_time > 0)
		return bfqd->bfq_wr_max_time;

	dur = bfqd->RT_prod;
	do_div(dur, bfqd->peak_rate);

	return dur;
}

static inline unsigned
bfq_bfqq_cooperations(struct bfq_queue *bfqq)
{
	return bfqq->bic ? bfqq->bic->cooperations : 0;
}

static inline void
bfq_bfqq_resume_state(struct bfq_queue *bfqq, struct bfq_io_cq *bic)
{
	if (bic->saved_idle_window)
		bfq_mark_bfqq_idle_window(bfqq);
	else
		bfq_clear_bfqq_idle_window(bfqq);
	if (bic->saved_IO_bound)
		bfq_mark_bfqq_IO_bound(bfqq);
	else
		bfq_clear_bfqq_IO_bound(bfqq);
	/* Assuming that the flag in_large_burst is already correctly set */
	if (bic->wr_time_left && bfqq->bfqd->low_latency &&
	    !bfq_bfqq_in_large_burst(bfqq) &&
	    bic->cooperations < bfqq->bfqd->bfq_coop_thresh) {
		/*
		 * Start a weight raising period with the duration given by
		 * the raising_time_left snapshot.
		 */
		if (bfq_bfqq_busy(bfqq))
			bfqq->bfqd->wr_busy_queues++;
		bfqq->wr_coeff = bfqq->bfqd->bfq_wr_coeff;
		bfqq->wr_cur_max_time = bic->wr_time_left;
		bfqq->last_wr_start_finish = jiffies;
		bfqq->entity.ioprio_changed = 1;
	}
	/*
	 * Clear wr_time_left to prevent bfq_bfqq_save_state() from
	 * getting confused about the queue's need of a weight-raising
	 * period.
	 */
	bic->wr_time_left = 0;
}

/* Must be called with the queue_lock held. */
static int bfqq_process_refs(struct bfq_queue *bfqq)
{
	int process_refs, io_refs;

	io_refs = bfqq->allocated[READ] + bfqq->allocated[WRITE];
	process_refs = atomic_read(&bfqq->ref) - io_refs - bfqq->entity.on_st;
	BUG_ON(process_refs < 0);
	return process_refs;
}

/* Empty burst list and add just bfqq (see comments to bfq_handle_burst) */
static inline void bfq_reset_burst_list(struct bfq_data *bfqd,
					struct bfq_queue *bfqq)
{
	struct bfq_queue *item;
	struct hlist_node *n;

	hlist_for_each_entry_safe(item, n, &bfqd->burst_list, burst_list_node)
		hlist_del_init(&item->burst_list_node);
	hlist_add_head(&bfqq->burst_list_node, &bfqd->burst_list);
	bfqd->burst_size = 1;
}

/* Add bfqq to the list of queues in current burst (see bfq_handle_burst) */
static void bfq_add_to_burst(struct bfq_data *bfqd, struct bfq_queue *bfqq)
{
	/* Increment burst size to take into account also bfqq */
	bfqd->burst_size++;

	if (bfqd->burst_size == bfqd->bfq_large_burst_thresh) {
		struct bfq_queue *pos, *bfqq_item;
		struct hlist_node *n;

		/*
		 * Enough queues have been activated shortly after each
		 * other to consider this burst as large.
		 */
		bfqd->large_burst = true;

		/*
		 * We can now mark all queues in the burst list as
		 * belonging to a large burst.
		 */
		hlist_for_each_entry(bfqq_item, &bfqd->burst_list,
				     burst_list_node)
		        bfq_mark_bfqq_in_large_burst(bfqq_item);
		bfq_mark_bfqq_in_large_burst(bfqq);

		/*
		 * From now on, and until the current burst finishes, any
		 * new queue being activated shortly after the last queue
		 * was inserted in the burst can be immediately marked as
		 * belonging to a large burst. So the burst list is not
		 * needed any more. Remove it.
		 */
		hlist_for_each_entry_safe(pos, n, &bfqd->burst_list,
					  burst_list_node)
			hlist_del_init(&pos->burst_list_node);
	} else /* burst not yet large: add bfqq to the burst list */
		hlist_add_head(&bfqq->burst_list_node, &bfqd->burst_list);
}

/*
 * If many queues happen to become active shortly after each other, then,
 * to help the processes associated to these queues get their job done as
 * soon as possible, it is usually better to not grant either weight-raising
 * or device idling to these queues. In this comment we describe, firstly,
 * the reasons why this fact holds, and, secondly, the next function, which
 * implements the main steps needed to properly mark these queues so that
 * they can then be treated in a different way.
 *
 * As for the terminology, we say that a queue becomes active, i.e.,
 * switches from idle to backlogged, either when it is created (as a
 * consequence of the arrival of an I/O request), or, if already existing,
 * when a new request for the queue arrives while the queue is idle.
 * Bursts of activations, i.e., activations of different queues occurring
 * shortly after each other, are typically caused by services or applications
 * that spawn or reactivate many parallel threads/processes. Examples are
 * systemd during boot or git grep.
 *
 * These services or applications benefit mostly from a high throughput:
 * the quicker the requests of the activated queues are cumulatively served,
 * the sooner the target job of these queues gets completed. As a consequence,
 * weight-raising any of these queues, which also implies idling the device
 * for it, is almost always counterproductive: in most cases it just lowers
 * throughput.
 *
 * On the other hand, a burst of activations may be also caused by the start
 * of an application that does not consist in a lot of parallel I/O-bound
 * threads. In fact, with a complex application, the burst may be just a
 * consequence of the fact that several processes need to be executed to
 * start-up the application. To start an application as quickly as possible,
 * the best thing to do is to privilege the I/O related to the application
 * with respect to all other I/O. Therefore, the best strategy to start as
 * quickly as possible an application that causes a burst of activations is
 * to weight-raise all the queues activated during the burst. This is the
 * exact opposite of the best strategy for the other type of bursts.
 *
 * In the end, to take the best action for each of the two cases, the two
 * types of bursts need to be distinguished. Fortunately, this seems
 * relatively easy to do, by looking at the sizes of the bursts. In
 * particular, we found a threshold such that bursts with a larger size
 * than that threshold are apparently caused only by services or commands
 * such as systemd or git grep. For brevity, hereafter we call just 'large'
 * these bursts. BFQ *does not* weight-raise queues whose activations occur
 * in a large burst. In addition, for each of these queues BFQ performs or
 * does not perform idling depending on which choice boosts the throughput
 * most. The exact choice depends on the device and request pattern at
 * hand.
 *
 * Turning back to the next function, it implements all the steps needed
 * to detect the occurrence of a large burst and to properly mark all the
 * queues belonging to it (so that they can then be treated in a different
 * way). This goal is achieved by maintaining a special "burst list" that
 * holds, temporarily, the queues that belong to the burst in progress. The
 * list is then used to mark these queues as belonging to a large burst if
 * the burst does become large. The main steps are the following.
 *
 * . when the very first queue is activated, the queue is inserted into the
 *   list (as it could be the first queue in a possible burst)
 *
 * . if the current burst has not yet become large, and a queue Q that does
 *   not yet belong to the burst is activated shortly after the last time
 *   at which a new queue entered the burst list, then the function appends
 *   Q to the burst list
 *
 * . if, as a consequence of the previous step, the burst size reaches
 *   the large-burst threshold, then
 *
 *     . all the queues in the burst list are marked as belonging to a
 *       large burst
 *
 *     . the burst list is deleted; in fact, the burst list already served
 *       its purpose (keeping temporarily track of the queues in a burst,
 *       so as to be able to mark them as belonging to a large burst in the
 *       previous sub-step), and now is not needed any more
 *
 *     . the device enters a large-burst mode
 *
 * . if a queue Q that does not belong to the burst is activated while
 *   the device is in large-burst mode and shortly after the last time
 *   at which a queue either entered the burst list or was marked as
 *   belonging to the current large burst, then Q is immediately marked
 *   as belonging to a large burst.
 *
 * . if a queue Q that does not belong to the burst is activated a while
 *   later, i.e., not shortly after, than the last time at which a queue
 *   either entered the burst list or was marked as belonging to the
 *   current large burst, then the current burst is deemed as finished and:
 *
 *        . the large-burst mode is reset if set
 *
 *        . the burst list is emptied
 *
 *        . Q is inserted in the burst list, as Q may be the first queue
 *          in a possible new burst (then the burst list contains just Q
 *          after this step).
 */
static void bfq_handle_burst(struct bfq_data *bfqd, struct bfq_queue *bfqq,
			     bool idle_for_long_time)
{
	/*
	 * If bfqq happened to be activated in a burst, but has been idle
	 * for at least as long as an interactive queue, then we assume
	 * that, in the overall I/O initiated in the burst, the I/O
	 * associated to bfqq is finished. So bfqq does not need to be
	 * treated as a queue belonging to a burst anymore. Accordingly,
	 * we reset bfqq's in_large_burst flag if set, and remove bfqq
	 * from the burst list if it's there. We do not decrement instead
	 * burst_size, because the fact that bfqq does not need to belong
	 * to the burst list any more does not invalidate the fact that
	 * bfqq may have been activated during the current burst.
	 */
	if (idle_for_long_time) {
		hlist_del_init(&bfqq->burst_list_node);
		bfq_clear_bfqq_in_large_burst(bfqq);
	}

	/*
	 * If bfqq is already in the burst list or is part of a large
	 * burst, then there is nothing else to do.
	 */
	if (!hlist_unhashed(&bfqq->burst_list_node) ||
	    bfq_bfqq_in_large_burst(bfqq))
		return;

	/*
	 * If bfqq's activation happens late enough, then the current
	 * burst is finished, and related data structures must be reset.
	 *
	 * In this respect, consider the special case where bfqq is the very
	 * first queue being activated. In this case, last_ins_in_burst is
	 * not yet significant when we get here. But it is easy to verify
	 * that, whether or not the following condition is true, bfqq will
	 * end up being inserted into the burst list. In particular the
	 * list will happen to contain only bfqq. And this is exactly what
	 * has to happen, as bfqq may be the first queue in a possible
	 * burst.
	 */
	if (time_is_before_jiffies(bfqd->last_ins_in_burst +
	    bfqd->bfq_burst_interval)) {
		bfqd->large_burst = false;
		bfq_reset_burst_list(bfqd, bfqq);
		return;
	}

	/*
	 * If we get here, then bfqq is being activated shortly after the
	 * last queue. So, if the current burst is also large, we can mark
	 * bfqq as belonging to this large burst immediately.
	 */
	if (bfqd->large_burst) {
		bfq_mark_bfqq_in_large_burst(bfqq);
		return;
	}

	/*
	 * If we get here, then a large-burst state has not yet been
	 * reached, but bfqq is being activated shortly after the last
	 * queue. Then we add bfqq to the burst.
	 */
	bfq_add_to_burst(bfqd, bfqq);
}

static void bfq_add_request(struct request *rq)
{
	struct bfq_queue *bfqq = RQ_BFQQ(rq);
	struct bfq_entity *entity = &bfqq->entity;
	struct bfq_data *bfqd = bfqq->bfqd;
	struct request *next_rq, *prev;
	unsigned long old_wr_coeff = bfqq->wr_coeff;
	bool interactive = false;

	bfq_log_bfqq(bfqd, bfqq, "add_request %d", rq_is_sync(rq));
	bfqq->queued[rq_is_sync(rq)]++;
	bfqd->queued++;

	elv_rb_add(&bfqq->sort_list, rq);

	/*
	 * Check if this request is a better next-serve candidate.
	 */
	prev = bfqq->next_rq;
	next_rq = bfq_choose_req(bfqd, bfqq->next_rq, rq, bfqd->last_position);
	BUG_ON(next_rq == NULL);
	bfqq->next_rq = next_rq;

	/*
	 * Adjust priority tree position, if next_rq changes.
	 */
	if (prev != bfqq->next_rq)
		bfq_rq_pos_tree_add(bfqd, bfqq);

	if (!bfq_bfqq_busy(bfqq)) {
		bool soft_rt, coop_or_in_burst,
		     idle_for_long_time = time_is_before_jiffies(
						bfqq->budget_timeout +
						bfqd->bfq_wr_min_idle_time);

		if (bfq_bfqq_sync(bfqq)) {
			bool already_in_burst =
			   !hlist_unhashed(&bfqq->burst_list_node) ||
			   bfq_bfqq_in_large_burst(bfqq);
			bfq_handle_burst(bfqd, bfqq, idle_for_long_time);
			/*
			 * If bfqq was not already in the current burst,
			 * then, at this point, bfqq either has been
			 * added to the current burst or has caused the
			 * current burst to terminate. In particular, in
			 * the second case, bfqq has become the first
			 * queue in a possible new burst.
			 * In both cases last_ins_in_burst needs to be
			 * moved forward.
			 */
			if (!already_in_burst)
				bfqd->last_ins_in_burst = jiffies;
		}

		coop_or_in_burst = bfq_bfqq_in_large_burst(bfqq) ||
			bfq_bfqq_cooperations(bfqq) >= bfqd->bfq_coop_thresh;
		soft_rt = bfqd->bfq_wr_max_softrt_rate > 0 &&
			!coop_or_in_burst &&
			time_is_before_jiffies(bfqq->soft_rt_next_start);
		interactive = !coop_or_in_burst && idle_for_long_time;
		entity->budget = max_t(unsigned long, bfqq->max_budget,
				       bfq_serv_to_charge(next_rq, bfqq));

		if (!bfq_bfqq_IO_bound(bfqq)) {
			if (time_before(jiffies,
					RQ_BIC(rq)->ttime.last_end_request +
					bfqd->bfq_slice_idle)) {
				bfqq->requests_within_timer++;
				if (bfqq->requests_within_timer >=
				    bfqd->bfq_requests_within_timer)
					bfq_mark_bfqq_IO_bound(bfqq);
			} else
				bfqq->requests_within_timer = 0;
		}

		if (!bfqd->low_latency)
			goto add_bfqq_busy;

		if (bfq_bfqq_just_split(bfqq))
			goto set_ioprio_changed;

		/*
		 * If the queue:
		 * - is not being boosted,
		 * - has been idle for enough time,
		 * - is not a sync queue or is linked to a bfq_io_cq (it is
		 *   shared "for its nature" or it is not shared and its
		 *   requests have not been redirected to a shared queue)
		 * start a weight-raising period.
		 */
		if (old_wr_coeff == 1 && (interactive || soft_rt) &&
		    (!bfq_bfqq_sync(bfqq) || bfqq->bic != NULL)) {
			bfqq->wr_coeff = bfqd->bfq_wr_coeff;
			if (interactive)
				bfqq->wr_cur_max_time = bfq_wr_duration(bfqd);
			else
				bfqq->wr_cur_max_time =
					bfqd->bfq_wr_rt_max_time;
			bfq_log_bfqq(bfqd, bfqq,
				     "wrais starting at %lu, rais_max_time %u",
				     jiffies,
				     jiffies_to_msecs(bfqq->wr_cur_max_time));
		} else if (old_wr_coeff > 1) {
			if (interactive)
				bfqq->wr_cur_max_time = bfq_wr_duration(bfqd);
			else if (coop_or_in_burst ||
				 (bfqq->wr_cur_max_time ==
				  bfqd->bfq_wr_rt_max_time &&
				  !soft_rt)) {
				bfqq->wr_coeff = 1;
				bfq_log_bfqq(bfqd, bfqq,
					"wrais ending at %lu, rais_max_time %u",
					jiffies,
					jiffies_to_msecs(bfqq->
						wr_cur_max_time));
			} else if (time_before(
					bfqq->last_wr_start_finish +
					bfqq->wr_cur_max_time,
					jiffies +
					bfqd->bfq_wr_rt_max_time) &&
				   soft_rt) {
				/*
				 *
				 * The remaining weight-raising time is lower
				 * than bfqd->bfq_wr_rt_max_time, which means
				 * that the application is enjoying weight
				 * raising either because deemed soft-rt in
				 * the near past, or because deemed interactive
				 * a long ago.
				 * In both cases, resetting now the current
				 * remaining weight-raising time for the
				 * application to the weight-raising duration
				 * for soft rt applications would not cause any
				 * latency increase for the application (as the
				 * new duration would be higher than the
				 * remaining time).
				 *
				 * In addition, the application is now meeting
				 * the requirements for being deemed soft rt.
				 * In the end we can correctly and safely
				 * (re)charge the weight-raising duration for
				 * the application with the weight-raising
				 * duration for soft rt applications.
				 *
				 * In particular, doing this recharge now, i.e.,
				 * before the weight-raising period for the
				 * application finishes, reduces the probability
				 * of the following negative scenario:
				 * 1) the weight of a soft rt application is
				 *    raised at startup (as for any newly
				 *    created application),
				 * 2) since the application is not interactive,
				 *    at a certain time weight-raising is
				 *    stopped for the application,
				 * 3) at that time the application happens to
				 *    still have pending requests, and hence
				 *    is destined to not have a chance to be
				 *    deemed soft rt before these requests are
				 *    completed (see the comments to the
				 *    function bfq_bfqq_softrt_next_start()
				 *    for details on soft rt detection),
				 * 4) these pending requests experience a high
				 *    latency because the application is not
				 *    weight-raised while they are pending.
				 */
				bfqq->last_wr_start_finish = jiffies;
				bfqq->wr_cur_max_time =
					bfqd->bfq_wr_rt_max_time;
			}
		}
set_ioprio_changed:
		if (old_wr_coeff != bfqq->wr_coeff)
			entity->ioprio_changed = 1;
add_bfqq_busy:
		bfqq->last_idle_bklogged = jiffies;
		bfqq->service_from_backlogged = 0;
		bfq_clear_bfqq_softrt_update(bfqq);
		bfq_add_bfqq_busy(bfqd, bfqq);
	} else {
		if (bfqd->low_latency && old_wr_coeff == 1 && !rq_is_sync(rq) &&
		    time_is_before_jiffies(
				bfqq->last_wr_start_finish +
				bfqd->bfq_wr_min_inter_arr_async)) {
			bfqq->wr_coeff = bfqd->bfq_wr_coeff;
			bfqq->wr_cur_max_time = bfq_wr_duration(bfqd);

			bfqd->wr_busy_queues++;
			entity->ioprio_changed = 1;
			bfq_log_bfqq(bfqd, bfqq,
			    "non-idle wrais starting at %lu, rais_max_time %u",
			    jiffies,
			    jiffies_to_msecs(bfqq->wr_cur_max_time));
		}
		if (prev != bfqq->next_rq)
			bfq_updated_next_req(bfqd, bfqq);
	}

	if (bfqd->low_latency &&
		(old_wr_coeff == 1 || bfqq->wr_coeff == 1 || interactive))
		bfqq->last_wr_start_finish = jiffies;
}

static struct request *bfq_find_rq_fmerge(struct bfq_data *bfqd,
					  struct bio *bio)
{
	struct task_struct *tsk = current;
	struct bfq_io_cq *bic;
	struct bfq_queue *bfqq;

	bic = bfq_bic_lookup(bfqd, tsk->io_context);
	if (bic == NULL)
		return NULL;

	bfqq = bic_to_bfqq(bic, bfq_bio_sync(bio));
	if (bfqq != NULL)
		return elv_rb_find(&bfqq->sort_list, bio_end_sector(bio));

	return NULL;
}

static void bfq_activate_request(struct request_queue *q, struct request *rq)
{
	struct bfq_data *bfqd = q->elevator->elevator_data;

	bfqd->rq_in_driver++;
	bfqd->last_position = blk_rq_pos(rq) + blk_rq_sectors(rq);
	bfq_log(bfqd, "activate_request: new bfqd->last_position %llu",
		(long long unsigned)bfqd->last_position);
}

static inline void bfq_deactivate_request(struct request_queue *q,
					  struct request *rq)
{
	struct bfq_data *bfqd = q->elevator->elevator_data;

	BUG_ON(bfqd->rq_in_driver == 0);
	bfqd->rq_in_driver--;
}

static void bfq_remove_request(struct request *rq)
{
	struct bfq_queue *bfqq = RQ_BFQQ(rq);
	struct bfq_data *bfqd = bfqq->bfqd;
	const int sync = rq_is_sync(rq);

	if (bfqq->next_rq == rq) {
		bfqq->next_rq = bfq_find_next_rq(bfqd, bfqq, rq);
		bfq_updated_next_req(bfqd, bfqq);
	}

	if (rq->queuelist.prev != &rq->queuelist)
		list_del_init(&rq->queuelist);
	BUG_ON(bfqq->queued[sync] == 0);
	bfqq->queued[sync]--;
	bfqd->queued--;
	elv_rb_del(&bfqq->sort_list, rq);

	if (RB_EMPTY_ROOT(&bfqq->sort_list)) {
		if (bfq_bfqq_busy(bfqq) && bfqq != bfqd->in_service_queue)
			bfq_del_bfqq_busy(bfqd, bfqq, 1);
		/*
		 * Remove queue from request-position tree as it is empty.
		 */
		if (bfqq->pos_root != NULL) {
			rb_erase(&bfqq->pos_node, bfqq->pos_root);
			bfqq->pos_root = NULL;
		}
	}

	if (rq->cmd_flags & REQ_META) {
		BUG_ON(bfqq->meta_pending == 0);
		bfqq->meta_pending--;
	}
}

static int bfq_merge(struct request_queue *q, struct request **req,
		     struct bio *bio)
{
	struct bfq_data *bfqd = q->elevator->elevator_data;
	struct request *__rq;

	__rq = bfq_find_rq_fmerge(bfqd, bio);
	if (__rq != NULL && elv_rq_merge_ok(__rq, bio)) {
		*req = __rq;
		return ELEVATOR_FRONT_MERGE;
	}

	return ELEVATOR_NO_MERGE;
}

static void bfq_merged_request(struct request_queue *q, struct request *req,
			       int type)
{
	if (type == ELEVATOR_FRONT_MERGE &&
	    rb_prev(&req->rb_node) &&
	    blk_rq_pos(req) <
	    blk_rq_pos(container_of(rb_prev(&req->rb_node),
				    struct request, rb_node))) {
		struct bfq_queue *bfqq = RQ_BFQQ(req);
		struct bfq_data *bfqd = bfqq->bfqd;
		struct request *prev, *next_rq;

		/* Reposition request in its sort_list */
		elv_rb_del(&bfqq->sort_list, req);
		elv_rb_add(&bfqq->sort_list, req);
		/* Choose next request to be served for bfqq */
		prev = bfqq->next_rq;
		next_rq = bfq_choose_req(bfqd, bfqq->next_rq, req,
					 bfqd->last_position);
		BUG_ON(next_rq == NULL);
		bfqq->next_rq = next_rq;
		/*
		 * If next_rq changes, update both the queue's budget to
		 * fit the new request and the queue's position in its
		 * rq_pos_tree.
		 */
		if (prev != bfqq->next_rq) {
			bfq_updated_next_req(bfqd, bfqq);
			bfq_rq_pos_tree_add(bfqd, bfqq);
		}
	}
}

static void bfq_merged_requests(struct request_queue *q, struct request *rq,
				struct request *next)
{
	struct bfq_queue *bfqq = RQ_BFQQ(rq), *next_bfqq = RQ_BFQQ(next);

	/*
	 * If next and rq belong to the same bfq_queue and next is older
	 * than rq, then reposition rq in the fifo (by substituting next
	 * with rq). Otherwise, if next and rq belong to different
	 * bfq_queues, never reposition rq: in fact, we would have to
	 * reposition it with respect to next's position in its own fifo,
	 * which would most certainly be too expensive with respect to
	 * the benefits.
	 */
	if (bfqq == next_bfqq &&
	    !list_empty(&rq->queuelist) && !list_empty(&next->queuelist) &&
	    time_before(rq_fifo_time(next), rq_fifo_time(rq))) {
		list_del_init(&rq->queuelist);
		list_replace_init(&next->queuelist, &rq->queuelist);
		rq_set_fifo_time(rq, rq_fifo_time(next));
	}

	if (bfqq->next_rq == next)
		bfqq->next_rq = rq;

	bfq_remove_request(next);
}

/* Must be called with bfqq != NULL */
static inline void bfq_bfqq_end_wr(struct bfq_queue *bfqq)
{
	BUG_ON(bfqq == NULL);
	if (bfq_bfqq_busy(bfqq))
		bfqq->bfqd->wr_busy_queues--;
	bfqq->wr_coeff = 1;
	bfqq->wr_cur_max_time = 0;
	/* Trigger a weight change on the next activation of the queue */
	bfqq->entity.ioprio_changed = 1;
}

static void bfq_end_wr_async_queues(struct bfq_data *bfqd,
				    struct bfq_group *bfqg)
{
	int i, j;

	for (i = 0; i < 2; i++)
		for (j = 0; j < IOPRIO_BE_NR; j++)
			if (bfqg->async_bfqq[i][j] != NULL)
				bfq_bfqq_end_wr(bfqg->async_bfqq[i][j]);
	if (bfqg->async_idle_bfqq != NULL)
		bfq_bfqq_end_wr(bfqg->async_idle_bfqq);
}

static void bfq_end_wr(struct bfq_data *bfqd)
{
	struct bfq_queue *bfqq;

	spin_lock_irq(bfqd->queue->queue_lock);

	list_for_each_entry(bfqq, &bfqd->active_list, bfqq_list)
		bfq_bfqq_end_wr(bfqq);
	list_for_each_entry(bfqq, &bfqd->idle_list, bfqq_list)
		bfq_bfqq_end_wr(bfqq);
	bfq_end_wr_async(bfqd);

	spin_unlock_irq(bfqd->queue->queue_lock);
}

static inline sector_t bfq_io_struct_pos(void *io_struct, bool request)
{
	if (request)
		return blk_rq_pos(io_struct);
	else
		return ((struct bio *)io_struct)->bi_sector;
}

static inline sector_t bfq_dist_from(sector_t pos1,
				     sector_t pos2)
{
	if (pos1 >= pos2)
		return pos1 - pos2;
	else
		return pos2 - pos1;
}

static inline int bfq_rq_close_to_sector(void *io_struct, bool request,
					 sector_t sector)
{
	return bfq_dist_from(bfq_io_struct_pos(io_struct, request), sector) <=
	       BFQQ_SEEK_THR;
}

static struct bfq_queue *bfqq_close(struct bfq_data *bfqd, sector_t sector)
{
	struct rb_root *root = &bfqd->rq_pos_tree;
	struct rb_node *parent, *node;
	struct bfq_queue *__bfqq;

	if (RB_EMPTY_ROOT(root))
		return NULL;

	/*
	 * First, if we find a request starting at the end of the last
	 * request, choose it.
	 */
	__bfqq = bfq_rq_pos_tree_lookup(bfqd, root, sector, &parent, NULL);
	if (__bfqq != NULL)
		return __bfqq;

	/*
	 * If the exact sector wasn't found, the parent of the NULL leaf
	 * will contain the closest sector (rq_pos_tree sorted by
	 * next_request position).
	 */
	__bfqq = rb_entry(parent, struct bfq_queue, pos_node);
	if (bfq_rq_close_to_sector(__bfqq->next_rq, true, sector))
		return __bfqq;

	if (blk_rq_pos(__bfqq->next_rq) < sector)
		node = rb_next(&__bfqq->pos_node);
	else
		node = rb_prev(&__bfqq->pos_node);
	if (node == NULL)
		return NULL;

	__bfqq = rb_entry(node, struct bfq_queue, pos_node);
	if (bfq_rq_close_to_sector(__bfqq->next_rq, true, sector))
		return __bfqq;

	return NULL;
}

/*
 * bfqd - obvious
 * cur_bfqq - passed in so that we don't decide that the current queue
 *            is closely cooperating with itself
 * sector - used as a reference point to search for a close queue
 */
static struct bfq_queue *bfq_close_cooperator(struct bfq_data *bfqd,
					      struct bfq_queue *cur_bfqq,
					      sector_t sector)
{
	struct bfq_queue *bfqq;

	if (bfq_class_idle(cur_bfqq))
		return NULL;
	if (!bfq_bfqq_sync(cur_bfqq))
		return NULL;
	if (BFQQ_SEEKY(cur_bfqq))
		return NULL;

	/* If device has only one backlogged bfq_queue, don't search. */
	if (bfqd->busy_queues == 1)
		return NULL;

	/*
	 * We should notice if some of the queues are cooperating, e.g.
	 * working closely on the same area of the disk. In that case,
	 * we can group them together and don't waste time idling.
	 */
	bfqq = bfqq_close(bfqd, sector);
	if (bfqq == NULL || bfqq == cur_bfqq)
		return NULL;

	/*
	 * Do not merge queues from different bfq_groups.
	*/
	if (bfqq->entity.parent != cur_bfqq->entity.parent)
		return NULL;

	/*
	 * It only makes sense to merge sync queues.
	 */
	if (!bfq_bfqq_sync(bfqq))
		return NULL;
	if (BFQQ_SEEKY(bfqq))
		return NULL;

	/*
	 * Do not merge queues of different priority classes.
	 */
	if (bfq_class_rt(bfqq) != bfq_class_rt(cur_bfqq))
		return NULL;

	return bfqq;
}

static struct bfq_queue *
bfq_setup_merge(struct bfq_queue *bfqq, struct bfq_queue *new_bfqq)
{
	int process_refs, new_process_refs;
	struct bfq_queue *__bfqq;

	/*
	 * If there are no process references on the new_bfqq, then it is
	 * unsafe to follow the ->new_bfqq chain as other bfqq's in the chain
	 * may have dropped their last reference (not just their last process
	 * reference).
	 */
	if (!bfqq_process_refs(new_bfqq))
		return NULL;

	/* Avoid a circular list and skip interim queue merges. */
	while ((__bfqq = new_bfqq->new_bfqq)) {
		if (__bfqq == bfqq)
			return NULL;
		new_bfqq = __bfqq;
	}

	process_refs = bfqq_process_refs(bfqq);
	new_process_refs = bfqq_process_refs(new_bfqq);
	/*
	 * If the process for the bfqq has gone away, there is no
	 * sense in merging the queues.
	 */
	if (process_refs == 0 || new_process_refs == 0)
		return NULL;

	bfq_log_bfqq(bfqq->bfqd, bfqq, "scheduling merge with queue %d",
		new_bfqq->pid);

	/*
	 * Merging is just a redirection: the requests of the process
	 * owning one of the two queues are redirected to the other queue.
	 * The latter queue, in its turn, is set as shared if this is the
	 * first time that the requests of some process are redirected to
	 * it.
	 *
	 * We redirect bfqq to new_bfqq and not the opposite, because we
	 * are in the context of the process owning bfqq, hence we have
	 * the io_cq of this process. So we can immediately configure this
	 * io_cq to redirect the requests of the process to new_bfqq.
	 *
	 * NOTE, even if new_bfqq coincides with the in-service queue, the
	 * io_cq of new_bfqq is not available, because, if the in-service
	 * queue is shared, bfqd->in_service_bic may not point to the
	 * io_cq of the in-service queue.
	 * Redirecting the requests of the process owning bfqq to the
	 * currently in-service queue is in any case the best option, as
	 * we feed the in-service queue with new requests close to the
	 * last request served and, by doing so, hopefully increase the
	 * throughput.
	 */
	bfqq->new_bfqq = new_bfqq;
	atomic_add(process_refs, &new_bfqq->ref);
	return new_bfqq;
}

/*
 * Attempt to schedule a merge of bfqq with the currently in-service queue
 * or with a close queue among the scheduled queues.
 * Return NULL if no merge was scheduled, a pointer to the shared bfq_queue
 * structure otherwise.
 *
 * The OOM queue is not allowed to participate to cooperation: in fact, since
 * the requests temporarily redirected to the OOM queue could be redirected
 * again to dedicated queues at any time, the state needed to correctly
 * handle merging with the OOM queue would be quite complex and expensive
 * to maintain. Besides, in such a critical condition as an out of memory,
 * the benefits of queue merging may be little relevant, or even negligible.
 */
static struct bfq_queue *
bfq_setup_cooperator(struct bfq_data *bfqd, struct bfq_queue *bfqq,
		     void *io_struct, bool request)
{
	struct bfq_queue *in_service_bfqq, *new_bfqq;

	if (bfqq->new_bfqq)
		return bfqq->new_bfqq;

	if (!io_struct || unlikely(bfqq == &bfqd->oom_bfqq))
		return NULL;

	in_service_bfqq = bfqd->in_service_queue;

	if (in_service_bfqq == NULL || in_service_bfqq == bfqq ||
	    !bfqd->in_service_bic ||
	    unlikely(in_service_bfqq == &bfqd->oom_bfqq))
		goto check_scheduled;

	if (bfq_class_idle(in_service_bfqq) || bfq_class_idle(bfqq))
		goto check_scheduled;

	if (bfq_class_rt(in_service_bfqq) != bfq_class_rt(bfqq))
		goto check_scheduled;

	if (in_service_bfqq->entity.parent != bfqq->entity.parent)
		goto check_scheduled;

	if (bfq_rq_close_to_sector(io_struct, request, bfqd->last_position) &&
	    bfq_bfqq_sync(in_service_bfqq) && bfq_bfqq_sync(bfqq)) {
		new_bfqq = bfq_setup_merge(bfqq, in_service_bfqq);
		if (new_bfqq != NULL)
			return new_bfqq; /* Merge with in-service queue */
	}

	/*
	 * Check whether there is a cooperator among currently scheduled
	 * queues. The only thing we need is that the bio/request is not
	 * NULL, as we need it to establish whether a cooperator exists.
	 */
check_scheduled:
	new_bfqq = bfq_close_cooperator(bfqd, bfqq,
					bfq_io_struct_pos(io_struct, request));
	if (new_bfqq && likely(new_bfqq != &bfqd->oom_bfqq))
		return bfq_setup_merge(bfqq, new_bfqq);

	return NULL;
}

static inline void
bfq_bfqq_save_state(struct bfq_queue *bfqq)
{
	/*
	 * If bfqq->bic == NULL, the queue is already shared or its requests
	 * have already been redirected to a shared queue; both idle window
	 * and weight raising state have already been saved. Do nothing.
	 */
	if (bfqq->bic == NULL)
		return;
	if (bfqq->bic->wr_time_left)
		/*
		 * This is the queue of a just-started process, and would
		 * deserve weight raising: we set wr_time_left to the full
		 * weight-raising duration to trigger weight-raising when
		 * and if the queue is split and the first request of the
		 * queue is enqueued.
		 */
		bfqq->bic->wr_time_left = bfq_wr_duration(bfqq->bfqd);
	else if (bfqq->wr_coeff > 1) {
		unsigned long wr_duration =
			jiffies - bfqq->last_wr_start_finish;
		/*
		 * It may happen that a queue's weight raising period lasts
		 * longer than its wr_cur_max_time, as weight raising is
		 * handled only when a request is enqueued or dispatched (it
		 * does not use any timer). If the weight raising period is
		 * about to end, don't save it.
		 */
		if (bfqq->wr_cur_max_time <= wr_duration)
			bfqq->bic->wr_time_left = 0;
		else
			bfqq->bic->wr_time_left =
				bfqq->wr_cur_max_time - wr_duration;
		/*
		 * The bfq_queue is becoming shared or the requests of the
		 * process owning the queue are being redirected to a shared
		 * queue. Stop the weight raising period of the queue, as in
		 * both cases it should not be owned by an interactive or
		 * soft real-time application.
		 */
		bfq_bfqq_end_wr(bfqq);
	} else
		bfqq->bic->wr_time_left = 0;
	bfqq->bic->saved_idle_window = bfq_bfqq_idle_window(bfqq);
	bfqq->bic->saved_IO_bound = bfq_bfqq_IO_bound(bfqq);
	bfqq->bic->saved_in_large_burst = bfq_bfqq_in_large_burst(bfqq);
	bfqq->bic->was_in_burst_list = !hlist_unhashed(&bfqq->burst_list_node);
	bfqq->bic->cooperations++;
	bfqq->bic->failed_cooperations = 0;
}

static inline void
bfq_get_bic_reference(struct bfq_queue *bfqq)
{
	/*
	 * If bfqq->bic has a non-NULL value, the bic to which it belongs
	 * is about to begin using a shared bfq_queue.
	 */
	if (bfqq->bic)
		atomic_long_inc(&bfqq->bic->icq.ioc->refcount);
}

static void
bfq_merge_bfqqs(struct bfq_data *bfqd, struct bfq_io_cq *bic,
		struct bfq_queue *bfqq, struct bfq_queue *new_bfqq)
{
	bfq_log_bfqq(bfqd, bfqq, "merging with queue %lu",
		(long unsigned)new_bfqq->pid);
	/* Save weight raising and idle window of the merged queues */
	bfq_bfqq_save_state(bfqq);
	bfq_bfqq_save_state(new_bfqq);
	if (bfq_bfqq_IO_bound(bfqq))
		bfq_mark_bfqq_IO_bound(new_bfqq);
	bfq_clear_bfqq_IO_bound(bfqq);
	/*
	 * Grab a reference to the bic, to prevent it from being destroyed
	 * before being possibly touched by a bfq_split_bfqq().
	 */
	bfq_get_bic_reference(bfqq);
	bfq_get_bic_reference(new_bfqq);
	/*
	 * Merge queues (that is, let bic redirect its requests to new_bfqq)
	 */
	bic_set_bfqq(bic, new_bfqq, 1);
	bfq_mark_bfqq_coop(new_bfqq);
	/*
	 * new_bfqq now belongs to at least two bics (it is a shared queue):
	 * set new_bfqq->bic to NULL. bfqq either:
	 * - does not belong to any bic any more, and hence bfqq->bic must
	 *   be set to NULL, or
	 * - is a queue whose owning bics have already been redirected to a
	 *   different queue, hence the queue is destined to not belong to
	 *   any bic soon and bfqq->bic is already NULL (therefore the next
	 *   assignment causes no harm).
	 */
	new_bfqq->bic = NULL;
	bfqq->bic = NULL;
	bfq_put_queue(bfqq);
}

static inline void bfq_bfqq_increase_failed_cooperations(struct bfq_queue *bfqq)
{
	struct bfq_io_cq *bic = bfqq->bic;
	struct bfq_data *bfqd = bfqq->bfqd;

	if (bic && bfq_bfqq_cooperations(bfqq) >= bfqd->bfq_coop_thresh) {
		bic->failed_cooperations++;
		if (bic->failed_cooperations >= bfqd->bfq_failed_cooperations)
			bic->cooperations = 0;
	}
}

static int bfq_allow_merge(struct request_queue *q, struct request *rq,
			   struct bio *bio)
{
	struct bfq_data *bfqd = q->elevator->elevator_data;
	struct bfq_io_cq *bic;
	struct bfq_queue *bfqq, *new_bfqq;

	/*
	 * Disallow merge of a sync bio into an async request.
	 */
	if (bfq_bio_sync(bio) && !rq_is_sync(rq))
		return 0;

	/*
	 * Lookup the bfqq that this bio will be queued with. Allow
	 * merge only if rq is queued there.
	 * Queue lock is held here.
	 */
	bic = bfq_bic_lookup(bfqd, current->io_context);
	if (bic == NULL)
		return 0;

	bfqq = bic_to_bfqq(bic, bfq_bio_sync(bio));
	/*
	 * We take advantage of this function to perform an early merge
	 * of the queues of possible cooperating processes.
	 */
	if (bfqq != NULL) {
		new_bfqq = bfq_setup_cooperator(bfqd, bfqq, bio, false);
		if (new_bfqq != NULL) {
			bfq_merge_bfqqs(bfqd, bic, bfqq, new_bfqq);
			/*
			 * If we get here, the bio will be queued in the
			 * shared queue, i.e., new_bfqq, so use new_bfqq
			 * to decide whether bio and rq can be merged.
			 */
			bfqq = new_bfqq;
		} else
			bfq_bfqq_increase_failed_cooperations(bfqq);
	}

	return bfqq == RQ_BFQQ(rq);
}

static void __bfq_set_in_service_queue(struct bfq_data *bfqd,
				       struct bfq_queue *bfqq)
{
	if (bfqq != NULL) {
		bfq_mark_bfqq_must_alloc(bfqq);
		bfq_mark_bfqq_budget_new(bfqq);
		bfq_clear_bfqq_fifo_expire(bfqq);

		bfqd->budgets_assigned = (bfqd->budgets_assigned*7 + 256) / 8;

		bfq_log_bfqq(bfqd, bfqq,
			     "set_in_service_queue, cur-budget = %lu",
			     bfqq->entity.budget);
	}

	bfqd->in_service_queue = bfqq;
}

/*
 * Get and set a new queue for service.
 */
static struct bfq_queue *bfq_set_in_service_queue(struct bfq_data *bfqd)
{
	struct bfq_queue *bfqq = bfq_get_next_queue(bfqd);

	__bfq_set_in_service_queue(bfqd, bfqq);
	return bfqq;
}

/*
 * If enough samples have been computed, return the current max budget
 * stored in bfqd, which is dynamically updated according to the
 * estimated disk peak rate; otherwise return the default max budget
 */
static inline unsigned long bfq_max_budget(struct bfq_data *bfqd)
{
	if (bfqd->budgets_assigned < 194)
		return bfq_default_max_budget;
	else
		return bfqd->bfq_max_budget;
}

/*
 * Return min budget, which is a fraction of the current or default
 * max budget (trying with 1/32)
 */
static inline unsigned long bfq_min_budget(struct bfq_data *bfqd)
{
	if (bfqd->budgets_assigned < 194)
		return bfq_default_max_budget / 32;
	else
		return bfqd->bfq_max_budget / 32;
}

static void bfq_arm_slice_timer(struct bfq_data *bfqd)
{
	struct bfq_queue *bfqq = bfqd->in_service_queue;
	struct bfq_io_cq *bic;
	unsigned long sl;

	BUG_ON(!RB_EMPTY_ROOT(&bfqq->sort_list));

	/* Processes have exited, don't wait. */
	bic = bfqd->in_service_bic;
	if (bic == NULL || atomic_read(&bic->icq.ioc->active_ref) == 0)
		return;

	bfq_mark_bfqq_wait_request(bfqq);

	/*
	 * We don't want to idle for seeks, but we do want to allow
	 * fair distribution of slice time for a process doing back-to-back
	 * seeks. So allow a little bit of time for him to submit a new rq.
	 *
	 * To prevent processes with (partly) seeky workloads from
	 * being too ill-treated, grant them a small fraction of the
	 * assigned budget before reducing the waiting time to
	 * BFQ_MIN_TT. This happened to help reduce latency.
	 */
	sl = bfqd->bfq_slice_idle;
	/*
	 * Unless the queue is being weight-raised or the scenario is
	 * asymmetric, grant only minimum idle time if the queue either
	 * has been seeky for long enough or has already proved to be
	 * constantly seeky.
	 */
	if (bfq_sample_valid(bfqq->seek_samples) &&
	    ((BFQQ_SEEKY(bfqq) && bfqq->entity.service >
				  bfq_max_budget(bfqq->bfqd) / 8) ||
	      bfq_bfqq_constantly_seeky(bfqq)) && bfqq->wr_coeff == 1 &&
	    symmetric_scenario)
		sl = min(sl, msecs_to_jiffies(BFQ_MIN_TT));
	else if (bfqq->wr_coeff > 1)
		sl = sl * 3;
	bfqd->last_idling_start = ktime_get();
	mod_timer(&bfqd->idle_slice_timer, jiffies + sl);
	bfq_log(bfqd, "arm idle: %u/%u ms",
		jiffies_to_msecs(sl), jiffies_to_msecs(bfqd->bfq_slice_idle));
}

/*
 * Set the maximum time for the in-service queue to consume its
 * budget. This prevents seeky processes from lowering the disk
 * throughput (always guaranteed with a time slice scheme as in CFQ).
 */
static void bfq_set_budget_timeout(struct bfq_data *bfqd)
{
	struct bfq_queue *bfqq = bfqd->in_service_queue;
	unsigned int timeout_coeff;
	if (bfqq->wr_cur_max_time == bfqd->bfq_wr_rt_max_time)
		timeout_coeff = 1;
	else
		timeout_coeff = bfqq->entity.weight / bfqq->entity.orig_weight;

	bfqd->last_budget_start = ktime_get();

	bfq_clear_bfqq_budget_new(bfqq);
	bfqq->budget_timeout = jiffies +
		bfqd->bfq_timeout[bfq_bfqq_sync(bfqq)] * timeout_coeff;

	bfq_log_bfqq(bfqd, bfqq, "set budget_timeout %u",
		jiffies_to_msecs(bfqd->bfq_timeout[bfq_bfqq_sync(bfqq)] *
		timeout_coeff));
}

/*
 * Move request from internal lists to the request queue dispatch list.
 */
static void bfq_dispatch_insert(struct request_queue *q, struct request *rq)
{
	struct bfq_data *bfqd = q->elevator->elevator_data;
	struct bfq_queue *bfqq = RQ_BFQQ(rq);

	/*
	 * For consistency, the next instruction should have been executed
	 * after removing the request from the queue and dispatching it.
	 * We execute instead this instruction before bfq_remove_request()
	 * (and hence introduce a temporary inconsistency), for efficiency.
	 * In fact, in a forced_dispatch, this prevents two counters related
	 * to bfqq->dispatched to risk to be uselessly decremented if bfqq
	 * is not in service, and then to be incremented again after
	 * incrementing bfqq->dispatched.
	 */
	bfqq->dispatched++;
	bfq_remove_request(rq);
	elv_dispatch_sort(q, rq);

	if (bfq_bfqq_sync(bfqq))
		bfqd->sync_flight++;
}

/*
 * Return expired entry, or NULL to just start from scratch in rbtree.
 */
static struct request *bfq_check_fifo(struct bfq_queue *bfqq)
{
	struct request *rq = NULL;

	if (bfq_bfqq_fifo_expire(bfqq))
		return NULL;

	bfq_mark_bfqq_fifo_expire(bfqq);

	if (list_empty(&bfqq->fifo))
		return NULL;

	rq = rq_entry_fifo(bfqq->fifo.next);

	if (time_before(jiffies, rq_fifo_time(rq)))
		return NULL;

	return rq;
}

static inline unsigned long bfq_bfqq_budget_left(struct bfq_queue *bfqq)
{
	struct bfq_entity *entity = &bfqq->entity;
	return entity->budget - entity->service;
}

static void __bfq_bfqq_expire(struct bfq_data *bfqd, struct bfq_queue *bfqq)
{
	BUG_ON(bfqq != bfqd->in_service_queue);

	__bfq_bfqd_reset_in_service(bfqd);

	/*
	 * If this bfqq is shared between multiple processes, check
	 * to make sure that those processes are still issuing I/Os
	 * within the mean seek distance. If not, it may be time to
	 * break the queues apart again.
	 */
	if (bfq_bfqq_coop(bfqq) && BFQQ_SEEKY(bfqq))
		bfq_mark_bfqq_split_coop(bfqq);

	if (RB_EMPTY_ROOT(&bfqq->sort_list)) {
		/*
		 * Overloading budget_timeout field to store the time
		 * at which the queue remains with no backlog; used by
		 * the weight-raising mechanism.
		 */
		bfqq->budget_timeout = jiffies;
		bfq_del_bfqq_busy(bfqd, bfqq, 1);
	} else {
		bfq_activate_bfqq(bfqd, bfqq);
		/*
		 * Resort priority tree of potential close cooperators.
		 */
		bfq_rq_pos_tree_add(bfqd, bfqq);
	}
}

/**
 * __bfq_bfqq_recalc_budget - try to adapt the budget to the @bfqq behavior.
 * @bfqd: device data.
 * @bfqq: queue to update.
 * @reason: reason for expiration.
 *
 * Handle the feedback on @bfqq budget.  See the body for detailed
 * comments.
 */
static void __bfq_bfqq_recalc_budget(struct bfq_data *bfqd,
				     struct bfq_queue *bfqq,
				     enum bfqq_expiration reason)
{
	struct request *next_rq;
	unsigned long budget, min_budget;

	budget = bfqq->max_budget;
	min_budget = bfq_min_budget(bfqd);

	BUG_ON(bfqq != bfqd->in_service_queue);

	bfq_log_bfqq(bfqd, bfqq, "recalc_budg: last budg %lu, budg left %lu",
		bfqq->entity.budget, bfq_bfqq_budget_left(bfqq));
	bfq_log_bfqq(bfqd, bfqq, "recalc_budg: last max_budg %lu, min budg %lu",
		budget, bfq_min_budget(bfqd));
	bfq_log_bfqq(bfqd, bfqq, "recalc_budg: sync %d, seeky %d",
		bfq_bfqq_sync(bfqq), BFQQ_SEEKY(bfqd->in_service_queue));

	if (bfq_bfqq_sync(bfqq)) {
		switch (reason) {
		/*
		 * Caveat: in all the following cases we trade latency
		 * for throughput.
		 */
		case BFQ_BFQQ_TOO_IDLE:
			/*
			 * This is the only case where we may reduce
			 * the budget: if there is no request of the
			 * process still waiting for completion, then
			 * we assume (tentatively) that the timer has
			 * expired because the batch of requests of
			 * the process could have been served with a
			 * smaller budget.  Hence, betting that
			 * process will behave in the same way when it
			 * becomes backlogged again, we reduce its
			 * next budget.  As long as we guess right,
			 * this budget cut reduces the latency
			 * experienced by the process.
			 *
			 * However, if there are still outstanding
			 * requests, then the process may have not yet
			 * issued its next request just because it is
			 * still waiting for the completion of some of
			 * the still outstanding ones.  So in this
			 * subcase we do not reduce its budget, on the
			 * contrary we increase it to possibly boost
			 * the throughput, as discussed in the
			 * comments to the BUDGET_TIMEOUT case.
			 */
			if (bfqq->dispatched > 0) /* still outstanding reqs */
				budget = min(budget * 2, bfqd->bfq_max_budget);
			else {
				if (budget > 5 * min_budget)
					budget -= 4 * min_budget;
				else
					budget = min_budget;
			}
			break;
		case BFQ_BFQQ_BUDGET_TIMEOUT:
			/*
			 * We double the budget here because: 1) it
			 * gives the chance to boost the throughput if
			 * this is not a seeky process (which may have
			 * bumped into this timeout because of, e.g.,
			 * ZBR), 2) together with charge_full_budget
			 * it helps give seeky processes higher
			 * timestamps, and hence be served less
			 * frequently.
			 */
			budget = min(budget * 2, bfqd->bfq_max_budget);
			break;
		case BFQ_BFQQ_BUDGET_EXHAUSTED:
			/*
			 * The process still has backlog, and did not
			 * let either the budget timeout or the disk
			 * idling timeout expire. Hence it is not
			 * seeky, has a short thinktime and may be
			 * happy with a higher budget too. So
			 * definitely increase the budget of this good
			 * candidate to boost the disk throughput.
			 */
			budget = min(budget * 4, bfqd->bfq_max_budget);
			break;
		case BFQ_BFQQ_NO_MORE_REQUESTS:
		       /*
			* Leave the budget unchanged.
			*/
		default:
			return;
		}
	} else /* async queue */
	    /* async queues get always the maximum possible budget
	     * (their ability to dispatch is limited by
	     * @bfqd->bfq_max_budget_async_rq).
	     */
		budget = bfqd->bfq_max_budget;

	bfqq->max_budget = budget;

	if (bfqd->budgets_assigned >= 194 && bfqd->bfq_user_max_budget == 0 &&
	    bfqq->max_budget > bfqd->bfq_max_budget)
		bfqq->max_budget = bfqd->bfq_max_budget;

	/*
	 * Make sure that we have enough budget for the next request.
	 * Since the finish time of the bfqq must be kept in sync with
	 * the budget, be sure to call __bfq_bfqq_expire() after the
	 * update.
	 */
	next_rq = bfqq->next_rq;
	if (next_rq != NULL)
		bfqq->entity.budget = max_t(unsigned long, bfqq->max_budget,
					    bfq_serv_to_charge(next_rq, bfqq));
	else
		bfqq->entity.budget = bfqq->max_budget;

	bfq_log_bfqq(bfqd, bfqq, "head sect: %u, new budget %lu",
			next_rq != NULL ? blk_rq_sectors(next_rq) : 0,
			bfqq->entity.budget);
}

static unsigned long bfq_calc_max_budget(u64 peak_rate, u64 timeout)
{
	unsigned long max_budget;

	/*
	 * The max_budget calculated when autotuning is equal to the
	 * amount of sectors transfered in timeout_sync at the
	 * estimated peak rate.
	 */
	max_budget = (unsigned long)(peak_rate * 1000 *
				     timeout >> BFQ_RATE_SHIFT);

	return max_budget;
}

/*
 * In addition to updating the peak rate, checks whether the process
 * is "slow", and returns 1 if so. This slow flag is used, in addition
 * to the budget timeout, to reduce the amount of service provided to
 * seeky processes, and hence reduce their chances to lower the
 * throughput. See the code for more details.
 */
static int bfq_update_peak_rate(struct bfq_data *bfqd, struct bfq_queue *bfqq,
				int compensate, enum bfqq_expiration reason)
{
	u64 bw, usecs, expected, timeout;
	ktime_t delta;
	int update = 0;

	if (!bfq_bfqq_sync(bfqq) || bfq_bfqq_budget_new(bfqq))
		return 0;

	if (compensate)
		delta = bfqd->last_idling_start;
	else
		delta = ktime_get();
	delta = ktime_sub(delta, bfqd->last_budget_start);
	usecs = ktime_to_us(delta);

	/* Don't trust short/unrealistic values. */
	if (usecs < 100 || usecs >= LONG_MAX)
		return 0;

	/*
	 * Calculate the bandwidth for the last slice.  We use a 64 bit
	 * value to store the peak rate, in sectors per usec in fixed
	 * point math.  We do so to have enough precision in the estimate
	 * and to avoid overflows.
	 */
	bw = (u64)bfqq->entity.service << BFQ_RATE_SHIFT;
	do_div(bw, (unsigned long)usecs);

	timeout = jiffies_to_msecs(bfqd->bfq_timeout[BLK_RW_SYNC]);

	/*
	 * Use only long (> 20ms) intervals to filter out spikes for
	 * the peak rate estimation.
	 */
	if (usecs > 20000) {
		if (bw > bfqd->peak_rate ||
		   (!BFQQ_SEEKY(bfqq) &&
		    reason == BFQ_BFQQ_BUDGET_TIMEOUT)) {
			bfq_log(bfqd, "measured bw =%llu", bw);
			/*
			 * To smooth oscillations use a low-pass filter with
			 * alpha=7/8, i.e.,
			 * new_rate = (7/8) * old_rate + (1/8) * bw
			 */
			do_div(bw, 8);
			if (bw == 0)
				return 0;
			bfqd->peak_rate *= 7;
			do_div(bfqd->peak_rate, 8);
			bfqd->peak_rate += bw;
			update = 1;
			bfq_log(bfqd, "new peak_rate=%llu", bfqd->peak_rate);
		}

		update |= bfqd->peak_rate_samples == BFQ_PEAK_RATE_SAMPLES - 1;

		if (bfqd->peak_rate_samples < BFQ_PEAK_RATE_SAMPLES)
			bfqd->peak_rate_samples++;

		if (bfqd->peak_rate_samples == BFQ_PEAK_RATE_SAMPLES &&
		    update) {
			int dev_type = blk_queue_nonrot(bfqd->queue);
			if (bfqd->bfq_user_max_budget == 0) {
				bfqd->bfq_max_budget =
					bfq_calc_max_budget(bfqd->peak_rate,
							    timeout);
				bfq_log(bfqd, "new max_budget=%lu",
					bfqd->bfq_max_budget);
			}
			if (bfqd->device_speed == BFQ_BFQD_FAST &&
			    bfqd->peak_rate < device_speed_thresh[dev_type]) {
				bfqd->device_speed = BFQ_BFQD_SLOW;
				bfqd->RT_prod = R_slow[dev_type] *
						T_slow[dev_type];
			} else if (bfqd->device_speed == BFQ_BFQD_SLOW &&
			    bfqd->peak_rate > device_speed_thresh[dev_type]) {
				bfqd->device_speed = BFQ_BFQD_FAST;
				bfqd->RT_prod = R_fast[dev_type] *
						T_fast[dev_type];
			}
		}
	}

	/*
	 * If the process has been served for a too short time
	 * interval to let its possible sequential accesses prevail on
	 * the initial seek time needed to move the disk head on the
	 * first sector it requested, then give the process a chance
	 * and for the moment return false.
	 */
	if (bfqq->entity.budget <= bfq_max_budget(bfqd) / 8)
		return 0;

	/*
	 * A process is considered ``slow'' (i.e., seeky, so that we
	 * cannot treat it fairly in the service domain, as it would
	 * slow down too much the other processes) if, when a slice
	 * ends for whatever reason, it has received service at a
	 * rate that would not be high enough to complete the budget
	 * before the budget timeout expiration.
	 */
	expected = bw * 1000 * timeout >> BFQ_RATE_SHIFT;

	/*
	 * Caveat: processes doing IO in the slower disk zones will
	 * tend to be slow(er) even if not seeky. And the estimated
	 * peak rate will actually be an average over the disk
	 * surface. Hence, to not be too harsh with unlucky processes,
	 * we keep a budget/3 margin of safety before declaring a
	 * process slow.
	 */
	return expected > (4 * bfqq->entity.budget) / 3;
}

/*
 * To be deemed as soft real-time, an application must meet two
 * requirements. First, the application must not require an average
 * bandwidth higher than the approximate bandwidth required to playback or
 * record a compressed high-definition video.
 * The next function is invoked on the completion of the last request of a
 * batch, to compute the next-start time instant, soft_rt_next_start, such
 * that, if the next request of the application does not arrive before
 * soft_rt_next_start, then the above requirement on the bandwidth is met.
 *
 * The second requirement is that the request pattern of the application is
 * isochronous, i.e., that, after issuing a request or a batch of requests,
 * the application stops issuing new requests until all its pending requests
 * have been completed. After that, the application may issue a new batch,
 * and so on.
 * For this reason the next function is invoked to compute
 * soft_rt_next_start only for applications that meet this requirement,
 * whereas soft_rt_next_start is set to infinity for applications that do
 * not.
 *
 * Unfortunately, even a greedy application may happen to behave in an
 * isochronous way if the CPU load is high. In fact, the application may
 * stop issuing requests while the CPUs are busy serving other processes,
 * then restart, then stop again for a while, and so on. In addition, if
 * the disk achieves a low enough throughput with the request pattern
 * issued by the application (e.g., because the request pattern is random
 * and/or the device is slow), then the application may meet the above
 * bandwidth requirement too. To prevent such a greedy application to be
 * deemed as soft real-time, a further rule is used in the computation of
 * soft_rt_next_start: soft_rt_next_start must be higher than the current
 * time plus the maximum time for which the arrival of a request is waited
 * for when a sync queue becomes idle, namely bfqd->bfq_slice_idle.
 * This filters out greedy applications, as the latter issue instead their
 * next request as soon as possible after the last one has been completed
 * (in contrast, when a batch of requests is completed, a soft real-time
 * application spends some time processing data).
 *
 * Unfortunately, the last filter may easily generate false positives if
 * only bfqd->bfq_slice_idle is used as a reference time interval and one
 * or both the following cases occur:
 * 1) HZ is so low that the duration of a jiffy is comparable to or higher
 *    than bfqd->bfq_slice_idle. This happens, e.g., on slow devices with
 *    HZ=100.
 * 2) jiffies, instead of increasing at a constant rate, may stop increasing
 *    for a while, then suddenly 'jump' by several units to recover the lost
 *    increments. This seems to happen, e.g., inside virtual machines.
 * To address this issue, we do not use as a reference time interval just
 * bfqd->bfq_slice_idle, but bfqd->bfq_slice_idle plus a few jiffies. In
 * particular we add the minimum number of jiffies for which the filter
 * seems to be quite precise also in embedded systems and KVM/QEMU virtual
 * machines.
 */
static inline unsigned long bfq_bfqq_softrt_next_start(struct bfq_data *bfqd,
						       struct bfq_queue *bfqq)
{
	return max(bfqq->last_idle_bklogged +
		   HZ * bfqq->service_from_backlogged /
		   bfqd->bfq_wr_max_softrt_rate,
		   jiffies + bfqq->bfqd->bfq_slice_idle + 4);
}

/*
 * Return the largest-possible time instant such that, for as long as possible,
 * the current time will be lower than this time instant according to the macro
 * time_is_before_jiffies().
 */
static inline unsigned long bfq_infinity_from_now(unsigned long now)
{
	return now + ULONG_MAX / 2;
}

/**
 * bfq_bfqq_expire - expire a queue.
 * @bfqd: device owning the queue.
 * @bfqq: the queue to expire.
 * @compensate: if true, compensate for the time spent idling.
 * @reason: the reason causing the expiration.
 *
 *
 * If the process associated to the queue is slow (i.e., seeky), or in
 * case of budget timeout, or, finally, if it is async, we
 * artificially charge it an entire budget (independently of the
 * actual service it received). As a consequence, the queue will get
 * higher timestamps than the correct ones upon reactivation, and
 * hence it will be rescheduled as if it had received more service
 * than what it actually received. In the end, this class of processes
 * will receive less service in proportion to how slowly they consume
 * their budgets (and hence how seriously they tend to lower the
 * throughput).
 *
 * In contrast, when a queue expires because it has been idling for
 * too much or because it exhausted its budget, we do not touch the
 * amount of service it has received. Hence when the queue will be
 * reactivated and its timestamps updated, the latter will be in sync
 * with the actual service received by the queue until expiration.
 *
 * Charging a full budget to the first type of queues and the exact
 * service to the others has the effect of using the WF2Q+ policy to
 * schedule the former on a timeslice basis, without violating the
 * service domain guarantees of the latter.
 */
static void bfq_bfqq_expire(struct bfq_data *bfqd,
			    struct bfq_queue *bfqq,
			    int compensate,
			    enum bfqq_expiration reason)
{
	int slow;
	BUG_ON(bfqq != bfqd->in_service_queue);

	/* Update disk peak rate for autotuning and check whether the
	 * process is slow (see bfq_update_peak_rate).
	 */
	slow = bfq_update_peak_rate(bfqd, bfqq, compensate, reason);

	/*
	 * As above explained, 'punish' slow (i.e., seeky), timed-out
	 * and async queues, to favor sequential sync workloads.
	 *
	 * Processes doing I/O in the slower disk zones will tend to be
	 * slow(er) even if not seeky. Hence, since the estimated peak
	 * rate is actually an average over the disk surface, these
	 * processes may timeout just for bad luck. To avoid punishing
	 * them we do not charge a full budget to a process that
	 * succeeded in consuming at least 2/3 of its budget.
	 */
	if (slow || (reason == BFQ_BFQQ_BUDGET_TIMEOUT &&
		     bfq_bfqq_budget_left(bfqq) >=  bfqq->entity.budget / 3))
		bfq_bfqq_charge_full_budget(bfqq);

	bfqq->service_from_backlogged += bfqq->entity.service;

	if (BFQQ_SEEKY(bfqq) && reason == BFQ_BFQQ_BUDGET_TIMEOUT &&
	    !bfq_bfqq_constantly_seeky(bfqq)) {
		bfq_mark_bfqq_constantly_seeky(bfqq);
		if (!blk_queue_nonrot(bfqd->queue))
			bfqd->const_seeky_busy_in_flight_queues++;
	}

	if (reason == BFQ_BFQQ_TOO_IDLE &&
	    bfqq->entity.service <= 2 * bfqq->entity.budget / 10 )
		bfq_clear_bfqq_IO_bound(bfqq);

	if (bfqd->low_latency && bfqq->wr_coeff == 1)
		bfqq->last_wr_start_finish = jiffies;

	if (bfqd->low_latency && bfqd->bfq_wr_max_softrt_rate > 0 &&
	    RB_EMPTY_ROOT(&bfqq->sort_list)) {
		/*
		 * If we get here, and there are no outstanding requests,
		 * then the request pattern is isochronous (see the comments
		 * to the function bfq_bfqq_softrt_next_start()). Hence we
		 * can compute soft_rt_next_start. If, instead, the queue
		 * still has outstanding requests, then we have to wait
		 * for the completion of all the outstanding requests to
		 * discover whether the request pattern is actually
		 * isochronous.
		 */
		if (bfqq->dispatched == 0)
			bfqq->soft_rt_next_start =
				bfq_bfqq_softrt_next_start(bfqd, bfqq);
		else {
			/*
			 * The application is still waiting for the
			 * completion of one or more requests:
			 * prevent it from possibly being incorrectly
			 * deemed as soft real-time by setting its
			 * soft_rt_next_start to infinity. In fact,
			 * without this assignment, the application
			 * would be incorrectly deemed as soft
			 * real-time if:
			 * 1) it issued a new request before the
			 *    completion of all its in-flight
			 *    requests, and
			 * 2) at that time, its soft_rt_next_start
			 *    happened to be in the past.
			 */
			bfqq->soft_rt_next_start =
				bfq_infinity_from_now(jiffies);
			/*
			 * Schedule an update of soft_rt_next_start to when
			 * the task may be discovered to be isochronous.
			 */
			bfq_mark_bfqq_softrt_update(bfqq);
		}
	}

	bfq_log_bfqq(bfqd, bfqq,
		"expire (%d, slow %d, num_disp %d, idle_win %d)", reason,
		slow, bfqq->dispatched, bfq_bfqq_idle_window(bfqq));

	/*
	 * Increase, decrease or leave budget unchanged according to
	 * reason.
	 */
	__bfq_bfqq_recalc_budget(bfqd, bfqq, reason);
	__bfq_bfqq_expire(bfqd, bfqq);
}

/*
 * Budget timeout is not implemented through a dedicated timer, but
 * just checked on request arrivals and completions, as well as on
 * idle timer expirations.
 */
static int bfq_bfqq_budget_timeout(struct bfq_queue *bfqq)
{
	if (bfq_bfqq_budget_new(bfqq) ||
	    time_before(jiffies, bfqq->budget_timeout))
		return 0;
	return 1;
}

/*
 * If we expire a queue that is waiting for the arrival of a new
 * request, we may prevent the fictitious timestamp back-shifting that
 * allows the guarantees of the queue to be preserved (see [1] for
 * this tricky aspect). Hence we return true only if this condition
 * does not hold, or if the queue is slow enough to deserve only to be
 * kicked off for preserving a high throughput.
*/
static inline int bfq_may_expire_for_budg_timeout(struct bfq_queue *bfqq)
{
	bfq_log_bfqq(bfqq->bfqd, bfqq,
		"may_budget_timeout: wait_request %d left %d timeout %d",
		bfq_bfqq_wait_request(bfqq),
			bfq_bfqq_budget_left(bfqq) >=  bfqq->entity.budget / 3,
		bfq_bfqq_budget_timeout(bfqq));

	return (!bfq_bfqq_wait_request(bfqq) ||
		bfq_bfqq_budget_left(bfqq) >=  bfqq->entity.budget / 3)
		&&
		bfq_bfqq_budget_timeout(bfqq);
}

/*
 * Device idling is allowed only for the queues for which this function
 * returns true. For this reason, the return value of this function plays a
 * critical role for both throughput boosting and service guarantees. The
 * return value is computed through a logical expression. In this rather
 * long comment, we try to briefly describe all the details and motivations
 * behind the components of this logical expression.
 *
 * First, the expression is false if bfqq is not sync, or if: bfqq happened
 * to become active during a large burst of queue activations, and the
 * pattern of requests bfqq contains boosts the throughput if bfqq is
 * expired. In fact, queues that became active during a large burst benefit
 * only from throughput, as discussed in the comments to bfq_handle_burst.
 * In this respect, expiring bfqq certainly boosts the throughput on NCQ-
 * capable flash-based devices, whereas, on rotational devices, it boosts
 * the throughput only if bfqq contains random requests.
 *
 * On the opposite end, if (a) bfqq is sync, (b) the above burst-related
 * condition does not hold, and (c) bfqq is being weight-raised, then the
 * expression always evaluates to true, as device idling is instrumental
 * for preserving low-latency guarantees (see [1]). If, instead, conditions
 * (a) and (b) do hold, but (c) does not, then the expression evaluates to
 * true only if: (1) bfqq is I/O-bound and has a non-null idle window, and
 * (2) at least one of the following two conditions holds.
 * The first condition is that the device is not performing NCQ, because
 * idling the device most certainly boosts the throughput if this condition
 * holds and bfqq is I/O-bound and has been granted a non-null idle window.
 * The second compound condition is made of the logical AND of two components.
 *
 * The first component is true only if there is no weight-raised busy
 * queue. This guarantees that the device is not idled for a sync non-
 * weight-raised queue when there are busy weight-raised queues. The former
 * is then expired immediately if empty. Combined with the timestamping
 * rules of BFQ (see [1] for details), this causes sync non-weight-raised
 * queues to get a lower number of requests served, and hence to ask for a
 * lower number of requests from the request pool, before the busy weight-
 * raised queues get served again.
 *
 * This is beneficial for the processes associated with weight-raised
 * queues, when the request pool is saturated (e.g., in the presence of
 * write hogs). In fact, if the processes associated with the other queues
 * ask for requests at a lower rate, then weight-raised processes have a
 * higher probability to get a request from the pool immediately (or at
 * least soon) when they need one. Hence they have a higher probability to
 * actually get a fraction of the disk throughput proportional to their
 * high weight. This is especially true with NCQ-capable drives, which
 * enqueue several requests in advance and further reorder internally-
 * queued requests.
 *
 * In the end, mistreating non-weight-raised queues when there are busy
 * weight-raised queues seems to mitigate starvation problems in the
 * presence of heavy write workloads and NCQ, and hence to guarantee a
 * higher application and system responsiveness in these hostile scenarios.
 *
 * If the first component of the compound condition is instead true, i.e.,
 * there is no weight-raised busy queue, then the second component of the
 * compound condition takes into account service-guarantee and throughput
 * issues related to NCQ (recall that the compound condition is evaluated
 * only if the device is detected as supporting NCQ).
 *
 * As for service guarantees, allowing the drive to enqueue more than one
 * request at a time, and hence delegating de facto final scheduling
 * decisions to the drive's internal scheduler, causes loss of control on
 * the actual request service order. In this respect, when the drive is
 * allowed to enqueue more than one request at a time, the service
 * distribution enforced by the drive's internal scheduler is likely to
 * coincide with the desired device-throughput distribution only in the
 * following, perfectly symmetric, scenario:
 * 1) all active queues have the same weight,
 * 2) all active groups at the same level in the groups tree have the same
 *    weight,
 * 3) all active groups at the same level in the groups tree have the same
 *    number of children.
 *
 * Even in such a scenario, sequential I/O may still receive a preferential
 * treatment, but this is not likely to be a big issue with flash-based
 * devices, because of their non-dramatic loss of throughput with random
 * I/O. Things do differ with HDDs, for which additional care is taken, as
 * explained after completing the discussion for flash-based devices.
 *
 * Unfortunately, keeping the necessary state for evaluating exactly the
 * above symmetry conditions would be quite complex and time-consuming.
 * Therefore BFQ evaluates instead the following stronger sub-conditions,
 * for which it is much easier to maintain the needed state:
 * 1) all active queues have the same weight,
 * 2) all active groups have the same weight,
 * 3) all active groups have at most one active child each.
 * In particular, the last two conditions are always true if hierarchical
 * support and the cgroups interface are not enabled, hence no state needs
 * to be maintained in this case.
 *
 * According to the above considerations, the second component of the
 * compound condition evaluates to true if any of the above symmetry
 * sub-condition does not hold, or the device is not flash-based. Therefore,
 * if also the first component is true, then idling is allowed for a sync
 * queue. These are the only sub-conditions considered if the device is
 * flash-based, as, for such a device, it is sensible to force idling only
 * for service-guarantee issues. In fact, as for throughput, idling
 * NCQ-capable flash-based devices would not boost the throughput even
 * with sequential I/O; rather it would lower the throughput in proportion
 * to how fast the device is. In the end, (only) if all the three
 * sub-conditions hold and the device is flash-based, the compound
 * condition evaluates to false and therefore no idling is performed.
 *
 * As already said, things change with a rotational device, where idling
 * boosts the throughput with sequential I/O (even with NCQ). Hence, for
 * such a device the second component of the compound condition evaluates
 * to true also if the following additional sub-condition does not hold:
 * the queue is constantly seeky. Unfortunately, this different behavior
 * with respect to flash-based devices causes an additional asymmetry: if
 * some sync queues enjoy idling and some other sync queues do not, then
 * the latter get a low share of the device throughput, simply because the
 * former get many requests served after being set as in service, whereas
 * the latter do not. As a consequence, to guarantee the desired throughput
 * distribution, on HDDs the compound expression evaluates to true (and
 * hence device idling is performed) also if the following last symmetry
 * condition does not hold: no other queue is benefiting from idling. Also
 * this last condition is actually replaced with a simpler-to-maintain and
 * stronger condition: there is no busy queue which is not constantly seeky
 * (and hence may also benefit from idling).
 *
 * To sum up, when all the required symmetry and throughput-boosting
 * sub-conditions hold, the second component of the compound condition
 * evaluates to false, and hence no idling is performed. This helps to
 * keep the drives' internal queues full on NCQ-capable devices, and hence
 * to boost the throughput, without causing 'almost' any loss of service
 * guarantees. The 'almost' follows from the fact that, if the internal
 * queue of one such device is filled while all the sub-conditions hold,
 * but at some point in time some sub-condition stops to hold, then it may
 * become impossible to let requests be served in the new desired order
 * until all the requests already queued in the device have been served.
 */
static inline bool bfq_bfqq_must_not_expire(struct bfq_queue *bfqq)
{
	struct bfq_data *bfqd = bfqq->bfqd;
#define cond_for_seeky_on_ncq_hdd (bfq_bfqq_constantly_seeky(bfqq) && \
				   bfqd->busy_in_flight_queues == \
				   bfqd->const_seeky_busy_in_flight_queues)

#define cond_for_expiring_in_burst	(bfq_bfqq_in_large_burst(bfqq) && \
					 bfqd->hw_tag && \
					 (blk_queue_nonrot(bfqd->queue) || \
					  bfq_bfqq_constantly_seeky(bfqq)))

/*
 * Condition for expiring a non-weight-raised queue (and hence not idling
 * the device).
 */
#define cond_for_expiring_non_wr  (bfqd->hw_tag && \
				   (bfqd->wr_busy_queues > 0 || \
				    (blk_queue_nonrot(bfqd->queue) || \
				      cond_for_seeky_on_ncq_hdd)))

	return bfq_bfqq_sync(bfqq) &&
		!cond_for_expiring_in_burst &&
		(bfqq->wr_coeff > 1 || !symmetric_scenario ||
		 (bfq_bfqq_IO_bound(bfqq) && bfq_bfqq_idle_window(bfqq) &&
		  !cond_for_expiring_non_wr)
	);
}

/*
 * If the in-service queue is empty but sync, and the function
 * bfq_bfqq_must_not_expire returns true, then:
 * 1) the queue must remain in service and cannot be expired, and
 * 2) the disk must be idled to wait for the possible arrival of a new
 *    request for the queue.
 * See the comments to the function bfq_bfqq_must_not_expire for the reasons
 * why performing device idling is the best choice to boost the throughput
 * and preserve service guarantees when bfq_bfqq_must_not_expire itself
 * returns true.
 */
static inline bool bfq_bfqq_must_idle(struct bfq_queue *bfqq)
{
	struct bfq_data *bfqd = bfqq->bfqd;

	return RB_EMPTY_ROOT(&bfqq->sort_list) && bfqd->bfq_slice_idle != 0 &&
	       bfq_bfqq_must_not_expire(bfqq);
}

/*
 * Select a queue for service.  If we have a current queue in service,
 * check whether to continue servicing it, or retrieve and set a new one.
 */
static struct bfq_queue *bfq_select_queue(struct bfq_data *bfqd)
{
	struct bfq_queue *bfqq;
	struct request *next_rq;
	enum bfqq_expiration reason = BFQ_BFQQ_BUDGET_TIMEOUT;

	bfqq = bfqd->in_service_queue;
	if (bfqq == NULL)
		goto new_queue;

	bfq_log_bfqq(bfqd, bfqq, "select_queue: already in-service queue");

	if (bfq_may_expire_for_budg_timeout(bfqq) &&
	    !timer_pending(&bfqd->idle_slice_timer) &&
	    !bfq_bfqq_must_idle(bfqq))
		goto expire;

	next_rq = bfqq->next_rq;
	/*
	 * If bfqq has requests queued and it has enough budget left to
	 * serve them, keep the queue, otherwise expire it.
	 */
	if (next_rq != NULL) {
		if (bfq_serv_to_charge(next_rq, bfqq) >
			bfq_bfqq_budget_left(bfqq)) {
			reason = BFQ_BFQQ_BUDGET_EXHAUSTED;
			goto expire;
		} else {
			/*
			 * The idle timer may be pending because we may
			 * not disable disk idling even when a new request
			 * arrives.
			 */
			if (timer_pending(&bfqd->idle_slice_timer)) {
				/*
				 * If we get here: 1) at least a new request
				 * has arrived but we have not disabled the
				 * timer because the request was too small,
				 * 2) then the block layer has unplugged
				 * the device, causing the dispatch to be
				 * invoked.
				 *
				 * Since the device is unplugged, now the
				 * requests are probably large enough to
				 * provide a reasonable throughput.
				 * So we disable idling.
				 */
				bfq_clear_bfqq_wait_request(bfqq);
				del_timer(&bfqd->idle_slice_timer);
			}
			goto keep_queue;
		}
	}

	/*
	 * No requests pending. However, if the in-service queue is idling
	 * for a new request, or has requests waiting for a completion and
	 * may idle after their completion, then keep it anyway.
	 */
	if (timer_pending(&bfqd->idle_slice_timer) ||
	    (bfqq->dispatched != 0 && bfq_bfqq_must_not_expire(bfqq))) {
		bfqq = NULL;
		goto keep_queue;
	}

	reason = BFQ_BFQQ_NO_MORE_REQUESTS;
expire:
	bfq_bfqq_expire(bfqd, bfqq, 0, reason);
new_queue:
	bfqq = bfq_set_in_service_queue(bfqd);
	bfq_log(bfqd, "select_queue: new queue %d returned",
		bfqq != NULL ? bfqq->pid : 0);
keep_queue:
	return bfqq;
}

static void bfq_update_wr_data(struct bfq_data *bfqd, struct bfq_queue *bfqq)
{
	struct bfq_entity *entity = &bfqq->entity;
	if (bfqq->wr_coeff > 1) { /* queue is being weight-raised */
		bfq_log_bfqq(bfqd, bfqq,
			"raising period dur %u/%u msec, old coeff %u, w %d(%d)",
			jiffies_to_msecs(jiffies - bfqq->last_wr_start_finish),
			jiffies_to_msecs(bfqq->wr_cur_max_time),
			bfqq->wr_coeff,
			bfqq->entity.weight, bfqq->entity.orig_weight);

		BUG_ON(bfqq != bfqd->in_service_queue && entity->weight !=
		       entity->orig_weight * bfqq->wr_coeff);
		if (entity->ioprio_changed)
			bfq_log_bfqq(bfqd, bfqq, "WARN: pending prio change");

		/*
		 * If the queue was activated in a burst, or
		 * too much time has elapsed from the beginning
		 * of this weight-raising period, or the queue has
		 * exceeded the acceptable number of cooperations,
		 * then end weight raising.
		 */
		if (bfq_bfqq_in_large_burst(bfqq) ||
		    bfq_bfqq_cooperations(bfqq) >= bfqd->bfq_coop_thresh ||
		    time_is_before_jiffies(bfqq->last_wr_start_finish +
					   bfqq->wr_cur_max_time)) {
			bfqq->last_wr_start_finish = jiffies;
			bfq_log_bfqq(bfqd, bfqq,
				     "wrais ending at %lu, rais_max_time %u",
				     bfqq->last_wr_start_finish,
				     jiffies_to_msecs(bfqq->wr_cur_max_time));
			bfq_bfqq_end_wr(bfqq);
		}
	}
	/* Update weight both if it must be raised and if it must be lowered */
	if ((entity->weight > entity->orig_weight) != (bfqq->wr_coeff > 1))
		__bfq_entity_update_weight_prio(
			bfq_entity_service_tree(entity),
			entity);
}

/*
 * Dispatch one request from bfqq, moving it to the request queue
 * dispatch list.
 */
static int bfq_dispatch_request(struct bfq_data *bfqd,
				struct bfq_queue *bfqq)
{
	int dispatched = 0;
	struct request *rq;
	unsigned long service_to_charge;

	BUG_ON(RB_EMPTY_ROOT(&bfqq->sort_list));

	/* Follow expired path, else get first next available. */
	rq = bfq_check_fifo(bfqq);
	if (rq == NULL)
		rq = bfqq->next_rq;
	service_to_charge = bfq_serv_to_charge(rq, bfqq);

	if (service_to_charge > bfq_bfqq_budget_left(bfqq)) {
		/*
		 * This may happen if the next rq is chosen in fifo order
		 * instead of sector order. The budget is properly
		 * dimensioned to be always sufficient to serve the next
		 * request only if it is chosen in sector order. The reason
		 * is that it would be quite inefficient and little useful
		 * to always make sure that the budget is large enough to
		 * serve even the possible next rq in fifo order.
		 * In fact, requests are seldom served in fifo order.
		 *
		 * Expire the queue for budget exhaustion, and make sure
		 * that the next act_budget is enough to serve the next
		 * request, even if it comes from the fifo expired path.
		 */
		bfqq->next_rq = rq;
		/*
		 * Since this dispatch is failed, make sure that
		 * a new one will be performed
		 */
		if (!bfqd->rq_in_driver)
			bfq_schedule_dispatch(bfqd);
		goto expire;
	}

	/* Finally, insert request into driver dispatch list. */
	bfq_bfqq_served(bfqq, service_to_charge);
	bfq_dispatch_insert(bfqd->queue, rq);

	bfq_update_wr_data(bfqd, bfqq);

	bfq_log_bfqq(bfqd, bfqq,
			"dispatched %u sec req (%llu), budg left %lu",
			blk_rq_sectors(rq),
			(long long unsigned)blk_rq_pos(rq),
			bfq_bfqq_budget_left(bfqq));

	dispatched++;

	if (bfqd->in_service_bic == NULL) {
		atomic_long_inc(&RQ_BIC(rq)->icq.ioc->refcount);
		bfqd->in_service_bic = RQ_BIC(rq);
	}

	if (bfqd->busy_queues > 1 && ((!bfq_bfqq_sync(bfqq) &&
	    dispatched >= bfqd->bfq_max_budget_async_rq) ||
	    bfq_class_idle(bfqq)))
		goto expire;

	return dispatched;

expire:
	bfq_bfqq_expire(bfqd, bfqq, 0, BFQ_BFQQ_BUDGET_EXHAUSTED);
	return dispatched;
}

static int __bfq_forced_dispatch_bfqq(struct bfq_queue *bfqq)
{
	int dispatched = 0;

	while (bfqq->next_rq != NULL) {
		bfq_dispatch_insert(bfqq->bfqd->queue, bfqq->next_rq);
		dispatched++;
	}

	BUG_ON(!list_empty(&bfqq->fifo));
	return dispatched;
}

/*
 * Drain our current requests.
 * Used for barriers and when switching io schedulers on-the-fly.
 */
static int bfq_forced_dispatch(struct bfq_data *bfqd)
{
	struct bfq_queue *bfqq, *n;
	struct bfq_service_tree *st;
	int dispatched = 0;

	bfqq = bfqd->in_service_queue;
	if (bfqq != NULL)
		__bfq_bfqq_expire(bfqd, bfqq);

	/*
	 * Loop through classes, and be careful to leave the scheduler
	 * in a consistent state, as feedback mechanisms and vtime
	 * updates cannot be disabled during the process.
	 */
	list_for_each_entry_safe(bfqq, n, &bfqd->active_list, bfqq_list) {
		st = bfq_entity_service_tree(&bfqq->entity);

		dispatched += __bfq_forced_dispatch_bfqq(bfqq);
		bfqq->max_budget = bfq_max_budget(bfqd);

		bfq_forget_idle(st);
	}

	BUG_ON(bfqd->busy_queues != 0);

	return dispatched;
}

static int bfq_dispatch_requests(struct request_queue *q, int force)
{
	struct bfq_data *bfqd = q->elevator->elevator_data;
	struct bfq_queue *bfqq;
	int max_dispatch;

	bfq_log(bfqd, "dispatch requests: %d busy queues", bfqd->busy_queues);
	if (bfqd->busy_queues == 0)
		return 0;

	if (unlikely(force))
		return bfq_forced_dispatch(bfqd);

	bfqq = bfq_select_queue(bfqd);
	if (bfqq == NULL)
		return 0;

	if (bfq_class_idle(bfqq))
		max_dispatch = 1;

	if (!bfq_bfqq_sync(bfqq))
		max_dispatch = bfqd->bfq_max_budget_async_rq;

	if (!bfq_bfqq_sync(bfqq) && bfqq->dispatched >= max_dispatch) {
		if (bfqd->busy_queues > 1)
			return 0;
		if (bfqq->dispatched >= 4 * max_dispatch)
			return 0;
	}

	if (bfqd->sync_flight != 0 && !bfq_bfqq_sync(bfqq))
		return 0;

	bfq_clear_bfqq_wait_request(bfqq);
	BUG_ON(timer_pending(&bfqd->idle_slice_timer));

	if (!bfq_dispatch_request(bfqd, bfqq))
		return 0;

	bfq_log_bfqq(bfqd, bfqq, "dispatched %s request",
			bfq_bfqq_sync(bfqq) ? "sync" : "async");

	return 1;
}

/*
 * Task holds one reference to the queue, dropped when task exits.  Each rq
 * in-flight on this queue also holds a reference, dropped when rq is freed.
 *
 * Queue lock must be held here.
 */
static void bfq_put_queue(struct bfq_queue *bfqq)
{
	struct bfq_data *bfqd = bfqq->bfqd;

	BUG_ON(atomic_read(&bfqq->ref) <= 0);

	bfq_log_bfqq(bfqd, bfqq, "put_queue: %p %d", bfqq,
		     atomic_read(&bfqq->ref));
	if (!atomic_dec_and_test(&bfqq->ref))
		return;

	BUG_ON(rb_first(&bfqq->sort_list) != NULL);
	BUG_ON(bfqq->allocated[READ] + bfqq->allocated[WRITE] != 0);
	BUG_ON(bfqq->entity.tree != NULL);
	BUG_ON(bfq_bfqq_busy(bfqq));
	BUG_ON(bfqd->in_service_queue == bfqq);

	if (bfq_bfqq_sync(bfqq))
		/*
		 * The fact that this queue is being destroyed does not
		 * invalidate the fact that this queue may have been
		 * activated during the current burst. As a consequence,
		 * although the queue does not exist anymore, and hence
		 * needs to be removed from the burst list if there,
		 * the burst size has not to be decremented.
		 */
		hlist_del_init(&bfqq->burst_list_node);

	bfq_log_bfqq(bfqd, bfqq, "put_queue: %p freed", bfqq);

	kmem_cache_free(bfq_pool, bfqq);
}

static void bfq_put_cooperator(struct bfq_queue *bfqq)
{
	struct bfq_queue *__bfqq, *next;

	/*
	 * If this queue was scheduled to merge with another queue, be
	 * sure to drop the reference taken on that queue (and others in
	 * the merge chain). See bfq_setup_merge and bfq_merge_bfqqs.
	 */
	__bfqq = bfqq->new_bfqq;
	while (__bfqq) {
		if (__bfqq == bfqq)
			break;
		next = __bfqq->new_bfqq;
		bfq_put_queue(__bfqq);
		__bfqq = next;
	}
}

static void bfq_exit_bfqq(struct bfq_data *bfqd, struct bfq_queue *bfqq)
{
	if (bfqq == bfqd->in_service_queue) {
		__bfq_bfqq_expire(bfqd, bfqq);
		bfq_schedule_dispatch(bfqd);
	}

	bfq_log_bfqq(bfqd, bfqq, "exit_bfqq: %p, %d", bfqq,
		     atomic_read(&bfqq->ref));

	bfq_put_cooperator(bfqq);

	bfq_put_queue(bfqq);
}

static inline void bfq_init_icq(struct io_cq *icq)
{
	struct bfq_io_cq *bic = icq_to_bic(icq);

	bic->ttime.last_end_request = jiffies;
	/*
	 * A newly created bic indicates that the process has just
	 * started doing I/O, and is probably mapping into memory its
	 * executable and libraries: it definitely needs weight raising.
	 * There is however the possibility that the process performs,
	 * for a while, I/O close to some other process. EQM intercepts
	 * this behavior and may merge the queue corresponding to the
	 * process  with some other queue, BEFORE the weight of the queue
	 * is raised. Merged queues are not weight-raised (they are assumed
	 * to belong to processes that benefit only from high throughput).
	 * If the merge is basically the consequence of an accident, then
	 * the queue will be split soon and will get back its old weight.
	 * It is then important to write down somewhere that this queue
	 * does need weight raising, even if it did not make it to get its
	 * weight raised before being merged. To this purpose, we overload
	 * the field raising_time_left and assign 1 to it, to mark the queue
	 * as needing weight raising.
	 */
	bic->wr_time_left = 1;
}

static void bfq_exit_icq(struct io_cq *icq)
{
	struct bfq_io_cq *bic = icq_to_bic(icq);
	struct bfq_data *bfqd = bic_to_bfqd(bic);

	if (bic->bfqq[BLK_RW_ASYNC]) {
		bfq_exit_bfqq(bfqd, bic->bfqq[BLK_RW_ASYNC]);
		bic->bfqq[BLK_RW_ASYNC] = NULL;
	}

	if (bic->bfqq[BLK_RW_SYNC]) {
		/*
		 * If the bic is using a shared queue, put the reference
		 * taken on the io_context when the bic started using a
		 * shared bfq_queue.
		 */
		if (bfq_bfqq_coop(bic->bfqq[BLK_RW_SYNC]))
			put_io_context(icq->ioc);
		bfq_exit_bfqq(bfqd, bic->bfqq[BLK_RW_SYNC]);
		bic->bfqq[BLK_RW_SYNC] = NULL;
	}
}

/*
 * Update the entity prio values; note that the new values will not
 * be used until the next (re)activation.
 */
static void bfq_set_next_ioprio_data(struct bfq_queue *bfqq, struct bfq_io_cq *bic)
{
	struct task_struct *tsk = current;
	int ioprio_class;

	ioprio_class = IOPRIO_PRIO_CLASS(bic->ioprio);
	switch (ioprio_class) {
	default:
		dev_err(bfqq->bfqd->queue->backing_dev_info.dev,
			"bfq: bad prio class %d\n", ioprio_class);
	case IOPRIO_CLASS_NONE:
		/*
		 * No prio set, inherit CPU scheduling settings.
		 */
		bfqq->entity.new_ioprio = task_nice_ioprio(tsk);
		bfqq->entity.new_ioprio_class = task_nice_ioclass(tsk);
		break;
	case IOPRIO_CLASS_RT:
		bfqq->entity.new_ioprio = IOPRIO_PRIO_DATA(bic->ioprio);
		bfqq->entity.new_ioprio_class = IOPRIO_CLASS_RT;
		break;
	case IOPRIO_CLASS_BE:
		bfqq->entity.new_ioprio = IOPRIO_PRIO_DATA(bic->ioprio);
		bfqq->entity.new_ioprio_class = IOPRIO_CLASS_BE;
		break;
	case IOPRIO_CLASS_IDLE:
		bfqq->entity.new_ioprio_class = IOPRIO_CLASS_IDLE;
		bfqq->entity.new_ioprio = 7;
		bfq_clear_bfqq_idle_window(bfqq);
		break;
	}

	if (bfqq->entity.new_ioprio < 0 ||
	    bfqq->entity.new_ioprio >= IOPRIO_BE_NR) {
		printk(KERN_CRIT "bfq_set_next_ioprio_data: new_ioprio %d\n",
				 bfqq->entity.new_ioprio);
		BUG();
	}

	bfqq->entity.new_weight = bfq_ioprio_to_weight(bfqq->entity.new_ioprio);
	bfqq->entity.ioprio_changed = 1;
}

static void bfq_check_ioprio_change(struct bfq_io_cq *bic)
{
	struct bfq_data *bfqd;
	struct bfq_queue *bfqq, *new_bfqq;
	struct bfq_group *bfqg;
	unsigned long uninitialized_var(flags);
	int ioprio = bic->icq.ioc->ioprio;

	bfqd = bfq_get_bfqd_locked(&(bic->icq.q->elevator->elevator_data),
				   &flags);
	/*
	 * This condition may trigger on a newly created bic, be sure to
	 * drop the lock before returning.
	 */
	if (unlikely(bfqd == NULL) || likely(bic->ioprio == ioprio))
		goto out;

	bic->ioprio = ioprio;

	bfqq = bic->bfqq[BLK_RW_ASYNC];
	if (bfqq != NULL) {
		bfqg = container_of(bfqq->entity.sched_data, struct bfq_group,
				    sched_data);
		new_bfqq = bfq_get_queue(bfqd, bfqg, BLK_RW_ASYNC, bic,
					 GFP_ATOMIC);
		if (new_bfqq != NULL) {
			bic->bfqq[BLK_RW_ASYNC] = new_bfqq;
			bfq_log_bfqq(bfqd, bfqq,
				     "check_ioprio_change: bfqq %p %d",
				     bfqq, atomic_read(&bfqq->ref));
			bfq_put_queue(bfqq);
		}
	}

	bfqq = bic->bfqq[BLK_RW_SYNC];
	if (bfqq != NULL)
		bfq_set_next_ioprio_data(bfqq, bic);

out:
	bfq_put_bfqd_unlock(bfqd, &flags);
}

static void bfq_init_bfqq(struct bfq_data *bfqd, struct bfq_queue *bfqq,
			  struct bfq_io_cq *bic, pid_t pid, int is_sync)
{
	RB_CLEAR_NODE(&bfqq->entity.rb_node);
	INIT_LIST_HEAD(&bfqq->fifo);
	INIT_HLIST_NODE(&bfqq->burst_list_node);

	atomic_set(&bfqq->ref, 0);
	bfqq->bfqd = bfqd;

	if (bic)
		bfq_set_next_ioprio_data(bfqq, bic);

	if (is_sync) {
		if (!bfq_class_idle(bfqq))
			bfq_mark_bfqq_idle_window(bfqq);
		bfq_mark_bfqq_sync(bfqq);
	}
	bfq_mark_bfqq_IO_bound(bfqq);

	/* Tentative initial value to trade off between thr and lat */
	bfqq->max_budget = (2 * bfq_max_budget(bfqd)) / 3;
	bfqq->pid = pid;

	bfqq->wr_coeff = 1;
	bfqq->last_wr_start_finish = 0;
	/*
	 * Set to the value for which bfqq will not be deemed as
	 * soft rt when it becomes backlogged.
	 */
	bfqq->soft_rt_next_start = bfq_infinity_from_now(jiffies);
}

static struct bfq_queue *bfq_find_alloc_queue(struct bfq_data *bfqd,
					      struct bfq_group *bfqg,
					      int is_sync,
					      struct bfq_io_cq *bic,
					      gfp_t gfp_mask)
{
	struct bfq_queue *bfqq, *new_bfqq = NULL;

retry:
	/* bic always exists here */
	bfqq = bic_to_bfqq(bic, is_sync);

	/*
	 * Always try a new alloc if we fall back to the OOM bfqq
	 * originally, since it should just be a temporary situation.
	 */
	if (bfqq == NULL || bfqq == &bfqd->oom_bfqq) {
		bfqq = NULL;
		if (new_bfqq != NULL) {
			bfqq = new_bfqq;
			new_bfqq = NULL;
		} else if (gfp_mask & __GFP_WAIT) {
			spin_unlock_irq(bfqd->queue->queue_lock);
			new_bfqq = kmem_cache_alloc_node(bfq_pool,
					gfp_mask | __GFP_ZERO,
					bfqd->queue->node);
			spin_lock_irq(bfqd->queue->queue_lock);
			if (new_bfqq != NULL)
				goto retry;
		} else {
			bfqq = kmem_cache_alloc_node(bfq_pool,
					gfp_mask | __GFP_ZERO,
					bfqd->queue->node);
		}

		if (bfqq != NULL) {
			bfq_init_bfqq(bfqd, bfqq, bic, current->pid,
                                      is_sync);
			bfq_init_entity(&bfqq->entity, bfqg);
			bfq_log_bfqq(bfqd, bfqq, "allocated");
		} else {
			bfqq = &bfqd->oom_bfqq;
			bfq_log_bfqq(bfqd, bfqq, "using oom bfqq");
		}
	}

	if (new_bfqq != NULL)
		kmem_cache_free(bfq_pool, new_bfqq);

	return bfqq;
}

static struct bfq_queue **bfq_async_queue_prio(struct bfq_data *bfqd,
					       struct bfq_group *bfqg,
					       int ioprio_class, int ioprio)
{
	switch (ioprio_class) {
	case IOPRIO_CLASS_RT:
		return &bfqg->async_bfqq[0][ioprio];
	case IOPRIO_CLASS_NONE:
		ioprio = IOPRIO_NORM;
		/* fall through */
	case IOPRIO_CLASS_BE:
		return &bfqg->async_bfqq[1][ioprio];
	case IOPRIO_CLASS_IDLE:
		return &bfqg->async_idle_bfqq;
	default:
		BUG();
	}
}

static struct bfq_queue *bfq_get_queue(struct bfq_data *bfqd,
				       struct bfq_group *bfqg, int is_sync,
				       struct bfq_io_cq *bic, gfp_t gfp_mask)
{
	const int ioprio = IOPRIO_PRIO_DATA(bic->ioprio);
	const int ioprio_class = IOPRIO_PRIO_CLASS(bic->ioprio);
	struct bfq_queue **async_bfqq = NULL;
	struct bfq_queue *bfqq = NULL;

	if (!is_sync) {
		async_bfqq = bfq_async_queue_prio(bfqd, bfqg, ioprio_class,
						  ioprio);
		bfqq = *async_bfqq;
	}

	if (bfqq == NULL)
		bfqq = bfq_find_alloc_queue(bfqd, bfqg, is_sync, bic, gfp_mask);

	/*
	 * Pin the queue now that it's allocated, scheduler exit will
	 * prune it.
	 */
	if (!is_sync && *async_bfqq == NULL) {
		atomic_inc(&bfqq->ref);
		bfq_log_bfqq(bfqd, bfqq, "get_queue, bfqq not in async: %p, %d",
			     bfqq, atomic_read(&bfqq->ref));
		*async_bfqq = bfqq;
	}

	atomic_inc(&bfqq->ref);
	bfq_log_bfqq(bfqd, bfqq, "get_queue, at end: %p, %d", bfqq,
		     atomic_read(&bfqq->ref));
	return bfqq;
}

static void bfq_update_io_thinktime(struct bfq_data *bfqd,
				    struct bfq_io_cq *bic)
{
	unsigned long elapsed = jiffies - bic->ttime.last_end_request;
	unsigned long ttime = min(elapsed, 2UL * bfqd->bfq_slice_idle);

	bic->ttime.ttime_samples = (7*bic->ttime.ttime_samples + 256) / 8;
	bic->ttime.ttime_total = (7*bic->ttime.ttime_total + 256*ttime) / 8;
	bic->ttime.ttime_mean = (bic->ttime.ttime_total + 128) /
				bic->ttime.ttime_samples;
}

static void bfq_update_io_seektime(struct bfq_data *bfqd,
				   struct bfq_queue *bfqq,
				   struct request *rq)
{
	sector_t sdist;
	u64 total;

	if (bfqq->last_request_pos < blk_rq_pos(rq))
		sdist = blk_rq_pos(rq) - bfqq->last_request_pos;
	else
		sdist = bfqq->last_request_pos - blk_rq_pos(rq);

	/*
	 * Don't allow the seek distance to get too large from the
	 * odd fragment, pagein, etc.
	 */
	if (bfqq->seek_samples == 0) /* first request, not really a seek */
		sdist = 0;
	else if (bfqq->seek_samples <= 60) /* second & third seek */
		sdist = min(sdist, (bfqq->seek_mean * 4) + 2*1024*1024);
	else
		sdist = min(sdist, (bfqq->seek_mean * 4) + 2*1024*64);

	bfqq->seek_samples = (7*bfqq->seek_samples + 256) / 8;
	bfqq->seek_total = (7*bfqq->seek_total + (u64)256*sdist) / 8;
	total = bfqq->seek_total + (bfqq->seek_samples/2);
	do_div(total, bfqq->seek_samples);
	bfqq->seek_mean = (sector_t)total;

	bfq_log_bfqq(bfqd, bfqq, "dist=%llu mean=%llu", (u64)sdist,
			(u64)bfqq->seek_mean);
}

/*
 * Disable idle window if the process thinks too long or seeks so much that
 * it doesn't matter.
 */
static void bfq_update_idle_window(struct bfq_data *bfqd,
				   struct bfq_queue *bfqq,
				   struct bfq_io_cq *bic)
{
	int enable_idle;

	/* Don't idle for async or idle io prio class. */
	if (!bfq_bfqq_sync(bfqq) || bfq_class_idle(bfqq))
		return;

	/* Idle window just restored, statistics are meaningless. */
	if (bfq_bfqq_just_split(bfqq))
		return;

	enable_idle = bfq_bfqq_idle_window(bfqq);

	if (atomic_read(&bic->icq.ioc->active_ref) == 0 ||
	    bfqd->bfq_slice_idle == 0 ||
		(bfqd->hw_tag && BFQQ_SEEKY(bfqq) &&
			bfqq->wr_coeff == 1))
		enable_idle = 0;
	else if (bfq_sample_valid(bic->ttime.ttime_samples)) {
		if (bic->ttime.ttime_mean > bfqd->bfq_slice_idle &&
			bfqq->wr_coeff == 1)
			enable_idle = 0;
		else
			enable_idle = 1;
	}
	bfq_log_bfqq(bfqd, bfqq, "update_idle_window: enable_idle %d",
		enable_idle);

	if (enable_idle)
		bfq_mark_bfqq_idle_window(bfqq);
	else
		bfq_clear_bfqq_idle_window(bfqq);
}

/*
 * Called when a new fs request (rq) is added to bfqq.  Check if there's
 * something we should do about it.
 */
static void bfq_rq_enqueued(struct bfq_data *bfqd, struct bfq_queue *bfqq,
			    struct request *rq)
{
	struct bfq_io_cq *bic = RQ_BIC(rq);

	if (rq->cmd_flags & REQ_META)
		bfqq->meta_pending++;

	bfq_update_io_thinktime(bfqd, bic);
	bfq_update_io_seektime(bfqd, bfqq, rq);
	if (!BFQQ_SEEKY(bfqq) && bfq_bfqq_constantly_seeky(bfqq)) {
		bfq_clear_bfqq_constantly_seeky(bfqq);
		if (!blk_queue_nonrot(bfqd->queue)) {
			BUG_ON(!bfqd->const_seeky_busy_in_flight_queues);
			bfqd->const_seeky_busy_in_flight_queues--;
		}
	}
	if (bfqq->entity.service > bfq_max_budget(bfqd) / 8 ||
	    !BFQQ_SEEKY(bfqq))
		bfq_update_idle_window(bfqd, bfqq, bic);
	bfq_clear_bfqq_just_split(bfqq);

	bfq_log_bfqq(bfqd, bfqq,
		     "rq_enqueued: idle_window=%d (seeky %d, mean %llu)",
		     bfq_bfqq_idle_window(bfqq), BFQQ_SEEKY(bfqq),
		     (long long unsigned)bfqq->seek_mean);

	bfqq->last_request_pos = blk_rq_pos(rq) + blk_rq_sectors(rq);

	if (bfqq == bfqd->in_service_queue && bfq_bfqq_wait_request(bfqq)) {
		int small_req = bfqq->queued[rq_is_sync(rq)] == 1 &&
				blk_rq_sectors(rq) < 32;
		int budget_timeout = bfq_bfqq_budget_timeout(bfqq);

		/*
		 * There is just this request queued: if the request
		 * is small and the queue is not to be expired, then
		 * just exit.
		 *
		 * In this way, if the disk is being idled to wait for
		 * a new request from the in-service queue, we avoid
		 * unplugging the device and committing the disk to serve
		 * just a small request. On the contrary, we wait for
		 * the block layer to decide when to unplug the device:
		 * hopefully, new requests will be merged to this one
		 * quickly, then the device will be unplugged and
		 * larger requests will be dispatched.
		 */
		if (small_req && !budget_timeout)
			return;

		/*
		 * A large enough request arrived, or the queue is to
		 * be expired: in both cases disk idling is to be
		 * stopped, so clear wait_request flag and reset
		 * timer.
		 */
		bfq_clear_bfqq_wait_request(bfqq);
		del_timer(&bfqd->idle_slice_timer);

		/*
		 * The queue is not empty, because a new request just
		 * arrived. Hence we can safely expire the queue, in
		 * case of budget timeout, without risking that the
		 * timestamps of the queue are not updated correctly.
		 * See [1] for more details.
		 */
		if (budget_timeout)
			bfq_bfqq_expire(bfqd, bfqq, 0, BFQ_BFQQ_BUDGET_TIMEOUT);

		/*
		 * Let the request rip immediately, or let a new queue be
		 * selected if bfqq has just been expired.
		 */
		__blk_run_queue(bfqd->queue);
	}
}

static void bfq_insert_request(struct request_queue *q, struct request *rq)
{
	struct bfq_data *bfqd = q->elevator->elevator_data;
	struct bfq_queue *bfqq = RQ_BFQQ(rq), *new_bfqq;

	assert_spin_locked(bfqd->queue->queue_lock);

	/*
	 * An unplug may trigger a requeue of a request from the device
	 * driver: make sure we are in process context while trying to
	 * merge two bfq_queues.
	 */
	if (!in_interrupt()) {
		new_bfqq = bfq_setup_cooperator(bfqd, bfqq, rq, true);
		if (new_bfqq != NULL) {
			if (bic_to_bfqq(RQ_BIC(rq), 1) != bfqq)
				new_bfqq = bic_to_bfqq(RQ_BIC(rq), 1);
			/*
			 * Release the request's reference to the old bfqq
			 * and make sure one is taken to the shared queue.
			 */
			new_bfqq->allocated[rq_data_dir(rq)]++;
			bfqq->allocated[rq_data_dir(rq)]--;
			atomic_inc(&new_bfqq->ref);
			bfq_put_queue(bfqq);
			if (bic_to_bfqq(RQ_BIC(rq), 1) == bfqq)
				bfq_merge_bfqqs(bfqd, RQ_BIC(rq),
						bfqq, new_bfqq);
			rq->elv.priv[1] = new_bfqq;
			bfqq = new_bfqq;
		} else
			bfq_bfqq_increase_failed_cooperations(bfqq);
	}

	bfq_add_request(rq);

	/*
	 * Here a newly-created bfq_queue has already started a weight-raising
	 * period: clear raising_time_left to prevent bfq_bfqq_save_state()
	 * from assigning it a full weight-raising period. See the detailed
	 * comments about this field in bfq_init_icq().
	 */
	if (bfqq->bic != NULL)
		bfqq->bic->wr_time_left = 0;
	rq_set_fifo_time(rq, jiffies + bfqd->bfq_fifo_expire[rq_is_sync(rq)]);
	list_add_tail(&rq->queuelist, &bfqq->fifo);

	bfq_rq_enqueued(bfqd, bfqq, rq);
}

static void bfq_update_hw_tag(struct bfq_data *bfqd)
{
	bfqd->max_rq_in_driver = max(bfqd->max_rq_in_driver,
				     bfqd->rq_in_driver);

	if (bfqd->hw_tag == 1)
		return;

	/*
	 * This sample is valid if the number of outstanding requests
	 * is large enough to allow a queueing behavior.  Note that the
	 * sum is not exact, as it's not taking into account deactivated
	 * requests.
	 */
	if (bfqd->rq_in_driver + bfqd->queued < BFQ_HW_QUEUE_THRESHOLD)
		return;

	if (bfqd->hw_tag_samples++ < BFQ_HW_QUEUE_SAMPLES)
		return;

	bfqd->hw_tag = bfqd->max_rq_in_driver > BFQ_HW_QUEUE_THRESHOLD;
	bfqd->max_rq_in_driver = 0;
	bfqd->hw_tag_samples = 0;
}

static void bfq_completed_request(struct request_queue *q, struct request *rq)
{
	struct bfq_queue *bfqq = RQ_BFQQ(rq);
	struct bfq_data *bfqd = bfqq->bfqd;
	bool sync = bfq_bfqq_sync(bfqq);

	bfq_log_bfqq(bfqd, bfqq, "completed one req with %u sects left (%d)",
		     blk_rq_sectors(rq), sync);

	bfq_update_hw_tag(bfqd);

	BUG_ON(!bfqd->rq_in_driver);
	BUG_ON(!bfqq->dispatched);
	bfqd->rq_in_driver--;
	bfqq->dispatched--;

	if (!bfqq->dispatched && !bfq_bfqq_busy(bfqq)) {
		bfq_weights_tree_remove(bfqd, &bfqq->entity,
					&bfqd->queue_weights_tree);
		if (!blk_queue_nonrot(bfqd->queue)) {
			BUG_ON(!bfqd->busy_in_flight_queues);
			bfqd->busy_in_flight_queues--;
			if (bfq_bfqq_constantly_seeky(bfqq)) {
				BUG_ON(!bfqd->
					const_seeky_busy_in_flight_queues);
				bfqd->const_seeky_busy_in_flight_queues--;
			}
		}
	}

	if (sync) {
		bfqd->sync_flight--;
		RQ_BIC(rq)->ttime.last_end_request = jiffies;
	}

	/*
	 * If we are waiting to discover whether the request pattern of the
	 * task associated with the queue is actually isochronous, and
	 * both requisites for this condition to hold are satisfied, then
	 * compute soft_rt_next_start (see the comments to the function
	 * bfq_bfqq_softrt_next_start()).
	 */
	if (bfq_bfqq_softrt_update(bfqq) && bfqq->dispatched == 0 &&
	    RB_EMPTY_ROOT(&bfqq->sort_list))
		bfqq->soft_rt_next_start =
			bfq_bfqq_softrt_next_start(bfqd, bfqq);

	/*
	 * If this is the in-service queue, check if it needs to be expired,
	 * or if we want to idle in case it has no pending requests.
	 */
	if (bfqd->in_service_queue == bfqq) {
		if (bfq_bfqq_budget_new(bfqq))
			bfq_set_budget_timeout(bfqd);

		if (bfq_bfqq_must_idle(bfqq)) {
			bfq_arm_slice_timer(bfqd);
			goto out;
		} else if (bfq_may_expire_for_budg_timeout(bfqq))
			bfq_bfqq_expire(bfqd, bfqq, 0, BFQ_BFQQ_BUDGET_TIMEOUT);
		else if (RB_EMPTY_ROOT(&bfqq->sort_list) &&
			 (bfqq->dispatched == 0 ||
			  !bfq_bfqq_must_not_expire(bfqq)))
			bfq_bfqq_expire(bfqd, bfqq, 0,
					BFQ_BFQQ_NO_MORE_REQUESTS);
	}

	if (!bfqd->rq_in_driver)
		bfq_schedule_dispatch(bfqd);

out:
	return;
}

static inline int __bfq_may_queue(struct bfq_queue *bfqq)
{
	if (bfq_bfqq_wait_request(bfqq) && bfq_bfqq_must_alloc(bfqq)) {
		bfq_clear_bfqq_must_alloc(bfqq);
		return ELV_MQUEUE_MUST;
	}

	return ELV_MQUEUE_MAY;
}

static int bfq_may_queue(struct request_queue *q, int rw)
{
	struct bfq_data *bfqd = q->elevator->elevator_data;
	struct task_struct *tsk = current;
	struct bfq_io_cq *bic;
	struct bfq_queue *bfqq;

	/*
	 * Don't force setup of a queue from here, as a call to may_queue
	 * does not necessarily imply that a request actually will be
	 * queued. So just lookup a possibly existing queue, or return
	 * 'may queue' if that fails.
	 */
	bic = bfq_bic_lookup(bfqd, tsk->io_context);
	if (bic == NULL)
		return ELV_MQUEUE_MAY;

	bfqq = bic_to_bfqq(bic, rw_is_sync(rw));
	if (bfqq != NULL)
		return __bfq_may_queue(bfqq);

	return ELV_MQUEUE_MAY;
}

/*
 * Queue lock held here.
 */
static void bfq_put_request(struct request *rq)
{
	struct bfq_queue *bfqq = RQ_BFQQ(rq);

	if (bfqq != NULL) {
		const int rw = rq_data_dir(rq);

		BUG_ON(!bfqq->allocated[rw]);
		bfqq->allocated[rw]--;

		rq->elv.priv[0] = NULL;
		rq->elv.priv[1] = NULL;

		bfq_log_bfqq(bfqq->bfqd, bfqq, "put_request %p, %d",
			     bfqq, atomic_read(&bfqq->ref));
		bfq_put_queue(bfqq);
	}
}

/*
 * Returns NULL if a new bfqq should be allocated, or the old bfqq if this
 * was the last process referring to said bfqq.
 */
static struct bfq_queue *
bfq_split_bfqq(struct bfq_io_cq *bic, struct bfq_queue *bfqq)
{
	bfq_log_bfqq(bfqq->bfqd, bfqq, "splitting queue");

	put_io_context(bic->icq.ioc);

	if (bfqq_process_refs(bfqq) == 1) {
		bfqq->pid = current->pid;
		bfq_clear_bfqq_coop(bfqq);
		bfq_clear_bfqq_split_coop(bfqq);
		return bfqq;
	}

	bic_set_bfqq(bic, NULL, 1);

	bfq_put_cooperator(bfqq);

	bfq_put_queue(bfqq);
	return NULL;
}

/*
 * Allocate bfq data structures associated with this request.
 */
static int bfq_set_request(struct request_queue *q, struct request *rq,
			   struct bio *bio, gfp_t gfp_mask)
{
	struct bfq_data *bfqd = q->elevator->elevator_data;
	struct bfq_io_cq *bic = icq_to_bic(rq->elv.icq);
	const int rw = rq_data_dir(rq);
	const int is_sync = rq_is_sync(rq);
	struct bfq_queue *bfqq;
	struct bfq_group *bfqg;
	unsigned long flags;
	bool split = false;

	might_sleep_if(gfp_mask & __GFP_WAIT);

	bfq_check_ioprio_change(bic);

	spin_lock_irqsave(q->queue_lock, flags);

	if (bic == NULL)
		goto queue_fail;

	bfqg = bfq_bic_update_cgroup(bic);

new_queue:
	bfqq = bic_to_bfqq(bic, is_sync);
	if (bfqq == NULL || bfqq == &bfqd->oom_bfqq) {
		bfqq = bfq_get_queue(bfqd, bfqg, is_sync, bic, gfp_mask);
		bic_set_bfqq(bic, bfqq, is_sync);
		if (split && is_sync) {
			if ((bic->was_in_burst_list && bfqd->large_burst) ||
			    bic->saved_in_large_burst)
				bfq_mark_bfqq_in_large_burst(bfqq);
			else {
			    bfq_clear_bfqq_in_large_burst(bfqq);
			    if (bic->was_in_burst_list)
			       hlist_add_head(&bfqq->burst_list_node,
				              &bfqd->burst_list);
			}
		}
	} else {
		/* If the queue was seeky for too long, break it apart. */
		if (bfq_bfqq_coop(bfqq) && bfq_bfqq_split_coop(bfqq)) {
			bfq_log_bfqq(bfqd, bfqq, "breaking apart bfqq");
			bfqq = bfq_split_bfqq(bic, bfqq);
			split = true;
			if (!bfqq)
				goto new_queue;
		}
	}

	bfqq->allocated[rw]++;
	atomic_inc(&bfqq->ref);
	bfq_log_bfqq(bfqd, bfqq, "set_request: bfqq %p, %d", bfqq,
		     atomic_read(&bfqq->ref));

	rq->elv.priv[0] = bic;
	rq->elv.priv[1] = bfqq;

	/*
	 * If a bfq_queue has only one process reference, it is owned
	 * by only one bfq_io_cq: we can set the bic field of the
	 * bfq_queue to the address of that structure. Also, if the
	 * queue has just been split, mark a flag so that the
	 * information is available to the other scheduler hooks.
	 */
	if (likely(bfqq != &bfqd->oom_bfqq) && bfqq_process_refs(bfqq) == 1) {
		bfqq->bic = bic;
		if (split) {
			bfq_mark_bfqq_just_split(bfqq);
			/*
			 * If the queue has just been split from a shared
			 * queue, restore the idle window and the possible
			 * weight raising period.
			 */
			bfq_bfqq_resume_state(bfqq, bic);
		}
	}

	spin_unlock_irqrestore(q->queue_lock, flags);

	return 0;

queue_fail:
	bfq_schedule_dispatch(bfqd);
	spin_unlock_irqrestore(q->queue_lock, flags);

	return 1;
}

static void bfq_kick_queue(struct work_struct *work)
{
	struct bfq_data *bfqd =
		container_of(work, struct bfq_data, unplug_work);
	struct request_queue *q = bfqd->queue;

	spin_lock_irq(q->queue_lock);
	__blk_run_queue(q);
	spin_unlock_irq(q->queue_lock);
}

/*
 * Handler of the expiration of the timer running if the in-service queue
 * is idling inside its time slice.
 */
static void bfq_idle_slice_timer(unsigned long data)
{
	struct bfq_data *bfqd = (struct bfq_data *)data;
	struct bfq_queue *bfqq;
	unsigned long flags;
	enum bfqq_expiration reason;

	spin_lock_irqsave(bfqd->queue->queue_lock, flags);

	bfqq = bfqd->in_service_queue;
	/*
	 * Theoretical race here: the in-service queue can be NULL or
	 * different from the queue that was idling if the timer handler
	 * spins on the queue_lock and a new request arrives for the
	 * current queue and there is a full dispatch cycle that changes
	 * the in-service queue.  This can hardly happen, but in the worst
	 * case we just expire a queue too early.
	 */
	if (bfqq != NULL) {
		bfq_log_bfqq(bfqd, bfqq, "slice_timer expired");
		if (bfq_bfqq_budget_timeout(bfqq))
			/*
			 * Also here the queue can be safely expired
			 * for budget timeout without wasting
			 * guarantees
			 */
			reason = BFQ_BFQQ_BUDGET_TIMEOUT;
		else if (bfqq->queued[0] == 0 && bfqq->queued[1] == 0)
			/*
			 * The queue may not be empty upon timer expiration,
			 * because we may not disable the timer when the
			 * first request of the in-service queue arrives
			 * during disk idling.
			 */
			reason = BFQ_BFQQ_TOO_IDLE;
		else
			goto schedule_dispatch;

		bfq_bfqq_expire(bfqd, bfqq, 1, reason);
	}

schedule_dispatch:
	bfq_schedule_dispatch(bfqd);

	spin_unlock_irqrestore(bfqd->queue->queue_lock, flags);
}

static void bfq_shutdown_timer_wq(struct bfq_data *bfqd)
{
	del_timer_sync(&bfqd->idle_slice_timer);
	cancel_work_sync(&bfqd->unplug_work);
}

static inline void __bfq_put_async_bfqq(struct bfq_data *bfqd,
					struct bfq_queue **bfqq_ptr)
{
	struct bfq_group *root_group = bfqd->root_group;
	struct bfq_queue *bfqq = *bfqq_ptr;

	bfq_log(bfqd, "put_async_bfqq: %p", bfqq);
	if (bfqq != NULL) {
		bfq_bfqq_move(bfqd, bfqq, &bfqq->entity, root_group);
		bfq_log_bfqq(bfqd, bfqq, "put_async_bfqq: putting %p, %d",
			     bfqq, atomic_read(&bfqq->ref));
		bfq_put_queue(bfqq);
		*bfqq_ptr = NULL;
	}
}

/*
 * Release all the bfqg references to its async queues.  If we are
 * deallocating the group these queues may still contain requests, so
 * we reparent them to the root cgroup (i.e., the only one that will
 * exist for sure until all the requests on a device are gone).
 */
static void bfq_put_async_queues(struct bfq_data *bfqd, struct bfq_group *bfqg)
{
	int i, j;

	for (i = 0; i < 2; i++)
		for (j = 0; j < IOPRIO_BE_NR; j++)
			__bfq_put_async_bfqq(bfqd, &bfqg->async_bfqq[i][j]);

	__bfq_put_async_bfqq(bfqd, &bfqg->async_idle_bfqq);
}

static void bfq_exit_queue(struct elevator_queue *e)
{
	struct bfq_data *bfqd = e->elevator_data;
	struct request_queue *q = bfqd->queue;
	struct bfq_queue *bfqq, *n;

	bfq_shutdown_timer_wq(bfqd);

	spin_lock_irq(q->queue_lock);

	BUG_ON(bfqd->in_service_queue != NULL);
	list_for_each_entry_safe(bfqq, n, &bfqd->idle_list, bfqq_list)
		bfq_deactivate_bfqq(bfqd, bfqq, 0);

	bfq_disconnect_groups(bfqd);
	spin_unlock_irq(q->queue_lock);

	bfq_shutdown_timer_wq(bfqd);

	synchronize_rcu();

	BUG_ON(timer_pending(&bfqd->idle_slice_timer));

	bfq_free_root_group(bfqd);
	kfree(bfqd);
}

static int bfq_init_queue(struct request_queue *q, struct elevator_type *e)
{
	struct bfq_group *bfqg;
	struct bfq_data *bfqd;
	struct elevator_queue *eq;

	eq = elevator_alloc(q, e);
	if (eq == NULL)
		return -ENOMEM;

	bfqd = kzalloc_node(sizeof(*bfqd), GFP_KERNEL, q->node);
	if (bfqd == NULL) {
		kobject_put(&eq->kobj);
		return -ENOMEM;
	}
	eq->elevator_data = bfqd;

	/*
	 * Our fallback bfqq if bfq_find_alloc_queue() runs into OOM issues.
	 * Grab a permanent reference to it, so that the normal code flow
	 * will not attempt to free it.
	 */
	bfq_init_bfqq(bfqd, &bfqd->oom_bfqq, NULL, 1, 0);
	atomic_inc(&bfqd->oom_bfqq.ref);
	bfqd->oom_bfqq.entity.new_ioprio = BFQ_DEFAULT_QUEUE_IOPRIO;
	bfqd->oom_bfqq.entity.new_ioprio_class = IOPRIO_CLASS_BE;
	bfqd->oom_bfqq.entity.new_weight =
		bfq_ioprio_to_weight(bfqd->oom_bfqq.entity.new_ioprio);
	/*
	 * Trigger weight initialization, according to ioprio, at the
	 * oom_bfqq's first activation. The oom_bfqq's ioprio and ioprio
	 * class won't be changed any more.
	 */
	bfqd->oom_bfqq.entity.ioprio_changed = 1;

	bfqd->queue = q;

	spin_lock_irq(q->queue_lock);
	q->elevator = eq;
	spin_unlock_irq(q->queue_lock);

	bfqg = bfq_alloc_root_group(bfqd, q->node);
	if (bfqg == NULL) {
		kfree(bfqd);
		kobject_put(&eq->kobj);
		return -ENOMEM;
	}

	bfqd->root_group = bfqg;
	bfq_init_entity(&bfqd->oom_bfqq.entity, bfqd->root_group);
#ifdef CONFIG_CGROUP_BFQIO
	bfqd->active_numerous_groups = 0;
#endif

	init_timer(&bfqd->idle_slice_timer);
	bfqd->idle_slice_timer.function = bfq_idle_slice_timer;
	bfqd->idle_slice_timer.data = (unsigned long)bfqd;

	bfqd->rq_pos_tree = RB_ROOT;
	bfqd->queue_weights_tree = RB_ROOT;
	bfqd->group_weights_tree = RB_ROOT;

	INIT_WORK(&bfqd->unplug_work, bfq_kick_queue);

	INIT_LIST_HEAD(&bfqd->active_list);
	INIT_LIST_HEAD(&bfqd->idle_list);
	INIT_HLIST_HEAD(&bfqd->burst_list);

	bfqd->hw_tag = -1;

	bfqd->bfq_max_budget = bfq_default_max_budget;

	bfqd->bfq_fifo_expire[0] = bfq_fifo_expire[0];
	bfqd->bfq_fifo_expire[1] = bfq_fifo_expire[1];
	bfqd->bfq_back_max = bfq_back_max;
	bfqd->bfq_back_penalty = bfq_back_penalty;
	bfqd->bfq_slice_idle = bfq_slice_idle;
	bfqd->bfq_class_idle_last_service = 0;
	bfqd->bfq_max_budget_async_rq = bfq_max_budget_async_rq;
	bfqd->bfq_timeout[BLK_RW_ASYNC] = bfq_timeout_async;
	bfqd->bfq_timeout[BLK_RW_SYNC] = bfq_timeout_sync;

	bfqd->bfq_coop_thresh = 2;
	bfqd->bfq_failed_cooperations = 7000;
	bfqd->bfq_requests_within_timer = 120;

	bfqd->bfq_large_burst_thresh = 11;
	bfqd->bfq_burst_interval = msecs_to_jiffies(500);

	bfqd->low_latency = true;

	bfqd->bfq_wr_coeff = 20;
	bfqd->bfq_wr_rt_max_time = msecs_to_jiffies(300);
	bfqd->bfq_wr_max_time = 0;
	bfqd->bfq_wr_min_idle_time = msecs_to_jiffies(2000);
	bfqd->bfq_wr_min_inter_arr_async = msecs_to_jiffies(500);
	bfqd->bfq_wr_max_softrt_rate = 7000; /*
					      * Approximate rate required
					      * to playback or record a
					      * high-definition compressed
					      * video.
					      */
	bfqd->wr_busy_queues = 0;
	bfqd->busy_in_flight_queues = 0;
	bfqd->const_seeky_busy_in_flight_queues = 0;

	/*
	 * Begin by assuming, optimistically, that the device peak rate is
	 * equal to the highest reference rate.
	 */
	bfqd->RT_prod = R_fast[blk_queue_nonrot(bfqd->queue)] *
			T_fast[blk_queue_nonrot(bfqd->queue)];
	bfqd->peak_rate = R_fast[blk_queue_nonrot(bfqd->queue)];
	bfqd->device_speed = BFQ_BFQD_FAST;

	return 0;
}

static void bfq_slab_kill(void)
{
	if (bfq_pool != NULL)
		kmem_cache_destroy(bfq_pool);
}

static int __init bfq_slab_setup(void)
{
	bfq_pool = KMEM_CACHE(bfq_queue, 0);
	if (bfq_pool == NULL)
		return -ENOMEM;
	return 0;
}

static ssize_t bfq_var_show(unsigned int var, char *page)
{
	return sprintf(page, "%d\n", var);
}

static ssize_t bfq_var_store(unsigned long *var, const char *page,
			     size_t count)
{
	unsigned long new_val;
	int ret = kstrtoul(page, 10, &new_val);

	if (ret == 0)
		*var = new_val;

	return count;
}

static ssize_t bfq_wr_max_time_show(struct elevator_queue *e, char *page)
{
	struct bfq_data *bfqd = e->elevator_data;
	return sprintf(page, "%d\n", bfqd->bfq_wr_max_time > 0 ?
		       jiffies_to_msecs(bfqd->bfq_wr_max_time) :
		       jiffies_to_msecs(bfq_wr_duration(bfqd)));
}

static ssize_t bfq_weights_show(struct elevator_queue *e, char *page)
{
	struct bfq_queue *bfqq;
	struct bfq_data *bfqd = e->elevator_data;
	ssize_t num_char = 0;

	num_char += sprintf(page + num_char, "Tot reqs queued %d\n\n",
			    bfqd->queued);

	spin_lock_irq(bfqd->queue->queue_lock);

	num_char += sprintf(page + num_char, "Active:\n");
	list_for_each_entry(bfqq, &bfqd->active_list, bfqq_list) {
	  num_char += sprintf(page + num_char,
			      "pid%d: weight %hu, nr_queued %d %d, dur %d/%u\n",
			      bfqq->pid,
			      bfqq->entity.weight,
			      bfqq->queued[0],
			      bfqq->queued[1],
			jiffies_to_msecs(jiffies - bfqq->last_wr_start_finish),
			jiffies_to_msecs(bfqq->wr_cur_max_time));
	}

	num_char += sprintf(page + num_char, "Idle:\n");
	list_for_each_entry(bfqq, &bfqd->idle_list, bfqq_list) {
			num_char += sprintf(page + num_char,
				"pid%d: weight %hu, dur %d/%u\n",
				bfqq->pid,
				bfqq->entity.weight,
				jiffies_to_msecs(jiffies -
					bfqq->last_wr_start_finish),
				jiffies_to_msecs(bfqq->wr_cur_max_time));
	}

	spin_unlock_irq(bfqd->queue->queue_lock);

	return num_char;
}

#define SHOW_FUNCTION(__FUNC, __VAR, __CONV)				\
static ssize_t __FUNC(struct elevator_queue *e, char *page)		\
{									\
	struct bfq_data *bfqd = e->elevator_data;			\
	unsigned int __data = __VAR;					\
	if (__CONV)							\
		__data = jiffies_to_msecs(__data);			\
	return bfq_var_show(__data, (page));				\
}
SHOW_FUNCTION(bfq_fifo_expire_sync_show, bfqd->bfq_fifo_expire[1], 1);
SHOW_FUNCTION(bfq_fifo_expire_async_show, bfqd->bfq_fifo_expire[0], 1);
SHOW_FUNCTION(bfq_back_seek_max_show, bfqd->bfq_back_max, 0);
SHOW_FUNCTION(bfq_back_seek_penalty_show, bfqd->bfq_back_penalty, 0);
SHOW_FUNCTION(bfq_slice_idle_show, bfqd->bfq_slice_idle, 1);
SHOW_FUNCTION(bfq_max_budget_show, bfqd->bfq_user_max_budget, 0);
SHOW_FUNCTION(bfq_max_budget_async_rq_show,
	      bfqd->bfq_max_budget_async_rq, 0);
SHOW_FUNCTION(bfq_timeout_sync_show, bfqd->bfq_timeout[BLK_RW_SYNC], 1);
SHOW_FUNCTION(bfq_timeout_async_show, bfqd->bfq_timeout[BLK_RW_ASYNC], 1);
SHOW_FUNCTION(bfq_low_latency_show, bfqd->low_latency, 0);
SHOW_FUNCTION(bfq_wr_coeff_show, bfqd->bfq_wr_coeff, 0);
SHOW_FUNCTION(bfq_wr_rt_max_time_show, bfqd->bfq_wr_rt_max_time, 1);
SHOW_FUNCTION(bfq_wr_min_idle_time_show, bfqd->bfq_wr_min_idle_time, 1);
SHOW_FUNCTION(bfq_wr_min_inter_arr_async_show, bfqd->bfq_wr_min_inter_arr_async,
	1);
SHOW_FUNCTION(bfq_wr_max_softrt_rate_show, bfqd->bfq_wr_max_softrt_rate, 0);
#undef SHOW_FUNCTION

#define STORE_FUNCTION(__FUNC, __PTR, MIN, MAX, __CONV)			\
static ssize_t								\
__FUNC(struct elevator_queue *e, const char *page, size_t count)	\
{									\
	struct bfq_data *bfqd = e->elevator_data;			\
	unsigned long uninitialized_var(__data);			\
	int ret = bfq_var_store(&__data, (page), count);		\
	if (__data < (MIN))						\
		__data = (MIN);						\
	else if (__data > (MAX))					\
		__data = (MAX);						\
	if (__CONV)							\
		*(__PTR) = msecs_to_jiffies(__data);			\
	else								\
		*(__PTR) = __data;					\
	return ret;							\
}
STORE_FUNCTION(bfq_fifo_expire_sync_store, &bfqd->bfq_fifo_expire[1], 1,
		INT_MAX, 1);
STORE_FUNCTION(bfq_fifo_expire_async_store, &bfqd->bfq_fifo_expire[0], 1,
		INT_MAX, 1);
STORE_FUNCTION(bfq_back_seek_max_store, &bfqd->bfq_back_max, 0, INT_MAX, 0);
STORE_FUNCTION(bfq_back_seek_penalty_store, &bfqd->bfq_back_penalty, 1,
		INT_MAX, 0);
STORE_FUNCTION(bfq_slice_idle_store, &bfqd->bfq_slice_idle, 0, INT_MAX, 1);
STORE_FUNCTION(bfq_max_budget_async_rq_store, &bfqd->bfq_max_budget_async_rq,
		1, INT_MAX, 0);
STORE_FUNCTION(bfq_timeout_async_store, &bfqd->bfq_timeout[BLK_RW_ASYNC], 0,
		INT_MAX, 1);
STORE_FUNCTION(bfq_wr_coeff_store, &bfqd->bfq_wr_coeff, 1, INT_MAX, 0);
STORE_FUNCTION(bfq_wr_max_time_store, &bfqd->bfq_wr_max_time, 0, INT_MAX, 1);
STORE_FUNCTION(bfq_wr_rt_max_time_store, &bfqd->bfq_wr_rt_max_time, 0, INT_MAX,
		1);
STORE_FUNCTION(bfq_wr_min_idle_time_store, &bfqd->bfq_wr_min_idle_time, 0,
		INT_MAX, 1);
STORE_FUNCTION(bfq_wr_min_inter_arr_async_store,
		&bfqd->bfq_wr_min_inter_arr_async, 0, INT_MAX, 1);
STORE_FUNCTION(bfq_wr_max_softrt_rate_store, &bfqd->bfq_wr_max_softrt_rate, 0,
		INT_MAX, 0);
#undef STORE_FUNCTION

/* do nothing for the moment */
static ssize_t bfq_weights_store(struct elevator_queue *e,
				    const char *page, size_t count)
{
	return count;
}

static inline unsigned long bfq_estimated_max_budget(struct bfq_data *bfqd)
{
	u64 timeout = jiffies_to_msecs(bfqd->bfq_timeout[BLK_RW_SYNC]);

	if (bfqd->peak_rate_samples >= BFQ_PEAK_RATE_SAMPLES)
		return bfq_calc_max_budget(bfqd->peak_rate, timeout);
	else
		return bfq_default_max_budget;
}

static ssize_t bfq_max_budget_store(struct elevator_queue *e,
				    const char *page, size_t count)
{
	struct bfq_data *bfqd = e->elevator_data;
	unsigned long uninitialized_var(__data);
	int ret = bfq_var_store(&__data, (page), count);

	if (__data == 0)
		bfqd->bfq_max_budget = bfq_estimated_max_budget(bfqd);
	else {
		if (__data > INT_MAX)
			__data = INT_MAX;
		bfqd->bfq_max_budget = __data;
	}

	bfqd->bfq_user_max_budget = __data;

	return ret;
}

static ssize_t bfq_timeout_sync_store(struct elevator_queue *e,
				      const char *page, size_t count)
{
	struct bfq_data *bfqd = e->elevator_data;
	unsigned long uninitialized_var(__data);
	int ret = bfq_var_store(&__data, (page), count);

	if (__data < 1)
		__data = 1;
	else if (__data > INT_MAX)
		__data = INT_MAX;

	bfqd->bfq_timeout[BLK_RW_SYNC] = msecs_to_jiffies(__data);
	if (bfqd->bfq_user_max_budget == 0)
		bfqd->bfq_max_budget = bfq_estimated_max_budget(bfqd);

	return ret;
}

static ssize_t bfq_low_latency_store(struct elevator_queue *e,
				     const char *page, size_t count)
{
	struct bfq_data *bfqd = e->elevator_data;
	unsigned long uninitialized_var(__data);
	int ret = bfq_var_store(&__data, (page), count);

	if (__data > 1)
		__data = 1;
	if (__data == 0 && bfqd->low_latency != 0)
		bfq_end_wr(bfqd);
	bfqd->low_latency = __data;

	return ret;
}

#define BFQ_ATTR(name) \
	__ATTR(name, S_IRUGO|S_IWUSR, bfq_##name##_show, bfq_##name##_store)

static struct elv_fs_entry bfq_attrs[] = {
	BFQ_ATTR(fifo_expire_sync),
	BFQ_ATTR(fifo_expire_async),
	BFQ_ATTR(back_seek_max),
	BFQ_ATTR(back_seek_penalty),
	BFQ_ATTR(slice_idle),
	BFQ_ATTR(max_budget),
	BFQ_ATTR(max_budget_async_rq),
	BFQ_ATTR(timeout_sync),
	BFQ_ATTR(timeout_async),
	BFQ_ATTR(low_latency),
	BFQ_ATTR(wr_coeff),
	BFQ_ATTR(wr_max_time),
	BFQ_ATTR(wr_rt_max_time),
	BFQ_ATTR(wr_min_idle_time),
	BFQ_ATTR(wr_min_inter_arr_async),
	BFQ_ATTR(wr_max_softrt_rate),
	BFQ_ATTR(weights),
	__ATTR_NULL
};

static struct elevator_type iosched_bfq = {
	.ops = {
		.elevator_merge_fn =		bfq_merge,
		.elevator_merged_fn =		bfq_merged_request,
		.elevator_merge_req_fn =	bfq_merged_requests,
		.elevator_allow_merge_fn =	bfq_allow_merge,
		.elevator_dispatch_fn =		bfq_dispatch_requests,
		.elevator_add_req_fn =		bfq_insert_request,
		.elevator_activate_req_fn =	bfq_activate_request,
		.elevator_deactivate_req_fn =	bfq_deactivate_request,
		.elevator_completed_req_fn =	bfq_completed_request,
		.elevator_former_req_fn =	elv_rb_former_request,
		.elevator_latter_req_fn =	elv_rb_latter_request,
		.elevator_init_icq_fn =		bfq_init_icq,
		.elevator_exit_icq_fn =		bfq_exit_icq,
		.elevator_set_req_fn =		bfq_set_request,
		.elevator_put_req_fn =		bfq_put_request,
		.elevator_may_queue_fn =	bfq_may_queue,
		.elevator_init_fn =		bfq_init_queue,
		.elevator_exit_fn =		bfq_exit_queue,
	},
	.icq_size =		sizeof(struct bfq_io_cq),
	.icq_align =		__alignof__(struct bfq_io_cq),
	.elevator_attrs =	bfq_attrs,
	.elevator_name =	"bfq",
	.elevator_owner =	THIS_MODULE,
};

static int __init bfq_init(void)
{
	/*
	 * Can be 0 on HZ < 1000 setups.
	 */
	if (bfq_slice_idle == 0)
		bfq_slice_idle = 1;

	if (bfq_timeout_async == 0)
		bfq_timeout_async = 1;

	if (bfq_slab_setup())
		return -ENOMEM;

	/*
	 * Times to load large popular applications for the typical systems
	 * installed on the reference devices (see the comments before the
	 * definitions of the two arrays).
	 */
	T_slow[0] = msecs_to_jiffies(2600);
	T_slow[1] = msecs_to_jiffies(1000);
	T_fast[0] = msecs_to_jiffies(5500);
	T_fast[1] = msecs_to_jiffies(2000);

	/*
	 * Thresholds that determine the switch between speed classes (see
	 * the comments before the definition of the array).
	 */
	device_speed_thresh[0] = (R_fast[0] + R_slow[0]) / 2;
	device_speed_thresh[1] = (R_fast[1] + R_slow[1]) / 2;

	elv_register(&iosched_bfq);
	pr_info("BFQ I/O-scheduler: v7r8");

	return 0;
}

static void __exit bfq_exit(void)
{
	elv_unregister(&iosched_bfq);
	bfq_slab_kill();
}

module_init(bfq_init);
module_exit(bfq_exit);

MODULE_AUTHOR("Fabio Checconi, Paolo Valente");
MODULE_LICENSE("GPL");
