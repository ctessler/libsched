#ifndef DAG_TASK_H
#define DAG_TASK_H

#include <gvc.h>
#include "task.h"

#define DT_NAMELEN	TASK_NAMELEN

#define DT_THREADS	"threads" 	/** Number of threads per node */
#define DT_OBJECT	"object"	/** Executable object */
#define DT_WCET_ONE	"wcetone"	/** WCET of ONE thread */
#define DT_WCET		"wcet"		/** WCET of DT_THREADS */
#define DT_FACTOR	"factor"	/** Growth factor of node */
/** Sadly necessary attributes */
#define DT_VISITED	"visited"	/** marked visited */
#define DT_MARKED	"marked"	/** "permanent" mark */
#define DT_DISTANCE	"distance"	/** distance from current node */

typedef struct dnode_s dnode_t;

typedef struct {
	Agraph_t *dt_graph;
	char	dt_name[DT_NAMELEN];
	tint_t	dt_period;	/** Period of the task, user settable */
	tint_t	dt_deadline;	/** Relative deadline of the task, settable */
	tint_t	dt_cpathlen;	/** Critical path length, NOT settable */
	tint_t	dt_workload;	/** Workload, NOT settable */
	dnode_t *dt_source;	/** Source node, NOT settable */
	struct {
		unsigned int dirty:1;
	} dt_flags;
} dtask_t;

struct dnode_s {
	/*
	 * There are no user settable fields for nodes, use the
	 * accessor methods
	 */
	char	dn_name[DT_NAMELEN];
	char	dn_label[DT_NAMELEN * 2];
	tint_t	dn_object;
	tint_t	dn_threads;
	tint_t	dn_wcet_one;	/** Single thread WCET */
	tint_t	dn_wcet;	/** Total WCET for dn_threads */
	float_t	dn_factor;
	/** The last task this node was inserted into */
	dtask_t		*dn_task;
	/** This node in dn_graph */
	Agnode_t	*dn_node;
	/** Flags, these are ugly here but save more complex
	    structures in the walks */
	struct {
		unsigned int dirty:1;
		unsigned int marked:1;
		unsigned int visited:1;
	} dn_flags;
	/** Distance, again saves more complex structure in longest
	    path calculation */
	tint_t dn_distance;
};

typedef struct {
	char de_name[DT_NAMELEN];
	char de_label[DT_NAMELEN * 2];
	char de_sname[DT_NAMELEN]; /* Source node name */
	char de_dname[DT_NAMELEN]; /* Destination node name */
	Agedge_t *de_edge;
	Agraph_t *de_graph;
} dedge_t;

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
 * Copies a DAG task
 *
 * @param[in] task the dag task being copied
 *
 * @return the new dag task upon success, NULL otherwise.
 */
dtask_t *dtask_copy(dtask_t *task);


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
 * Removes a node from the DAG
 *
 * @param[in|out] task the dag task
 * @param[in] the node in the task
 *
 * @return non-zero upon success, zero otherwise
 */
int dtask_remove(dtask_t *task, dnode_t *node);

/**
 * Removes a node from the DAG by name
 *
 * @param[in] task the dag task
 * @param[in] the name of the node in the task
 *
 * @return non-zero upon success, zero otherwise
 */
int dtask_name_remove(dtask_t *task, char *name);

/**
 * Finds a node in the DAG by name
 *
 * @param[in] name of the node
 *
 * @return the dnode_t upon success, NULL if not found
 */
dnode_t *dtask_name_search(dtask_t *task, char *name);

/**
 * Adds an edge into the DAG task
 *
 * @param[in|out] task the dag task
 * @param[in] src the node the edge starts from
 * @param[in] dst the node the edge ends with
 *
 * @return non-zero upon success, zero otherwise
 */
int dtask_insert_edge(dtask_t *task, dnode_t *src, dnode_t *dst);

/**
 * Removes an edge from the DAG task
 *
 * @param[in|out] task the dag task
 * @param[in] src the node the edge starts from
 * @param[in] dst the node the edge ends with
 *
 * @return non-zero upon success, zero otherwise
 */
int dtask_remove_edge(dtask_t *task, dnode_t *src, dnode_t *dst);

/**
 * Finds an edge by names
 *
 * @param[in] task the dag task
 * @param[in] sname source node name
 * @param[in] dname dest node name
 *
 * @return a dedge_t if the edge exists (that must be freed), NULL
 * otherwise
 */
dedge_t* dtask_search_edge(dtask_t* task, char *sname, char *dname);

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
 * Returns the source node of the dag task
 *
 * must be dnode_freed()'d
 */
dnode_t *dtask_source(dtask_t *task);
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
 * Clears the marks on all nodes and edges in the DAG task
 *
 * @param[in] task the dag task to clear marks upon
 */
void dtask_unmark(dtask_t *task);


/*********************************************************************
 DAG Node
 *********************************************************************/

/**
 * Allocates a DAG node
 *
 * @param[in] name the name of the node, immutable.
 *
 * @return the dag node upon success, NULL otherwise
 */
dnode_t *dnode_alloc(char* name);

/**
 * Releases a DAG node
 */
void dnode_free(dnode_t *node);

/**
 * Node getters and setters
 */
tint_t dnode_get_object(dnode_t *node);
void dnode_set_object(dnode_t *node, tint_t obj);
tint_t dnode_get_threads(dnode_t *node);
void dnode_set_threads(dnode_t *node, tint_t threads);
tint_t dnode_get_wcet_one(dnode_t *node);
void dnode_set_wcet_one(dnode_t *node, tint_t wcet_one);
tint_t dnode_get_wcet(dnode_t *node);
float_t dnode_get_factor(dnode_t *node);
void dnode_set_factor(dnode_t *node, float_t factor);

/**
 * Updates the node
 *
 * If a node is present in a task, and the parameters are modified the
 * node must be updated for it to be reflected in the task
 *
 * @param[in] node the node
 *
 * @return non-zero upon success, zero otherwise
 */
int dnode_update(dnode_t *node);

/*********************************************************************
 DAG edge
 *********************************************************************/
dedge_t* dedge_alloc(char *name);
void dedge_free(dedge_t *edge);

void dedge_set_src(dedge_t *edge, char *name);
void dedge_set_dst(dedge_t *edge, char *name);

dedge_t* dedge_out_first(dnode_t *node);
dedge_t* dedge_out_first_name(dtask_t *task, char *name);
dedge_t* dedge_out_next(dedge_t *edge);

#endif /* TASK_H */
