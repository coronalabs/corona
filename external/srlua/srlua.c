/*
* srlua.c
* Lua interpreter for self-running programs
* Luiz Henrique de Figueiredo <lhf@tecgraf.puc-rio.br>
* 28 Apr 2006 23:01:50
* This code is hereby placed in the public domain.
*/

#ifdef _WIN32
#include <windows.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "glue.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

typedef struct
{
 FILE *f;
 long size;
 char buff[512];
} State;

static const char *myget(lua_State *L, void *data, size_t *size)
{
 State* s=data;
 size_t n;
 if (s->size<=0) return NULL;
 n=(sizeof(s->buff)<=s->size)? sizeof(s->buff) : s->size;
 n=fread(s->buff,1,n,s->f);
 if (n==-1) return NULL;
 s->size-=n;
 *size=n;
 return s->buff;
}

#define cannot(x) luaL_error(L,"cannot %s %s: %s",x,name,strerror(errno))

static void load(lua_State *L, const char *name)
{
 Glue t;
 State S;
 FILE *f=fopen(name,"rb");
 if (f==NULL) cannot("open");
 if (fseek(f,-sizeof(t),SEEK_END)!=0) cannot("seek");
 if (fread(&t,sizeof(t),1,f)!=1) cannot("read");
 if (memcmp(t.sig,GLUESIG,GLUELEN)!=0) luaL_error(L,"no Lua program found in %s",name);
 if (fseek(f,t.size1,SEEK_SET)!=0) cannot("seek");
 S.f=f; S.size=t.size2;
 if (lua_load(L,myget,&S,name)!=0) lua_error(L);
 fclose(f);
}

static int pmain(lua_State *L)
{
 char **argv=lua_touserdata(L,1);
 int i;
 luaL_openlibs(L);
 load(L,argv[0]);
 lua_newtable(L);
 for (i=0; argv[i]; i++)
 {
  lua_pushstring(L,argv[i]);
  lua_rawseti(L,-2,i);
 }
 lua_pushliteral(L,"n");
 lua_pushnumber(L,i-1);
 lua_rawset(L,-3);
 lua_setglobal(L,"arg");
 lua_pushstring(L,argv[0]);
 lua_setglobal(L,"_PROGNAME");
 luaL_checkstack(L, i, "too many arguments to script");
 for (i=1; argv[i]; i++)
 {
  lua_pushstring(L,argv[i]);
 }
 lua_call(L,i-1,0);
 return 0;
}

#ifdef _WIN32
#define report(s) MessageBox(NULL,s,argv[0], MB_ICONERROR | MB_OK)
#else
#define report(s) fprintf(stderr,"%s: %s\n",argv[0],s)
#endif

int main(int argc, char *argv[])
{
 lua_State *L;
#ifdef _WIN32
 char name[MAX_PATH];
 if (GetModuleFileName(NULL,name,sizeof(name))==0)
 {
  report("cannot locate this executable");
  return EXIT_FAILURE;
 }
 argv[0]=name;
#endif
 L=lua_open();
 if (L==NULL)
 {
  report("not enough memory for state");
  return EXIT_FAILURE;
 }
 if (lua_cpcall(L,pmain,argv)) report(lua_tostring(L,-1));
 lua_close(L);
 return EXIT_SUCCESS;
}
