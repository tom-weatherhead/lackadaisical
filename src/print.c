/* atrocity/src/print.c */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "types.h"

#include "create-and-destroy.h"
#include "print.h"
#include "string-builder.h"
#include "utilities.h"

void printValue(LISP_VALUE * value) {
	/* SCHEME_UNIVERSAL_TYPE * ptr = NULL;
	char * separatorStr = ""; */

	if (value == NULL) {
		printf("NULL");

		return;
	} else if (isList(value) /* && value->type != lispValueType_Null */) {
		char separator = '\0';

		printf("(");

		while (value->type != lispValueType_Null) {
			printf("%c", separator);
			printValue(getHeadInPair(value));
			separator = ' ';
			value = getTailInPair(value);
		}

		printf(")");

		return;
	}

	switch (value->type) {
		case lispValueType_Number:
			printf("%d", getIntegerValueInValue(value));
			break;

		/* case lispValueType_String:
			printf("\"%s\"", getNameInValue(value));
			break;

		case lispValueType_Symbol:
			printf("'%s", getNameInValue(value));
			break; */

		case lispValueType_PrimitiveOperator:
			printf("%s", getNameInValue(value));
			break;

		case lispValueType_Closure:
			printf("<closure>");
			break;

		case lispValueType_Pair:
			printf("Pair: (");
			printValue(getHeadInPair(value));
			printf(" . ");
			printValue(getTailInPair(value));
			printf(")");
			break;

		/* case lispValueType_Null:
			printf("()");
			break; */

		/* case lispValueType_Array:
			printf("[");

			for (ptr = getHeadInArray(value); ptr != NULL; ptr = ptr->next) {
				printf("%s", separatorStr);
				printValue(getValueInArrayListElement(ptr));
				separatorStr = ", ";
			}

			printf("]");
			break;

		case lispValueType_AssociativeArray:
			printf("<associative array (i.e. dictionary)>");
			break;

		case lispPseudoValueType_Continuation:
			printf("<continuation; id %d>", getContinuationIdInValue(value));
			break;

		case lispPseudoValueType_ContinuationReturn:
			printf("<continuation return; id %d, value: ", getContinuationIdInValue(value));
			printValue(getContinuationReturnValueInValue(value));
			printf(">");
			break; */

		default:
			printf("<invalid value>");
			break;
	}
}

STRING_BUILDER_TYPE * printValueToStringEx(STRING_BUILDER_TYPE * sb, LISP_VALUE * value, char * separatorBetweenListItems, BOOL printBracketsAroundList) {
	/* SCHEME_UNIVERSAL_TYPE * ptr = NULL;
	char * separatorStr = ""; */

	if (sb == NULL) {
		sb = createStringBuilder(0);
	}

	if (separatorBetweenListItems == NULL) {
		separatorBetweenListItems = " ";
	}

	if (isList(value) && value->type != lispValueType_Null) {
		char * separator = "";

		if (printBracketsAroundList) {
			appendToStringBuilder(sb, "(");
		}

		while (value->type != lispValueType_Null) {
			appendToStringBuilder(sb, separator);

			printValueToStringEx(sb, getHeadInPair(value), separatorBetweenListItems, TRUE);

			separator = separatorBetweenListItems;
			value = getTailInPair(value);
		}

		if (printBracketsAroundList) {
			appendToStringBuilder(sb, ")");
		}

		return sb;
	} else if (value->type == lispValueType_Pair) {
		appendToStringBuilder(sb, "(");
		printValueToStringEx(sb, getHeadInPair(value), separatorBetweenListItems, printBracketsAroundList);
		appendToStringBuilder(sb, " . ");
		printValueToStringEx(sb, getTailInPair(value), separatorBetweenListItems, printBracketsAroundList);
		appendToStringBuilder(sb, ")");

		return sb;
	}

	/* char * strContinuation = "<contin>";
	char * strContinuationReturn = "<contRtn>"; */
	char * strClosure = "<closure>";
	char * strInvalid = "<invalid value>";

	/* (listtostring '("abc" 123 "def")) -> TODO: BUG: Double quotes are not removed from string literals inside a (single-)quoted list */
	char intSprintfBuffer[16];

	switch (value->type) {
		case lispValueType_Number:
			memset(intSprintfBuffer, 0, 16 * sizeof(char));
			sprintf(intSprintfBuffer, "%d", getIntegerValueInValue(value));
			appendToStringBuilder(sb, intSprintfBuffer);
			break;

		case lispValueType_PrimitiveOperator:
		/* case lispValueType_String:
		case lispValueType_Symbol: */
			appendToStringBuilder(sb, getNameInValue(value));
			break;

		case lispValueType_Closure:
			appendToStringBuilder(sb, strClosure);
			break;

		case lispValueType_Null:
			appendToStringBuilder(sb, "()");
			break;

		/* case lispValueType_AssociativeArray:
			appendToStringBuilder(sb, "<associative array>");
			break;

		case lispValueType_Array:
			appendToStringBuilder(sb, "[");

			for (ptr = getHeadInArray(value); ptr != NULL; ptr = ptr->next) {
				appendToStringBuilder(sb, separatorStr);
				printValueToStringEx(sb, getValueInArrayListElement(ptr), separatorBetweenListItems, printBracketsAroundList);
				separatorStr = ", ";
			}

			appendToStringBuilder(sb, "]");
			break;

		case lispPseudoValueType_Continuation:
			appendToStringBuilder(sb, strContinuation);
			break;

		case lispPseudoValueType_ContinuationReturn:
			appendToStringBuilder(sb, strContinuationReturn);
			break; */

		default:
			appendToStringBuilder(sb, strInvalid);
			break;
	}

	return sb;
}

STRING_BUILDER_TYPE * printValueToString(STRING_BUILDER_TYPE * sb, LISP_VALUE * value) {
	return printValueToStringEx(sb, value, NULL, FALSE);
}

static void printLetExpressionToString(STRING_BUILDER_TYPE * sb, char * letKeyword, LISP_EXPR * expr, BOOL (*fnHandler)(STRING_BUILDER_TYPE * sb, LISP_EXPR * expr)) {
	char * separator = "";
	LISP_VAR_EXPR_PAIR_LIST_ELEMENT * varExprPairList;

	appendToStringBuilder(sb, "(");
	appendToStringBuilder(sb, letKeyword);
	appendToStringBuilder(sb, " (");

	for (varExprPairList = getVarExprPairListInExpr(expr); varExprPairList != NULL; varExprPairList = varExprPairList->next) {
		appendToStringBuilder(sb, separator);
		appendToStringBuilder(sb, "(");
		appendToStringBuilder(sb, varExprPairList->name);
		appendToStringBuilder(sb, " ");
		printExpressionToStringEx(sb, getExprInVarExprPairListElement(varExprPairList), fnHandler);
		appendToStringBuilder(sb, ")");
		separator = " ";
	}

	appendToStringBuilder(sb, ") ");
	printExpressionToStringEx(sb, getExprInExpr(expr), fnHandler);
	appendToStringBuilder(sb, ")");
}

STRING_BUILDER_TYPE * printExpressionToStringEx(STRING_BUILDER_TYPE * sb, LISP_EXPR * expr, BOOL (*fnHandler)(STRING_BUILDER_TYPE * sb, LISP_EXPR * expr)) {
	LISP_EXPR_LIST_ELEMENT * exprList;

	if (sb == NULL) {
		sb = createStringBuilder(0);
	}

	if (fnHandler != NULL && (*fnHandler)(sb, expr)) {
		return sb;
	}

	switch (expr->type) {
		/* case lispExpressionType_Begin:
			appendToStringBuilder(sb, "(begin");

			for (exprList = getExprListInBeginExpr(expr); exprList != NULL; exprList = exprList->next) {
				appendToStringBuilder(sb, " ");
				printExpressionToStringEx(sb, getExprInExprList(exprList), fnHandler);
			}

			appendToStringBuilder(sb, ")");
			break; */

		case lispExpressionType_FunctionCall:
			appendToStringBuilder(sb, "(");
			printExpressionToStringEx(sb, getFirstExprInFunctionCall(expr), fnHandler);

			for (exprList = getActualParamExprsInFunctionCall(expr); exprList != NULL; exprList = exprList->next) {
				appendToStringBuilder(sb, " ");
				printExpressionToStringEx(sb, getExprInExprList(exprList), fnHandler);
			}

			appendToStringBuilder(sb, ")");
			break;

		case lispExpressionType_Let:
			printLetExpressionToString(sb, "let", expr, fnHandler);
			break;

		/* case lispExpressionType_LetStar:
			printLetExpressionToString(sb, "let*", expr, fnHandler);
			break;

		case lispExpressionType_Letrec:
			printLetExpressionToString(sb, "letrec", expr, fnHandler);
			break;

		case lispValueType_QuotedConstantWithApostrophe:
			appendToStringBuilder(sb, "'");
			printExpressionToStringEx(sb, getValueInApostropheQuotedValue(expr), fnHandler);
			break;

		case lispValueType_QuotedConstantWithQuoteKeyword:
			appendToStringBuilder(sb, "(quote ");
			printExpressionToStringEx(sb, getValueInQuoteQuotedValue(expr), fnHandler);
			appendToStringBuilder(sb, ")");
			break; */

		case lispExpressionType_SetExpr:
			appendToStringBuilder(sb, "(set! ");
			appendToStringBuilder(sb, getVarInSetExpr(expr)->name);
			appendToStringBuilder(sb, " ");
			printExpressionToStringEx(sb, getExprInSetExpr(expr), fnHandler);
			appendToStringBuilder(sb, ")");
			break;

		case lispExpressionType_Value:
			printValueToString(sb, getValueInExpr(expr));
			break;

		case lispExpressionType_Variable:
			appendToStringBuilder(sb, getVarInExpr(expr)->name);
			break;

		/* case lispExpressionType_While:
			appendToStringBuilder(sb, "(while ");
			printExpressionToStringEx(sb, getExprInExpr(expr), fnHandler);
			appendToStringBuilder(sb, " ");
			printExpressionToStringEx(sb, getExpr2InExpr(expr), fnHandler);
			appendToStringBuilder(sb, ")");
			break;

		case lispExpressionType_Cond: */
		case lispExpressionType_LambdaExpr:
		default:
			fprintf(stderr, "printExpressionToStringEx() : expr type is %d\n", expr->type);
			fprintf(stderr, "printExpressionToStringEx() : lispExpressionType_SetExpr is %d\n", lispExpressionType_SetExpr);
			fatalError("printExpressionToStringEx() : Unsupported expr type");
			break;
	}

	return sb;
}

STRING_BUILDER_TYPE * printExpressionToString(STRING_BUILDER_TYPE * sb, LISP_EXPR * expr) {
	return printExpressionToStringEx(sb, expr, NULL);
}

/* **** The End **** */
