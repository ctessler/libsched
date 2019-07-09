#ifndef DAG_NODE_LIST_H
#define DAG_NODE_LIST_H

#include <stdint.h>
#include <stdlib.h>
#include <sys/queue.h>
#include "dag-task.h"

/**
 * @file dag-node-list.h A list of DAG nodes
 */

/* Needed before including taskset.h */
typedef LIST_HEAD(dnl_head, dnl_elem) dnl_t;

typedef struct dnl_elem {
	LIST_ENTRY(dnl_elem) dnl_glue;	/**< Queue glue */
	dnode_t *dnl_node;		/**< Node */
} dnl_elem_t;

/**
 * Initializes the list
 *
 * Usage:
 *     dnl_t *head;
 *     dnl_init(head);
 *
 * @param[in|out] head head of the list, must be a pointer.
 */
#define dnl_init(head) LIST_INIT(head)
#define dnl_empty(head) LIST_EMPTY(head)

/**
 * Adds an element to the head of the list
 *
 * Usage:
 *   dnl_t *head;
 *   dnl_elem_t *elem = dnle_alloc();
 *   dnl_insert_head(head, elem);
 *
 * @param[in] head list head
 * @param[in] elem item being added to the head
 */
#define dnl_insert_head(head, elem) LIST_INSERT_HEAD(head, elem, dnl_glue)

/**
 * Adds an element before or after another in the list
 *
 * Usage:
 *    dnl_t *head;
 *    dnl_elem_t *elem = dnle_alloc();
 *    dnl_insert_head(head, elem);
 *    dnl_insert_after(elem, dnle_alloc());
 *    dnl_insert_before(elem, dnle_alloc());
 *
 * @param[in] elem the element being added to the list
 * @param[in] onlist the element already on the list
 */
#define dnl_insert_after(onlist, elem) \
	LIST_INSERT_AFTER(onlist, elem, dnl_glue)
#define dnl_insert_before(onlist, elem) \
	LIST_INSERT_BEFORE(onlist, elem, dnl_glue)

/**
 * Removes an element from the list
 *
 * @param[in] elem the element being removed
 */
#define dnl_remove(elem) LIST_REMOVE(elem, dnl_glue)

/**
 * Adds an element to the list according to the value of its absolute deadline.
 *
 * @param[in] head the head of the list
 * @param[in] elem the element being added
 */
int dnl_insert(dnl_t* head, dnl_elem_t *elem);

/**
 * Foreach element
 *
 * Usage:
 *    dnl_elem_t *elem, *t;
 *    dnl_t head;
 *
 *    dnl_foreach(head, elem) {
 *        ... do stuff with elem
 *            but don't delete it ...
 *    }
 *
 * @param[in] head head of the list
 * @param[in] elem current element
 */
#define dnl_foreach(head, elem) LIST_FOREACH(elem, head, dnl_glue)

/**
 * First element of the list
 * 
 * @param[in] head the head of the list
 */
#define dnl_first(head) LIST_FIRST(head)

/**
 * Next element of the list
 *
 * @param[in] elem the element on the list
 *
 * @return the next element on the list
 */
#define dnl_next(elem) LIST_NEXT(elem, dnl_glue)

/**
 * Removes all deadlines from the ordered list, it ddnles *not* free the
 * tasks associated weth the deadlines. 
 *
 * @param[out] head the head of the list to be cleared, will be
 *     dnl_init()'d before returned;
 */
void dnl_clear(dnl_t *head);

/**
 * Finds the element of the deadline
 *
 * @param[in] head list head
 * @param[in] name the name of the node
 *
 * @return NULL if not found, the element otherwise.
 */
dnl_elem_t *dnl_find(dnl_t* head, char *name);

/**
 * Allocate a new element
 *
 * Usage:
 *    dnode_t *node;
 *    ...
 *    dnode_update(node);
 *    dnl_elem_t *item = dnle_alloc(node);
 *    // node is copied upon insertion 
 *    dnode_free(node)
 *
 */
dnl_elem_t* dnle_alloc(dnode_t* node);
dnl_elem_t* dnle_free(dnl_elem_t* e);
dnl_elem_t* dnle_copy(dnl_elem_t *e);

/**
 * Gets the list of immediate predecessors to this node
 *
 * @param[in] node the destination node
 *
 * @return a list of nodes with edges ending at the given node 
 */
dnl_t *dnl_preds(dnode_t *node);

/**
 * Gets the list of immediate successors of this nodes
 *
 * @param[in] node the source node
 *
 * @return a list of nodes with edges starting at the given node 
 */
dnl_t *dnl_succs(dnode_t *node);

/**
 * Gets the list of nodes with the given object
 *
 * @param[in] task the task
 * @param[in] object the object id
 *
 * @return a new list of nodes that share the same object
 */
dnl_t *dnl_by_obj(dtask_t *task, tint_t object);

/**
 * Appends (uniquely) the nodes of nlist to orig
 *
 * @param[in|out] orig the list being appended to
 * @param[in] nlist the new list being appended to a
 *
 * @return the number of nodes appended
 */
int dnl_append(dnl_t* orig, dnl_t* nlist);

/**
 * Returns the number of common nodes among two lists
 *
 * @param[in] a list
 * @param[in] b another list
 *
 * @return the number of common nodes
 */
int dnl_sharedc(dnl_t *a, dnl_t *b);


#endif /* DAG_NODE_LIST_H */
