/* lackadaisical/src/types.h */

/* Preprocessor defines */

#if !defined(BOOL) && !defined(FALSE) && !defined(TRUE)
/* Our poor man's Boolean data type: */
#define BOOL int
#define FALSE 0
#define TRUE 1
#endif

/* Type definitions */

/* Definitions of values and expressions: */
/* Every value is an expression. */
/* Every expression can be evaluated to a value. */

typedef struct SCHEME_UNIVERSAL_STRUCT {
	/* Contains ten members. */

	int mark; /* All dynamically allocated structs must have this member */

	int type;
	/* TODO: Use a union? */
	int integerValue;

	int maxNameLength; /* Size (in chars) of the allocated buffer to which name points */
	char * name; /* Or use the char name[1]; trick at the end of the struct? */

	struct SCHEME_UNIVERSAL_STRUCT * value1;
	struct SCHEME_UNIVERSAL_STRUCT * value2;
	struct SCHEME_UNIVERSAL_STRUCT * value3;

	struct SCHEME_UNIVERSAL_STRUCT * next; /* To allow linked lists */

	/* void * aux; */
} SCHEME_UNIVERSAL_TYPE;

#define LISP_CLOSURE SCHEME_UNIVERSAL_TYPE
#define LISP_ENV SCHEME_UNIVERSAL_TYPE
#define LISP_EXPR SCHEME_UNIVERSAL_TYPE
#define LISP_EXPR_LIST_ELEMENT SCHEME_UNIVERSAL_TYPE
#define LISP_EXPR_PAIR_LIST_ELEMENT SCHEME_UNIVERSAL_TYPE
#define LISP_FUNCTION_CALL SCHEME_UNIVERSAL_TYPE
#define LISP_LAMBDA_EXPR SCHEME_UNIVERSAL_TYPE
#define LISP_NAME_VALUE_LIST_ELEMENT SCHEME_UNIVERSAL_TYPE
#define LISP_PAIR SCHEME_UNIVERSAL_TYPE
#define LISP_VALUE SCHEME_UNIVERSAL_TYPE
#define LISP_VALUE_LIST_ELEMENT SCHEME_UNIVERSAL_TYPE
#define LISP_VAR SCHEME_UNIVERSAL_TYPE
#define LISP_VAR_EXPR_PAIR_LIST_ELEMENT SCHEME_UNIVERSAL_TYPE
#define LISP_VAR_LIST_ELEMENT SCHEME_UNIVERSAL_TYPE
#define STRING_BUILDER_TYPE SCHEME_UNIVERSAL_TYPE

/* #define getHeadInArray(a) ((a)->value1)

#define getValueInArrayListElement(ale) ((ale)->value1)

#define getNumBucketsInAssociativeArray(aa) ((aa)->integerValue)

#define getKeyInAssociativeArrayListElement(aale) ((aale)->value1)
#define getValueInAssociativeArrayListElement(aale) ((aale)->value2) */

#define getExprListInBeginExpr(be) ((be)->value1)

#define getArgsInClosure(c) ((c)->value1)
#define getBodyInClosure(c) ((c)->value3)
#define getEnvInClosure(c) ((c)->value2)

#define getNameValuePairListInEnv(env) ((env)->value1)

#define getExprInExprList(el) ((el)->value1)

#define getFirstExprInFunctionCall(fc) ((fc)->value2)
#define getActualParamExprsInFunctionCall(fc) ((fc)->value1)

#define getArgsInLambdaExpr(le) ((le)->value1)
#define getBodyInLambdaExpr(le) ((le)->value2)

/* #define getNameInMacro(m) ((m)->name)
#define getArgsInMacro(m) ((m)->value1)
#define getBodyInMacro(m) ((m)->value2)

#define getMacroInMacroListElement(mle) ((mle)->value1) */

#define getValueInNameValuePairListElement(nvple) ((nvple)->value1)

#define getHeadInPair(p) ((p)->value1)
#define getTailInPair(p) ((p)->value2)

#define getExprInPairListElement(ple) ((ple)->value1)
#define getExpr2InPairListElement(ple) ((ple)->value2)

#define getVarInSetExpr(se) ((se)->value1)
#define getExprInSetExpr(se) ((se)->value2)

#define getBufferSizeIncrementInStringBuilder(sb) ((sb)->integerValue)
#define getStringInStringBuilder(sb) ((sb)->name)

#define getValueInValueListElement(vle) ((vle)->value1)

#define getExprInVarExprPairListElement(veple) ((veple)->value1)

#define getIntegerValueInValue(v) ((v)->integerValue)
#define getNameInValue(v) ((v)->name)
/* #define getContinuationIdInValue(v) ((v)->integerValue)
#define getContinuationReturnValueInValue(v) ((v)->value1) */

#define getExprListInExpr(e) ((e)->value1)
#define getExprInExpr(e) ((e)->value2)
#define getExpr2InExpr(e) ((e)->value1)
#define getVarExprPairListInExpr(e) ((e)->value1)
#define getExprPairListInExpr(e) ((e)->value1)

#define getValueInExpr(e) ((e)->value1)
#define getVarInExpr(e) ((e)->value1)

/* #define getValueInApostropheQuotedValue(e) ((e)->value1)
#define getValueInQuoteQuotedValue(e) ((e)->value1) */

#define getValueInExThunk(et) ((et)->value1)

/* The NameValueList is a crude dictionary of values. */

enum {
	lispType_FirstValueType,
	lispValueType_Number = lispType_FirstValueType,
	lispValueType_PrimitiveOperator,
	lispValueType_Closure,
	lispValueType_Pair,
	lispValueType_Null, /* 6 */ /* TODO? Interpret the NULL pointer as a Null value? */
	/* lispValueType_String,
	lispValueType_Symbol,
	lispValueType_QuotedConstantWithApostrophe, / * `'${sexpression}` * /
	lispValueType_QuotedConstantWithQuoteKeyword, / * `(quote ${sexpression})` * /
	lispValueType_Array,
	lispValueType_AssociativeArray, / * I.e. a Dictionary * /
	lispPseudoValueType_Continuation,
	lispPseudoValueType_ContinuationReturn, */
	lispType_LastValueType = lispValueType_Null,
	lispValueType_Thunk, /* -> A suspended computation; used to implement lazy evaluation in SASL */
	lispValueType_ExThunk, /* An evaluated thunk; it points to the actual value */
	/* lispValueType_Exception, */

	/* lispType_FirstExpressionType, */
	lispExpressionType_Value /* = lispType_FirstExpressionType */,
	lispExpressionType_Variable,
	lispExpressionType_LambdaExpr,
	lispExpressionType_FunctionCall,
	lispExpressionType_SetExpr,
	lispExpressionType_Let,
	/* lispExpressionType_LetStar,
	lispExpressionType_Letrec,
	lispExpressionType_Begin,
	lispExpressionType_While,
	lispExpressionType_Cons,
	lispExpressionType_Cond,
	lispExpressionType_Car,
	lispExpressionType_Cdr,
	lispExpressionType_CallCC,
	lispExpressionType_Macro,
	lispType_LastExpressionType = lispExpressionType_DefineMacro, */

	schemeStructType_Environment,
	schemeStructType_ExpressionListElement,
	schemeStructType_NameValueListElement,
	schemeStructType_VariableExpressionPairListElement,
	schemeStructType_VariableListElement,
	/* schemeStructType_ArrayListElement,
	schemeStructType_AssociativeArrayListElement,
	schemeStructType_ExpressionPairListElement,
	schemeStructType_MacroListElement,
	schemeStructType_ValueListElement, */

	stringBuilderType
};

void fail(char * str, char * file, int line);

#define failIf(b, str) if (b) { fail(str, __FILE__, __LINE__); }

#define fatalError(str) fail(str, __FILE__, __LINE__)

/* **** The End **** */
