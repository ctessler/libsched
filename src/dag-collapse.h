#ifndef DAG_COLLAPSE_H
#define DAG_COLLAPSE_H

#include "dag-task.h"
#include "dag-node-list.h"
#include "dag-walk.h"

/**
 * Counts the number of candidates
 *
 * Candiades are unique pairs with identical objects 
 *
 * @param[in] task the task
 *
 * @return the number of candidates in the task
 */
int dtask_count_cand(dtask_t *task);

/**
 * Returns true if the two nodes can be collapsed
 *
 * @note any reference to nodes after a call to dtask_can_collapse
 * will be inconsistent. 
 *
 * @param[in] a node to be collapsed with b
 * @param[in] b node to be collapsed with a
 */
int dag_can_collapse(dnode_t *a, dnode_t *b);

/**
 * Collapses two nodes
 *
 * @note Assumes caller has verified a and b pass dag_can_collapse()
 *
 * @param[in] a node to be collapsed with b
 * @param[in] b node to be collapsed with a
 *
 * @return true if the nodes have been collapsed, false otherwise.
 */
int dag_collapse(dnode_t *a, dnode_t *b);

/**
 * Collapses all candidates in a task
 *
 * @param[in] task the dag task
 *
 * @return the number of collapses
 */
int dag_collapse_all(dtask_t *task);



#endif
