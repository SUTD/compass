/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_ZZ_HOME_YUFENG_RESEARCH_OTHERS_COMPASS_MISTRAL_BUILD_SMTPARSER_SMTPARSER_HPP_INCLUDED
# define YY_ZZ_HOME_YUFENG_RESEARCH_OTHERS_COMPASS_MISTRAL_BUILD_SMTPARSER_SMTPARSER_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int zzdebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    TOKEN_PLUS = 258,
    TOKEN_MINUS = 259,
    TOKEN_IDENTIFIER = 260,
    TOKEN_TIMES = 261,
    TOKEN_DIVIDE = 262,
    TOKEN_INT = 263,
    TOKEN_LPAREN = 264,
    TOKEN_RPAREN = 265,
    TOKEN_AND = 266,
    TOKEN_OR = 267,
    TOKEN_NOT = 268,
    TOKEN_EQ = 269,
    TOKEN_GT = 270,
    TOKEN_GEQ = 271,
    TOKEN_LT = 272,
    TOKEN_LEQ = 273,
    TOKEN_COND = 274,
    TOKEN_ERROR = 275,
    TOKEN_LET = 276,
    TOKEN_ASSERT = 277,
    EXPR = 278,
    TOKEN_PRINT = 279,
    TOKEN_NEQ = 280,
    TOKEN_ISNIL = 281,
    TOKEN_CONS = 282,
    TOKEN_HD = 283,
    TOKEN_TL = 284
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE zzlval;

int zzparse (void);

#endif /* !YY_ZZ_HOME_YUFENG_RESEARCH_OTHERS_COMPASS_MISTRAL_BUILD_SMTPARSER_SMTPARSER_HPP_INCLUDED  */
