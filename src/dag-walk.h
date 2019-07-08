#ifndef DAG_WALK_H
#define DAG_WALK_H
#include "dag-dfs.h"

/**
 * Returns a NULL terminated array of nodes in topological order.
 *
 * @note after any walk, the nodes are marked and *CANNOT* be rused reliably.
 */
dnode_t **dag_topological(dnode_t* node);

/**
 * Returns a NULL terminated array of nodes with their maximum
 * distance from the starting node. Zero indicates no path from the
 * node to the final one.
 *
 * @note after any walk, the nodes are marked and *CANNOT* be rused reliably.
 */
dnode_t **dag_maxd(dnode_t* node);

/**
 * Returns the number of nodes in the path from a to b, including a and b.
 *
 * @note after any walk, the nodes are marked and *CANNOT* be rused reliably.
 *
 * @param[in] a a node
 * @param[in] b another node
 *
 * @return the number of nodes in the path <a, ~>, b>
 */
int dag_pathlen(dnode_t *a, dnode_t *b);

#endif /* DAG_WALK_H */
