/*
** $Id: ldump.c,v 2.8.1.1 2007/12/27 13:02:25 roberto Exp $
** save precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#include <stddef.h>

#define ldump_c
#define LUA_CORE

#include "lua.h"

#include "lobject.h"
#include "lstate.h"
#include "lundump.h"

/* C99 provides the exact size typedefs we need in the most correct and portable way possible.
 * Visual Studio finally got this in 2010.
 */
#if defined(_MSC_VER) && _MSC_VER < 1600
	#if !defined(_STDINT_H_) && (!defined(HAVE_STDINT_H) || !_HAVE_STDINT_H)
		#define _STDINT_H_ 1
		typedef signed char int8_t;
		typedef unsigned char uint8_t;
		typedef signed short int16_t;
		typedef unsigned short uint16_t;
		typedef signed long int32_t;
		typedef unsigned long uint32_t;
		typedef signed long long int64_t;
		typedef unsigned long long uint64_t;
	#endif
#else
	#include <stdint.h>
#endif


typedef struct {
 lua_State* L;
 lua_Writer writer;
 void* data;
 int strip;
 int status;
} DumpState;

#define DumpMem(b,n,size,D)	DumpBlock(b,(n)*(size),D)
#define DumpVar(x,D)	 	DumpMem(&x,1,sizeof(x),D)

static void DumpBlock(const void* b, size_t size, DumpState* D)
{
 if (D->status==0)
 {
  lua_unlock(D->L);
  D->status=(*D->writer)(D->L,b,size,D->data);
  lua_lock(D->L);
 }
}

static void DumpChar(int y, DumpState* D)
{
 char x=(char)y;
 DumpVar(x,D);
}

/* modified from int for 32-bit bytecode */
static void DumpInt(int32_t x, DumpState* D)
{
 DumpVar(x,D);
}

static void DumpNumber(lua_Number x, DumpState* D)
{
 DumpVar(x,D);
}

static void DumpVector(const void* b, int n, size_t size, DumpState* D)
{
 DumpInt(n,D);
 DumpMem(b,n,size,D);
}

/* modified from size_t for 32-bit bytecode */
static void DumpString(const TString* s, DumpState* D)
{
 if (s==NULL || getstr(s)==NULL)
 {
  uint32_t size=0;
  DumpVar(size,D);
 }
 else
 {
  uint32_t size=s->tsv.len+1;		/* include trailing '\0' */
  DumpVar(size,D);
  DumpBlock(getstr(s),size,D);
 }
}

#define DumpCode(f,D)	 DumpVector(f->code,f->sizecode,sizeof(Instruction),D)

static void DumpFunction(const Proto* f, const TString* p, DumpState* D);

static void DumpConstants(const Proto* f, DumpState* D)
{
 int i,n=f->sizek;
 DumpInt(n,D);
 for (i=0; i<n; i++)
 {
  const TValue* o=&f->k[i];
  DumpChar(ttype(o),D);
  switch (ttype(o))
  {
   case LUA_TNIL:
	break;
   case LUA_TBOOLEAN:
	DumpChar(bvalue(o),D);
	break;
   case LUA_TNUMBER:
	DumpNumber(nvalue(o),D);
	break;
   case LUA_TSTRING:
	DumpString(rawtsvalue(o),D);
	break;
   default:
	lua_assert(0);			/* cannot happen */
	break;
  }
 }
 n=f->sizep;
 DumpInt(n,D);
 for (i=0; i<n; i++) DumpFunction(f->p[i],f->source,D);
}

static void DumpDebug(const Proto* f, DumpState* D)
{
 int i,n;
 n= (D->strip) ? 0 : f->sizelineinfo;
 DumpVector(f->lineinfo,n,sizeof(int),D);
 n= (D->strip) ? 0 : f->sizelocvars;
 DumpInt(n,D);
 for (i=0; i<n; i++)
 {
  DumpString(f->locvars[i].varname,D);
  DumpInt(f->locvars[i].startpc,D);
  DumpInt(f->locvars[i].endpc,D);
 }
 n= (D->strip) ? 0 : f->sizeupvalues;
 DumpInt(n,D);
 for (i=0; i<n; i++) DumpString(f->upvalues[i],D);
}

static void DumpFunction(const Proto* f, const TString* p, DumpState* D)
{
 DumpString((f->source==p || D->strip) ? NULL : f->source,D);
 DumpInt(f->linedefined,D);
 DumpInt(f->lastlinedefined,D);
 DumpChar(f->nups,D);
 DumpChar(f->numparams,D);
 DumpChar(f->is_vararg,D);
 DumpChar(f->maxstacksize,D);
 DumpCode(f,D);
 DumpConstants(f,D);
 DumpDebug(f,D);
}

static void DumpHeader(DumpState* D)
{
 char h[LUAC_HEADERSIZE];
 luaU_header(h);
 DumpBlock(h,LUAC_HEADERSIZE,D);
}

/*
** dump Lua function as precompiled chunk
*/
int luaU_dump (lua_State* L, const Proto* f, lua_Writer w, void* data, int strip)
{
 DumpState D;
 D.L=L;
 D.writer=w;
 D.data=data;
 D.strip=strip;
 D.status=0;
 DumpHeader(&D);
 DumpFunction(f,NULL,&D);
 return D.status;
}
