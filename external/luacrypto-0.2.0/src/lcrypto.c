/*
** $Id: lcrypto.c,v 1.2 2006/08/25 03:24:17 nezroy Exp $
** See Copyright Notice in license.html
*/

#include <string.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>

#include "lua.h"
#include "lauxlib.h"
#if ! defined (LUA_VERSION_NUM) || LUA_VERSION_NUM < 501
#include "compat-5.1.h"
#endif

#include "lcrypto.h"

LUACRYPTO_API int luaopen_crypto(lua_State *L);

static int crypto_error(lua_State *L)
{
  char buf[120];
  unsigned long e = ERR_get_error();
  ERR_load_crypto_strings();
  lua_pushnil(L);
  lua_pushstring(L, ERR_error_string(e, buf));
  return 2;
}

static EVP_MD_CTX *evp_pget(lua_State *L, int i)
{
  if (luaL_checkudata(L, i, LUACRYPTO_EVPNAME) == NULL) luaL_typerror(L, i, LUACRYPTO_EVPNAME);
  return lua_touserdata(L, i);
}

static EVP_MD_CTX *evp_pnew(lua_State *L)
{
  EVP_MD_CTX *c = lua_newuserdata(L, sizeof(EVP_MD_CTX));
  luaL_getmetatable(L, LUACRYPTO_EVPNAME);
  lua_setmetatable(L, -2);
  return c;
}

static int evp_fnew(lua_State *L)
{
  EVP_MD_CTX *c = NULL;
  const char *s = luaL_checkstring(L, 1);
  const EVP_MD *type = EVP_get_digestbyname(s);
  
  if (type == NULL) {
    luaL_argerror(L, 1, "invalid digest type");
    return 0;
  }
  
  c = evp_pnew(L);
  EVP_MD_CTX_init(c);
  EVP_DigestInit_ex(c, type, NULL);
  
  return 1;
}

static int evp_clone(lua_State *L)
{
  EVP_MD_CTX *c = evp_pget(L, 1);
  EVP_MD_CTX *d = evp_pnew(L);
  EVP_MD_CTX_init(d);
  EVP_MD_CTX_copy_ex(d, c);
  return 1;
}

static int evp_reset(lua_State *L)
{
  EVP_MD_CTX *c = evp_pget(L, 1);
  const EVP_MD *t = EVP_MD_CTX_md(c);
  EVP_MD_CTX_cleanup(c);
  EVP_MD_CTX_init(c);
  EVP_DigestInit_ex(c, t, NULL);
  return 0;
}

static int evp_update(lua_State *L)
{
  EVP_MD_CTX *c = evp_pget(L, 1);
  const char *s = luaL_checkstring(L, 2);
  
  EVP_DigestUpdate(c, s, lua_strlen(L, 2));
  
  lua_settop(L, 1);
  return 1;
}

static int evp_digest(lua_State *L) 
{
  EVP_MD_CTX *c = evp_pget(L, 1);
  EVP_MD_CTX *d = NULL;
  unsigned char digest[EVP_MAX_MD_SIZE];
  size_t written = 0;
  unsigned int i;
  char *hex;
  
  if (lua_isstring(L, 2))
  {  
    const char *s = luaL_checkstring(L, 2);
    EVP_DigestUpdate(c, s, lua_strlen(L, 2));
  }
  
  d = EVP_MD_CTX_create();
  EVP_MD_CTX_copy_ex(d, c);
  EVP_DigestFinal_ex(d, digest, &written);
  EVP_MD_CTX_destroy(d);
  
  if (lua_toboolean(L, 3))
    lua_pushlstring(L, (char *)digest, written);
  else
  {
    hex = calloc(sizeof(char), written*2 + 1);
    for (i = 0; i < written; i++)
      sprintf(hex + 2*i, "%02x", digest[i]);
    lua_pushlstring(L, hex, written*2);
    free(hex);
  }
  
  return 1;
}

static int evp_tostring(lua_State *L)
{
  EVP_MD_CTX *c = evp_pget(L, 1);
  char s[64];
  sprintf(s, "%s %p", LUACRYPTO_EVPNAME, (void *)c);
  lua_pushstring(L, s);
  return 1;
}

static int evp_gc(lua_State *L)
{
  EVP_MD_CTX *c = evp_pget(L, 1);
  EVP_MD_CTX_cleanup(c);
  return 1;
}

static int evp_fdigest(lua_State *L)
{
  EVP_MD_CTX *c = NULL;
  const char *type_name = luaL_checkstring(L, 1);
  const char *s = luaL_checkstring(L, 2);
  const EVP_MD *type = EVP_get_digestbyname(type_name);
  unsigned char digest[EVP_MAX_MD_SIZE];
  size_t written = 0;
  unsigned int i;
  char *hex;
  
  if (type == NULL) {
    luaL_argerror(L, 1, "invalid digest type");
    return 0;
  }
  
  c = EVP_MD_CTX_create();
  EVP_DigestInit_ex(c, type, NULL);
  EVP_DigestUpdate(c, s, lua_strlen(L, 2));
  EVP_DigestFinal_ex(c, digest, &written);
  
  if (lua_toboolean(L, 3))
    lua_pushlstring(L, (char *)digest, written);
  else
  {
    hex = calloc(sizeof(char), written*2 + 1);
    for (i = 0; i < written; i++)
      sprintf(hex + 2*i, "%02x", digest[i]);
    lua_pushlstring(L, hex, written*2);
    free(hex);
  }
  
  return 1;
}

static HMAC_CTX *hmac_pget(lua_State *L, int i)
{
 if (luaL_checkudata(L, i, LUACRYPTO_HMACNAME) == NULL) luaL_typerror(L, i, LUACRYPTO_HMACNAME);
 return lua_touserdata(L, i);
}

static HMAC_CTX *hmac_pnew(lua_State *L)
{
  HMAC_CTX *c = lua_newuserdata(L, sizeof(HMAC_CTX));
  luaL_getmetatable(L, LUACRYPTO_HMACNAME);
  lua_setmetatable(L, -2);
  return c;
}

static int hmac_fnew(lua_State *L)
{
  HMAC_CTX *c = hmac_pnew(L);
  const char *s = luaL_checkstring(L, 1);
  const char *k = luaL_checkstring(L, 2);
  const EVP_MD *type = EVP_get_digestbyname(s);

  if (type == NULL) {
    luaL_argerror(L, 1, "invalid digest type");
    return 0;
  }

  HMAC_CTX_init(c);
  HMAC_Init_ex(c, k, lua_strlen(L, 2), type, NULL);

  return 1;
}

static int hmac_clone(lua_State *L)
{
 HMAC_CTX *c = hmac_pget(L, 1);
 HMAC_CTX *d = hmac_pnew(L);
 *d = *c;
 return 1;
}

static int hmac_reset(lua_State *L)
{
  HMAC_CTX *c = hmac_pget(L, 1);
  HMAC_Init_ex(c, NULL, 0, NULL, NULL);
  return 0;
}

static int hmac_update(lua_State *L)
{
  HMAC_CTX *c = hmac_pget(L, 1);
  const char *s = luaL_checkstring(L, 2);

  HMAC_Update(c, (unsigned char *)s, lua_strlen(L, 2));

  lua_settop(L, 1);
  return 1;
}

static int hmac_digest(lua_State *L)
{
  HMAC_CTX *c = hmac_pget(L, 1);
  unsigned char digest[EVP_MAX_MD_SIZE];
  size_t written = 0;
  unsigned int i;
  char *hex;

  if (lua_isstring(L, 2))
  {
    const char *s = luaL_checkstring(L, 2);
    HMAC_Update(c, (unsigned char *)s, lua_strlen(L, 2));
  }

  HMAC_Final(c, digest, &written);

  if (lua_toboolean(L, 3))
    lua_pushlstring(L, (char *)digest, written);
  else
  {
    hex = calloc(sizeof(char), written*2 + 1);
    for (i = 0; i < written; i++)
      sprintf(hex + 2*i, "%02x", digest[i]);
    lua_pushlstring(L, hex, written*2);
    free(hex);
  }

  return 1;
}

static int hmac_tostring(lua_State *L)
{
  HMAC_CTX *c = hmac_pget(L, 1);
  char s[64];
  sprintf(s, "%s %p", LUACRYPTO_HMACNAME, (void *)c);
  lua_pushstring(L, s);
  return 1;
}

static int hmac_gc(lua_State *L)
{
  HMAC_CTX *c = hmac_pget(L, 1);
  HMAC_CTX_cleanup(c);
  return 1;
}

static int hmac_fdigest(lua_State *L)
{
  HMAC_CTX c;
  unsigned char digest[EVP_MAX_MD_SIZE];
  size_t written = 0;
  unsigned int i;
  char *hex;
  const char *t = luaL_checkstring(L, 1);
  const char *s = luaL_checkstring(L, 2);
  const char *k = luaL_checkstring(L, 3);
  const EVP_MD *type = EVP_get_digestbyname(t);

  if (type == NULL) {
    luaL_argerror(L, 1, "invalid digest type");
    return 0;
  }

  HMAC_CTX_init(&c);
  HMAC_Init_ex(&c, k, lua_strlen(L, 3), type, NULL);
  HMAC_Update(&c, (unsigned char *)s, lua_strlen(L, 2));
  HMAC_Final(&c, digest, &written);

  if (lua_toboolean(L, 4))
    lua_pushlstring(L, (char *)digest, written);
  else
  {
    hex = calloc(sizeof(char), written*2 + 1);
    for (i = 0; i < written; i++)
      sprintf(hex + 2*i, "%02x", digest[i]);
    lua_pushlstring(L, hex, written*2);
    free(hex);
  }

  return 1;
}

static int rand_do_bytes(lua_State *L, int (*bytes)(unsigned char *, int))
{
  size_t count = luaL_checkint(L, 1);
  unsigned char tmp[256], *buf = tmp;
  if (count > sizeof tmp)
    buf = malloc(count);
    if (!buf)
      return luaL_error(L, "out of memory");
    else if (!bytes(buf, count))
      return crypto_error(L);
    lua_pushlstring(L, (char *)buf, count);
    if (buf != tmp)
      free(buf);
    return 1;
}

static int rand_bytes(lua_State *L)
{
  return rand_do_bytes(L, RAND_bytes);
}

static int rand_pseudo_bytes(lua_State *L)
{
  return rand_do_bytes(L, RAND_pseudo_bytes);
}

static int rand_add(lua_State *L)
{
  size_t num;
  const void *buf = luaL_checklstring(L, 1, &num);
  double entropy = luaL_optnumber(L, 2, num);
  RAND_add(buf, num, entropy);
  return 0;
}

static int rand_status(lua_State *L)
{
  lua_pushboolean(L, RAND_status());
  return 1;
}

enum { WRITE_FILE_COUNT = 1024 };
static int rand_load(lua_State *L)
{
  const char *name = luaL_optstring(L, 1, 0);
  char tmp[256];
  int n;
  if (!name && !(name = RAND_file_name(tmp, sizeof tmp)))
    return crypto_error(L);
  n = RAND_load_file(name, WRITE_FILE_COUNT);
  if (n == 0)
    return crypto_error(L);
  lua_pushnumber(L, n);
  return 1;
}

static int rand_write(lua_State *L)
{
  const char *name = luaL_optstring(L, 1, 0);
  char tmp[256];
  int n;
  if (!name && !(name = RAND_file_name(tmp, sizeof tmp)))
    return crypto_error(L);
  n = RAND_write_file(name);
  if (n == 0)
    return crypto_error(L);
  lua_pushnumber(L, n);
  return 1;
}

static int rand_cleanup(lua_State *L)
{
  RAND_cleanup();
  return 0;
}

/*
** Create a metatable and leave it on top of the stack.
*/
LUACRYPTO_API int luacrypto_createmeta (lua_State *L, const char *name, const luaL_reg *methods) {
  if (!luaL_newmetatable (L, name))
    return 0;
  
  /* define methods */
  luaL_openlib (L, NULL, methods, 0);
  
  /* define metamethods */
  lua_pushliteral (L, "__index");
  lua_pushvalue (L, -2);
  lua_settable (L, -3);

  lua_pushliteral (L, "__metatable");
  lua_pushliteral (L, LUACRYPTO_PREFIX"you're not allowed to get this metatable");
  lua_settable (L, -3);

  return 1;
}

/*
** Create metatables for each class of object.
*/
static void create_metatables (lua_State *L)
{
  struct luaL_reg evp_functions[] = {
    { "digest", evp_fdigest },
    { "new", evp_fnew },
    {NULL, NULL},
  };
  struct luaL_reg evp_methods[] = {
    { "__tostring", evp_tostring },
    { "__gc", evp_gc },
    { "clone", evp_clone },
    { "digest", evp_digest },
    { "reset", evp_reset },
    { "tostring", evp_tostring },
    { "update",	evp_update },
    {NULL, NULL},
  };
  struct luaL_reg hmac_functions[] = {
    { "digest", hmac_fdigest },
    { "new", hmac_fnew },
    { NULL, NULL }
  };
  struct luaL_reg hmac_methods[] = {
    { "__tostring", hmac_tostring },
    { "__gc", hmac_gc },
    { "clone", hmac_clone },
    { "digest", hmac_digest },
    { "reset", hmac_reset },
    { "tostring", hmac_tostring },
    { "update", hmac_update },
    { NULL, NULL }
  };
  struct luaL_reg rand_functions[] = {
    { "bytes", rand_bytes },
    { "pseudo_bytes", rand_pseudo_bytes },
    { "add", rand_add },
    { "seed", rand_add },
    { "status", rand_status },
    { "load", rand_load },
    { "write", rand_write },
    { "cleanup", rand_cleanup },
    { NULL, NULL }
  };

  luaL_openlib (L, LUACRYPTO_EVPNAME, evp_functions, 0);
  luacrypto_createmeta(L, LUACRYPTO_EVPNAME, evp_methods);
  luaL_openlib (L, LUACRYPTO_HMACNAME, hmac_functions, 0);
  luacrypto_createmeta(L, LUACRYPTO_HMACNAME, hmac_methods);
  luaL_openlib (L, LUACRYPTO_RANDNAME, rand_functions, 0);
  lua_pop (L, 3);
}

/*
** Define the metatable for the object on top of the stack
*/
LUACRYPTO_API void luacrypto_setmeta (lua_State *L, const char *name) {
  luaL_getmetatable (L, name);
  lua_setmetatable (L, -2);
}

/*
** Assumes the table is on top of the stack.
*/
LUACRYPTO_API void luacrypto_set_info (lua_State *L) {
  lua_pushliteral (L, "_COPYRIGHT");
  lua_pushliteral (L, "Copyright (C) 2005-2006 Keith Howe");
  lua_settable (L, -3);
  lua_pushliteral (L, "_DESCRIPTION");
  lua_pushliteral (L, "LuaCrypto is a Lua wrapper for OpenSSL");
  lua_settable (L, -3);
  lua_pushliteral (L, "_VERSION");
  lua_pushliteral (L, "LuaCrypto 0.2.0");
  lua_settable (L, -3);
}

/*
** Creates the metatables for the objects and registers the
** driver open method.
*/
LUACRYPTO_API int luaopen_crypto(lua_State *L)
{
  struct luaL_reg core[] = {
    {NULL, NULL},
  };

  OpenSSL_add_all_digests();
  
  create_metatables (L);
  luaL_openlib (L, LUACRYPTO_CORENAME, core, 0);
  luacrypto_set_info (L);
  return 1;
}
