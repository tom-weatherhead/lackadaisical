/* lackadaisical/src/environment.c */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "types.h"

#include "create-and-destroy.h"
/* #include "parse-and-evaluate.h"
#include "print.h" */

/* Global variables */

LISP_VALUE * globalNullValue = NULL;
LISP_VALUE * globalTrueValue = NULL;
/* TODO? : LISP_ENV * globalEnvironment = NULL; */

/* extern SCHEME_UNIVERSAL_TYPE * macroList; */

/* Function prototypes */

/* Functions */

static LISP_VALUE * lookupVariableInNameValueList(LISP_VAR * var, LISP_NAME_VALUE_LIST_ELEMENT * nvle) {

	while (nvle != NULL) {

		if (!strcmp(nvle->name, var->name)) {
			return getValueInNameValuePairListElement(nvle);
		}

		nvle = nvle->next;
	}

	return NULL;
}

LISP_VALUE * lookupVariableInEnvironment(LISP_VAR * var, LISP_ENV * env) {
	LISP_VALUE * value = NULL;

	while (env != NULL) {
		value = lookupVariableInNameValueList(var, getNameValuePairListInEnv(env));

		if (value != NULL) {
			break;
		}

		if (env == env->next) {
			fprintf(stderr, "lookupVariableInEnvironment: env == env->next; breaking...\n");
			break;
		}

		env = env->next;
	}

	return value;
}

BOOL updateNameIfFoundInNameValueList(LISP_NAME_VALUE_LIST_ELEMENT * nvle, char * name, LISP_VALUE * value) {

	while (nvle != NULL) {

		if (!strcmp(nvle->name, name)) {
			getValueInNameValuePairListElement(nvle) = value;
			return TRUE;
		}

		nvle = nvle->next;
	}

	return FALSE;
}

BOOL updateIfFoundInNameValueList(LISP_NAME_VALUE_LIST_ELEMENT * nvle, LISP_VAR * var, LISP_VALUE * value) {
	return updateNameIfFoundInNameValueList(nvle, var->name, value);
}

static BOOL updateIfFoundInEnvironment(LISP_ENV * env, LISP_VAR * var, LISP_VALUE * value) {


	while (env != NULL) {

		if (updateIfFoundInNameValueList(getNameValuePairListInEnv(env), var, value)) {
			return TRUE;
		}

		env = env->next;
	}

	return FALSE;
}

void addNameToEnvironment(LISP_ENV * env, char * name, LISP_VALUE * value) {
	getNameValuePairListInEnv(env) = createNameValueListElement(name, value, getNameValuePairListInEnv(env));
}

void addToEnvironment(LISP_ENV * env, LISP_VAR * var, LISP_VALUE * value) {

	if (lookupVariableInEnvironment(var, env) != NULL) {
		fprintf(stderr, "addToEnvironment() : The variable '%s' already exists in this environment. Update not yet implemented.", var->name);
		return;
	}

	addNameToEnvironment(env, var->name, value);
}

void setValueInEnvironment(LISP_ENV * env, LISP_VAR * var, LISP_VALUE * value) {

	if (!updateIfFoundInEnvironment(env, var, value)) {
		/* Instead of calling something like addBubbleDown(),
		move env to point to the last element in the list;
		that element will be the globalEnv. */

		while (env->next != NULL) {
			env = env->next;
		}

		/* Now env == globalEnv */
		/* The pair (var, value) will be added to the globalEnv */

		addToEnvironment(env, var, value);
	}
}

LISP_ENV * composeEnvironment(LISP_VAR_LIST_ELEMENT * variableList, LISP_VALUE_LIST_ELEMENT * valueList, LISP_ENV * env) {
	LISP_ENV * newEnv = createEnvironment(env);

	while (variableList != NULL || valueList != NULL) {

		if (variableList == NULL || valueList == NULL) {
			fatalError("composeEnvironment() : The formal and actual parameter lists have different lengths.");
			return NULL;
		}

		failIf(variableList->type != schemeStructType_VariableListElement, "composeEnvironment() : variableList->type != schemeStructType_VariableListElement");

		LISP_VALUE * value = getValueInValueListElement(valueList);

		addNameToEnvironment(newEnv, variableList->name, value);
		variableList = variableList->next;
		valueList = valueList->next;
	}

	return newEnv;
}

/* void printEnvironment(LISP_ENV * env) {
	int i = 0;

	printf("printEnvironment:\n");

	while (env != NULL) {
		LISP_NAME_VALUE_LIST_ELEMENT * nvle = env->nameValueList;
		int j = 0;

		printf("  Frame %d:\n", i++);

		while (nvle != NULL) {
			printf("    Value %d: %s = ", j++, nvle->name);
			printValue(nvle->value);
			printf("\n");
			nvle = nvle->next;
		}

		env = env->next;
	}

	printf("End of printEnvironment\n");
} */

LISP_ENV * createGlobalEnvironment() {
	failIf(globalNullValue != NULL, "globalNullValue is already non-NULL");
	failIf(globalTrueValue != NULL, "globalTrueValue is already non-NULL");
	/* failIf(macroList != NULL, "macroList is already non-NULL"); */

	globalNullValue = createNull();
	/* globalTrueValue = createSymbolValue("T"); / * I.e. 'T */
	globalTrueValue = createNumericValue(1);
	/* macroList = NULL; */

	LISP_ENV * globalEnv = createEnvironment(NULL);

	/* BEGIN: Predefined variables in the global environment */

	/* TODO? : We could implement Boolean logic functionally:
	parseAndEvaluateEx("(set! true (lambda (t f) t))", globalEnv, FALSE);
	parseAndEvaluateEx("(set! false (lambda (t f) f))", globalEnv, FALSE);
	parseAndEvaluateEx("(set! if (lambda (b x y) (b x y)))", globalEnv, FALSE);
	parseAndEvaluateEx("(set! not (lambda (x) (if x false true)))", globalEnv, FALSE);

	TODO? : We could write 'soft' implementations of: mod > <= >= !=
	parseAndEvaluateEx("(set! > (reverse2args <))", globalEnv, FALSE);
	parseAndEvaluateEx("(set! >= (compose2args < not))", globalEnv, FALSE);
	parseAndEvaluateEx("(set! <= (reverse2args >=))", globalEnv, FALSE);
	parseAndEvaluateEx("(set! <> (compose2args = not))", globalEnv, FALSE);
	parseAndEvaluateEx("(set! != <>)", globalEnv, FALSE);

	parseAndEvaluateEx("(set! mod (lambda (m n) (- m (* n (/ m n)))))", globalEnv, FALSE);
	*/

	/* parseAndEvaluateEx(
		"(set! combine (lambda (f sum zero) (letrec ((loop (lambda (l) (if (null? l) zero (sum (f (car l)) (loop (cdr l))))))) loop)))",
		globalEnv,
		FALSE
	);

	parseAndEvaluateEx("(set compose (lambda (f g) (lambda (x) (g (f x)))))", globalEnv, FALSE);

	parseAndEvaluateEx("(set! curry (lambda (f) (lambda (x) (lambda (y) (f x y)))))", globalEnv, FALSE);

	parseAndEvaluateEx("(set! id (lambda (x) x))", globalEnv, FALSE);

	parseAndEvaluateEx("(set! and (lambda (x y) (if x y x)))", globalEnv, FALSE);
	parseAndEvaluateEx("(set! or (lambda (x y) (if x x y)))", globalEnv, FALSE);

	parseAndEvaluateEx("(set! compose2args (lambda (f g) (lambda (x y) (g (f x y)))))", globalEnv, FALSE);
	parseAndEvaluateEx("(set! reverse2args (lambda (f) (lambda (x y) (f y x))))", globalEnv, FALSE);

	parseAndEvaluateEx("(set! not (lambda (x) (if x '() 'T)))", globalEnv, FALSE);

	parseAndEvaluateEx("(set! mod %)", globalEnv, FALSE);
	parseAndEvaluateEx("(set! gcd (lambda (m n) (if (= n 0) m (gcd n (mod m n)))))", globalEnv, FALSE);

	parseAndEvaluateEx("(set! <> !=)", globalEnv, FALSE);

	parseAndEvaluateEx("(set! atom? (compose pair? not))", globalEnv, FALSE);

	parseAndEvaluateEx("(set! any (combine id or '()))", globalEnv, FALSE);
	parseAndEvaluateEx("(set! all (combine id and 'T))", globalEnv, FALSE);
	parseAndEvaluateEx("(set! mapc (lambda (f) (combine f cons '())))", globalEnv, FALSE);
	parseAndEvaluateEx("(set! mapcar (lambda (f l) ((mapc f) l)))", globalEnv, FALSE);

	parseAndEvaluateEx("(set! append (lambda (l1 l2) ((combine id cons l2) l1)))", globalEnv, FALSE);
	parseAndEvaluateEx("(set! reverse (lambda (l) (letrec ((rev-aux (lambda (l1 l2) (if (null? l1) l2 (rev-aux (cdr l1) (cons (car l1) l2)))))) (rev-aux l '()))))", globalEnv, FALSE);

	parseAndEvaluateEx("(set! length (lambda (l) (if (null? l) 0 (+1 (length (cdr l))))))", globalEnv, FALSE);

	parseAndEvaluateEx("(set! +1 ((curry +) 1))", globalEnv, FALSE);

	parseAndEvaluateEx("(set! null '())", globalEnv, FALSE); */

	/* END: Predefined variables in the global environment */

	failIf(globalTrueValue == NULL, "globalTrueValue is NULL");
	failIf(globalNullValue == NULL, "globalNullValue is NULL");

	return globalEnv;
}

void freeGlobalEnvironment(/* LISP_ENV * globalEnv */) {
	globalTrueValue = NULL;
	globalNullValue = NULL;
	/* macroList = NULL; */

	/* TODO? Free all allocated memory: return freeAllStructs(); */
}

/* **** The End **** */
