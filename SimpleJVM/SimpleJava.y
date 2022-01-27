%{

#include "stdafx.h"
#include <stdio.h>
#include "SyntaxTree.h"

#pragma warning (disable:4996)
#pragma warning (disable:4267)
#pragma warning (disable:4244)
#pragma warning (disable:4273)
#pragma warning (disable:4065)

// #define YYSTYPE		_SSYYSTYPE
#define YYSTYPE		STreeNode*

int yylex (void);
void yyerror (char const *);

/*void Outputx(const char *str)
{
	printf("%s\n", str);
}*/


/*
 | stmt_function | stmt_class
*/
%}

%token	TK_IF		TK_ELSE		TK_WHILE	TK_DO		TK_FOR		TK_NEW
%token	TK_CONTINUE	TK_BREAK	TK_TRY		TK_CATCH	TK_THROW	TK_THROWS
%token	TK_RETURN	TK_SUPER	TK_NULL		TK_TRUE		TK_FALSE
%token	TK_EXTENDS	TK_PUBLIC	TK_PRIVATE	TK_NATIVE	TK_PROTECTED	TK_IMPLEMENTS
%token	TK_PACKAGE	TK_IMPORT	TK_STATIC	TK_ABSTRACT	TK_CLASS	TK_INTERFACE
%token	TK_ID
%token	TK_NUMBER	TK_STRING
%token	TK_L_PARENTHESIS		TK_R_PARENTHESIS		TK_COLON	TK_SEMICOLON 
%token	TK_L_BRACE	TK_R_BRACE	TK_COMMA	TK_DOT		TK_ASSIGN
%token	TK_OP_PLUS_ASSIGN		TK_OP_MINUS_ASSIGN
%token	TK_OP_INC	TK_OP_DEC	TK_PLUS		TK_MINUS	TK_MULT		TK_DIV
%token	TK_LT_EQ	TK_GT_EQ	TK_LITTLE	TK_GREATER	TK_EQUAL	TK_NOT_EQUAL
%token	TK_BOOL_AND	TK_BOOL_OR
%token	TK_ERROR

%left	TK_BOOL_OR
%left	TK_BOOL_AND	
%left	TK_LT_EQ	TK_GT_EQ	TK_LITTLE	TK_GREATER	TK_EQUAL	TK_NOT_EQUAL
%left	TK_PLUS		TK_MINUS
%left	TK_MULT		TK_DIV
%left	TK_OP_INC	TK_OP_DEC

%%		/* Grammer for skin script */

program				:	decl_seq
						{	_syntaxTree = $$;	}
					;
					
decl_seq			:	decl_seq decl_class
						{
							if ($1 != NULL)
								$$ = AppendToSibling($1, $2);
							else
								$$ = $2;
						}
					|	decl_class
						{	$$ = $1;	}
					|	TK_PACKAGE package_name
					|	TK_IMPORT package_name
					|	TK_SEMICOLON
					;

package_name		:	package_name TK_DOT TK_ID
					|	TK_ID TK_DOT TK_ID
					;

decl_var			:	var_type var_list TK_SEMICOLON
						{
							$$ = newStnVarDeclare($1, $2, false);
						}
					|	TK_STATIC var_type var_list TK_SEMICOLON
						{
							$$ = newStnVarDeclare($2, $3, true);
						}
					;

var_list			:	var_list TK_COMMA TK_ID
						{
							STreeNode	*newTail = newStnVar($3, NULL);
							$$ = AppendToSibling($1, newTail);
						}
					|	var_list TK_COMMA TK_ID TK_ASSIGN expression
						{
							STreeNode	*newTail = newStnVar($3, $5);
							$$ = AppendToSibling($1, newTail);
						}
					|	TK_ID
						{
							$$ = newStnVar($1, NULL);
						}
					|	TK_ID TK_ASSIGN expression
						{
							$$ = newStnVar($1, $3);
						}
					;

throws_exceptions	:	throws_exceptions TK_COMMA TK_ID
						{
							STreeNode	*newTail = newStnVar($3, NULL);
							$$ = AppendToSibling($3, newTail);
						}
					|	TK_ID
						{
							$$ = newStnVar($1, NULL);
						}
					;

throws				:	TK_THROWS throws_exceptions
						{
						}
					|	{ $$ = NULL; }
					;

decl_constructor	:	TK_ID TK_L_PARENTHESIS fun_param_list TK_R_PARENTHESIS throws
						fun_body
						{
							ASSERT($1->nodeType == STreeNode::T_STRING);
							$$ = newStnMethodDeclareNode($1, NULL, $3, $6);
						}

					|	TK_NATIVE TK_ID TK_L_PARENTHESIS fun_param_list TK_R_PARENTHESIS throws TK_SEMICOLON
						{
							// For native function, body = NULL
							ASSERT($2->nodeType == STreeNode::T_STRING);
							$$ = newStnMethodDeclareNode($2, NULL, $4, NULL);
						}
					;

decl_function		:	var_type TK_ID TK_L_PARENTHESIS fun_param_list TK_R_PARENTHESIS throws
						fun_body
						{
							ASSERT($2->nodeType == STreeNode::T_STRING);
							$$ = newStnMethodDeclareNode($2, $1, $4, $7);
						}

					|	TK_NATIVE var_type TK_ID TK_L_PARENTHESIS fun_param_list TK_R_PARENTHESIS throws TK_SEMICOLON
						{
							// For native function, body = NULL
							ASSERT($3->nodeType == STreeNode::T_STRING);
							$$ = newStnMethodDeclareNode($3, $2, $5, NULL);
						}
					;

var_type			:	TK_ID { $$ = $1; }
					;


fun_param_list		:	fun_param TK_COMMA fun_param_list
						{
							$$ = AppendToSibling($1, $3);
						}
					|	fun_param
						{
							$$ = $1;
						}
					|	{	$$ = NULL;	}
					;

fun_param			:	var_type TK_ID
						{
							STreeNode	*temp = newStnVar($2, NULL);
							$$ = newStnVarDeclare($1, temp, false);
						}
					;

fun_body			:	TK_L_BRACE stmt_list TK_R_BRACE
						{
							$$ = $2;
							if ($$ == NULL)
								$$ = newStnBlock(g_srcLineNo, NULL);
						}
					|	TK_L_BRACE TK_R_BRACE
						{
							$$ = newStnBlock(g_srcLineNo, NULL);
						}
					;

stmt_list			:	stmt_list stmt_block
						{
							$$ = AppendToSibling($1, $2);
						}
					|	stmt_block { $$ = $1; }
					;

stmt_brace_block	:	TK_L_BRACE stmt_list TK_R_BRACE
						{
							$$ = newStnBlock(g_srcLineNo, $2);
						}
					|	TK_L_BRACE TK_R_BRACE
						{
							$$ = newStnBlock(g_srcLineNo, NULL);
						}
					;

stmt_block			:	stmt_brace_block { $$ = $1; }
					|	stmt { $$ = $1; }
					;


stmt				:	if_stmt		{ $$ = $1; }
					|	while_stmt	{ $$ = $1; }
					|	for_stmt	{ $$ = $1; }
					|	do_while_stmt	{ $$ = $1; }
					|	TK_BREAK	{ $$ = newStnBreak(g_srcLineNo); }
					|	TK_CONTINUE	{ $$ = newStnContinue(g_srcLineNo); }
					|	TK_SEMICOLON	{ $$ = NULL; }
					|	expression TK_SEMICOLON	{ $$ = $1; }
					|	TK_RETURN expression TK_SEMICOLON { $$ = $1; $1->sReturn.pExpression = $2; }
					|	TK_RETURN TK_SEMICOLON { $$ = $1; }
					|	decl_var { $$ = $1; /* empty */ }
					|	try_stmt	{ $$ = $1; }
					|	throw_exception_stmt { $$ = $1; }
					;

if_stmt				:	TK_IF TK_L_PARENTHESIS expression TK_R_PARENTHESIS stmt_block TK_ELSE stmt_block
						{
							$$ = newStnIf($3, $5, $7);
						}
					|	TK_IF TK_L_PARENTHESIS expression TK_R_PARENTHESIS stmt_block
						{
							$$ = newStnIf($3, $5, NULL);
						}
					;

for_stmt			:	TK_FOR TK_L_PARENTHESIS decl_var expression TK_SEMICOLON expression TK_R_PARENTHESIS stmt_block
						{
							$$ = newStnBlock(g_srcLineNo, newStnFor($3, $4, $6, $8));
						}
					;

while_stmt			:	TK_WHILE TK_L_PARENTHESIS expression TK_R_PARENTHESIS stmt_block
						{
							$$ = newStnWhile($3, $5);
						}
					;

do_while_stmt		:	TK_DO stmt_block TK_WHILE TK_L_PARENTHESIS expression TK_R_PARENTHESIS TK_SEMICOLON
						{
							$$ = newStnDoWhile($5, $2);
						}
					;

try_stmt			:	TK_TRY stmt_brace_block catch_seq
						{
							$$ = newStnTry($2, $3);
						}
					;

catch				:	TK_CATCH TK_L_PARENTHESIS fun_param TK_R_PARENTHESIS stmt_brace_block
						{
							$$ = newStnCatch($3, $5);
						}
					;

catch_seq			:	catch
						{
							$$ = $1;
						}
					|	catch_seq catch
						{
							$$ = appendCatchTail($1, $2);
						}
					;

throw_exception_stmt :	TK_THROW exp_new TK_SEMICOLON
						{
							$$ = newStnThrow($2);
						}
					;

expression			:	TK_NUMBER
						{ $$ = newStnExprConstInt($1); }
					|	TK_STRING
						{ $$ = newStnExprConstString($1); }
					|	TK_TRUE
						{ $$ = newStnExprConstBool(g_srcLineNo, true); }
					|	TK_FALSE
						{ $$ = newStnExprConstBool(g_srcLineNo, false); }
					|	TK_NULL
						{ $$ = newStnExprNull(g_srcLineNo); }
					|	exp_new	{ $$ = $1; }
					|	TK_L_PARENTHESIS expression TK_R_PARENTHESIS
						{ $$ = $2; }
					|	exp_var { $$ = $1; }
					|	exp_fun_call { $$ = $1; }
					|	TK_L_PARENTHESIS TK_ID TK_R_PARENTHESIS exp_var	{ $$ = newStnExpDynamicCast($2, $4); }
					|	expression TK_LT_EQ		 expression { $$ = newStnExpOp(OP_LT_EQ	, $1, $3); }
					|	expression TK_GT_EQ		 expression { $$ = newStnExpOp(OP_GT_EQ	, $1, $3); }
					|	expression TK_LITTLE	 expression { $$ = newStnExpOp(OP_LITTLE	, $1, $3); }
					|	expression TK_GREATER	 expression { $$ = newStnExpOp(OP_GREATER	, $1, $3); }
					|	expression TK_EQUAL		 expression { $$ = newStnExpOp(OP_EQUAL	, $1, $3); }
					|	expression TK_NOT_EQUAL  expression { $$ = newStnExpOp(OP_NOT_EQUAL, $1, $3); }
					|	expression TK_OP_INC				{ $$ = newStnExpOp(OP_OP_INC, $1, NULL); }
					|	expression TK_OP_DEC				{ $$ = newStnExpOp(OP_OP_DEC, $1, NULL); }
					|	expression TK_PLUS		 expression { $$ = newStnExpOp(OP_PLUS		, $1, $3); }
					|	expression TK_MINUS		 expression { $$ = newStnExpOp(OP_MINUS	, $1, $3); }
					|	expression TK_MULT		 expression { $$ = newStnExpOp(OP_MULT		, $1, $3); }
					|	expression TK_DIV		 expression { $$ = newStnExpOp(OP_DIV		, $1, $3); }
					|	expression TK_BOOL_AND	 expression { $$ = newStnExpOp(OP_BOOL_AND	, $1, $3); }
					|	expression TK_BOOL_OR	 expression { $$ = newStnExpOp(OP_BOOL_OR	, $1, $3); }
					|	exp_var TK_OP_PLUS_ASSIGN expression { $$ = newStnExpOp(OP_PLUS_ASSIGN	, $1, $3); }
					|	exp_var TK_OP_MINUS_ASSIGN expression { $$ = newStnExpOp(OP_MINUS_ASSIGN	, $1, $3); }
					|	exp_var TK_ASSIGN expression		{ $$ = newStnExpAssign($1, $3); }
					;

exp_new				:	TK_NEW TK_ID TK_L_PARENTHESIS fun_call_param_list TK_R_PARENTHESIS
						{ $$ = newStnExpNew($2, $4); }
					;

exp_var				:	exp_var TK_DOT TK_ID
						{
							$$ = newStnExpVar($3, $1);
						}
					|	TK_ID
						{
							$$ = newStnExpVar($1, NULL);
						}
					;

exp_fun_call		:	exp_var TK_L_PARENTHESIS fun_call_param_list TK_R_PARENTHESIS
						{
							// Get Method name, and Parent Var
							STreeNode	*pParentVar = NULL;
							STreeNode	*pMethodName = $1;
							if (pMethodName) {
								if (pMethodName->sExpression.var.pVarParent != NULL)
								{
									pParentVar = pMethodName->sExpression.var.pVarParent;
									pMethodName->sExpression.var.pVarParent = NULL;
								}
							}

							ASSERT(pMethodName->nodeType == STreeNode::T_EXPRESSION &&
								pMethodName->sExpression.type == E_VAR);
							$$ = newStnExpFuncCall(pMethodName->nLineNo, pMethodName->sExpression.var.szVarName, pParentVar, $3);
							deleteStn(pMethodName);
						}
					;

fun_call_param_list	:	fun_call_param_list TK_COMMA expression
						{
							$$ = AppendToSibling($1, $3);
						}
					|	expression
						{
							$$ = $1;
						}
					|	{  /* no param */ $$ = NULL; }
					;

decl_class			:	TK_CLASS TK_ID TK_L_BRACE class_member_decl_seq TK_R_BRACE
						{
							$$ = newStnClassDeclare($2, NULL, $4);
						}
					|	TK_CLASS TK_ID TK_EXTENDS TK_ID TK_L_BRACE class_member_decl_seq TK_R_BRACE
						{
							$$ = newStnClassDeclare($2, $4, $6);
						}

					|	TK_CLASS TK_ID TK_L_BRACE TK_R_BRACE
						{
							$$ = newStnClassDeclare($2, NULL, NULL);
						}
					|	TK_CLASS TK_ID TK_EXTENDS TK_ID TK_L_BRACE TK_R_BRACE
						{
							$$ = newStnClassDeclare($2, $4, NULL);
						}

					;

class_member_decl_seq	:	class_member_decl_seq class_member_decl
						{
							$$ = AppendToSibling($1, $2);
						}
					|	class_member_decl
					;

class_member_decl		:	decl_var		{	$$ = $1;	}
					|	decl_constructor	{	$$ = $1;	}
					|	decl_function		{	$$ = $1;	}
					|	TK_STATIC decl_function	{	$$ = $2; $2->sMemberMethod.bStatic = true;	}
					|	TK_SEMICOLON
					;

%%

void yyerror (char const * s)
{
    printf("%s on line: %d\n", s, g_srcLineNo);
}

/*
void main(int  argc, char **argv )
{
    ++argv, --argc;
    if ( argc > 0 )
            yyin = fopen( argv[0], "r" );
    else
            yyin = stdin;

    yyparse();

}
*/