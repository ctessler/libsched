#ifndef DAG_WALK_H
#define DAG_WALK_H
#include "dag-dfs.h"

/**
 * Returns a NULL terminated array of nodes in order.
 */
dnode_t **dag_topological(dnode_t* node);


#endif /* DAG_WALK_H */
