
#define pr_fmt(fmt) "list_sort_test: " fmt

#include <linux/kernel.h>
#include <linux/bug.h>
#include <linux/compiler.h>
#include <linux/export.h>
#include <linux/string.h>
#include <linux/list_sort.h>
#include <linux/list.h>

typedef int __attribute__((nonnull(2,3))) (*cmp_func)(void *,
		struct list_head const *, struct list_head const *);

/*
 * Returns a list organized in an intermediate format suited
 * to chaining of merge() calls: null-terminated, no reserved or
 * sentinel head node, "prev" links not maintained.
 */
__attribute__((nonnull(2,3,4)))
static struct list_head *merge(void *priv, cmp_func cmp,
				struct list_head *a, struct list_head *b)
{
	struct list_head *head, **tail = &head;

	for (;;) {
		/* if equal, take 'a' -- important for sort stability */
		if (cmp(priv, a, b) <= 0) {
			*tail = a;
			tail = &a->next;
			a = a->next;
			if (!a) {
				*tail = b;
				break;
			}
		} else {
			*tail = b;
			tail = &b->next;
			b = b->next;
			if (!b) {
				*tail = a;
				break;
			}
		}
	}
	return head;
}

/*
 * Combine final list merge with restoration of standard doubly-linked
 * list structure.  This approach duplicates code from merge(), but
 * runs faster than the tidier alternatives of either a separate final
 * prev-link restoration pass, or maintaining the prev links
 * throughout.
 */
__attribute__((nonnull(2,3,4,5)))
static void merge_final(void *priv, cmp_func cmp, struct list_head *head,
			struct list_head *a, struct list_head *b)
{
	struct list_head *tail = head;
	u8 count = 0;

	for (;;) {
		/* if equal, take 'a' -- important for sort stability */
		if (cmp(priv, a, b) <= 0) {
			tail->next = a;
			a->prev = tail;
			tail = a;
			a = a->next;
			if (!a)
				break;
		} else {
			tail->next = b;
			b->prev = tail;
			tail = b;
			b = b->next;
			if (!b) {
				b = a;
				break;
			}
		}
	}

	/* Finish linking remainder of list b on to tail */
	tail->next = b;
	do {
		/*
		 * If the merge is highly unbalanced (e.g. the input is
		 * already sorted), this loop may run many iterations.
		 * Continue callbacks to the client even though no
		 * element comparison is needed, so the client's cmp()
		 * routine can invoke cond_resched() periodically.
		 */
		if (unlikely(!++count))
			cmp(priv, b, b);
		b->prev = tail;
		tail = b;
		b = b->next;
	} while (b);

	/* And the final links to make a circular doubly-linked list */
	tail->next = head;
	head->prev = tail;
}

/**
 * list_sort - sort a list
 * @priv: private data, opaque to list_sort(), passed to @cmp
 * @head: the list to sort
 * @cmp: the elements comparison function
 *
 * This function implements a bottom-up merge sort, which has O(nlog(n))
 * complexity.  We use depth-first order to take advantage of cacheing.
 * (E.g. when we get to the fourth element, we immediately merge the
 * first two 2-element lists.)
 *
 * The comparison funtion @cmp must return > 0 if @a should sort after
 * @b ("@a > @b" if you want an ascending sort), and <= 0 if @a should
 * sort before @b *or* their original order should be preserved.  It is
 * always called with the element that came first in the input in @a,
 * and list_sort is a stable sort, so it is not necessary to distinguish
 * the @a < @b and @a == @b cases.
 *
 * This is compatible with two styles of @cmp function:
 * - The traditional style which returns <0 / =0 / >0, or
 * - Returning a boolean 0/1.
 * The latter offers a chance to save a few cycles in the comparison
 * (which is used by e.g. plug_ctx_cmp() in block/blk-mq.c).
 *
 * A good way to write a multi-word comparison is
 *	if (a->high != b->high)
 *		return a->high > b->high;
 *	if (a->middle != b->middle)
 *		return a->middle > b->middle;
 *	return a->low > b->low;
 */
__attribute__((nonnull(2,3)))
void list_sort(void *priv, struct list_head *head,
		int (*cmp)(void *priv, struct list_head *a,
			struct list_head *b))
{
	struct list_head *list = head->next, *pending = NULL;
	size_t count = 0;	/* Count of pending */

	if (list == head->prev)	/* Zero or one elements */
		return;

	/* Convert to a null-terminated singly-linked list. */
	head->prev->next = NULL;

	/*
	 * Data structure invariants:
	 * - All lists are singly linked and null-terminated; prev
	 *   pointers are not maintained.
	 * - pending is a prev-linked "list of lists" of sorted
	 *   sublists awaiting further merging.
	 * - Each of the sorted sublists is power-of-two in size,
	 *   corresponding to bits set in "count".
	 * - Sublists are sorted by size and age, smallest & newest at front.
	 */
	do {
		size_t bits;
		struct list_head *cur = list;

		/* Extract the head of "list" as a single-element list "cur" */
		list = list->next;
		cur->next = NULL;

		/* Do merges corresponding to set lsbits in count */
		for (bits = count; bits & 1; bits >>= 1) {
			cur = merge(priv, (cmp_func)cmp, pending, cur);
			pending = pending->prev;  /* Untouched by merge() */
		}
		/* And place the result at the head of "pending" */
		cur->prev = pending;
		pending = cur;
		count++;
	} while (list->next);

	/* Now merge together last element with all pending lists */
	while (pending->prev) {
		list = merge(priv, (cmp_func)cmp, pending, list);
		pending = pending->prev;
	}
	/* The final merge, rebuilding prev links */
	merge_final(priv, (cmp_func)cmp, head, pending, list);
}
EXPORT_SYMBOL(list_sort);

#ifdef CONFIG_TEST_LIST_SORT

#include <linux/slab.h>
#include <linux/random.h>

/*
 * The pattern of set bits in the list length determines which cases
 * are hit in list_sort().
 */
#define TEST_LIST_LEN (512+128+2) /* not including head */

#define TEST_POISON1 0xDEADBEEF
#define TEST_POISON2 0xA324354C

struct debug_el {
	unsigned int poison1;
	struct list_head list;
	unsigned int poison2;
	int value;
	unsigned serial;
};

/* Array, containing pointers to all elements in the test list */
static struct debug_el **elts __initdata;

static int __init check(struct debug_el *ela, struct debug_el *elb)
{
	if (ela->serial >= TEST_LIST_LEN) {
		pr_err("error: incorrect serial %d\n", ela->serial);
		return -EINVAL;
	}
	if (elb->serial >= TEST_LIST_LEN) {
		pr_err("error: incorrect serial %d\n", elb->serial);
		return -EINVAL;
	}
	if (elts[ela->serial] != ela || elts[elb->serial] != elb) {
		pr_err("error: phantom element\n");
		return -EINVAL;
	}
	if (ela->poison1 != TEST_POISON1 || ela->poison2 != TEST_POISON2) {
		pr_err("error: bad poison: %#x/%#x\n",
			ela->poison1, ela->poison2);
		return -EINVAL;
	}
	if (elb->poison1 != TEST_POISON1 || elb->poison2 != TEST_POISON2) {
		pr_err("error: bad poison: %#x/%#x\n",
			elb->poison1, elb->poison2);
		return -EINVAL;
	}
	return 0;
}

static int __init cmp(void *priv, struct list_head *a, struct list_head *b)
{
	struct debug_el *ela, *elb;

	ela = container_of(a, struct debug_el, list);
	elb = container_of(b, struct debug_el, list);

	check(ela, elb);
	return ela->value - elb->value;
}

static int __init list_sort_test(void)
{
	int i, count = 1, err = -ENOMEM;
	struct debug_el *el;
	struct list_head *cur;
	LIST_HEAD(head);

	pr_debug("start testing list_sort()\n");

	elts = kcalloc(TEST_LIST_LEN, sizeof(*elts), GFP_KERNEL);
	if (!elts) {
		pr_err("error: cannot allocate memory\n");
		return err;
	}

	for (i = 0; i < TEST_LIST_LEN; i++) {
		el = kmalloc(sizeof(*el), GFP_KERNEL);
		if (!el) {
			pr_err("error: cannot allocate memory\n");
			goto exit;
		}
		 /* force some equivalencies */
		el->value = prandom_u32() % (TEST_LIST_LEN / 3);
		el->serial = i;
		el->poison1 = TEST_POISON1;
		el->poison2 = TEST_POISON2;
		elts[i] = el;
		list_add_tail(&el->list, &head);
	}

	list_sort(NULL, &head, cmp);

	err = -EINVAL;
	for (cur = head.next; cur->next != &head; cur = cur->next) {
		struct debug_el *el1;
		int cmp_result;

		if (cur->next->prev != cur) {
			pr_err("error: list is corrupted\n");
			goto exit;
		}

		cmp_result = cmp(NULL, cur, cur->next);
		if (cmp_result > 0) {
			pr_err("error: list is not sorted\n");
			goto exit;
		}

		el = container_of(cur, struct debug_el, list);
		el1 = container_of(cur->next, struct debug_el, list);
		if (cmp_result == 0 && el->serial >= el1->serial) {
			pr_err("error: order of equivalent elements not "
				"preserved\n");
			goto exit;
		}

		if (check(el, el1)) {
			pr_err("error: element check failed\n");
			goto exit;
		}
		count++;
	}
	if (head.prev != cur) {
		pr_err("error: list is corrupted\n");
		goto exit;
	}


	if (count != TEST_LIST_LEN) {
		pr_err("error: bad list length %d", count);
		goto exit;
	}

	err = 0;
exit:
	for (i = 0; i < TEST_LIST_LEN; i++)
		kfree(elts[i]);
	kfree(elts);
	return err;
}
late_initcall(list_sort_test);
#endif /* CONFIG_TEST_LIST_SORT */
