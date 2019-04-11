#include "ordt.h"

static int
compare(const void *xa, const void *xb) {
	const ot_elem_t *a = xa;
	const ot_elem_t *b = xb;

	if (a->ote_deadline < b->ote_deadline) {
		return -1;
	}
	if (a->ote_deadline > b->ote_deadline) {
		return 1;
	}
	return 0;
}

ot_elem_t*
ot_find(ot_t *tree, tint_t deadline) {
	ot_elem_t key;
	key.ote_deadline = deadline;
	key.ote_tasks = NULL;

	void *cookie  = tfind(&key, &tree->ot_root, compare);
	if (!cookie) {
		return NULL;
	}
	ot_elem_t *needle = *(ot_elem_t**) cookie;
	return needle;
}
	

int
ot_ins(ot_t *tree, ot_elem_t *elem) {
	void *cookie = tsearch(elem, &(tree->ot_root), compare);
	ot_elem_t *needle = *(ot_elem_t**) cookie;
	if (needle != elem) {
		return -1;
	}
	tree->ot_count++;
	return 0;
}
int
ot_rem(ot_t *tree, ot_elem_t *elem) {
	void *cookie = tdelete(elem, &(tree->ot_root), compare);
	if (NULL == tree->ot_root) {
		tree->ot_count--;		
		return 0;
	}
	ot_elem_t *needle = *(ot_elem_t**) cookie;
	if (needle != elem) {
		return -1;
	}
	tree->ot_count--;
	return 0;
}

void
ot_empty(ot_t *tree) {
	tdestroy(tree->ot_root, (void(*)(void*))ote_free);
	tree->ot_count = 0;
	tree->ot_root = NULL;
}

void ote_free(ot_elem_t *elem) {
	ts_free(elem->ote_tasks);
	elem->ote_tasks = NULL;
	free(elem);
	elem = NULL;
}

