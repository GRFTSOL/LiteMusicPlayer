%{

/*\"[^"]*\"	printf( "String: %s\n", yytext );
*/
#include <stdio.h>
#include <math.h>
#include "SyntaxTree.h"
#define YY_NEVER_INTERACTIVE	1
#define YYSTYPE		STreeNode*
#define YY_NEVER_INTERACTIVE 1

#include "SimpleJava.tab.h"

#pragma warning (disable:4996)
#pragma warning (disable:4267)
#pragma warning (disable:4244)
#pragma warning (disable:4273)

#ifdef _DEBUG
#define	INC_LINENO()	g_srcLineNo++
#define	IF_INC_LINENO(c)	if (c == '\n') g_srcLineNo++
#else
#define	INC_LINENO()
#define	IF_INC_LINENO(c)	
#endif


%}

DIGIT		[0-9]
ID			[a-zA-Z_][a-zA-Z0-9_]*

%%

[+-]?[0-9]+	{
				yylval = newStnString(g_srcLineNo, yytext);
				return TK_NUMBER;
            }
            
private		{ /* return TK_PRIVATE; */ }
protected	{ /* return TK_PROTECTED; */ }
public		{ /* return TK_PUBLIC; */ }

interface	{ return TK_INTERFACE; }
implements	{ return TK_IMPLEMENTS; }
extends		{ return TK_EXTENDS; }
abstract	{ return TK_ABSTRACT; }
class		{ return TK_CLASS; }
native		{ return TK_NATIVE; }
import		{ return TK_IMPORT; }
package		{ return TK_PACKAGE; }
static		{ return TK_STATIC; }
new			{ return TK_NEW; }
if			{ return TK_IF; }
else		{ return TK_ELSE; }
while		{ return TK_WHILE; }
do			{ return TK_DO; }
for			{ return TK_FOR; }
break		{ return TK_BREAK; }
continue	{ return TK_CONTINUE; }
try			{ return TK_TRY; }
catch		{ return TK_CATCH; }
throw		{ return TK_THROW; }
throws		{ return TK_THROWS; }
return		{
				yylval = newStnReturn(g_srcLineNo, NULL);
				return TK_RETURN;
			}
true		{ return TK_TRUE; }
false		{ return TK_FALSE; }
null		{ return TK_NULL; }

"&&"		{ return TK_BOOL_AND; }
"||"		{ return TK_BOOL_OR; }

"<="		{ return TK_LT_EQ; }
">="		{ return TK_GT_EQ; }
"<"			{ return TK_LITTLE; }
">"			{ return TK_GREATER; }
"=="		{ return TK_EQUAL; }
"!="		{ return TK_NOT_EQUAL; }

"("			{ return TK_L_PARENTHESIS; }
")"			{ return TK_R_PARENTHESIS; }
":"			{ return TK_COLON; }
";"			{ return TK_SEMICOLON; }
"{"			{ return TK_L_BRACE; }
"}"			{ return TK_R_BRACE; }
","			{ return TK_COMMA; }
"."			{ return TK_DOT; }
"="			{ return TK_ASSIGN; }

"++"		{ return TK_OP_INC; }
"--"		{ return TK_OP_DEC; }
"+="		{ return TK_OP_PLUS_ASSIGN; }
"-="		{ return TK_OP_MINUS_ASSIGN; }
"+"			{ return TK_PLUS; }
"-"			{ return TK_MINUS; }
"*"			{ return TK_MULT; }
"/"			{ return TK_DIV; }


[A-Za-z_]+[A-Za-z_0-9]* 	{
							 	yylval = newStnString(g_srcLineNo, yytext);
								return TK_ID;
							}


\"			{
				/* string */
				char	c;
				// char	str[265];
				string	str;
				do
				{
					c = yyinput();
					// str[nLen++] = c;
					str += c;

					while (c != '\\' && c != '\"' && c != EOF && c != '\n')
					{
						c = yyinput();
						// str[nLen++] = c;
						str += c;
					}

					if (c == '\\')
					{
						c = yyinput();
						// str[nLen++] = c;
						str += c;
					}
					else
						break;
				}
				while (c != EOF && c != '\n');

				if (c != '\"')
					return TK_ERROR;

				str.resize(str.size() - 1);
				// str[nLen - 1] = '\0';

				// printf("An String: %s\n", str);
				yylval = newStnString(g_srcLineNo, str.c_str());
				return TK_STRING;
			}

[ \t\r]+		{	/* eat up whitespace */ }

\n			{ INC_LINENO(); }

"//"		{
				/* eat cpp comment */
				char	c;
				c = yyinput();
				while (c != '\n' && c != EOF)
					c = yyinput();
				IF_INC_LINENO(c);
			}

"/*"        {
				/* eat c comment */
				int c;

				for ( ; ; )
                {
					while ( (c = yyinput()) != '*' &&
							c != EOF )
					{
						/* eat up text of comment */
						IF_INC_LINENO(c);
					}

					if ( c == '*' )
					{
						while ( (c = yyinput()) == '*' )
							;
						if ( c == '/' )
							break;    /* found the end */
					}

					if ( c == EOF )
					{
						printf( "EOF in comment" );
						break;
					}
				}
            }

.           {
				printf( "Unrecognized character: %s\n", yytext );
				return TK_ERROR;
			}

%%

int yywrap(void)
{
	return 1;
}

/*
int yyparse();

void main(int  argc, char **argv )
{
    ++argv, --argc;
    if ( argc > 0 )
	{
            yyin = fopen( argv[0], "r" );
			if (!yyin)
				printf("Can't open file: %s", argv[0]);
	}
    else
            yyin = stdin;

    // yylex();
	yyparse();

	int		nIndent = 0;
	PrintSyntaxTree(_syntaxTree, nIndent);
}
*/