
#include <linux/ceph/ceph_debug.h>

#include <linux/module.h>
#include <linux/slab.h>
#include <asm/div64.h>

#include <linux/ceph/libceph.h>
#include <linux/ceph/osdmap.h>
#include <linux/ceph/decode.h>
#include <linux/crush/hash.h>
#include <linux/crush/mapper.h>

char *ceph_osdmap_state_str(char *str, int len, int state)
{
	if (!len)
		return str;

	if ((state & CEPH_OSD_EXISTS) && (state & CEPH_OSD_UP))
		snprintf(str, len, "exists, up");
	else if (state & CEPH_OSD_EXISTS)
		snprintf(str, len, "exists");
	else if (state & CEPH_OSD_UP)
		snprintf(str, len, "up");
	else
		snprintf(str, len, "doesn't exist");

	return str;
}

/* maps */

static int calc_bits_of(unsigned int t)
{
	int b = 0;
	while (t) {
		t = t >> 1;
		b++;
	}
	return b;
}

/*
 * the foo_mask is the smallest value 2^n-1 that is >= foo.
 */
static void calc_pg_masks(struct ceph_pg_pool_info *pi)
{
	pi->pg_num_mask = (1 << calc_bits_of(pi->pg_num-1)) - 1;
	pi->pgp_num_mask = (1 << calc_bits_of(pi->pgp_num-1)) - 1;
}

/*
 * decode crush map
 */
static int crush_decode_uniform_bucket(void **p, void *end,
				       struct crush_bucket_uniform *b)
{
	dout("crush_decode_uniform_bucket %p to %p\n", *p, end);
	ceph_decode_need(p, end, (1+b->h.size) * sizeof(u32), bad);
	b->item_weight = ceph_decode_32(p);
	return 0;
bad:
	return -EINVAL;
}

static int crush_decode_list_bucket(void **p, void *end,
				    struct crush_bucket_list *b)
{
	int j;
	dout("crush_decode_list_bucket %p to %p\n", *p, end);
	b->item_weights = kcalloc(b->h.size, sizeof(u32), GFP_NOFS);
	if (b->item_weights == NULL)
		return -ENOMEM;
	b->sum_weights = kcalloc(b->h.size, sizeof(u32), GFP_NOFS);
	if (b->sum_weights == NULL)
		return -ENOMEM;
	ceph_decode_need(p, end, 2 * b->h.size * sizeof(u32), bad);
	for (j = 0; j < b->h.size; j++) {
		b->item_weights[j] = ceph_decode_32(p);
		b->sum_weights[j] = ceph_decode_32(p);
	}
	return 0;
bad:
	return -EINVAL;
}

static int crush_decode_tree_bucket(void **p, void *end,
				    struct crush_bucket_tree *b)
{
	int j;
	dout("crush_decode_tree_bucket %p to %p\n", *p, end);
	ceph_decode_8_safe(p, end, b->num_nodes, bad);
	b->node_weights = kcalloc(b->num_nodes, sizeof(u32), GFP_NOFS);
	if (b->node_weights == NULL)
		return -ENOMEM;
	ceph_decode_need(p, end, b->num_nodes * sizeof(u32), bad);
	for (j = 0; j < b->num_nodes; j++)
		b->node_weights[j] = ceph_decode_32(p);
	return 0;
bad:
	return -EINVAL;
}

static int crush_decode_straw_bucket(void **p, void *end,
				     struct crush_bucket_straw *b)
{
	int j;
	dout("crush_decode_straw_bucket %p to %p\n", *p, end);
	b->item_weights = kcalloc(b->h.size, sizeof(u32), GFP_NOFS);
	if (b->item_weights == NULL)
		return -ENOMEM;
	b->straws = kcalloc(b->h.size, sizeof(u32), GFP_NOFS);
	if (b->straws == NULL)
		return -ENOMEM;
	ceph_decode_need(p, end, 2 * b->h.size * sizeof(u32), bad);
	for (j = 0; j < b->h.size; j++) {
		b->item_weights[j] = ceph_decode_32(p);
		b->straws[j] = ceph_decode_32(p);
	}
	return 0;
bad:
	return -EINVAL;
}

static int skip_name_map(void **p, void *end)
{
        int len;
        ceph_decode_32_safe(p, end, len ,bad);
        while (len--) {
                int strlen;
                *p += sizeof(u32);
                ceph_decode_32_safe(p, end, strlen, bad);
                *p += strlen;
}
        return 0;
bad:
        return -EINVAL;
}

static struct crush_map *crush_decode(void *pbyval, void *end)
{
	struct crush_map *c;
	int err = -EINVAL;
	int i, j;
	void **p = &pbyval;
	void *start = pbyval;
	u32 magic;
	u32 num_name_maps;

	dout("crush_decode %p to %p len %d\n", *p, end, (int)(end - *p));

	c = kzalloc(sizeof(*c), GFP_NOFS);
	if (c == NULL)
		return ERR_PTR(-ENOMEM);

        /* set tunables to default values */
        c->choose_local_tries = 2;
        c->choose_local_fallback_tries = 5;
        c->choose_total_tries = 19;
	c->chooseleaf_descend_once = 0;

	ceph_decode_need(p, end, 4*sizeof(u32), bad);
	magic = ceph_decode_32(p);
	if (magic != CRUSH_MAGIC) {
		pr_err("crush_decode magic %x != current %x\n",
		       (unsigned int)magic, (unsigned int)CRUSH_MAGIC);
		goto bad;
	}
	c->max_buckets = ceph_decode_32(p);
	c->max_rules = ceph_decode_32(p);
	c->max_devices = ceph_decode_32(p);

	c->buckets = kcalloc(c->max_buckets, sizeof(*c->buckets), GFP_NOFS);
	if (c->buckets == NULL)
		goto badmem;
	c->rules = kcalloc(c->max_rules, sizeof(*c->rules), GFP_NOFS);
	if (c->rules == NULL)
		goto badmem;

	/* buckets */
	for (i = 0; i < c->max_buckets; i++) {
		int size = 0;
		u32 alg;
		struct crush_bucket *b;

		ceph_decode_32_safe(p, end, alg, bad);
		if (alg == 0) {
			c->buckets[i] = NULL;
			continue;
		}
		dout("crush_decode bucket %d off %x %p to %p\n",
		     i, (int)(*p-start), *p, end);

		switch (alg) {
		case CRUSH_BUCKET_UNIFORM:
			size = sizeof(struct crush_bucket_uniform);
			break;
		case CRUSH_BUCKET_LIST:
			size = sizeof(struct crush_bucket_list);
			break;
		case CRUSH_BUCKET_TREE:
			size = sizeof(struct crush_bucket_tree);
			break;
		case CRUSH_BUCKET_STRAW:
			size = sizeof(struct crush_bucket_straw);
			break;
		default:
			err = -EINVAL;
			goto bad;
		}
		BUG_ON(size == 0);
		b = c->buckets[i] = kzalloc(size, GFP_NOFS);
		if (b == NULL)
			goto badmem;

		ceph_decode_need(p, end, 4*sizeof(u32), bad);
		b->id = ceph_decode_32(p);
		b->type = ceph_decode_16(p);
		b->alg = ceph_decode_8(p);
		b->hash = ceph_decode_8(p);
		b->weight = ceph_decode_32(p);
		b->size = ceph_decode_32(p);

		dout("crush_decode bucket size %d off %x %p to %p\n",
		     b->size, (int)(*p-start), *p, end);

		b->items = kcalloc(b->size, sizeof(__s32), GFP_NOFS);
		if (b->items == NULL)
			goto badmem;
		b->perm = kcalloc(b->size, sizeof(u32), GFP_NOFS);
		if (b->perm == NULL)
			goto badmem;
		b->perm_n = 0;

		ceph_decode_need(p, end, b->size*sizeof(u32), bad);
		for (j = 0; j < b->size; j++)
			b->items[j] = ceph_decode_32(p);

		switch (b->alg) {
		case CRUSH_BUCKET_UNIFORM:
			err = crush_decode_uniform_bucket(p, end,
				  (struct crush_bucket_uniform *)b);
			if (err < 0)
				goto bad;
			break;
		case CRUSH_BUCKET_LIST:
			err = crush_decode_list_bucket(p, end,
			       (struct crush_bucket_list *)b);
			if (err < 0)
				goto bad;
			break;
		case CRUSH_BUCKET_TREE:
			err = crush_decode_tree_bucket(p, end,
				(struct crush_bucket_tree *)b);
			if (err < 0)
				goto bad;
			break;
		case CRUSH_BUCKET_STRAW:
			err = crush_decode_straw_bucket(p, end,
				(struct crush_bucket_straw *)b);
			if (err < 0)
				goto bad;
			break;
		}
	}

	/* rules */
	dout("rule vec is %p\n", c->rules);
	for (i = 0; i < c->max_rules; i++) {
		u32 yes;
		struct crush_rule *r;

		ceph_decode_32_safe(p, end, yes, bad);
		if (!yes) {
			dout("crush_decode NO rule %d off %x %p to %p\n",
			     i, (int)(*p-start), *p, end);
			c->rules[i] = NULL;
			continue;
		}

		dout("crush_decode rule %d off %x %p to %p\n",
		     i, (int)(*p-start), *p, end);

		/* len */
		ceph_decode_32_safe(p, end, yes, bad);
#if BITS_PER_LONG == 32
		err = -EINVAL;
		if (yes > (ULONG_MAX - sizeof(*r))
			  / sizeof(struct crush_rule_step))
			goto bad;
#endif
		r = c->rules[i] = kmalloc(sizeof(*r) +
					  yes*sizeof(struct crush_rule_step),
					  GFP_NOFS);
		if (r == NULL)
			goto badmem;
		dout(" rule %d is at %p\n", i, r);
		r->len = yes;
		ceph_decode_copy_safe(p, end, &r->mask, 4, bad); /* 4 u8's */
		ceph_decode_need(p, end, r->len*3*sizeof(u32), bad);
		for (j = 0; j < r->len; j++) {
			r->steps[j].op = ceph_decode_32(p);
			r->steps[j].arg1 = ceph_decode_32(p);
			r->steps[j].arg2 = ceph_decode_32(p);
		}
	}

	/* ignore trailing name maps. */
        for (num_name_maps = 0; num_name_maps < 3; num_name_maps++) {
                err = skip_name_map(p, end);
                if (err < 0)
                        goto done;
        }

        /* tunables */
        ceph_decode_need(p, end, 3*sizeof(u32), done);
        c->choose_local_tries = ceph_decode_32(p);
        c->choose_local_fallback_tries =  ceph_decode_32(p);
        c->choose_total_tries = ceph_decode_32(p);
        dout("crush decode tunable choose_local_tries = %d",
             c->choose_local_tries);
        dout("crush decode tunable choose_local_fallback_tries = %d",
             c->choose_local_fallback_tries);
        dout("crush decode tunable choose_total_tries = %d",
             c->choose_total_tries);

	ceph_decode_need(p, end, sizeof(u32), done);
	c->chooseleaf_descend_once = ceph_decode_32(p);
	dout("crush decode tunable chooseleaf_descend_once = %d",
	     c->chooseleaf_descend_once);

done:
	dout("crush_decode success\n");
	return c;

badmem:
	err = -ENOMEM;
bad:
	dout("crush_decode fail %d\n", err);
	crush_destroy(c);
	return ERR_PTR(err);
}

/*
 * rbtree of pg_mapping for handling pg_temp (explicit mapping of pgid
 * to a set of osds)
 */
static int pgid_cmp(struct ceph_pg l, struct ceph_pg r)
{
	if (l.pool < r.pool)
		return -1;
	if (l.pool > r.pool)
		return 1;
	if (l.seed < r.seed)
		return -1;
	if (l.seed > r.seed)
		return 1;
	return 0;
}

static int __insert_pg_mapping(struct ceph_pg_mapping *new,
			       struct rb_root *root)
{
	struct rb_node **p = &root->rb_node;
	struct rb_node *parent = NULL;
	struct ceph_pg_mapping *pg = NULL;
	int c;

	dout("__insert_pg_mapping %llx %p\n", *(u64 *)&new->pgid, new);
	while (*p) {
		parent = *p;
		pg = rb_entry(parent, struct ceph_pg_mapping, node);
		c = pgid_cmp(new->pgid, pg->pgid);
		if (c < 0)
			p = &(*p)->rb_left;
		else if (c > 0)
			p = &(*p)->rb_right;
		else
			return -EEXIST;
	}

	rb_link_node(&new->node, parent, p);
	rb_insert_color(&new->node, root);
	return 0;
}

static struct ceph_pg_mapping *__lookup_pg_mapping(struct rb_root *root,
						   struct ceph_pg pgid)
{
	struct rb_node *n = root->rb_node;
	struct ceph_pg_mapping *pg;
	int c;

	while (n) {
		pg = rb_entry(n, struct ceph_pg_mapping, node);
		c = pgid_cmp(pgid, pg->pgid);
		if (c < 0) {
			n = n->rb_left;
		} else if (c > 0) {
			n = n->rb_right;
		} else {
			dout("__lookup_pg_mapping %lld.%x got %p\n",
			     pgid.pool, pgid.seed, pg);
			return pg;
		}
	}
	return NULL;
}

static int __remove_pg_mapping(struct rb_root *root, struct ceph_pg pgid)
{
	struct ceph_pg_mapping *pg = __lookup_pg_mapping(root, pgid);

	if (pg) {
		dout("__remove_pg_mapping %lld.%x %p\n", pgid.pool, pgid.seed,
		     pg);
		rb_erase(&pg->node, root);
		kfree(pg);
		return 0;
	}
	dout("__remove_pg_mapping %lld.%x dne\n", pgid.pool, pgid.seed);
	return -ENOENT;
}

/*
 * rbtree of pg pool info
 */
static int __insert_pg_pool(struct rb_root *root, struct ceph_pg_pool_info *new)
{
	struct rb_node **p = &root->rb_node;
	struct rb_node *parent = NULL;
	struct ceph_pg_pool_info *pi = NULL;

	while (*p) {
		parent = *p;
		pi = rb_entry(parent, struct ceph_pg_pool_info, node);
		if (new->id < pi->id)
			p = &(*p)->rb_left;
		else if (new->id > pi->id)
			p = &(*p)->rb_right;
		else
			return -EEXIST;
	}

	rb_link_node(&new->node, parent, p);
	rb_insert_color(&new->node, root);
	return 0;
}

static struct ceph_pg_pool_info *__lookup_pg_pool(struct rb_root *root, u64 id)
{
	struct ceph_pg_pool_info *pi;
	struct rb_node *n = root->rb_node;

	while (n) {
		pi = rb_entry(n, struct ceph_pg_pool_info, node);
		if (id < pi->id)
			n = n->rb_left;
		else if (id > pi->id)
			n = n->rb_right;
		else
			return pi;
	}
	return NULL;
}

const char *ceph_pg_pool_name_by_id(struct ceph_osdmap *map, u64 id)
{
	struct ceph_pg_pool_info *pi;

	if (id == CEPH_NOPOOL)
		return NULL;

	if (WARN_ON_ONCE(id > (u64) INT_MAX))
		return NULL;

	pi = __lookup_pg_pool(&map->pg_pools, (int) id);

	return pi ? pi->name : NULL;
}
EXPORT_SYMBOL(ceph_pg_pool_name_by_id);

int ceph_pg_poolid_by_name(struct ceph_osdmap *map, const char *name)
{
	struct rb_node *rbp;

	for (rbp = rb_first(&map->pg_pools); rbp; rbp = rb_next(rbp)) {
		struct ceph_pg_pool_info *pi =
			rb_entry(rbp, struct ceph_pg_pool_info, node);
		if (pi->name && strcmp(pi->name, name) == 0)
			return pi->id;
	}
	return -ENOENT;
}
EXPORT_SYMBOL(ceph_pg_poolid_by_name);

static void __remove_pg_pool(struct rb_root *root, struct ceph_pg_pool_info *pi)
{
	rb_erase(&pi->node, root);
	kfree(pi->name);
	kfree(pi);
}

static int __decode_pool(void **p, void *end, struct ceph_pg_pool_info *pi)
{
	u8 ev, cv;
	unsigned len, num;
	void *pool_end;

	ceph_decode_need(p, end, 2 + 4, bad);
	ev = ceph_decode_8(p);  /* encoding version */
	cv = ceph_decode_8(p); /* compat version */
	if (ev < 5) {
		pr_warning("got v %d < 5 cv %d of ceph_pg_pool\n", ev, cv);
		return -EINVAL;
	}
	if (cv > 7) {
		pr_warning("got v %d cv %d > 7 of ceph_pg_pool\n", ev, cv);
		return -EINVAL;
	}
	len = ceph_decode_32(p);
	ceph_decode_need(p, end, len, bad);
	pool_end = *p + len;

	pi->type = ceph_decode_8(p);
	pi->size = ceph_decode_8(p);
	pi->crush_ruleset = ceph_decode_8(p);
	pi->object_hash = ceph_decode_8(p);

	pi->pg_num = ceph_decode_32(p);
	pi->pgp_num = ceph_decode_32(p);

	*p += 4 + 4;  /* skip lpg* */
	*p += 4;      /* skip last_change */
	*p += 8 + 4;  /* skip snap_seq, snap_epoch */

	/* skip snaps */
	num = ceph_decode_32(p);
	while (num--) {
		*p += 8;  /* snapid key */
		*p += 1 + 1; /* versions */
		len = ceph_decode_32(p);
		*p += len;
	}

	/* skip removed snaps */
	num = ceph_decode_32(p);
	*p += num * (8 + 8);

	*p += 8;  /* skip auid */
	pi->flags = ceph_decode_64(p);

	/* ignore the rest */

	*p = pool_end;
	calc_pg_masks(pi);
	return 0;

bad:
	return -EINVAL;
}

static int __decode_pool_names(void **p, void *end, struct ceph_osdmap *map)
{
	struct ceph_pg_pool_info *pi;
	u32 num, len;
	u64 pool;

	ceph_decode_32_safe(p, end, num, bad);
	dout(" %d pool names\n", num);
	while (num--) {
		ceph_decode_64_safe(p, end, pool, bad);
		ceph_decode_32_safe(p, end, len, bad);
		dout("  pool %llu len %d\n", pool, len);
		ceph_decode_need(p, end, len, bad);
		pi = __lookup_pg_pool(&map->pg_pools, pool);
		if (pi) {
			char *name = kstrndup(*p, len, GFP_NOFS);

			if (!name)
				return -ENOMEM;
			kfree(pi->name);
			pi->name = name;
			dout("  name is %s\n", pi->name);
		}
		*p += len;
	}
	return 0;

bad:
	return -EINVAL;
}

/*
 * osd map
 */
void ceph_osdmap_destroy(struct ceph_osdmap *map)
{
	dout("osdmap_destroy %p\n", map);
	if (map->crush)
		crush_destroy(map->crush);
	while (!RB_EMPTY_ROOT(&map->pg_temp)) {
		struct ceph_pg_mapping *pg =
			rb_entry(rb_first(&map->pg_temp),
				 struct ceph_pg_mapping, node);
		rb_erase(&pg->node, &map->pg_temp);
		kfree(pg);
	}
	while (!RB_EMPTY_ROOT(&map->pg_pools)) {
		struct ceph_pg_pool_info *pi =
			rb_entry(rb_first(&map->pg_pools),
				 struct ceph_pg_pool_info, node);
		__remove_pg_pool(&map->pg_pools, pi);
	}
	kfree(map->osd_state);
	kfree(map->osd_weight);
	kfree(map->osd_addr);
	kfree(map);
}

/*
 * adjust max osd value.  reallocate arrays.
 */
static int osdmap_set_max_osd(struct ceph_osdmap *map, int max)
{
	u8 *state;
	struct ceph_entity_addr *addr;
	u32 *weight;

	state = kcalloc(max, sizeof(*state), GFP_NOFS);
	addr = kcalloc(max, sizeof(*addr), GFP_NOFS);
	weight = kcalloc(max, sizeof(*weight), GFP_NOFS);
	if (state == NULL || addr == NULL || weight == NULL) {
		kfree(state);
		kfree(addr);
		kfree(weight);
		return -ENOMEM;
	}

	/* copy old? */
	if (map->osd_state) {
		memcpy(state, map->osd_state, map->max_osd*sizeof(*state));
		memcpy(addr, map->osd_addr, map->max_osd*sizeof(*addr));
		memcpy(weight, map->osd_weight, map->max_osd*sizeof(*weight));
		kfree(map->osd_state);
		kfree(map->osd_addr);
		kfree(map->osd_weight);
	}

	map->osd_state = state;
	map->osd_weight = weight;
	map->osd_addr = addr;
	map->max_osd = max;
	return 0;
}

/*
 * decode a full map.
 */
struct ceph_osdmap *osdmap_decode(void **p, void *end)
{
	struct ceph_osdmap *map;
	u16 version;
	u32 len, max, i;
	int err = -EINVAL;
	void *start = *p;
	struct ceph_pg_pool_info *pi;

	dout("osdmap_decode %p to %p len %d\n", *p, end, (int)(end - *p));

	map = kzalloc(sizeof(*map), GFP_NOFS);
	if (map == NULL)
		return ERR_PTR(-ENOMEM);
	map->pg_temp = RB_ROOT;

	ceph_decode_16_safe(p, end, version, bad);
	if (version > 6) {
		pr_warning("got unknown v %d > 6 of osdmap\n", version);
		goto bad;
	}
	if (version < 6) {
		pr_warning("got old v %d < 6 of osdmap\n", version);
		goto bad;
	}

	ceph_decode_need(p, end, 2*sizeof(u64)+6*sizeof(u32), bad);
	ceph_decode_copy(p, &map->fsid, sizeof(map->fsid));
	map->epoch = ceph_decode_32(p);
	ceph_decode_copy(p, &map->created, sizeof(map->created));
	ceph_decode_copy(p, &map->modified, sizeof(map->modified));

	ceph_decode_32_safe(p, end, max, bad);
	while (max--) {
		ceph_decode_need(p, end, 8 + 2, bad);
		err = -ENOMEM;
		pi = kzalloc(sizeof(*pi), GFP_NOFS);
		if (!pi)
			goto bad;
		pi->id = ceph_decode_64(p);
		err = __decode_pool(p, end, pi);
		if (err < 0) {
			kfree(pi);
			goto bad;
		}
		__insert_pg_pool(&map->pg_pools, pi);
	}

	err = __decode_pool_names(p, end, map);
	if (err < 0) {
		dout("fail to decode pool names");
		goto bad;
	}

	ceph_decode_32_safe(p, end, map->pool_max, bad);

	ceph_decode_32_safe(p, end, map->flags, bad);

	max = ceph_decode_32(p);

	/* (re)alloc osd arrays */
	err = osdmap_set_max_osd(map, max);
	if (err < 0)
		goto bad;
	dout("osdmap_decode max_osd = %d\n", map->max_osd);

	/* osds */
	err = -EINVAL;
	ceph_decode_need(p, end, 3*sizeof(u32) +
			 map->max_osd*(1 + sizeof(*map->osd_weight) +
				       sizeof(*map->osd_addr)), bad);
	*p += 4; /* skip length field (should match max) */
	ceph_decode_copy(p, map->osd_state, map->max_osd);

	*p += 4; /* skip length field (should match max) */
	for (i = 0; i < map->max_osd; i++)
		map->osd_weight[i] = ceph_decode_32(p);

	*p += 4; /* skip length field (should match max) */
	ceph_decode_copy(p, map->osd_addr, map->max_osd*sizeof(*map->osd_addr));
	for (i = 0; i < map->max_osd; i++)
		ceph_decode_addr(&map->osd_addr[i]);

	/* pg_temp */
	ceph_decode_32_safe(p, end, len, bad);
	for (i = 0; i < len; i++) {
		int n, j;
		struct ceph_pg pgid;
		struct ceph_pg_mapping *pg;

		err = ceph_decode_pgid(p, end, &pgid);
		if (err)
			goto bad;
		ceph_decode_need(p, end, sizeof(u32), bad);
		n = ceph_decode_32(p);
		err = -EINVAL;
		if (n > (UINT_MAX - sizeof(*pg)) / sizeof(u32))
			goto bad;
		ceph_decode_need(p, end, n * sizeof(u32), bad);
		err = -ENOMEM;
		pg = kmalloc(sizeof(*pg) + n*sizeof(u32), GFP_NOFS);
		if (!pg)
			goto bad;
		pg->pgid = pgid;
		pg->len = n;
		for (j = 0; j < n; j++)
			pg->osds[j] = ceph_decode_32(p);

		err = __insert_pg_mapping(pg, &map->pg_temp);
		if (err)
			goto bad;
		dout(" added pg_temp %lld.%x len %d\n", pgid.pool, pgid.seed,
		     len);
	}

	/* crush */
	ceph_decode_32_safe(p, end, len, bad);
	dout("osdmap_decode crush len %d from off 0x%x\n", len,
	     (int)(*p - start));
	ceph_decode_need(p, end, len, bad);
	map->crush = crush_decode(*p, end);
	*p += len;
	if (IS_ERR(map->crush)) {
		err = PTR_ERR(map->crush);
		map->crush = NULL;
		goto bad;
	}

	/* ignore the rest of the map */
	*p = end;

	dout("osdmap_decode done %p %p\n", *p, end);
	return map;

bad:
	dout("osdmap_decode fail err %d\n", err);
	ceph_osdmap_destroy(map);
	return ERR_PTR(err);
}

/*
 * Encoding order is (new_up_client, new_state, new_weight).  Need to
 * apply in the (new_weight, new_state, new_up_client) order, because
 * an incremental map may look like e.g.
 *
 *     new_up_client: { osd=6, addr=... } # set osd_state and addr
 *     new_state: { osd=6, xorstate=EXISTS } # clear osd_state
 */
static int decode_new_up_state_weight(void **p, void *end,
				      struct ceph_osdmap *map)
{
	void *new_up_client;
	void *new_state;
	void *new_weight_end;
	u32 len;

	new_up_client = *p;
	ceph_decode_32_safe(p, end, len, e_inval);
	len *= sizeof(u32) + sizeof(struct ceph_entity_addr);
	ceph_decode_need(p, end, len, e_inval);
	*p += len;

	new_state = *p;
	ceph_decode_32_safe(p, end, len, e_inval);
	len *= sizeof(u32) + sizeof(u8);
	ceph_decode_need(p, end, len, e_inval);
	*p += len;

	/* new_weight */
	ceph_decode_32_safe(p, end, len, e_inval);
	while (len--) {
		s32 osd;
		u32 w;

		ceph_decode_need(p, end, 2*sizeof(u32), e_inval);
		osd = ceph_decode_32(p);
		w = ceph_decode_32(p);
		BUG_ON(osd >= map->max_osd);
		pr_info("osd%d weight 0x%x %s\n", osd, w,
		     w == CEPH_OSD_IN ? "(in)" :
		     (w == CEPH_OSD_OUT ? "(out)" : ""));
		map->osd_weight[osd] = w;

		/*
		 * If we are marking in, set the EXISTS, and clear the
		 * AUTOOUT and NEW bits.
		 */
		if (w) {
			map->osd_state[osd] |= CEPH_OSD_EXISTS;
			map->osd_state[osd] &= ~(CEPH_OSD_AUTOOUT |
						 CEPH_OSD_NEW);
		}
	}
	new_weight_end = *p;

	/* new_state (up/down) */
	*p = new_state;
	len = ceph_decode_32(p);
	while (len--) {
		s32 osd;
		u8 xorstate;

		osd = ceph_decode_32(p);
		xorstate = ceph_decode_8(p);
		if (xorstate == 0)
			xorstate = CEPH_OSD_UP;
		BUG_ON(osd >= map->max_osd);
		if ((map->osd_state[osd] & CEPH_OSD_UP) &&
		    (xorstate & CEPH_OSD_UP))
			pr_info("osd%d down\n", osd);
		if ((map->osd_state[osd] & CEPH_OSD_EXISTS) &&
		    (xorstate & CEPH_OSD_EXISTS)) {
			pr_info("osd%d does not exist\n", osd);
			memset(map->osd_addr + osd, 0, sizeof(*map->osd_addr));
			map->osd_state[osd] = 0;
		} else {
			map->osd_state[osd] ^= xorstate;
		}
	}

	/* new_up_client */
	*p = new_up_client;
	len = ceph_decode_32(p);
	while (len--) {
		s32 osd;
		struct ceph_entity_addr addr;

		osd = ceph_decode_32(p);
		ceph_decode_copy(p, &addr, sizeof(addr));
		ceph_decode_addr(&addr);
		BUG_ON(osd >= map->max_osd);
		pr_info("osd%d up\n", osd);
		map->osd_state[osd] |= CEPH_OSD_EXISTS | CEPH_OSD_UP;
		map->osd_addr[osd] = addr;
	}

	*p = new_weight_end;
	return 0;

e_inval:
	return -EINVAL;
}

/*
 * decode and apply an incremental map update.
 */
struct ceph_osdmap *osdmap_apply_incremental(void **p, void *end,
					     struct ceph_osdmap *map,
					     struct ceph_messenger *msgr)
{
	struct crush_map *newcrush = NULL;
	struct ceph_fsid fsid;
	u32 epoch = 0;
	struct ceph_timespec modified;
	s32 len;
	u64 pool;
	__s64 new_pool_max;
	__s32 new_flags, max;
	void *start = *p;
	int err = -EINVAL;
	u16 version;

	ceph_decode_16_safe(p, end, version, bad);
	if (version != 6) {
		pr_warning("got unknown v %d != 6 of inc osdmap\n", version);
		goto bad;
	}

	ceph_decode_need(p, end, sizeof(fsid)+sizeof(modified)+2*sizeof(u32),
			 bad);
	ceph_decode_copy(p, &fsid, sizeof(fsid));
	epoch = ceph_decode_32(p);
	BUG_ON(epoch != map->epoch+1);
	ceph_decode_copy(p, &modified, sizeof(modified));
	new_pool_max = ceph_decode_64(p);
	new_flags = ceph_decode_32(p);

	/* full map? */
	ceph_decode_32_safe(p, end, len, bad);
	if (len > 0) {
		dout("apply_incremental full map len %d, %p to %p\n",
		     len, *p, end);
		return osdmap_decode(p, min(*p+len, end));
	}

	/* new crush? */
	ceph_decode_32_safe(p, end, len, bad);
	if (len > 0) {
		dout("apply_incremental new crush map len %d, %p to %p\n",
		     len, *p, end);
		newcrush = crush_decode(*p, min(*p+len, end));
		if (IS_ERR(newcrush))
			return ERR_CAST(newcrush);
		*p += len;
	}

	/* new flags? */
	if (new_flags >= 0)
		map->flags = new_flags;
	if (new_pool_max >= 0)
		map->pool_max = new_pool_max;

	ceph_decode_need(p, end, 5*sizeof(u32), bad);

	/* new max? */
	max = ceph_decode_32(p);
	if (max >= 0) {
		err = osdmap_set_max_osd(map, max);
		if (err < 0)
			goto bad;
	}

	map->epoch++;
	map->modified = modified;
	if (newcrush) {
		if (map->crush)
			crush_destroy(map->crush);
		map->crush = newcrush;
		newcrush = NULL;
	}

	/* new_pool */
	ceph_decode_32_safe(p, end, len, bad);
	while (len--) {
		struct ceph_pg_pool_info *pi;

		ceph_decode_64_safe(p, end, pool, bad);
		pi = __lookup_pg_pool(&map->pg_pools, pool);
		if (!pi) {
			pi = kzalloc(sizeof(*pi), GFP_NOFS);
			if (!pi) {
				err = -ENOMEM;
				goto bad;
			}
			pi->id = pool;
			__insert_pg_pool(&map->pg_pools, pi);
		}
		err = __decode_pool(p, end, pi);
		if (err < 0)
			goto bad;
	}
	if (version >= 5) {
		err = __decode_pool_names(p, end, map);
		if (err < 0)
			goto bad;
	}

	/* old_pool */
	ceph_decode_32_safe(p, end, len, bad);
	while (len--) {
		struct ceph_pg_pool_info *pi;

		ceph_decode_64_safe(p, end, pool, bad);
		pi = __lookup_pg_pool(&map->pg_pools, pool);
		if (pi)
			__remove_pg_pool(&map->pg_pools, pi);
	}

	/* new_up_client, new_state, new_weight */
	err = decode_new_up_state_weight(p, end, map);
	if (err)
		goto bad;

	/* new_pg_temp */
	ceph_decode_32_safe(p, end, len, bad);
	while (len--) {
		struct ceph_pg_mapping *pg;
		int j;
		struct ceph_pg pgid;
		u32 pglen;

		err = ceph_decode_pgid(p, end, &pgid);
		if (err)
			goto bad;
		ceph_decode_need(p, end, sizeof(u32), bad);
		pglen = ceph_decode_32(p);
		if (pglen) {
			ceph_decode_need(p, end, pglen*sizeof(u32), bad);

			/* removing existing (if any) */
			(void) __remove_pg_mapping(&map->pg_temp, pgid);

			/* insert */
			err = -EINVAL;
			if (pglen > (UINT_MAX - sizeof(*pg)) / sizeof(u32))
				goto bad;
			err = -ENOMEM;
			pg = kmalloc(sizeof(*pg) + sizeof(u32)*pglen, GFP_NOFS);
			if (!pg)
				goto bad;
			pg->pgid = pgid;
			pg->len = pglen;
			for (j = 0; j < pglen; j++)
				pg->osds[j] = ceph_decode_32(p);
			err = __insert_pg_mapping(pg, &map->pg_temp);
			if (err) {
				kfree(pg);
				goto bad;
			}
			dout(" added pg_temp %lld.%x len %d\n", pgid.pool,
			     pgid.seed, pglen);
		} else {
			/* remove */
			__remove_pg_mapping(&map->pg_temp, pgid);
		}
	}

	/* ignore the rest */
	*p = end;
	return map;

bad:
	pr_err("corrupt inc osdmap epoch %d off %d (%p of %p-%p)\n",
	       epoch, (int)(*p - start), *p, start, end);
	print_hex_dump(KERN_DEBUG, "osdmap: ",
		       DUMP_PREFIX_OFFSET, 16, 1,
		       start, end - start, true);
	if (newcrush)
		crush_destroy(newcrush);
	return ERR_PTR(err);
}




/*
 * calculate file layout from given offset, length.
 * fill in correct oid, logical length, and object extent
 * offset, length.
 *
 * for now, we write only a single su, until we can
 * pass a stride back to the caller.
 */
int ceph_calc_file_object_mapping(struct ceph_file_layout *layout,
				   u64 off, u64 len,
				   u64 *ono,
				   u64 *oxoff, u64 *oxlen)
{
	u32 osize = le32_to_cpu(layout->fl_object_size);
	u32 su = le32_to_cpu(layout->fl_stripe_unit);
	u32 sc = le32_to_cpu(layout->fl_stripe_count);
	u32 bl, stripeno, stripepos, objsetno;
	u32 su_per_object;
	u64 t, su_offset;

	dout("mapping %llu~%llu  osize %u fl_su %u\n", off, len,
	     osize, su);
	if (su == 0 || sc == 0)
		goto invalid;
	su_per_object = osize / su;
	if (su_per_object == 0)
		goto invalid;
	dout("osize %u / su %u = su_per_object %u\n", osize, su,
	     su_per_object);

	if ((su & ~PAGE_MASK) != 0)
		goto invalid;

	/* bl = *off / su; */
	t = off;
	do_div(t, su);
	bl = t;
	dout("off %llu / su %u = bl %u\n", off, su, bl);

	stripeno = bl / sc;
	stripepos = bl % sc;
	objsetno = stripeno / su_per_object;

	*ono = objsetno * sc + stripepos;
	dout("objset %u * sc %u = ono %u\n", objsetno, sc, (unsigned int)*ono);

	/* *oxoff = *off % layout->fl_stripe_unit;  # offset in su */
	t = off;
	su_offset = do_div(t, su);
	*oxoff = su_offset + (stripeno % su_per_object) * su;

	/*
	 * Calculate the length of the extent being written to the selected
	 * object. This is the minimum of the full length requested (len) or
	 * the remainder of the current stripe being written to.
	 */
	*oxlen = min_t(u64, len, su - su_offset);

	dout(" obj extent %llu~%llu\n", *oxoff, *oxlen);
	return 0;

invalid:
	dout(" invalid layout\n");
	*ono = 0;
	*oxoff = 0;
	*oxlen = 0;
	return -EINVAL;
}
EXPORT_SYMBOL(ceph_calc_file_object_mapping);

/*
 * calculate an object layout (i.e. pgid) from an oid,
 * file_layout, and osdmap
 */
int ceph_calc_ceph_pg(struct ceph_pg *pg, const char *oid,
			struct ceph_osdmap *osdmap, uint64_t pool)
{
	struct ceph_pg_pool_info *pool_info;

	BUG_ON(!osdmap);
	pool_info = __lookup_pg_pool(&osdmap->pg_pools, pool);
	if (!pool_info)
		return -EIO;
	pg->pool = pool;
	pg->seed = ceph_str_hash(pool_info->object_hash, oid, strlen(oid));

	dout("%s '%s' pgid %lld.%x\n", __func__, oid, pg->pool, pg->seed);
	return 0;
}
EXPORT_SYMBOL(ceph_calc_ceph_pg);

/*
 * Calculate raw osd vector for the given pgid.  Return pointer to osd
 * array, or NULL on failure.
 */
static int *calc_pg_raw(struct ceph_osdmap *osdmap, struct ceph_pg pgid,
			int *osds, int *num)
{
	struct ceph_pg_mapping *pg;
	struct ceph_pg_pool_info *pool;
	int ruleno;
	int r;
	u32 pps;

	pool = __lookup_pg_pool(&osdmap->pg_pools, pgid.pool);
	if (!pool)
		return NULL;

	/* pg_temp? */
	pgid.seed = ceph_stable_mod(pgid.seed, pool->pg_num,
				    pool->pg_num_mask);
	pg = __lookup_pg_mapping(&osdmap->pg_temp, pgid);
	if (pg) {
		*num = pg->len;
		return pg->osds;
	}

	/* crush */
	ruleno = crush_find_rule(osdmap->crush, pool->crush_ruleset,
				 pool->type, pool->size);
	if (ruleno < 0) {
		pr_err("no crush rule pool %lld ruleset %d type %d size %d\n",
		       pgid.pool, pool->crush_ruleset, pool->type,
		       pool->size);
		return NULL;
	}

	if (pool->flags & CEPH_POOL_FLAG_HASHPSPOOL) {
		/* hash pool id and seed sothat pool PGs do not overlap */
		pps = crush_hash32_2(CRUSH_HASH_RJENKINS1,
				     ceph_stable_mod(pgid.seed, pool->pgp_num,
						     pool->pgp_num_mask),
				     pgid.pool);
	} else {
		/*
		 * legacy ehavior: add ps and pool together.  this is
		 * not a great approach because the PGs from each pool
		 * will overlap on top of each other: 0.5 == 1.4 ==
		 * 2.3 == ...
		 */
		pps = ceph_stable_mod(pgid.seed, pool->pgp_num,
				      pool->pgp_num_mask) +
			(unsigned)pgid.pool;
	}
	r = crush_do_rule(osdmap->crush, ruleno, pps, osds,
			  min_t(int, pool->size, *num),
			  osdmap->osd_weight);
	if (r < 0) {
		pr_err("error %d from crush rule: pool %lld ruleset %d type %d"
		       " size %d\n", r, pgid.pool, pool->crush_ruleset,
		       pool->type, pool->size);
		return NULL;
	}
	*num = r;
	return osds;
}

/*
 * Return acting set for given pgid.
 */
int ceph_calc_pg_acting(struct ceph_osdmap *osdmap, struct ceph_pg pgid,
			int *acting)
{
	int rawosds[CEPH_PG_MAX_SIZE], *osds;
	int i, o, num = CEPH_PG_MAX_SIZE;

	osds = calc_pg_raw(osdmap, pgid, rawosds, &num);
	if (!osds)
		return -1;

	/* primary is first up osd */
	o = 0;
	for (i = 0; i < num; i++)
		if (ceph_osd_is_up(osdmap, osds[i]))
			acting[o++] = osds[i];
	return o;
}

/*
 * Return primary osd for given pgid, or -1 if none.
 */
int ceph_calc_pg_primary(struct ceph_osdmap *osdmap, struct ceph_pg pgid)
{
	int rawosds[CEPH_PG_MAX_SIZE], *osds;
	int i, num = CEPH_PG_MAX_SIZE;

	osds = calc_pg_raw(osdmap, pgid, rawosds, &num);
	if (!osds)
		return -1;

	/* primary is first up osd */
	for (i = 0; i < num; i++)
		if (ceph_osd_is_up(osdmap, osds[i]))
			return osds[i];
	return -1;
}
EXPORT_SYMBOL(ceph_calc_pg_primary);
