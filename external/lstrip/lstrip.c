/*
* lstrip.c
* compress Lua programs by removing comments and whitespaces,
* optionally preserving line breaks (for error messages).
* 20 Oct 2006 09:25:34
* Luiz Henrique de Figueiredo <lhf@tecgraf.puc-rio.br>
* This code is hereby placed in the public domain.
*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LUA_CORE

#include "lua.h"
#include "lauxlib.h"

#include "ldo.h"
#include "lfunc.h"
#include "llex.h"
#include "lobject.h"
#include "lparser.h"
#include "lstring.h"
#include "ltable.h"
#include "lzio.h"

#define PROGNAME	"lstrip"	/* default program name */
static const char* progname=PROGNAME;	/* actual program name */
static int preserve=0;			/* preserve line breaks? */
static int dump=0;			/* dump instead of stripping? */

static void fatal(const char* message)
{
 fprintf(stderr,"%s: %s\n",progname,message);
 exit(EXIT_FAILURE);
}

static void quote(const TString *ts)
{
 const char *s=getstr(ts);
 size_t n=ts->tsv.len;
 int i;
 printf("\"");
 for (i=0; i<n; i++)
 {
  int c=s[i];
  switch (c)
  {
   case '"':  printf("\\\""); break;
   case '\\': printf("\\\\"); break;
   case '\a': printf("\\a"); break;
   case '\b': printf("\\b"); break;
   case '\f': printf("\\f"); break;
   case '\n': printf("\\n"); break;
   case '\r': printf("\\r"); break;
   case '\t': printf("\\t"); break;
   case '\v': printf("\\v"); break;
   default:	if (isprint((unsigned char)c))
   			printf("%c",c);
		else
			printf("\\%03u",(unsigned char)c);
  }
 }
 printf("\"");
}

#define pair(a,b)	(1024*(a)+(b))

static int clash[]= {
	pair('-', '-'),
	pair('.', '.'),
	pair('.', TK_CONCAT),
	pair('.', TK_DOTS),
	pair('.', TK_NUMBER),
	pair('<', '='),
	pair('=', '='),
	pair('>', '='),
	pair('[', '='),
	pair('[', '['),
	pair('~', '='),
	pair(TK_CONCAT, '.'),
	pair(TK_CONCAT, TK_CONCAT),
	pair(TK_CONCAT, TK_DOTS),
	pair(TK_CONCAT, TK_NUMBER),
	pair(TK_NAME, TK_NAME),
	pair(TK_NAME, TK_NUMBER),
	pair(TK_NUMBER, '.'),
	pair(TK_NUMBER, TK_CONCAT),
	pair(TK_NUMBER, TK_DOTS),
	pair(TK_NUMBER, TK_NAME),
	pair(TK_NUMBER, TK_NUMBER),
	0
};

static int space(int a, int b)
{
 int i,c;
 if (a>=FIRST_RESERVED && a<=TK_WHILE) a=TK_NAME;
 if (b>=FIRST_RESERVED && b<=TK_WHILE) b=TK_NAME;
 c=pair(a,b);
 for (i=0; clash[i]!=0; i++)
   if (c==clash[i]) return 1;
 return 0;
}

static void dodump(LexState *X)
{
 for (;;)
 {
  int t;
  luaX_next(X);
  t=X->t.token;
  printf("%d\t",X->linenumber);
  if (t<FIRST_RESERVED)
   printf("%c",t);
  else
   printf("%s",luaX_tokens[t-FIRST_RESERVED]);
  printf("\t");
  switch (t)
  {
   case TK_EOS:
    printf("\n");
    return;
   case TK_STRING:
    quote(X->t.seminfo.ts);
    break;
   case TK_NAME:
    printf("%s",getstr(X->t.seminfo.ts));
    break;
   case TK_NUMBER:
    printf("%s",X->buff->buffer);
    break;
  }
  printf("\n");
 }
}

static void dostrip(LexState *X)
{
 int ln=1;
 int lt=0;
 for (;;)
 {
  int t;
  luaX_next(X);
  t=X->t.token;
  if (preserve)
  {
   if (X->linenumber!=ln)
   {
    if (preserve>1)
     while (X->linenumber!=ln++) printf("\n");
    else if (lt!=0)
     printf("\n");
    ln=X->linenumber;
    lt=0;
   }
  }
  if (space(lt,t)) printf(" ");
  switch (t)
  {
   case TK_EOS:
    return;
   case TK_STRING:
    quote(X->t.seminfo.ts);
    break;
   case TK_NAME:
    printf("%s",getstr(X->t.seminfo.ts));
    break;
   case TK_NUMBER:
    printf("%s",X->buff->buffer);
    break;
   default:
    if (t<FIRST_RESERVED)
     printf("%c",t);
    else
     printf("%s",luaX_tokens[t-FIRST_RESERVED]);
    break;
  }
  lt=t;
  if (preserve>2) printf("\n");
 }
}

Proto *luaY_parser(lua_State *L, ZIO *z, Mbuffer *buff, const char *name)
{
 LexState X;
 FuncState F;
 X.buff=buff;
 luaX_setinput(L,&X,z,luaS_new(L,name));
 X.fs=&F;
 X.fs->h=luaH_new(L,0,0);
 sethvalue2s(L,L->top,X.fs->h);
 incr_top(L);
 if (dump)
 {
  printf("0\t<file>\t%s\n",name);
  dodump(&X);
 }
 else dostrip(&X);
 return luaF_newproto(L);
}

static void strip(lua_State *L, const char *file)
{
 if (file!=NULL && strcmp(file,"-")==0) file=NULL;
 if (luaL_loadfile(L,file)!=0) fatal(lua_tostring(L,-1));
 lua_settop(L,0);
}

int main(int argc, char* argv[])
{
 lua_State *L=lua_open();
 if (argv[0]!=NULL && *argv[0]!=0) progname=argv[0];
 if (L==NULL) fatal("not enough memory for state");
 while (*++argv!=NULL && strcmp(*argv,"-p")==0) preserve++;
 --argv;
 while (*++argv!=NULL && strcmp(*argv,"-d")==0) dump++;
 if (*argv==NULL)
  strip(L,NULL);
 else
  while (*argv) strip(L,*argv++);
 lua_close(L);
 return EXIT_SUCCESS;
}
