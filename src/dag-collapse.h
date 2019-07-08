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



#endif
