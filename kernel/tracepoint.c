/*
 * Copyright (C) 2008 Mathieu Desnoyers
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/jhash.h>
#include <linux/list.h>
#include <linux/rcupdate.h>
#include <linux/tracepoint.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/static_key.h>

extern struct tracepoint * const __start___tracepoints_ptrs[];
extern struct tracepoint * const __stop___tracepoints_ptrs[];

/* Set to 1 to enable tracepoint debug output */
static const int tracepoint_debug;

/*
 * Tracepoints mutex protects the builtin and module tracepoints and the hash
 * table, as well as the local module list.
 */
static DEFINE_MUTEX(tracepoints_mutex);

#ifdef CONFIG_MODULES
/* Local list of struct module */
static LIST_HEAD(tracepoint_module_list);
#endif /* CONFIG_MODULES */

/*
 * Tracepoint hash table, containing the active tracepoints.
 * Protected by tracepoints_mutex.
 */
#define TRACEPOINT_HASH_BITS 6
#define TRACEPOINT_TABLE_SIZE (1 << TRACEPOINT_HASH_BITS)
static struct hlist_head tracepoint_table[TRACEPOINT_TABLE_SIZE];

/*
 * Note about RCU :
 * It is used to delay the free of multiple probes array until a quiescent
 * state is reached.
 * Tracepoint entries modifications are protected by the tracepoints_mutex.
 */
struct tracepoint_entry {
	struct hlist_node hlist;
	struct tracepoint_func *funcs;
	int refcount;	/* Number of times armed. 0 if disarmed. */
	char name[0];
};

struct tp_probes {
	union {
		struct rcu_head rcu;
		struct list_head list;
	} u;
	struct tracepoint_func probes[0];
};

static inline void *allocate_probes(int count)
{
	struct tp_probes *p  = kmalloc(count * sizeof(struct tracepoint_func)
			+ sizeof(struct tp_probes), GFP_KERNEL);
	return p == NULL ? NULL : p->probes;
}

static void rcu_free_old_probes(struct rcu_head *head)
{
	kfree(container_of(head, struct tp_probes, u.rcu));
}

static inline void release_probes(struct tracepoint_func *old)
{
	if (old) {
		struct tp_probes *tp_probes = container_of(old,
			struct tp_probes, probes[0]);
		call_rcu_sched(&tp_probes->u.rcu, rcu_free_old_probes);
	}
}

static void debug_print_probes(struct tracepoint_entry *entry)
{
	int i;

	if (!tracepoint_debug || !entry->funcs)
		return;

	for (i = 0; entry->funcs[i].func; i++)
		printk(KERN_DEBUG "Probe %d : %p\n", i, entry->funcs[i].func);
}

static struct tracepoint_func *
tracepoint_entry_add_probe(struct tracepoint_entry *entry,
			   void *probe, void *data)
{
	int nr_probes = 0;
	struct tracepoint_func *old, *new;

	if (WARN_ON(!probe))
		return ERR_PTR(-EINVAL);

	debug_print_probes(entry);
	old = entry->funcs;
	if (old) {
		/* (N -> N+1), (N != 0, 1) probes */
		for (nr_probes = 0; old[nr_probes].func; nr_probes++)
			if (old[nr_probes].func == probe &&
			    old[nr_probes].data == data)
				return ERR_PTR(-EEXIST);
	}
	/* + 2 : one for new probe, one for NULL func */
	new = allocate_probes(nr_probes + 2);
	if (new == NULL)
		return ERR_PTR(-ENOMEM);
	if (old)
		memcpy(new, old, nr_probes * sizeof(struct tracepoint_func));
	new[nr_probes].func = probe;
	new[nr_probes].data = data;
	new[nr_probes + 1].func = NULL;
	entry->refcount = nr_probes + 1;
	entry->funcs = new;
	debug_print_probes(entry);
	return old;
}

static void *
tracepoint_entry_remove_probe(struct tracepoint_entry *entry,
			      void *probe, void *data)
{
	int nr_probes = 0, nr_del = 0, i;
	struct tracepoint_func *old, *new;

	old = entry->funcs;

	if (!old)
		return ERR_PTR(-ENOENT);

	debug_print_probes(entry);
	/* (N -> M), (N > 1, M >= 0) probes */
	if (probe) {
		for (nr_probes = 0; old[nr_probes].func; nr_probes++) {
			if (old[nr_probes].func == probe &&
			     old[nr_probes].data == data)
				nr_del++;
		}
	}

	/*
	 * If probe is NULL, then nr_probes = nr_del = 0, and then the
	 * entire entry will be removed.
	 */
	if (nr_probes - nr_del == 0) {
		/* N -> 0, (N > 1) */
		entry->funcs = NULL;
		entry->refcount = 0;
		debug_print_probes(entry);
		return old;
	} else {
		int j = 0;
		/* N -> M, (N > 1, M > 0) */
		/* + 1 for NULL */
		new = allocate_probes(nr_probes - nr_del + 1);
		if (new == NULL)
			return ERR_PTR(-ENOMEM);
		for (i = 0; old[i].func; i++)
			if (old[i].func != probe || old[i].data != data)
				new[j++] = old[i];
		new[nr_probes - nr_del].func = NULL;
		entry->refcount = nr_probes - nr_del;
		entry->funcs = new;
	}
	debug_print_probes(entry);
	return old;
}

/*
 * Get tracepoint if the tracepoint is present in the tracepoint hash table.
 * Must be called with tracepoints_mutex held.
 * Returns NULL if not present.
 */
static struct tracepoint_entry *get_tracepoint(const char *name)
{
	struct hlist_head *head;
	struct tracepoint_entry *e;
	u32 hash = jhash(name, strlen(name), 0);

	head = &tracepoint_table[hash & (TRACEPOINT_TABLE_SIZE - 1)];
	hlist_for_each_entry(e, head, hlist) {
		if (!strcmp(name, e->name))
			return e;
	}
	return NULL;
}

/*
 * Add the tracepoint to the tracepoint hash table. Must be called with
 * tracepoints_mutex held.
 */
static struct tracepoint_entry *add_tracepoint(const char *name)
{
	struct hlist_head *head;
	struct tracepoint_entry *e;
	size_t name_len = strlen(name) + 1;
	u32 hash = jhash(name, name_len-1, 0);

	head = &tracepoint_table[hash & (TRACEPOINT_TABLE_SIZE - 1)];
	hlist_for_each_entry(e, head, hlist) {
		if (!strcmp(name, e->name)) {
			printk(KERN_NOTICE
				"tracepoint %s busy\n", name);
			return ERR_PTR(-EEXIST);	/* Already there */
		}
	}
	/*
	 * Using kmalloc here to allocate a variable length element. Could
	 * cause some memory fragmentation if overused.
	 */
	e = kmalloc(sizeof(struct tracepoint_entry) + name_len, GFP_KERNEL);
	if (!e)
		return ERR_PTR(-ENOMEM);
	memcpy(&e->name[0], name, name_len);
	e->funcs = NULL;
	e->refcount = 0;
	hlist_add_head(&e->hlist, head);
	return e;
}

/*
 * Remove the tracepoint from the tracepoint hash table. Must be called with
 * mutex_lock held.
 */
static inline void remove_tracepoint(struct tracepoint_entry *e)
{
	hlist_del(&e->hlist);
	kfree(e);
}

/*
 * Sets the probe callback corresponding to one tracepoint.
 */
static void set_tracepoint(struct tracepoint_entry **entry,
	struct tracepoint *elem, int active)
{
	WARN_ON(strcmp((*entry)->name, elem->name) != 0);

	if (elem->regfunc && !static_key_enabled(&elem->key) && active)
		elem->regfunc();
	else if (elem->unregfunc && static_key_enabled(&elem->key) && !active)
		elem->unregfunc();

	/*
	 * rcu_assign_pointer has a smp_wmb() which makes sure that the new
	 * probe callbacks array is consistent before setting a pointer to it.
	 * This array is referenced by __DO_TRACE from
	 * include/linux/tracepoints.h. A matching smp_read_barrier_depends()
	 * is used.
	 */
	rcu_assign_pointer(elem->funcs, (*entry)->funcs);
	if (active && !static_key_enabled(&elem->key))
		static_key_slow_inc(&elem->key);
	else if (!active && static_key_enabled(&elem->key))
		static_key_slow_dec(&elem->key);
}

/*
 * Disable a tracepoint and its probe callback.
 * Note: only waiting an RCU period after setting elem->call to the empty
 * function insures that the original callback is not used anymore. This insured
 * by preempt_disable around the call site.
 */
static void disable_tracepoint(struct tracepoint *elem)
{
	if (elem->unregfunc && static_key_enabled(&elem->key))
		elem->unregfunc();

	if (static_key_enabled(&elem->key))
		static_key_slow_dec(&elem->key);
	rcu_assign_pointer(elem->funcs, NULL);
}

/**
 * tracepoint_update_probe_range - Update a probe range
 * @begin: beginning of the range
 * @end: end of the range
 *
 * Updates the probe callback corresponding to a range of tracepoints.
 * Called with tracepoints_mutex held.
 */
static void tracepoint_update_probe_range(struct tracepoint * const *begin,
					  struct tracepoint * const *end)
{
	struct tracepoint * const *iter;
	struct tracepoint_entry *mark_entry;

	if (!begin)
		return;

	for (iter = begin; iter < end; iter++) {
		mark_entry = get_tracepoint((*iter)->name);
		if (mark_entry) {
			set_tracepoint(&mark_entry, *iter,
					!!mark_entry->refcount);
		} else {
			disable_tracepoint(*iter);
		}
	}
}

#ifdef CONFIG_MODULES
void module_update_tracepoints(void)
{
	struct tp_module *tp_mod;

	list_for_each_entry(tp_mod, &tracepoint_module_list, list)
		tracepoint_update_probe_range(tp_mod->tracepoints_ptrs,
			tp_mod->tracepoints_ptrs + tp_mod->num_tracepoints);
}
#else /* CONFIG_MODULES */
void module_update_tracepoints(void)
{
}
#endif /* CONFIG_MODULES */


/*
 * Update probes, removing the faulty probes.
 * Called with tracepoints_mutex held.
 */
static void tracepoint_update_probes(void)
{
	/* Core kernel tracepoints */
	tracepoint_update_probe_range(__start___tracepoints_ptrs,
		__stop___tracepoints_ptrs);
	/* tracepoints in modules. */
	module_update_tracepoints();
}

static struct tracepoint_func *
tracepoint_add_probe(const char *name, void *probe, void *data)
{
	struct tracepoint_entry *entry;
	struct tracepoint_func *old;

	entry = get_tracepoint(name);
	if (!entry) {
		entry = add_tracepoint(name);
		if (IS_ERR(entry))
			return (struct tracepoint_func *)entry;
	}
	old = tracepoint_entry_add_probe(entry, probe, data);
	if (IS_ERR(old) && !entry->refcount)
		remove_tracepoint(entry);
	return old;
}

/**
 * tracepoint_probe_register -  Connect a probe to a tracepoint
 * @name: tracepoint name
 * @probe: probe handler
 *
 * Returns 0 if ok, error value on error.
 * The probe address must at least be aligned on the architecture pointer size.
 */
int tracepoint_probe_register(const char *name, void *probe, void *data)
{
	struct tracepoint_func *old;

	mutex_lock(&tracepoints_mutex);
	old = tracepoint_add_probe(name, probe, data);
	if (IS_ERR(old)) {
		mutex_unlock(&tracepoints_mutex);
		return PTR_ERR(old);
	}
	tracepoint_update_probes();		/* may update entry */
	mutex_unlock(&tracepoints_mutex);
	release_probes(old);
	return 0;
}
EXPORT_SYMBOL_GPL(tracepoint_probe_register);

static struct tracepoint_func *
tracepoint_remove_probe(const char *name, void *probe, void *data)
{
	struct tracepoint_entry *entry;
	struct tracepoint_func *old;

	entry = get_tracepoint(name);
	if (!entry)
		return ERR_PTR(-ENOENT);
	old = tracepoint_entry_remove_probe(entry, probe, data);
	if (IS_ERR(old))
		return old;
	if (!entry->refcount)
		remove_tracepoint(entry);
	return old;
}

/**
 * tracepoint_probe_unregister -  Disconnect a probe from a tracepoint
 * @name: tracepoint name
 * @probe: probe function pointer
 *
 * We do not need to call a synchronize_sched to make sure the probes have
 * finished running before doing a module unload, because the module unload
 * itself uses stop_machine(), which insures that every preempt disabled section
 * have finished.
 */
int tracepoint_probe_unregister(const char *name, void *probe, void *data)
{
	struct tracepoint_func *old;

	mutex_lock(&tracepoints_mutex);
	old = tracepoint_remove_probe(name, probe, data);
	if (IS_ERR(old)) {
		mutex_unlock(&tracepoints_mutex);
		return PTR_ERR(old);
	}
	tracepoint_update_probes();		/* may update entry */
	mutex_unlock(&tracepoints_mutex);
	release_probes(old);
	return 0;
}
EXPORT_SYMBOL_GPL(tracepoint_probe_unregister);

static LIST_HEAD(old_probes);
static int need_update;

static void tracepoint_add_old_probes(void *old)
{
	need_update = 1;
	if (old) {
		struct tp_probes *tp_probes = container_of(old,
			struct tp_probes, probes[0]);
		list_add(&tp_probes->u.list, &old_probes);
	}
}

/**
 * tracepoint_probe_register_noupdate -  register a probe but not connect
 * @name: tracepoint name
 * @probe: probe handler
 *
 * caller must call tracepoint_probe_update_all()
 */
int tracepoint_probe_register_noupdate(const char *name, void *probe,
				       void *data)
{
	struct tracepoint_func *old;

	mutex_lock(&tracepoints_mutex);
	old = tracepoint_add_probe(name, probe, data);
	if (IS_ERR(old)) {
		mutex_unlock(&tracepoints_mutex);
		return PTR_ERR(old);
	}
	tracepoint_add_old_probes(old);
	mutex_unlock(&tracepoints_mutex);
	return 0;
}
EXPORT_SYMBOL_GPL(tracepoint_probe_register_noupdate);

/**
 * tracepoint_probe_unregister_noupdate -  remove a probe but not disconnect
 * @name: tracepoint name
 * @probe: probe function pointer
 *
 * caller must call tracepoint_probe_update_all()
 */
int tracepoint_probe_unregister_noupdate(const char *name, void *probe,
					 void *data)
{
	struct tracepoint_func *old;

	mutex_lock(&tracepoints_mutex);
	old = tracepoint_remove_probe(name, probe, data);
	if (IS_ERR(old)) {
		mutex_unlock(&tracepoints_mutex);
		return PTR_ERR(old);
	}
	tracepoint_add_old_probes(old);
	mutex_unlock(&tracepoints_mutex);
	return 0;
}
EXPORT_SYMBOL_GPL(tracepoint_probe_unregister_noupdate);

/**
 * tracepoint_probe_update_all -  update tracepoints
 */
void tracepoint_probe_update_all(void)
{
	LIST_HEAD(release_probes);
	struct tp_probes *pos, *next;

	mutex_lock(&tracepoints_mutex);
	if (!need_update) {
		mutex_unlock(&tracepoints_mutex);
		return;
	}
	if (!list_empty(&old_probes))
		list_replace_init(&old_probes, &release_probes);
	need_update = 0;
	tracepoint_update_probes();
	mutex_unlock(&tracepoints_mutex);
	list_for_each_entry_safe(pos, next, &release_probes, u.list) {
		list_del(&pos->u.list);
		call_rcu_sched(&pos->u.rcu, rcu_free_old_probes);
	}
}
EXPORT_SYMBOL_GPL(tracepoint_probe_update_all);

/**
 * tracepoint_get_iter_range - Get a next tracepoint iterator given a range.
 * @tracepoint: current tracepoints (in), next tracepoint (out)
 * @begin: beginning of the range
 * @end: end of the range
 *
 * Returns whether a next tracepoint has been found (1) or not (0).
 * Will return the first tracepoint in the range if the input tracepoint is
 * NULL.
 */
static int tracepoint_get_iter_range(struct tracepoint * const **tracepoint,
	struct tracepoint * const *begin, struct tracepoint * const *end)
{
	if (!*tracepoint && begin != end) {
		*tracepoint = begin;
		return 1;
	}
	if (*tracepoint >= begin && *tracepoint < end)
		return 1;
	return 0;
}

#ifdef CONFIG_MODULES
static void tracepoint_get_iter(struct tracepoint_iter *iter)
{
	int found = 0;
	struct tp_module *iter_mod;

	/* Core kernel tracepoints */
	if (!iter->module) {
		found = tracepoint_get_iter_range(&iter->tracepoint,
				__start___tracepoints_ptrs,
				__stop___tracepoints_ptrs);
		if (found)
			goto end;
	}
	/* Tracepoints in modules */
	mutex_lock(&tracepoints_mutex);
	list_for_each_entry(iter_mod, &tracepoint_module_list, list) {
		/*
		 * Sorted module list
		 */
		if (iter_mod < iter->module)
			continue;
		else if (iter_mod > iter->module)
			iter->tracepoint = NULL;
		found = tracepoint_get_iter_range(&iter->tracepoint,
			iter_mod->tracepoints_ptrs,
			iter_mod->tracepoints_ptrs
				+ iter_mod->num_tracepoints);
		if (found) {
			iter->module = iter_mod;
			break;
		}
	}
	mutex_unlock(&tracepoints_mutex);
end:
	if (!found)
		tracepoint_iter_reset(iter);
}
#else /* CONFIG_MODULES */
static void tracepoint_get_iter(struct tracepoint_iter *iter)
{
	int found = 0;

	/* Core kernel tracepoints */
	found = tracepoint_get_iter_range(&iter->tracepoint,
			__start___tracepoints_ptrs,
			__stop___tracepoints_ptrs);
	if (!found)
		tracepoint_iter_reset(iter);
}
#endif /* CONFIG_MODULES */

void tracepoint_iter_start(struct tracepoint_iter *iter)
{
	tracepoint_get_iter(iter);
}
EXPORT_SYMBOL_GPL(tracepoint_iter_start);

void tracepoint_iter_next(struct tracepoint_iter *iter)
{
	iter->tracepoint++;
	/*
	 * iter->tracepoint may be invalid because we blindly incremented it.
	 * Make sure it is valid by marshalling on the tracepoints, getting the
	 * tracepoints from following modules if necessary.
	 */
	tracepoint_get_iter(iter);
}
EXPORT_SYMBOL_GPL(tracepoint_iter_next);

void tracepoint_iter_stop(struct tracepoint_iter *iter)
{
}
EXPORT_SYMBOL_GPL(tracepoint_iter_stop);

void tracepoint_iter_reset(struct tracepoint_iter *iter)
{
#ifdef CONFIG_MODULES
	iter->module = NULL;
#endif /* CONFIG_MODULES */
	iter->tracepoint = NULL;
}
EXPORT_SYMBOL_GPL(tracepoint_iter_reset);

#ifdef CONFIG_MODULES
bool trace_module_has_bad_taint(struct module *mod)
{
	return mod->taints & ~((1 << TAINT_OOT_MODULE) | (1 << TAINT_CRAP));
}

static int tracepoint_module_coming(struct module *mod)
{
	struct tp_module *tp_mod, *iter;
	int ret = 0;

	if (!mod->num_tracepoints)
		return 0;

	/*
	 * We skip modules that taint the kernel, especially those with different
	 * module headers (for forced load), to make sure we don't cause a crash.
	 * Staging and out-of-tree GPL modules are fine.
	 */
	if (trace_module_has_bad_taint(mod))
		return 0;
	mutex_lock(&tracepoints_mutex);
	tp_mod = kmalloc(sizeof(struct tp_module), GFP_KERNEL);
	if (!tp_mod) {
		ret = -ENOMEM;
		goto end;
	}
	tp_mod->num_tracepoints = mod->num_tracepoints;
	tp_mod->tracepoints_ptrs = mod->tracepoints_ptrs;

	/*
	 * tracepoint_module_list is kept sorted by struct module pointer
	 * address for iteration on tracepoints from a seq_file that can release
	 * the mutex between calls.
	 */
	list_for_each_entry_reverse(iter, &tracepoint_module_list, list) {
		BUG_ON(iter == tp_mod);	/* Should never be in the list twice */
		if (iter < tp_mod) {
			/* We belong to the location right after iter. */
			list_add(&tp_mod->list, &iter->list);
			goto module_added;
		}
	}
	/* We belong to the beginning of the list */
	list_add(&tp_mod->list, &tracepoint_module_list);
module_added:
	tracepoint_update_probe_range(mod->tracepoints_ptrs,
		mod->tracepoints_ptrs + mod->num_tracepoints);
end:
	mutex_unlock(&tracepoints_mutex);
	return ret;
}

static int tracepoint_module_going(struct module *mod)
{
	struct tp_module *pos;

	if (!mod->num_tracepoints)
		return 0;

	mutex_lock(&tracepoints_mutex);
	tracepoint_update_probe_range(mod->tracepoints_ptrs,
		mod->tracepoints_ptrs + mod->num_tracepoints);
	list_for_each_entry(pos, &tracepoint_module_list, list) {
		if (pos->tracepoints_ptrs == mod->tracepoints_ptrs) {
			list_del(&pos->list);
			kfree(pos);
			break;
		}
	}
	/*
	 * In the case of modules that were tainted at "coming", we'll simply
	 * walk through the list without finding it. We cannot use the "tainted"
	 * flag on "going", in case a module taints the kernel only after being
	 * loaded.
	 */
	mutex_unlock(&tracepoints_mutex);
	return 0;
}

int tracepoint_module_notify(struct notifier_block *self,
			     unsigned long val, void *data)
{
	struct module *mod = data;
	int ret = 0;

	switch (val) {
	case MODULE_STATE_COMING:
		ret = tracepoint_module_coming(mod);
		break;
	case MODULE_STATE_LIVE:
		break;
	case MODULE_STATE_GOING:
		ret = tracepoint_module_going(mod);
		break;
	}
	return ret;
}

struct notifier_block tracepoint_module_nb = {
	.notifier_call = tracepoint_module_notify,
	.priority = 0,
};

static int init_tracepoints(void)
{
	return register_module_notifier(&tracepoint_module_nb);
}
__initcall(init_tracepoints);
#endif /* CONFIG_MODULES */

#ifdef CONFIG_HAVE_SYSCALL_TRACEPOINTS

/* NB: reg/unreg are called while guarded with the tracepoints_mutex */
static int sys_tracepoint_refcount;

void syscall_regfunc(void)
{
	struct task_struct *p, *t;

	if (!sys_tracepoint_refcount) {
		read_lock(&tasklist_lock);
		for_each_process_thread(p, t) {
			/* Skip kernel threads. */
			if (!(t->flags & PF_KTHREAD))
				set_tsk_thread_flag(t, TIF_SYSCALL_TRACEPOINT);
		}
		read_unlock(&tasklist_lock);
	}
	sys_tracepoint_refcount++;
}

void syscall_unregfunc(void)
{
	struct task_struct *p, *t;

	sys_tracepoint_refcount--;
	if (!sys_tracepoint_refcount) {
		read_lock(&tasklist_lock);
		for_each_process_thread(p, t) {
			clear_tsk_thread_flag(t, TIF_SYSCALL_TRACEPOINT);
		}
		read_unlock(&tasklist_lock);
	}
}
#endif
