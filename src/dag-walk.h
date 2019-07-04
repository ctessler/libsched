#ifndef DAG_WALK_H
#define DAG_WALK_H
#include "dag-dfs.h"

/**
 * Returns a NULL terminated array of nodes in topological order.
 */
dnode_t **dag_topological(dnode_t* node);

/**
 * Returns a NULL terminated array of nodes with their maximum
 * distance from the starting node. Zero indicates no path from the
 * node to the final one.
 */
dnode_t **dag_maxd(dnode_t* node);


#endif /* DAG_WALK_H */
