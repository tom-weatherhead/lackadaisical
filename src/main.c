/* lackadaisical/src/main.c */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "types.h"

#include "char-source.h"
#include "input-output.h"
#include "memory-manager.h"
#include "parse-and-evaluate.h"
/* #include "tests.h" */

/* **** The Main MoFo **** */

int main(int argc, char * argv[]) {
	/* BOOL enableTests = FALSE; */
	BOOL enableVersion = FALSE;
	/* char * filename = NULL; */
	int i;

	/* Use the current time to seed the random number generator: */
	srand(time(NULL));

	for (i = 1; i < argc; ++i) {
		/* printf("argv[%d] = %s\n", i, argv[i]); */

		/* if (!strcmp(argv[i], "-t")) {
			enableTests = TRUE;
		} else */ if (!strcmp(argv[i], "-v")) {
			enableVersion = TRUE;
		} /* else if (filename == NULL && argv[i][0] != '-') {
			filename = argv[i];
		} */
	}

	if (enableVersion) {
		printf("\nLackadaisical version 0.0.0\n");
	} /* else if (enableTests) {
		runTests();
	} else if (filename != NULL) {
		execScriptInFile(filename, NULL);
	} */ else {
		readEvalPrintLoop();
	}

	const int numFreed = freeAllStructs();

	printf("gc final: %d block(s) of memory freed.\n", numFreed);

	mmPrintReport();

	return 0; /* Zero (as a Unix exit code) means success. */
}

/* **** The End **** */
