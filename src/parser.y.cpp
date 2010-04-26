%{
/*
 * This file is part of the Hybris programming language interpreter.
 *
 * Copyleft of Simone Margaritelli aka evilsocket <evilsocket@gmail.com>
 *
 * Hybris is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Hybris is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Hybris.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "hybris.h"
#include "engine.h"
#include <getopt.h>

#define YY_(s) (char *)s
/*
 * Make bison parsing errors more verbose rather than a
 * idiot "syntax error" -.-
 *
 * A note to Bison developers :
 * For God sake, why don't you guys document this macro in a
 * more detailed way!
 * I spent 2 days just to end up reversing the bison generated
 * code to "find out" YYERROR_VERBOSE!!!
 */
#define YYERROR_VERBOSE 1

/** macros to define parse tree **/
/* get the node evaluation */
#define MK_NODE(a)                a
/* delete the node */
#define RM_NODE(a)                delete (a)
/* node list */
#define MK_NODE_LIST()            new NodeList()
/* identifiers, attributes and constants */
#define MK_IDENT_NODE(a)          new IdentifierNode(a)
#define MK_ATTR_NODE(a,b)		  new IdentifierNode(a,b)
#define MK_CONST_NODE(a)          new ConstantNode(a)
#define MK_IDLST_NODE(a)          new AttributeNode(a)
/* statements */
#define MK_WHILE_NODE(a, b)       new StatementNode( T_WHILE, 2, a, b )
#define MK_DO_NODE(a, b)          new StatementNode( T_DO, 2, a, b )
#define MK_FOR_NODE(a,b,c,d)      new StatementNode( T_FOR, 4, a, b, c, d )
#define MK_FOREACH_NODE(a,b,c)    new StatementNode( T_FOREACH, 3, MK_IDENT_NODE(a), b, c )
#define MK_FOREACHM_NODE(a,b,c,d) new StatementNode( T_FOREACHM, 4, MK_IDENT_NODE(a), MK_IDENT_NODE(b), c, d )
#define MK_BREAK_NODE()		  	  new StatementNode( T_BREAK, 0 )
#define MK_NEXT_NODE()		  	  new StatementNode( T_NEXT, 0 )
#define MK_RETURN_NODE(a)         new StatementNode( T_RETURN, 1, a )
#define MK_IF_NODE(a, b)          new StatementNode( T_IF, 2, a, b )
#define MK_IF_ELSE_NODE(a, b, c)  new StatementNode( T_IF, 3, a, b, c )
#define MK_SWITCH_NODE(a, b)      new StatementNode( T_SWITCH, a, b )
#define MK_SWITCH_DEF_NODE(a,b,c) new StatementNode( T_SWITCH, a, b, c )
#define MK_THROW_NODE(a)		  new StatementNode( T_THROW, 1, a )
#define MK_TRYCATCH_NODE(a,b,c,d) new StatementNode( T_TRY, 4, a, b, c, d )
#define MK_FUNCTION_NODE(a, b)    new FunctionNode( a, 1, b )
#define MK_STRUCT_NODE(a, b)      new StructureNode( a, b )
#define MK_CLASS_NODE(a, b, c)    new ClassNode( a, b, c )
#define MK_METHOD_NODE(a, b, c)   new MethodNode( a, b, 1, c )
/* expressions */
#define MK_EOSTMT_NODE(a, b)      new ExpressionNode( T_EOSTMT, 2, a, b )
#define MK_DOLLAR_NODE(a)		  new ExpressionNode( T_DOLLAR, 1, a )
#define MK_ASSIGN_NODE(a, b)      new ExpressionNode( T_ASSIGN, 2, a, b )
#define MK_SB_NODE(a, b)	      new ExpressionNode( T_SUBSCRIPTGET, 2, a, b )
#define MK_SB_PUSH_NODE(a, b)     new ExpressionNode( T_SUBSCRIPTADD, 2, a, b )
#define MK_SB_SET_NODE(a, b, c)   new ExpressionNode( T_SUBSCRIPTSET, 3, a, b, c )
#define MK_RANGE_NODE(a, b)		  new ExpressionNode( T_RANGE, 2, a, b )
#define MK_UMINUS_NODE(a)		  new ExpressionNode( T_UMINUS, 1, a )
#define MK_DOT_NODE(a, b)		  new ExpressionNode( T_DOT, 2, a, b )
#define MK_DOTE_NODE(a, b)		  new ExpressionNode( T_DOTE, 2, a, b )
#define MK_PLUS_NODE(a, b)		  new ExpressionNode( T_PLUS, 2, a, b )
#define MK_PLUSE_NODE(a, b)		  new ExpressionNode( T_PLUSE, 2, a, b )
#define MK_MINUS_NODE(a, b)		  new ExpressionNode( T_MINUS, 2, a, b )
#define MK_MINUSE_NODE(a, b)	  new ExpressionNode( T_MINUSE, 2, a, b )
#define MK_MUL_NODE(a, b)		  new ExpressionNode( T_MUL, 2, a, b )
#define MK_MULE_NODE(a, b)		  new ExpressionNode( T_MULE, 2, a, b )
#define MK_DIV_NODE(a, b)		  new ExpressionNode( T_DIV, 2, a, b )
#define MK_DIVE_NODE(a, b)		  new ExpressionNode( T_DIVE, 2, a, b )
#define MK_MOD_NODE(a, b)		  new ExpressionNode( T_MOD, 2, a, b )
#define MK_INC_NODE(a)			  new ExpressionNode( T_INC, 1, a )
#define MK_DEC_NODE(a)			  new ExpressionNode( T_DEC, 1, a )
#define MK_XOR_NODE(a, b)		  new ExpressionNode( T_XOR, 2, a, b )
#define MK_XORE_NODE(a, b)		  new ExpressionNode( T_XORE, 2, a, b )
#define MK_NOT_NODE(a)			  new ExpressionNode( T_NOT, 1, a )
#define MK_AND_NODE(a, b)		  new ExpressionNode( T_AND, 2, a, b )
#define MK_ANDE_NODE(a, b)		  new ExpressionNode( T_ANDE, 2, a, b )
#define MK_OR_NODE(a, b)		  new ExpressionNode( T_OR, 2, a, b )
#define MK_ORE_NODE(a, b)		  new ExpressionNode( T_ORE, 2, a, b )
#define MK_SHIFTL_NODE(a, b)	  new ExpressionNode( T_SHIFTL, 2, a, b )
#define MK_SHIFTLE_NODE(a, b)	  new ExpressionNode( T_SHIFTLE, 2, a, b )
#define MK_SHIFTR_NODE(a, b)	  new ExpressionNode( T_SHIFTR, 2, a, b )
#define MK_SHIFTRE_NODE(a, b)	  new ExpressionNode( T_SHIFTRE, 2, a, b )
#define MK_FACT_NODE(a)			  new ExpressionNode( T_FACT, 1, a )
#define MK_LNOT_NODE(a)			  new ExpressionNode( T_L_NOT, 1, a )
#define MK_LESS_NODE(a, b)		  new ExpressionNode( T_LESS, 2, a, b )
#define MK_GREATER_NODE(a, b)	  new ExpressionNode( T_GREATER, 2, a, b )
#define MK_GE_NODE(a, b)		  new ExpressionNode( T_GREATER_EQ, 2, a, b )
#define MK_LE_NODE(a, b)		  new ExpressionNode( T_LESS_EQ, 2, a, b )
#define MK_NE_NODE(a, b)		  new ExpressionNode( T_NOT_SAME, 2, a, b )
#define MK_EQ_NODE(a, b)		  new ExpressionNode( T_SAME, 2, a, b )
#define MK_LAND_NODE(a, b)		  new ExpressionNode( T_L_AND, 2, a, b )
#define MK_LOR_NODE(a, b)		  new ExpressionNode( T_L_OR, 2, a, b )
#define MK_PCRE_NODE(a, b)		  new ExpressionNode( T_REGEX_OP, 2, a, b )
#define MK_NEW_NODE( a, b )		  new NewNode( a, b )
#define MK_CALL_NODE(a, b)        new CallNode( a, b )
#define MK_METHOD_CALL_NODE(a,b)  new MethodCallNode( a, b )
#define MK_QUESTION_NODE(a, b, c) new StatementNode( T_QUESTION, 3, a, b, c )

extern int yyparse(void);
extern int yylex(void);
/*
 * The global virtual machine holder.
 */
VM __hyb_vm;
%}

%union {
    /* base types */
    long    integer;
    double  real;
    char    byte;
    char    string[0xFF];
    /* variable identifier */
    char    identifier[0xFF];
    /* function prototype declaration */
    function_decl_t *function;
    /* method prototype declaration */
    method_decl_t *method;
    /* node list for multiple nodes on one statement */
    NodeList *list;
    /* not reduced node */
    Node *node;
    /* access specifiers */
    access_t access;
};

%locations

%token <integer>    T_INTEGER
%token <real>       T_REAL;
%token <byte>       T_CHAR;
%token <string>     T_STRING;
%token <identifier> T_IDENT;
%token <function>   T_FUNCTION_PROTOTYPE;
%token <method>   	T_METHOD_PROTOTYPE;

%token T_EOSTMT
%token T_ASSIGN
%token T_RANGE
%token T_SUBSCRIPTADD
%token T_SUBSCRIPTSET
%token T_SUBSCRIPTGET
%token T_WHILE
%token T_DO
%token T_FOR
%token T_FOREACH
%token T_FOREACHM
%token T_OF
%token T_TO
%token T_IF
%token T_SWITCH
%token T_CASE
%token T_BREAK
%token T_NEXT
%token T_DEFAULT
%token T_QUESTION
%token T_DOLLAR
%token T_MAPS
%token T_PTR
%token T_OBJ
%token T_RETURN
%token T_CALL
%token T_NEW
%token T_STRUCT
%token T_CLASS
%token T_EXTENDS
%token T_PUBLIC
%token T_PRIVATE
%token T_PROTECTED
%token T_THROW
%token T_TRY
%token T_CATCH
%token T_FINALLY

%nonassoc T_IF_END
%nonassoc T_SB_END
%nonassoc T_SWITCH_END
%nonassoc T_CALL_END
%nonassoc T_ELSE
%nonassoc T_DOT_END
%nonassoc T_NEW_END

%left T_L_NOT T_L_AND T_L_OR
%left T_LESS T_GREATER T_SAME T_NOT_SAME T_LESS_EQ T_GREATER_EQ
%left T_DOLLAR T_DDOT T_REGEX_OP
%left T_PLUSE T_MINUSE T_DOTE T_XORE
%left T_DIVE T_MULE T_ANDE T_ORE
%left T_PLUS T_MINUS T_XOR
%left T_DIV T_MUL T_AND T_OR
%left T_MOD T_MODE T_DOT
%left T_SHIFTL T_SHIFTLE T_SHIFTR T_SHIFTRE

%nonassoc T_UMINUS
%nonassoc T_FACT
%nonassoc T_NOT
%nonassoc T_INC T_DEC

%type <node>   statement expression statements
%type <list>   argumentList
%type <list>   caseCascade
%type <list>   attrList
%type <list>   identChain
%type <list>   identList
%type <list>   methodList
%type <list>   classMembers
%type <list>   classExtensions
%type <node>   finallyBlock
%type <access> accessSpecifier
%%

program    : body           { __hyb_vm.timer( HYB_TIMER_STOP ); }

body       : body statement { __hyb_vm.timer( HYB_TIMER_START );
                              __hyb_vm.engine->exec( &__hyb_vm.vmem, $2 );
                              RM_NODE($2);
                            }
           | /* empty */ ;


argumentList : expression T_MAPS argumentList { $$ = MK_NODE($3);    $$->head($1); }
             | expression ',' argumentList    { $$ = MK_NODE($3);    $$->head($1); }
             | expression                     { $$ = MK_NODE_LIST(); $$->tail($1); }
             | /* empty */                    { $$ = MK_NODE_LIST(); };

identChain : T_IDENT T_MAPS identChain { $$ = MK_NODE($3);    $$->head( MK_IDENT_NODE($1) ); }
		   | T_IDENT T_MAPS T_IDENT    { $$ = MK_NODE_LIST(); $$->tail( MK_IDENT_NODE($1), MK_IDENT_NODE($3) ); }

caseCascade  : T_CASE expression ':' statements T_BREAK T_EOSTMT caseCascade { $$ = MK_NODE($7);    $$->head( $2, $4 ); }
             | T_CASE expression ':' statements T_BREAK T_EOSTMT             { $$ = MK_NODE_LIST(); $$->tail( $2, $4 ); }
             | /* empty */                                                   { $$ = MK_NODE_LIST(); };

classExtensions : T_EXTENDS identList { $$ = MK_NODE($2); }
			    | /* empty */	      { $$ = NULL;        };

finallyBlock	: T_FINALLY '{' statements '}' { $$ = MK_NODE($3); }
				| /* empty */			       { $$ = NULL; }

accessSpecifier : T_PUBLIC    { $$ = asPublic;    }
		        | T_PRIVATE   { $$ = asPrivate;   }
		        | T_PROTECTED { $$ = asProtected; }
		        | /* empty */ { $$ = asPublic;    };

identList : T_IDENT ',' identList { $$ = MK_NODE($3);    $$->head( MK_IDENT_NODE($1) ); }
		  | T_IDENT ',' T_IDENT   { $$ = MK_NODE_LIST(); $$->tail( MK_IDENT_NODE($1), MK_IDENT_NODE($3) );   }
		  | T_IDENT 			  { $$ = MK_NODE_LIST(); $$->head( MK_IDENT_NODE($1) ); }

attrList  : accessSpecifier identList T_EOSTMT attrList {
			  $$ = MK_NODE($4);
			  for( NodeIterator i = $2->begin(); i != $2->end(); i++ ){
				  $$->head( MK_ATTR_NODE( $1, *i ) );
			  }
			  delete $2;
		  }
		  | accessSpecifier identList T_EOSTMT {
			  $$ = MK_NODE_LIST();
			  for( NodeIterator i = $2->begin(); i != $2->end(); i++ ){
				  $$->head( MK_ATTR_NODE( $1, *i ) );
			  }
			  delete $2;
		  }
		  | methodList { $$ = MK_NODE($1); }

methodList : accessSpecifier T_METHOD_PROTOTYPE '{' statements '}' methodList {
				 $$ = MK_NODE($6);
				 $$->head( MK_METHOD_NODE( $1, $2, $4 ) );
		   }
		   | accessSpecifier T_METHOD_PROTOTYPE '{' statements '}' {
				 $$ = MK_NODE_LIST();
				 $$->head( MK_METHOD_NODE( $1, $2, $4 ) );
		   };

classMembers : attrList methodList classMembers {
				$$ = MK_NODE($3);
				for( NodeIterator i = $1->begin(); i != $1->end(); i++ ){
					$$->tail( *i );
				}
				for( NodeIterator j = $2->begin(); j != $2->end(); j++ ){
					$$->tail( *j );
				}
				delete $1;
				delete $2;
			 }
			 | methodList attrList {
				$$ = MK_NODE_LIST();
				for( NodeIterator i = $1->begin(); i != $1->end(); i++ ){
					$$->tail( *i );
				}
				for( NodeIterator j = $2->begin(); j != $2->end(); j++ ){
					$$->tail( *j );
				}
				delete $1;
				delete $2;
			 }
			 | methodList {
				 $$ = MK_NODE($1);
			 }
			 | attrList {
				 $$ = MK_NODE($1);
			 }
			 | /* empty */
			 {  $$ = MK_NODE_LIST(); };

statement  : T_EOSTMT                                                   { $$ = MK_EOSTMT_NODE( NULL, NULL ); }
	   	   | expression                                                 { $$ = MK_NODE($1); }
           | expression T_EOSTMT                                        { $$ = MK_NODE($1); }
           | T_BREAK T_EOSTMT											{ $$ = MK_BREAK_NODE(); }
           | T_NEXT T_EOSTMT											{ $$ = MK_NEXT_NODE(); }
           | T_RETURN expression T_EOSTMT                               { $$ = MK_RETURN_NODE( $2 ); }
           | T_THROW expression T_EOSTMT								{ $$ = MK_THROW_NODE( $2 ); }
           /* subscript operator special cases */
		   | expression '[' ']' T_ASSIGN expression T_EOSTMT            { $$ = MK_SB_PUSH_NODE( $1, $5 ); }
           | expression '[' expression ']' T_ASSIGN expression T_EOSTMT { $$ = MK_SB_SET_NODE( $1, $3, $6 ); }
           /* conditional and loops */
           | T_WHILE '(' expression ')' statement                       { $$ = MK_WHILE_NODE( $3, $5 ); }
           | T_DO statement T_WHILE '(' expression ')' T_EOSTMT         { $$ = MK_DO_NODE( $2, $5 ); }
		   | T_FOR '(' statement statement expression ')' statement     { $$ = MK_FOR_NODE( $3, $4, $5, $7 ); }
		   | T_FOREACH '(' T_IDENT T_OF expression ')' statement        { $$ = MK_FOREACH_NODE( $3, $5, $7 ); }
		   | T_FOREACH '(' T_IDENT T_MAPS T_IDENT T_OF expression ')' statement {
		   		$$ = MK_FOREACHM_NODE( $3, $5, $7, $9 );
		   }
           | T_IF '(' expression ')' statement %prec T_IF_END           { $$ = MK_IF_NODE( $3, $5 ); }
           | T_IF '(' expression ')' statement T_ELSE statement         { $$ = MK_IF_ELSE_NODE( $3, $5, $7 ); }
           | T_SWITCH '(' expression ')' '{'
                caseCascade
            '}' %prec T_SWITCH_END                                      { $$ = MK_SWITCH_NODE( $3, $6 ); }
           | T_SWITCH '(' expression ')' '{'
                caseCascade
                T_DEFAULT ':' statements
            '}'                                                         { $$ = MK_SWITCH_DEF_NODE( $3, $6, $9 ); }
           /* statement body */
           | '{' statements '}' { $$ = $2; }
           /* function declaration */
           | T_FUNCTION_PROTOTYPE '{' statements '}'                    { $$ = MK_FUNCTION_NODE( $1, $3 ); }
           /* structure declaration */
           | T_STRUCT T_IDENT '{' attrList '}'                       { $$ = MK_STRUCT_NODE( $2, $4 ); }
		   /* class declaration */
           | T_CLASS T_IDENT classExtensions '{' classMembers '}' {
        	   $$ = MK_CLASS_NODE( $2, $3, $5 );
           }
           /* exception handling */
           | T_TRY '{' statements '}' T_CATCH '(' T_IDENT ')' '{' statements '}' finallyBlock {
        	   $$ = MK_TRYCATCH_NODE( $3, MK_IDENT_NODE($7), $10, $12 );
           };


statements : /* empty */          { $$ = MK_NODE(0);  }
           | statement            { $$ = MK_NODE($1); }
           | statements statement { $$ = MK_EOSTMT_NODE( $1, $2 ); };

expression : T_INTEGER                                        { $$ = MK_CONST_NODE($1); }
           | T_REAL                                           { $$ = MK_CONST_NODE($1); }
           | T_CHAR                                           { $$ = MK_CONST_NODE($1); }
           | T_STRING                                         { $$ = MK_CONST_NODE($1); }
           /* identifiers and attributes */
           | T_IDENT                                          { $$ = MK_IDENT_NODE($1); }
           | identChain                                       { $$ = MK_IDLST_NODE($1);  }
           /* expression evaluation returns an identifier */
           | T_DOLLAR expression                              { $$ = MK_DOLLAR_NODE($2); }
           /* attribute declaration/assignation */
           | identChain T_ASSIGN expression                   { $$ = MK_ASSIGN_NODE( MK_IDLST_NODE($1), $3 ); }
		   /* identifier declaration/assignation */
		   | T_IDENT T_ASSIGN expression                      { $$ = MK_ASSIGN_NODE( MK_IDENT_NODE($1), $3 ); }
           /* a single subscript could be an expression itself */
           | expression '[' expression ']' %prec T_SB_END     { $$ = MK_SB_NODE( $1, $3 ); }
           /* range evaluation */
           | expression T_DDOT expression                     { $$ = MK_RANGE_NODE( $1, $3 ); }
           /* arithmetic & misc operators */
           | T_MINUS expression %prec T_UMINUS                { $$ = MK_UMINUS_NODE( $2 ); }
           | expression T_DOT expression                      { $$ = MK_DOT_NODE( $1, $3 ); }
		   | expression T_DOTE expression                     { $$ = MK_DOTE_NODE( $1, $3 ); }
           | expression T_PLUS expression                     { $$ = MK_PLUS_NODE( $1, $3 ); }
		   | expression T_PLUSE expression                    { $$ = MK_PLUSE_NODE( $1, $3 ); }
           | expression T_MINUS expression                    { $$ = MK_MINUS_NODE( $1, $3 ); }
		   | expression T_MINUSE expression                   { $$ = MK_MINUSE_NODE( $1, $3 ); }
           | expression T_MUL expression                      { $$ = MK_MUL_NODE( $1, $3 ); }
		   | expression T_MULE expression                     { $$ = MK_MULE_NODE( $1, $3 ); }
           | expression T_DIV expression                      { $$ = MK_DIV_NODE( $1, $3 ); }
		   | expression T_DIVE expression                     { $$ = MK_DIVE_NODE( $1, $3 ); }
           | expression T_MOD expression                      { $$ = MK_MOD_NODE( $1, $3 ); }
           | expression T_INC                                 { $$ = MK_INC_NODE( $1 ); }
           | expression T_DEC                                 { $$ = MK_DEC_NODE( $1 ); }
           /* bitwise */
           | expression T_XOR expression                      { $$ = MK_XOR_NODE( $1, $3 ); }
		   | expression T_XORE expression                     { $$ = MK_XORE_NODE( $1, $3 ); }
           | T_NOT expression                                 { $$ = MK_NOT_NODE( $2 ); }
           | expression T_AND expression                      { $$ = MK_AND_NODE( $1, $3 ); }
		   | expression T_ANDE expression                     { $$ = MK_ANDE_NODE( $1, $3 ); }
           | expression T_OR expression                       { $$ = MK_OR_NODE( $1, $3 ); }
		   | expression T_ORE expression                      { $$ = MK_ORE_NODE( $1, $3 ); }
		   | expression T_SHIFTL expression                   { $$ = MK_SHIFTL_NODE( $1, $3 ); }
		   | expression T_SHIFTLE expression                  { $$ = MK_SHIFTLE_NODE( $1, $3 ); }
		   | expression T_SHIFTR expression                   { $$ = MK_SHIFTR_NODE( $1, $3 ); }
		   | expression T_SHIFTRE expression                  { $$ = MK_SHIFTRE_NODE( $1, $3 ); }
		   | expression T_L_NOT                                { $$ = MK_FACT_NODE( $1 ); }
           /* logic */
		   | T_L_NOT expression                                { $$ = MK_LNOT_NODE( $2 ); }
           | expression T_LESS expression                     { $$ = MK_LESS_NODE( $1, $3 ); }
           | expression T_GREATER expression                  { $$ = MK_GREATER_NODE( $1, $3 ); }
           | expression T_GREATER_EQ expression                       { $$ = MK_GE_NODE( $1, $3 ); }
           | expression T_LESS_EQ expression                       { $$ = MK_LE_NODE( $1, $3 ); }
           | expression T_NOT_SAME expression                       { $$ = MK_NE_NODE( $1, $3 ); }
           | expression T_SAME expression                       { $$ = MK_EQ_NODE( $1, $3 ); }
           | expression T_L_AND expression                     { $$ = MK_LAND_NODE( $1, $3 ); }
           | expression T_L_OR expression                      { $$ = MK_LOR_NODE( $1, $3 ); }
           /* regex specific */
           | expression T_REGEX_OP expression                 { $$ = MK_PCRE_NODE( $1, $3 ); }
           /* structure or class creation */
           | T_NEW T_IDENT '(' argumentList ')' %prec T_NEW_END { $$ = MK_NEW_NODE( $2, $4 ); }
           /* function or method call (consider two different cases due to hybris function calls */
		   | T_IDENT    '(' argumentList ')' %prec T_CALL_END { $$ = MK_CALL_NODE( $1, $3 ); }
		   | identChain '(' argumentList ')' %prec T_CALL_END { $$ = MK_METHOD_CALL_NODE( $1, $3 ); }
           | expression '(' argumentList ')'                  { $$ = MK_CALL_NODE( $1, $3 ); }
           /* ternary operator */
           | '(' expression '?' expression ':' expression ')' { $$ = MK_QUESTION_NODE( $2, $4, $6 ); }
           /* group expression */
           | '(' expression ')'                               { $$ = MK_NODE($2); };

%%

int hyb_banner(){
    printf( "Hybris %s (built: %s %s)\n"
            "Released under GPL v3.0 by %s\n"
            "* Compiled with :\n"
            " - Import path  : %s\n"
            " - Include path : %s\n",
            VERSION,
            __DATE__,
            __TIME__,
            AUTHOR,
            LIB_PATH,
            INC_PATH );
}

int hyb_usage( char *argvz ){
    hyb_banner();
    printf( "\nUsage: %s <options> <file>\n\n"
    		"Where <options> is one or more among followring values :\n"
    		"\t-h (--help)    : Print this menu and exit.\n"
    		"\t-g (--gc)      : Set the garbage collection memory threshold, expressend in bytes, \n"
    		"\t                 kilobytes (with K postfix) or megabytes (with M postfix).\n"
    		"\t                 i.e. -g 10K or -g 1024 or --gc=100M\n"
            "\t-t (--time)    : Compute execution time and print it to stdout.\n"
            "\t-s (--trace)   : Enable stack trace report on errors .\n\n", argvz );
    return 0;
}

int main( int argc, char *argv[] ){
    static struct option options[] = {
            { "gc",      1, 0, 'g' },
            { "time",    0, 0, 't' },
            { "trace",   0, 0, 's' },
            { "help",    0, 0, 'h' },
            { 0, 0, 0, 0 }
    };

    int index = 0;
    char c, multiplier, *p;
    long gc_threshold;

    while( (c = getopt_long( argc, argv, "g:tsh", options, &index)) != -1 ){
        switch (c) {
			/*
			 * Handle garbage collection threshold argument.
			 * Allowed values are :
			 *
			 * nnn  (bytes)
			 * nnnK (kilo bytes)
			 * nnnM (mega bytes)
			 *
			 * Where nnn is the number of bytes, K is the kilo multiplier (1024),
			 * and M the mega multiplier (1024^2).
			 */
			case 'g':
				p = optarg;
				/*
				 * Shift the pointer until first non digit character is reached
				 * or the end of the string is reached.
				 */
				while( *(++p) != 0x00 && *p >= '0' && *p <= '9' );

				multiplier = *p;
				/*
				 * Optarg now contains only the integer part of the argument.
				 */
				optarg[ p - optarg ] = 0x00;

				gc_threshold = atol(optarg);
				/*
				 * Check for valid integer values.
				 */
				if( gc_threshold == 0 ){
					hyb_error( H_ET_GENERIC, "Invalid memory size %s given.", optarg );
				}
				/*
				 * Check for a valid multiplier.
				 */
				else if( multiplier != 0x00 && strchr( "kKmM", multiplier ) == 0 ){
					hyb_error( H_ET_GENERIC, "Invalid multiplier %c given.", multiplier );
				}
				/*
				 * Perform multiplication if multiplier was specified (multiplier != 0x00)
				 */
				switch(multiplier){
					case 'K' :
					case 'k' :
						gc_threshold *= 1024;
					break;

					case 'M' :
					case 'm' :
						gc_threshold *= 1048576;
					break;
				}
				/*
				 * Check for integer overflow.
				 */
				if( gc_threshold <= 0 ){
					hyb_error( H_ET_GENERIC, "Memory limit is too high." );
				}
				/*
				 * Done, let's pass it to the context structure.
				 */
				__hyb_vm.args.gc_threshold = gc_threshold;
			break;

        	case 't':
        		/*
        		 * Enable execution time measurement.
        		 */
        		__hyb_vm.args.tm_timer   = 1;
        	break;
        	case 's':
        		/*
        		 * Enable stack trace printing upon error.
        		 */
        		__hyb_vm.args.stacktrace = 1;
        	break;
        	case 'h':
        		return hyb_usage(argv[0]);
            break;
        }
    }

    if( optind < argc ){
        strncpy( __hyb_vm.args.source, argv[optind], sizeof(__hyb_vm.args.source) );
		if( hyb_file_exists(__hyb_vm.args.source) == 0 ){
			printf( "\033[22;31mERROR : '%s' no such file or directory.\n\n\033[00m", __hyb_vm.args.source );
			return hyb_usage( argv[0] );
		}
    }
    /*
     * VM will receive every argument starting from the script
     * name to build the script virtual argv.
     */
    __hyb_vm.init( argc - (optind - 1), argv + (optind - 1) );

    extern FILE *yyin;
    /*
     * At this point, yyin could be a file handle if a source was specified
     * or the stdin handle if not, in this case the interpreter will execute
     * user input.
     */
    yyin = __hyb_vm.openFile();

    while( !feof(yyin) ){
        yyparse();
    }

    __hyb_vm.closeFile();
    __hyb_vm.release();

    return 0;
}
