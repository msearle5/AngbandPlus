
/*
** $Id: lapi.h,v 1.1 2003/03/19 21:53:06 cipher Exp $
** Auxiliary functions from Lua API
** See Copyright Notice in lua.h
*/

#ifndef lapi_h
#define lapi_h

#include "lobject.h"

TObject        *luaA_index(lua_State * L,
                           int index);
void            luaA_pushobject(lua_State * L,
                                const TObject * o);

#endif
