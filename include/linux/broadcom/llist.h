/****************************************************************************
*
*	Copyright (c) 2006-2009 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license
*   agreement governing use of this software, this software is licensed to you
*   under the terms of the GNU General Public License version 2, available
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
*   Notwithstanding the above, under no circumstances may you combine this
*   software in any way with any other Broadcom software provided under a
*   license other than the GPL, without Broadcom's express prior written
*   consent.
*
****************************************************************************/

/**
*
*  @file    llist.h
*
*  @brief   Definitions for linked lists
*
****************************************************************************/
#if !defined( LLIST_H )
#define LLIST_H

/* ---- Include Files ---------------------------------------------------- */
/**
 * @addtogroup LIST
 * @{
 */

/* ---- Constants and Types ---------------------------------------------- */

/* Set to 1 if the linked list nodes are guaranteed to be declared
 * as the first member of a parent container. By allowing this assumption,
 * linked list operators can be simplified and are thus more optimal.
 */
#define LLIST_NODE_FIRST_IN_CONTAINER        1

/* Linked list node structure. To make use of this structure, it must
 * be declared as the first item within a parent structure if
 * LLIST_NODE_FIRST_IN_CONTAINER is enabled.
 */
typedef struct llist_node {
	struct llist_node *next;	/* Pointer to next node */
} LLIST_NODE;

/* Helper macro to iterate through the nodes of a linked list.
 *
 * @param nodep   Pointer to the list node of type (LLIST_NODE *)
 * @param headp   Pointer to the head of the list of type (LLIST_NODE *)
 */
#define llist_foreach_node( nodep, headp ) \
   for ( nodep = (headp)->next; nodep != NULL; nodep = (nodep)->next )

/* Macro is used to determine the pointer to the parent container structure.
 * Two versions are provided. The "no check" just returns the parent
 * container pointer without checking whether the input ptr is valid. The
 * "check" version will return NULL if the input ptr is NULL.
 *
 * @param ptr           Pointer to a member within a parent structure
 * @param parent_type   Type of the parent structure
 * @param member        Name of member within a parent structure
 */
#define parent_container_nocheck( ptr, parent_type, member ) \
   (parent_type *)((char *)ptr - (char *)&(((parent_type *)0)->member))

#define parent_container_check( ptr, parent_type, member ) \
   (parent_type *)((ptr) ? ((char *)ptr - (char *)&(((parent_type *)0)->member)) : 0 )

#if LLIST_NODE_FIRST_IN_CONTAINER

/* Helper macro to iterate through each list item.
 *
 * Assume that the node is at the beginner of the parent container.
 *
 * @param itemp   Pointer to containing structure item that is linked
 * @param headp   Pointer to the head of the list
 * @param member  Name of list node structure within containing structure
 */
#define llist_foreach_item( itemp, headp, member ) \
   for ( itemp = (typeof(itemp))((headp)->next); \
         itemp != NULL; \
         itemp = (typeof(itemp))(itemp->member.next) )

/* Helper macro to iterate through each list item safely to guard against
 * removal of item from the list
 *
 * Assume that the node is at the beginner of the parent container.
 *
 * @param itemp   Pointer to containing structure item that is linked
 * @param headp   Pointer to the head of the list
 * @param member  Name of list node structure within containing structure
 */
#define llist_foreach_item_safe( itemp, tmp, headp, member ) \
   for ( itemp = (typeof(itemp))((headp)->next), \
         tmp =  itemp ? (typeof(itemp))(itemp->member.next) : NULL; \
         itemp != NULL; \
         itemp = tmp, \
         tmp = itemp ? (typeof(itemp))(itemp->member.next) : NULL )

#else

/* Helper macro to iterate through each list item
 *
 * @param itemp   Pointer to containing structure item that is linked
 * @param headp   Pointer to the head of the list
 * @param member  Name of list node structure within containing structure
 */
#define llist_foreach_item( itemp, headp, member ) \
   for ( itemp = parent_container_check( (headp)->next, typeof(*itemp), member ); \
         itemp != NULL; \
         itemp = parent_container_check( itemp->member.next, typeof(*itemp), member ))

/* Helper macro to iterate through each list item safely to guard against
 * removal of item from the list
 *
 * @param itemp   Pointer to containing structure item that is linked
 * @param headp   Pointer to the head of the list
 * @param member  Name of list node structure within containing structure
 */
#define llist_foreach_item_safe( itemp, tmp, headp, member ) \
   for ( itemp = parent_container_check( (headp)->next, typeof(*itemp), member ), \
         tmp = parent_container_check( itemp->member.next, typeof(*itemp), member ); \
         itemp != NULL; \
         itemp = tmp, \
         tmp = itemp ? parent_container_check( itemp->member.next, typeof(*itemp), member ) : NULL )

#endif

/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */

/***************************************************************************/
/**
*  Initialize list head
*
*  @return  Nothing
*/
static inline void llist_head_init(LLIST_NODE *headp)
{
	headp->next = NULL;
}

/***************************************************************************/
/**
*  Add item to tail of list
*
*  @return  Nothing
*/
static inline void llist_add_tail(LLIST_NODE *itemp, LLIST_NODE *headp)
{
	LLIST_NODE *lnodep;

	itemp->next = NULL;

	for (lnodep = headp; lnodep != NULL; lnodep = lnodep->next) {
		if (lnodep->next == NULL) {
			break;
		}
	}

	lnodep->next = itemp;
}

/***************************************************************************/
/**
*  Add item to the front of the list
*
*  @return  Nothing
*/
static inline void llist_add_head(LLIST_NODE *itemp, LLIST_NODE *headp)
{
	itemp->next = headp->next;
	headp->next = itemp;
}

/***************************************************************************/
/**
*  Delete item from list if it exists
*
*  @return  1 if node deleted, otherwise 0 if node is not part of list
*/
static inline int llist_del(const LLIST_NODE *itemp, LLIST_NODE *headp)
{
	LLIST_NODE *lnodep;

	/* Find the node previous to the one to delete, and delete the node */
	for (lnodep = headp; lnodep != NULL; lnodep = lnodep->next) {
		if (lnodep->next == itemp) {
			lnodep->next = itemp->next;
			return 1;	/* node deleted */
		}
	}
	return 0;
}

/***************************************************************************/
/**
*  Add item after an existing node.
*
*  @return  Nothing
*/
static inline void llist_add(LLIST_NODE *itemp,	/*<< (i) New item to add to list */
			     LLIST_NODE *lnodep	/*<< (i) Add item after this node. */
    )
{
	itemp->next = lnodep->next;
	lnodep->next = itemp;
}

/** @} */

#endif /* LLIST_H */
