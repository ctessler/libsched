#ifndef STRING_LIST_H
#define STRING_LIST_H

#include <stdint.h>
#include <stdlib.h>
#include <sys/queue.h>
#include "dag-task.h"

/**
 * @file dag-node-list.h A list of DAG nodes
 */

/* Needed before including taskset.h */
typedef LIST_HEAD(sl_head, sl_elem) sl_t;

typedef struct sl_elem {
	LIST_ENTRY(sl_elem) sl_glue;	/**< Queue glue */
	char* sl_str;			/**< String in the list */
} sl_elem_t;

/**
 * Initializes the list
 *
 * Usage:
 *     sl_t *head;
 *     sl_init(head);
 *
 * @param[in|out] head head of the list, must be a pointer.
 */
#define sl_init(head) LIST_INIT(head)
#define sl_empty(head) LIST_EMPTY(head)

/**
 * Adds an element to the head of the list
 *
 * Usage:
 *   sl_t *head;
 *   sl_elem_t *elem = sle_alloc();
 *   sl_insert_head(head, elem);
 *
 * @param[in] head list head
 * @param[in] elem item being added to the head
 */
#define sl_insert_head(head, elem) LIST_INSERT_HEAD(head, elem, sl_glue)

/**
 * Adds an element before or after another in the list
 *
 * Usage:
 *    sl_t *head;
 *    sl_elem_t *elem = sle_alloc();
 *    sl_insert_head(head, elem);
 *    sl_insert_after(elem, sle_alloc());
 *    sl_insert_before(elem, sle_alloc());
 *
 * @param[in] elem the element being added to the list
 * @param[in] onlist the element already on the list
 */
#define sl_insert_after(onlist, elem) \
	LIST_INSERT_AFTER(onlist, elem, sl_glue)
#define sl_insert_before(onlist, elem) \
	LIST_INSERT_BEFORE(onlist, elem, sl_glue)

/**
 * Removes an element from the list
 *
 * @param[in] elem the element being removed
 */
#define sl_remove(elem) LIST_REMOVE(elem, sl_glue)

/**
 * Adds an element to the list 
 *
 * @param[in] head the head of the list
 * @param[in] elem the element being added
 */
int sl_insert(sl_t* head, sl_elem_t *elem);

/**
 * Foreach element
 *
 * Usage:
 *    sl_elem_t *elem, *t;
 *    sl_t head;
 *
 *    sl_foreach(head, elem) {
 *        ... do stuff with elem
 *            but don't delete it ...
 *    }
 *
 * @param[in] head head of the list
 * @param[in] elem current element
 */
#define sl_foreach(head, elem) LIST_FOREACH(elem, head, sl_glue)

/**
 * First element of the list
 * 
 * @param[in] head the head of the list
 */
#define sl_first(head) LIST_FIRST(head)

/**
 * Next element of the list
 *
 * @param[in] elem the element on the list
 *
 * @return the next element on the list
 */
#define sl_next(elem) LIST_NEXT(elem, sl_glue)

/**
 * Removes all deadlines from the ordered list, it dsles *not* free the
 * tasks associated weth the deadlines. 
 *
 * @param[out] head the head of the list to be cleared, will be
 *     sl_init()'d before returned;
 */
void sl_clear(sl_t *head);

/**
 * Finds an element in the list
 *
 * @param[in] head list head
 * @param[in] str the string
 *
 * @return NULL if not found, the element otherwise.
 */
sl_elem_t *sl_find(sl_t* head, char *str);

/**
 * Allocate a new element
 *
 * Usage:
 *    dnode_t *node;
 *    ...
 *    dnode_update(node);
 *    sl_elem_t *item = sle_alloc(node);
 *    // node is copied upon insertion 
 *    dnode_free(node)
 *
 */
sl_elem_t* sle_alloc();
sl_elem_t* sle_free(sl_elem_t* e);
sl_elem_t* sle_copy(sl_elem_t *e);

/**
 * Sets the string of the node
 *
 * @param[in] e the element being set
 * @param[in] str the string
 */
void sle_set(sl_elem_t *e, char *str);

/**
 * Appends (uniquely) the nodes of nlist to orig
 *
 * @param[in|out] orig the list being appended to
 * @param[in] nlist the new list being appended to a
 *
 * @return the number of nodes appended
 */
int sl_append(sl_t* orig, sl_t* nlist);



#endif /* STRING_LIST_H */
