#ifndef DAG_DFS_H
#define DAG_DFS_H

#include "dag-task.h"

/**
 * DAG Task DFS Operation (DDO)
 */
typedef enum {
	      DFS_ERR=0,
	      DFS_SKIP,
	      DFS_GOOD
} ddo_t;

/**
 * Callback functions for the caller
 *
 * ddfs_pre - called before a node is visited, the return values from
 *     the pre method have the following meanings:
 *      DFS_ERR - there was an error with the pre operation and the
 *          DFS is aborted.
 *      DFS_SKIP - skip this node (and its children) in the search
 *      DFS_GOOD - continue normally, visit this node and its children
 *
 * ddfs_visit - the visit operation of the dfs, returns values have
 *     the following meaning:
 *      DFS_ERR - there was an error with visit pre operation and the
 *          DFS is aborted.
 *      DFS_SKIP - treated as a DFS_GOOD
 *      DFS_GOOD - continue normally, the node has been visited
 *
 * ddfs_post - called after the node and all of its children have been
 *     visited, returns values have the following meaning:
 *      DFS_ERR - there was an error with visit pre operation and the
 *          DFS is aborted.
 *      DFS_SKIP - treated as a DFS_GOOD
 *      DFS_GOOD - continue normally, the node has been post operated
 *
 * @param[in] node the node under inspection of the DFS
 * @param[in] userdata the user data provided by the caller
 *
 */
typedef ddo_t (*ddfs_pre)(dnode_t *node, void *userd);
typedef ddo_t (*ddfs_visit)(dnode_t *node, void *userd);
typedef ddo_t (*ddfs_post)(dnode_t *node, void *userd);

ddo_t ddfs(dnode_t *node, ddfs_pre pre, ddfs_visit visit, ddfs_post post, void *userd);

#endif /* DAG_DFS_H */

