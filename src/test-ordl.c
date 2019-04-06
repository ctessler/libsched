#include "ordl.h"
#include <stdio.h>

void test_creation();
void test_isort();

int
main(int argc, char** argv) {
	test_creation();
	test_isort();
	return 0;
}

void test_creation() {
	ordl_t head;
	or_elem_t *c, *elem = oe_alloc();
	elem->oe_deadline = 20;
	c = elem;
	
	ordl_init(&head);
	ordl_insert_head(&head, elem);
	
	elem = oe_alloc();
	elem->oe_deadline = 40;
	ordl_insert_after(c, elem);
	
	or_elem_t *cursor;
	ordl_foreach(&head, cursor) {
		printf("Absolute Deadline: %u\n", cursor->oe_deadline);
	}

	or_elem_t *temp;
	for (cursor = ordl_first(&head); cursor; ) {
		temp = ordl_next(cursor);
		ordl_remove(cursor);
		oe_free(cursor);
		cursor = temp;
	}

	ordl_foreach(&head, cursor) {
		printf("Absolute Deadline: %u\n", cursor->oe_deadline);
	}
}

void test_isort() {
	tint_t deadlines[] = {30, 15, 30, 25, 22, 20, 40, 60, 800};
	tint_t s = sizeof(tint_t);
	tint_t ele = sizeof(deadlines) / s;

	printf("Unsorted deadlines: ");
	for (tint_t c = 0; c < ele; c++) {
		printf("%u", deadlines[c]);
		if (c < (ele -1)) {
			printf(", ");
		}
	}
	printf("\n");

	ordl_t head;
	or_elem_t *c, *elem;

	ordl_init(&head);
	for (tint_t i = 0; i < ele; i++) {
		elem = oe_alloc();
		elem->oe_deadline = deadlines[i];
		ordl_insert(&head, elem);
	}

	or_elem_t *cursor;
	printf("  Sorted deadlines: ");
	ordl_foreach(&head, cursor) {
		printf("%u", cursor->oe_deadline);
		if (ordl_next(cursor)) {
			printf(", ");
		}
	}
	printf("\n");

	or_elem_t *temp;
	for (cursor = ordl_first(&head); cursor; ) {
		temp = ordl_next(cursor);
		ordl_remove(cursor);
		oe_free(cursor);
		cursor = temp;
	}
	ordl_init(&head);
}
