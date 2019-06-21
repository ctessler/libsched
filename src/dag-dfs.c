#include "dag-dfs.h"

ddo_t
ddfs(dnode_t *cursor, ddfs_pre pre, ddfs_visit visit, ddfs_post post,
     void *ud) {
	ddo_t op = DFS_GOOD;
	int rv=1;

	/* Pre-visit callback */
	if (pre) {
		op = pre(cursor, ud);
	}
	switch (op) {
	case DFS_ERR:
		return DFS_ERR;
	case DFS_SKIP:
		return DFS_SKIP;
	case DFS_GOOD:
		break;
		/* Continue */
	}

	/* Visit this node */
	if (visit) {
		op = visit(cursor, ud);
	}
	switch (op) {
	case DFS_ERR:
		return DFS_ERR;
	case DFS_SKIP:
	case DFS_GOOD:
		/* Continue */
		break;
	}

	dedge_t *e;
	for (e = dedge_out_first(cursor); e; e = dedge_out_next(e)) {
		char *dname = e->de_dname;
		dnode_t *child = dtask_name_search(cursor->dn_task, dname);
		op = ddfs(child, pre, visit, post, ud);
		dnode_free(child);
		switch (op) {
		case DFS_ERR:
			return DFS_ERR;
		case DFS_SKIP:
		case DFS_GOOD:
			break;
			/* Continue */
		}
	}

	/* Post-visit callback */
	if (post) {
		op = post(cursor, ud);
	}
	switch (op) {
	case DFS_ERR:
		return DFS_ERR;
	case DFS_SKIP:
	case DFS_GOOD:
		break;
		/* Continue */
	}

	return DFS_GOOD;
};
