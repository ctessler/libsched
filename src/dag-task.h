#ifndef DAG_TASK_H
#define DAG_TASK_H

#include <gvc.h>
#include "task.h"

#define DT_NAMELEN	TASK_NAMELEN

#define DT_THREADS	"threads" 	/** Number of threads per node */
#define DT_OBJECT	"object"	/** Executable object */
#define DT_WCET_ONE	"wcetone"	/** WCET of ONE thread */
#define DT_WCET		"WCET"		/** WCET of DT_THREADS */
#define DT_FACTOR	"factor"	/** Growth factor of node */

#define DT_DIRTY	0x1	/** Task is dirty */

typedef struct {
	Agraph_t *dt_graph;
	char	dt_name[DT_NAMELEN];
	tint_t	dt_period;	/** Period of the task, user settable */
	tint_t	dt_deadline;	/** Relative deadline of the task, settable */
	tint_t	dt_cpathlen;	/** Critical path length, NOT settable */
	tint_t	dt_workload;	/** Workload, NOT settable */
	uint8_t dt_flags;	/** Internal flags, NOT settable */
} dtask_t;

typedef struct {
	/*
	 * There are no user settable fields for nodes, use the
	 * accessor methods
	 */
	char	dn_name[DT_NAMELEN];
	tint_t	dn_object;
	tint_t	dn_threads;
	tint_t	dn_wcet_one;	/** Single thread WCET */
	tint_t	dn_wcet;	/** Total WCET for dn_threads */
	float_t	dn_factor;
	uint8_t dn_dirty;
	/** The last task this node was inserted into */
	dtask_t		*dn_task;
	/** This node in dn_graph */
	Agnode_t	*dn_node;
} dnode_t;

/**
 * Allocates a DAG Task
 *
 * @param[in] name the name of the task
 *
 * @return the dag task upon success, NULL otherwise
 */
dtask_t *dtask_alloc(char* name);

/**
 * Releases a DAG Task
 */
void dtask_free(dtask_t *task);

/**
 * Inserts a node into the DAG
 *
 * @note After being inserted, the dn_node_t tracks its place in the
 * graph for ease of updating. If the dn_node_t is updated with new
 * information, it must be dt_update()'d.
 *
 * Once the node has been placed in the graph and parameters set, the
 * node may be dnode_free()'d, and the node will persist in the
 * graph. If the node is to be removed from the graph use dt_remove(),
 * then dnode_free().
 *
 * @param[in|out] the graph node is being inserted into
 * @param[in|out] node the node being inserted
 *
 * @return non-zero upon success, zero otherwise
 */
int dtask_insert(dtask_t *task, dnode_t *node);

/**
 * Finds a node in the DAG by name
 *
 * @param[in] name of the node
 *
 * @return the dnode_t upon success, NULL if not found
 */
dnode_t *dtask_name_search(dtask_t *task, char *name);

/**
 * Writes the task to dot file
 *
 * @param[in] task the dag task
 * @param[in] file the open file for writing
 *
 * @return non-zero upon success, zero otherwise
 */
int dtask_write(dtask_t *task, FILE *file);

/**
 * Gets the critical path length of the task
 *
 * @param[in] the dag task
 *
 * @return the critical path length
 */
tint_t dtask_cpathlen(dtask_t* task);

/**
 * Calculates the work load of a task
 *
 * @param[in] the dag task
 *
 * @return the workload
 */
tint_t dtask_workload(dtask_t* task);

/**
 * Allocates a DAG node
 *
 * @param[in] name the name of the node
 *
 * @return the dag node upon success, NULL otherwise
 */
dnode_t *dnode_alloc(char* name);

/**
 * Releases a DAG node
 */
void dnode_free(dnode_t *node);

#endif /* TASK_H */
