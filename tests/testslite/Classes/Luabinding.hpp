//
//  Luabinding.hpp
//  testslite
//
//  Created by koala on 28/4/2016.
//  Copyright © 2016 cocos2d. All rights reserved.
//

#ifndef Luabinding_hpp
#define Luabinding_hpp

#include <stdio.h>
#include "lua.hpp"

extern "C" {
    int l_create_namespace_cc(lua_State *L);
    int l_push_userdata(lua_State *L, void *p, const char *name);
}

#endif /* Luabinding_hpp */
