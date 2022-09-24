/* lackadaisical/src/memory-manager.c */

/* A mark-and-sweep garbage collector. */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "types.h"

#include "create-and-destroy.h"

/* External constants / variables */

extern LISP_VALUE * globalNullValue;
extern LISP_VALUE * globalTrueValue;

/* Local constants / variables */

static int numMallocs = 0;
static int numFrees = 0;

static void setMarksInExprTree(SCHEME_UNIVERSAL_TYPE * expr);

typedef struct MEMMGR_RECORD_STRUCT {
	SCHEME_UNIVERSAL_TYPE * item;
	struct MEMMGR_RECORD_STRUCT * next;
} MEMMGR_RECORD;

static MEMMGR_RECORD * memmgrRecords = NULL;

void * mmAlloc(int numBytes) {
	failIf(numBytes <= 0, "mmAlloc() : numBytes <= 0");

	void * result = malloc(numBytes);

	failIf(result == NULL, "mmAlloc() : malloc() returned NULL");

	++numMallocs;
	memset(result, 0, numBytes);

	return result;
}

void mmFree(void * ptr) {
	failIf(ptr == NULL, "mmFree() : ptr is NULL");

	if (ptr != NULL) {
		free(ptr);
		++numFrees;
	}
}

void mmPrintReport() {
	printf("\nMemory manager report:\n");
	printf("  Number of mallocs:  %d\n", numMallocs);
	printf("  Number of frees:    %d\n\n", numFrees);

	if (numFrees < numMallocs) {
		printf("  Number of leaks:    %d\n\n", numMallocs - numFrees);
	}
}

void addItemToMemMgrRecords(SCHEME_UNIVERSAL_TYPE * item) {
	MEMMGR_RECORD * mmRec = NULL;

	mmRec = (MEMMGR_RECORD *)mmAlloc(sizeof(MEMMGR_RECORD));

	mmRec->item = item;
	mmRec->next = memmgrRecords;
	memmgrRecords = mmRec;
}

/* int getNumMemMgrRecords() {
	int n = 0;
	MEMMGR_RECORD * mmRec;

	for (mmRec = memmgrRecords; mmRec != NULL; mmRec = mmRec->next) {
		++n;
	}

	return n;
} */

static void clearMarks() {
	MEMMGR_RECORD * mmRec;

	for (mmRec = memmgrRecords; mmRec != NULL; mmRec = mmRec->next) {
		mmRec->item->mark = 0;
	}
}

/* static void setMarksInAA(int n, SCHEME_UNIVERSAL_TYPE ** aux) {

	while (n-- > 0) {
		setMarksInExprTree(aux[n]);
	}
} */

static void setMarksInExprTree(SCHEME_UNIVERSAL_TYPE * expr) {

	if (expr == NULL || expr->mark == 1) {
		return;
	} else if (expr->mark != 0) {
		printf("mm error: expr->mark is %d\n", expr->mark);
		exit(1);
	}

	expr->mark = 1;

	setMarksInExprTree(expr->value1);
	setMarksInExprTree(expr->value2);
	setMarksInExprTree(expr->value3);
	setMarksInExprTree(expr->next);

	/* if (expr->type == lispValueType_AssociativeArray) {
		setMarksInAA(expr->integerValue, (SCHEME_UNIVERSAL_TYPE **)expr->aux);
	} */
}

static int freeUnmarkedStructs() {
	MEMMGR_RECORD ** ppmmRec = &memmgrRecords;
	MEMMGR_RECORD * mmRec = *ppmmRec;
	int numFreed = 0;

	while (mmRec != NULL) {

		if (mmRec->item->mark == 0) {
			/* Free mmRec->item. Do not free recursively.
			Allow mmRec->item->name to be freed. */
			mmRec->item->value1 = NULL;
			mmRec->item->value2 = NULL;
			mmRec->item->value3 = NULL;
			mmRec->item->next = NULL;
			freeUniversalStruct(mmRec->item);

			/* Then free mmRec, preserving the integrity of the linked list */
			MEMMGR_RECORD * nextmmRec = mmRec->next;

			mmRec->item = NULL;
			mmRec->next = NULL;
			mmFree(mmRec);
			++numFreed;
			*ppmmRec = nextmmRec;
		} else {
			ppmmRec = &mmRec->next;
		}

		mmRec = *ppmmRec;
	}

	return numFreed;
}

int collectGarbage(SCHEME_UNIVERSAL_TYPE * exprTreesToMark[]) {
	int i;

	clearMarks();

	for (i = 0; exprTreesToMark[i] != NULL; ++i) {
		setMarksInExprTree(exprTreesToMark[i]);
	}

	return freeUnmarkedStructs();
}

int freeAllStructs() {
	globalNullValue = NULL;
	globalTrueValue = NULL;
	clearMarks();

	return freeUnmarkedStructs();
}

/* **** The End **** */
