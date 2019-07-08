#ifndef DAG_COLLAPSE_H
#define DAG_COLLAPSE_H

#include "dag-task.h"
#include "dag-node-list.h"

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
 * @param[in] a node to be collapsed with b
 * @param[in] b node to be collapsed with a
 */
int dtask_can_collapse(dnode_t *a, dnode_t *b);



#endif
