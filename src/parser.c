/* atrocity/src/parser.c */

/* **** Parsing (recursive descent - a real half-assed parser) **** */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "types.h"

#include "char-source.h"

#include "evaluate.h"
#include "create-and-destroy.h"
#include "memory-manager.h"
#include "parser.h"
#include "string-builder.h"
#include "utilities.h"

/* Function prototypes */

/* Forward references */

static LISP_EXPR_LIST_ELEMENT * parseExpressionList(CharSource * cs);
static LISP_VALUE * createQuotedList(CharSource * cs);

/* Constants */

/* Functions */

static LISP_EXPR * parseFunctionCallExpression(CharSource * cs) {
	LISP_EXPR_LIST_ELEMENT * exprList = parseExpressionList(cs);

	return createFunctionCallExpression(exprList);
}

static LISP_VAR_LIST_ELEMENT * parseVariableList(CharSource * cs) {
	STRING_BUILDER_TYPE * sb = getIdentifier(cs, NULL, NULL);

	if (isStringBuilderEmpty(sb)) {
		fatalError("parseVariableList() : Expected ), found EOF");
	} else if (!strcmp(sb->name, ")")) {
		return NULL; /* End of list */
	}

	LISP_VAR * var = createVariable(sb->name);
	LISP_VAR_LIST_ELEMENT * next = parseVariableList(cs);

	return createVariableListElement(var, next);
}

static LISP_EXPR * parseLambdaExpression(CharSource * cs) {

	if (!consumeStr(cs, "(")) {
		fatalError("parseLambdaExpression() : Expected (");
	}

	/* Parse variable list and consume ) */
	LISP_VAR_LIST_ELEMENT * args = parseVariableList(cs);

	/* Parse expression */
	LISP_EXPR * expr = parseExpression(cs);

	if (!consumeStr(cs, ")")) {
		fatalError("parseLambdaExpression() : Expected )");
	}

	return createLambdaExpression(args, expr);
}

static LISP_EXPR * parseSetExpression(CharSource * cs) {
	STRING_BUILDER_TYPE * sb = getIdentifier(cs, NULL, NULL);

	/* Parse variable */

	if (isStringBuilderEmpty(sb)) {
		fatalError("parseSetExpression() : Expected variable, found EOF");
	} else if (!strcmp(sb->name, "(")) {
		fatalError("parseSetExpression() : Expected variable, found '('");
	} else if (!strcmp(sb->name, ")")) {
		fatalError("parseSetExpression() : Expected variable, found ')'");
	}

	LISP_VAR * var = createVariable(sb->name);

	/* Parse expression */
	LISP_EXPR * expr = parseExpression(cs);

	if (!consumeStr(cs, ")")) {
		fatalError("parseSetExpression() : Expected ')'");
	}

	return createSetExpression(var, expr);
}

static LISP_VAR_EXPR_PAIR_LIST_ELEMENT * parseVarExpressionPairList(CharSource * cs) {
	STRING_BUILDER_TYPE * sb = getIdentifier(cs, NULL, NULL);

	if (isStringBuilderEmpty(sb)) {
		fatalError("parseVarExpressionPairList() : Expected ( or ), found EOF");
	} else if (!strcmp(sb->name, ")")) {
		return NULL; /* The end of the list */
	} else if (strcmp(sb->name, "(")) {
		fprintf(stderr, "parseVarExpressionPairList() : Error : Expected ( or ), found '%s'\n", sb->name);
		fatalError("parseVarExpressionPairList() : Expected ( or )");
	}

	clearStringBuilder(sb);

	getIdentifier(cs, sb, NULL);

	if (isStringBuilderEmpty(sb)) {
		fatalError("parseVarExpressionPairList() : Expected variable, found EOF");
	}

	/* Parse expression */
	LISP_EXPR * expr = parseExpression(cs);

	if (!consumeStr(cs, ")")) {
		fatalError("parseVarExpressionPairList() : Expected )");
	}

	LISP_VAR_EXPR_PAIR_LIST_ELEMENT * next = parseVarExpressionPairList(cs);

	return createVariableExpressionPairListElement(sb->name, expr, next);
}

static LISP_EXPR * parseLetExpression(CharSource * cs, int exprType) {

	if (!consumeStr(cs, "(")) {
		fatalError("parseLetExpression() : Expected (");
	}

	/* Parse the variable-expression pair list */
	LISP_VAR_EXPR_PAIR_LIST_ELEMENT * varExprPairList = parseVarExpressionPairList(cs);

	/* Parse expression */
	LISP_EXPR * expr = parseExpression(cs);

	if (!consumeStr(cs, ")")) {
		fatalError("parseLetExpression() : Expected )");
	}

	return createLetExpression(exprType, varExprPairList, expr);
}

/* static LISP_EXPR * parseBeginExpression(CharSource * cs) {
	return createBeginExpression(parseExpressionList(cs));
}

static LISP_EXPR * parseWhileExpression(CharSource * cs) {
	LISP_EXPR * condition = parseExpression(cs);
	LISP_EXPR * body = parseExpression(cs);

	if (!consumeStr(cs, ")")) {
		fatalError("parseWhileExpression() : Expected )");
	}

	return createWhileExpression(condition, body);
}

static LISP_EXPR_PAIR_LIST_ELEMENT * parseExpressionPairList(CharSource * cs) {
	STRING_BUILDER_TYPE * sb = getIdentifier(cs, NULL, NULL);

	if (isStringBuilderEmpty(sb)) {
		fatalError("parseExpressionPairList() : Error : Expected ( or ), found EOF");
		return NULL;
	} else if (!strcmp(sb->name, ")")) {
		return NULL; / * End of list * /
	} else if (strcmp(sb->name, "(")) {
		fprintf(stderr, "parseExpressionPairList() : Error : Expected ( or ), found '%s'\n", sb->name);
		fatalError("parseExpressionPairList() : Error : Expected ( or )");
		return NULL;
	}

	LISP_EXPR * expr = parseExpression(cs);
	LISP_EXPR * expr2 = parseExpression(cs);

	if (!consumeStr(cs, ")")) {
		fatalError("parseExpressionPairList() : Encountered an unexpected ')'");
		return NULL;
	}

	LISP_EXPR_PAIR_LIST_ELEMENT * next = parseExpressionPairList(cs);

	return createExpressionPairListElement(expr, expr2, next);
}

static LISP_EXPR * parseCondExpression(CharSource * cs) {
	LISP_EXPR_PAIR_LIST_ELEMENT * exprPairList = parseExpressionPairList(cs);

	return createCondExpression(exprPairList);
}

static LISP_EXPR * parseDefineMacroExpression(CharSource * cs) {
	STRING_BUILDER_TYPE * sb = getIdentifier(cs, NULL, NULL);

	/ * Parse the name of the macro * /

	if (isStringBuilderEmpty(sb)) {
		fatalError("parseDefineMacroExpression() : Expected macro name, found EOF");
	} else if (!strcmp(sb->name, "(")) {
		fatalError("parseDefineMacroExpression() : Expected macro name, found '('");
	} else if (!strcmp(sb->name, ")")) {
		fatalError("parseDefineMacroExpression() : Expected macro name, found ')'");
	}

	if (!consumeStr(cs, "(")) {
		fatalError("parseDefineMacroExpression() : Expected (");
	}

	/ * Parse variable list and consume ) * /
	LISP_VAR_LIST_ELEMENT * args = parseVariableList(cs);

	/ * Parse expression * /
	LISP_EXPR * expr = parseExpression(cs);

	if (!consumeStr(cs, ")")) {
		fatalError("parseLambdaExpression() : Expected )");
	}

	return createDefineMacroExpression(sb->name, args, expr);
} */

static LISP_EXPR * parseBracketedExpression(CharSource * cs) {
	const int csRewindPoint = cs->i;

	/* 'if' is currently handled as a primop */

	STRING_BUILDER_TYPE * sb = getIdentifier(cs, NULL, NULL);

	if (isStringBuilderEmpty(sb)) {
		fatalError("parseBracketedExpression() : Error : Expected an expression or keyword, found EOF");
		return NULL;
	}

	char * dstBuf = sb->name;

	if (!strcmp(dstBuf, "lambda")) {
		return parseLambdaExpression(cs);
	} else if (!strcmp(dstBuf, "set!") || !strcmp(dstBuf, "set")) {
		return parseSetExpression(cs);
	} /*else if (!strcmp(dstBuf, "begin")) {
		return parseBeginExpression(cs);
	} else if (!strcmp(dstBuf, "while")) {
		return parseWhileExpression(cs);
	} else if (!strcmp(dstBuf, "cond")) {
		return parseCondExpression(cs);
	} */ else if (!strcmp(dstBuf, "let")) {
		return parseLetExpression(cs, lispExpressionType_Let);
	} /* else if (!strcmp(dstBuf, "let*")) {
		return parseLetExpression(cs, lispExpressionType_LetStar);
	} else if (!strcmp(dstBuf, "letrec")) {
		return parseLetExpression(cs, lispExpressionType_Letrec);
	} else if (!strcmp(dstBuf, "define-macro")) {
		return parseDefineMacroExpression(cs);
	} */ else {
		cs->i = csRewindPoint;
		return parseFunctionCallExpression(cs);
	}
}

/* Parse a list of expressions */

static LISP_EXPR_LIST_ELEMENT * parseExpressionList(CharSource * cs) {
	/* We are parsing a bracketed list of expressions. */
	/* Assume that the opening bracket has already been consumed. */

	const int csRewindPoint = cs->i;

	const int c = getNextChar(cs);

	if (c == EOF) {
		fatalError("parseExpressionList() : Error : Expected an expression list, found EOF");
		return NULL;
	}

	if (c == ')') {
		return NULL; /* End of the list */
	}

	cs->i = csRewindPoint;

	LISP_EXPR * expr = parseExpression(cs);
	LISP_EXPR_LIST_ELEMENT * next = parseExpressionList(cs);
	LISP_EXPR_LIST_ELEMENT * result = createExpressionListElement(expr, next);

	return result;
}

static LISP_VALUE * createQuotedValue(CharSource * cs) {
	int dstBufAsInt = 0;

	/* TODO: Use lispExpressionType_QuotedConstantWithApostrophe and
	(elsewhere) lispExpressionType_QuotedConstantWithQuoteKeyword */

	STRING_BUILDER_TYPE * sb = getIdentifier(cs, NULL, NULL);

	if (isStringBuilderEmpty(sb)) {
		fatalError("createQuotedValue() : Error : Expected a literal value, found EOF");
		return NULL;
	} else if (!strcmp(sb->name, "(")) {
		return createQuotedList(cs);
	} else if (safeAtoi(sb->name, &dstBufAsInt)) {
		return createNumericValue(dstBufAsInt);
	} else {
		return createSymbolValue(sb->name);
	}
}

static LISP_VALUE * createQuotedList(CharSource * cs) {
	const int csRewindPoint = cs->i;
	STRING_BUILDER_TYPE * sb = getIdentifier(cs, NULL, NULL);

	if (isStringBuilderEmpty(sb)) {
		fatalError("createQuotedList() : Error : Expected a literal value, found EOF");
		return NULL;
	} else if (!strcmp(sb->name, ")")) {
		return createNull();
	} else {
		cs->i = csRewindPoint;

		LISP_VALUE * head = createQuotedValue(cs);
		LISP_VALUE * tail = createQuotedList(cs);

		return createPair(head, tail);
	}
}

/* Parse an expression */

LISP_EXPR * parseExpression(CharSource * cs) {
	/* Be careful to not assume that sizeof(char) is always 1. */
	int dstBufAsInt = 0;
	BOOL isSingleQuoted = FALSE;

	STRING_BUILDER_TYPE * sb = getIdentifier(cs, NULL, &isSingleQuoted);

	if (isStringBuilderEmpty(sb)) {
		fatalError("parseExpression() : Expected an expression, found EOF");
		return NULL;
	}

	char * dstBuf = sb->name;

	if (safeAtoi(dstBuf, &dstBufAsInt)) {
		return createExpressionFromValue(createNumericValue(dstBufAsInt));
	} /* else if (isSingleQuoted) {
		/ * In the future, we will count quoted brackets, and keep track of
		the current quote state (i.e. is quoted or is not quoted)
		-> Or let the functions createQuotedValue() / createQuotedValue() match the quoted brackets (i.e. via recursive descent parsing) ?
		We can keep track of state data in the CharSource, if necessary. * /

		/ *  return createExpressionFromValue(createQuotedValue(cs)); * /

		return createApostropheQuotedValueFromValue(createQuotedValue(cs));
	} else if (strlen(dstBuf) >= 2 && dstBuf[0] == '"' && dstBuf[strlen(dstBuf) - 1] == '"') {
		return createExpressionFromValue(createStringValue(dstBuf));
	} */ else if (isPrimop(dstBuf)) {
		return createExpressionFromValue(createPrimitiveOperator(dstBuf));
	} /* else if (!strcmp(dstBuf, "[]")) {
		return createExpressionFromValue(createArray());
	} else if (!strcmp(dstBuf, "{}")) {
		return createExpressionFromValue(createAssociativeArray());
	} */ else if (!strcmp(dstBuf, "(")) {
		return parseBracketedExpression(cs);
	} else if (!strcmp(dstBuf, ")")) {
		fatalError("parseExpression() : Encountered an unexpected ')'");
		return NULL;
	} else {
		return createExpressionFromVariable(createVariable(dstBuf));
	}
}

/* **** The End **** */
