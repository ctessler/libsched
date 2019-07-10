#ifndef DAG_CANDIDATE_H
#define DAG_CANDIDATE_H

#include "dag-node-list.h"
#include "string-list.h"
#include "dag-collapse.h"

/* Needed before including taskset.h */
typedef LIST_HEAD(cand_head, cand) cand_list_t;

typedef struct cand {
	LIST_ENTRY(cand) cand_glue;	/**< Queue glue */
	dnode_t *c_a;
	dnode_t *c_b;
	int c_delta_c;	/** Workload change */
	int c_delta_l;  /** Critical path length change */
} cand_t;

cand_list_t *cand_list_alloc();
cand_list_t *cand_list_free(cand_list_t *head);
#define cand_list_init(head) LIST_INIT(head)
#define cand_list_empty(head) LIST_EMPTY(head)

/**
 * Generates the name of the candidate
 *
 * Must not be free()'d or reused by the caller
 */
char* cand_name(cand_t *c);

/**
 * Gets the next candidate from the task
 *
 * @param[in] task the dag task
 * @param[in] c the current candidate
 *
 * @return the next candidate if one exists, NULL otherwise
 */
cand_t* task_cand_next(dtask_t* task, cand_t *c);

/**
 * Adds an element to the head of the list
 *
 * Usage:
 *   cand_list_t *head;
 *   cand_t *elem = cande_alloc();
 *   cand_insert_head(head, elem);
 *
 * @param[in] head list head
 * @param[in] elem item being added to the head
 */
#define cand_insert_head(head, elem) LIST_INSERT_HEAD(head, elem, cand_glue)

/**
 * Adds an element before or after another in the list
 *
 * Usage:
 *    cand_list_t *head;
 *    cand_t *elem = cande_alloc();
 *    cand_insert_head(head, elem);
 *    cand_insert_after(elem, cande_alloc());
 *    cand_insert_before(elem, cande_alloc());
 *
 * @param[in] elem the element being added to the list
 * @param[in] onlist the element already on the list
 */
#define cand_insert_after(onlist, elem) \
	LIST_INSERT_AFTER(onlist, elem, cand_glue)
#define cand_insert_before(onlist, elem) \
	LIST_INSERT_BEFORE(onlist, elem, cand_glue)

/**
 * Removes an element from the list
 *
 * @param[in] elem the element being removed
 */
#define cand_remove(elem) LIST_REMOVE(elem, cand_glue)

/**
 * Foreach element
 *
 * Usage:
 *    cand_t *elem, *t;
 *    cand_list_t head;
 *
 *    cand_foreach(head, elem) {
 *        ... do stuff with elem
 *            but don't delete it ...
 *    }
 *
 * @param[in] head head of the list
 * @param[in] elem current element
 */
#define cand_foreach(head, elem) LIST_FOREACH(elem, head, cand_glue)

/**
 * First element of the list
 * 
 * @param[in] head the head of the list
 */
#define cand_first(head) LIST_FIRST(head)

/**
 * Next element of the list
 *
 * @param[in] elem the element on the list
 *
 * @return the next element on the list
 */
#define cand_next(elem) LIST_NEXT(elem, cand_glue)

/**
 * Insert an element into the candidate list in decreasing delta_c order
 *
 * @param[in] head of the candidate list
 * @param[in] cand the element being added
 */
void cand_ins_maxb(cand_list_t *head, cand_t *cand);

/**
 * Insert an element into the candidate list in increasing delta_l order
 *
 * @param[in] head of the candidate list
 * @param[in] cand the element being added
 */
void cand_ins_minp(cand_list_t *head, cand_t *cand);

/**
 * Removes all candidates from the list, each of the candidates is
 * cand_free()'d 
 *
 * @param[out] head the head of the list to be cleared, will be
 *     cand_init()'d before returned;
 */
void cand_list_clear(cand_list_t *head);

/**
 * cand_list_clear() and cand_list_free()'s the candidate list
 */
void cand_list_destroy(cand_list_t *head);

/**
 * Finds the element of the deadline
 *
 * @param[in] head list head
 * @param[in] name the name of the node
 *
 * @return NULL if not found, the element otherwise.
 */
cand_t *cand_find(cand_list_t* head, char *name);

/**
 * Allocate a new element
 *
 * Usage:
 *    dnode_t *node;
 *    ...
 *    dnode_update(node);
 *    cand_t *item = cande_alloc(node);
 *    // node is copied upon insertion 
 *    dnode_free(node)
 *
 */
cand_t*	cand_alloc();
void	cand_free(cand_t *c);
cand_t*	cand_copy(cand_t *e);


/**
 * Calculate the change in critical path length for a candidate
 *
 * Requires dag_can_collapse has been called before invocation
 *
 * delta = (old l) - (new l)
 */
int cand_delta_l(cand_t *cand);
/**
 * Calculate the change in workload for a candidate
 *
 * Requires dag_can_collapse has been called before invocation
 *
 * delta = (old c) - (new c)
 */
int cand_delta_c(cand_t *cand);

cand_list_t* corder_arb(dtask_t *task);
cand_list_t* corder_maxb(dtask_t *task);
cand_list_t* corder_minp(dtask_t *task);

#endif /* DAG_CANDIDATE_H */
