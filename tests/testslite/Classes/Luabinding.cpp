//
//  Luabinding.cpp
//  testslite
//
//  Created by koala on 28/4/2016.
//  Copyright © 2016 cocos2d. All rights reserved.
//

#include <string.h>

#include "Luabinding.hpp"
#include "cocos2d.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

USING_NS_CC;

// functions

typedef int (*lua_CFunction) (lua_State *L);

static int l_register_class(lua_State *L, const char *name, int(*f)(lua_State*))
{
    (*f)(L);                                // L: cls
    lua_pushstring(L, name);                // L: cls name
    lua_pushvalue(L, -2);                   // L: cls name cls
    lua_rawset(L, LUA_REGISTRYINDEX);       // L: cls
    return 1;
}

static int l_get_class(lua_State *L, const char *name)
{
    lua_pushstring(L, name);
    lua_rawget(L, LUA_REGISTRYINDEX);
    return 1;
}

static int l_set_base_class(lua_State *L, const char *name, const char *basename)
{
    l_get_class(L, name);
    lua_newtable(L);
    lua_pushliteral(L, "__index");
    l_get_class(L, basename);
    lua_rawset(L, -3);
    lua_setmetatable(L, -2);
    lua_pop(L, 1);
    return 0;
}

static int l_meta_index(lua_State *L)
{
    // L: self key
    lua_upvalueindex(1);    // L: self key getters
    lua_pushvalue(L, -2);   // L: self key getters key
    lua_rawget(L, -2);      // L: self key getters getter
    lua_CFunction getter = lua_tocfunction(L, -1);
    lua_pop(L, 2);          // L: self key
    return (*getter)(L);
}

static int l_meta_newindex(lua_State *L)
{
    // L: self key value
    lua_upvalueindex(1);    // L: self key value setters
    lua_pushvalue(L, -3);   // L: self key value setters key
    lua_rawget(L, -2);      // L: self key value setters setter
    lua_CFunction setter = lua_tocfunction(L, -1);
    lua_pop(L, 2);          // L: self key value
    return (*setter)(L);
}

// Ref

static int l_Ref_retain(lua_State *L)
{
    Ref *ref = static_cast<Ref*>(lua_touserdata(L, -1));
    ref->retain();
    return 0;
}

static int l_Ref_release(lua_State *L)
{
    Ref *ref = static_cast<Ref*>(lua_touserdata(L, -1));
    ref->release();
    return 0;
}

static int l_Ref_getReferenceCount(lua_State *L)
{
    Ref *ref = static_cast<Ref*>(lua_touserdata(L, -1));
    lua_pushinteger(L, ref->getReferenceCount());
    return 1;
}

static int l_create_class_Ref(lua_State *L)
{
    lua_newtable(L);

    lua_pushliteral(L, "retain");
    lua_pushcfunction(L, l_Ref_retain);
    lua_rawset(L, -3);

    lua_pushliteral(L, "release");
    lua_pushcfunction(L, l_Ref_release);
    lua_rawset(L, -3);

    lua_pushliteral(L, "getReferenceCount");
    lua_pushcfunction(L, l_Ref_getReferenceCount);
    lua_rawset(L, -3);

    // lua_pushliteral(L, "__index");
    // lua_pushvalue(L, -2);
    // lua_rawset(L, -3);

    // setmetatable(cls, {__index = cls})

    return 1;
}


// Node

static int l_Node_static_create(lua_State *L)
{
    Node *node = Node::create();
    return l_push_userdata(L, node, "cc.Node");
}

static int l_Node_addChild(lua_State *L)
{
    Node *self = static_cast<Node*>(lua_touserdata(L, -2));
    Node *child = static_cast<Node*>(lua_touserdata(L, -1));
    self->addChild(child);
    return 0;
}

static int l_Node_removeChild(lua_State *L)
{
    Node *self = static_cast<Node*>(lua_touserdata(L, -2));
    Node *child = static_cast<Node*>(lua_touserdata(L, -1));
    self->removeChild(child);
    return 0;
}

static int l_Node_setPosition(lua_State *L)
{
    Node *node = static_cast<Node*>(lua_touserdata(L, -3));
    node->setPosition(lua_tonumber(L, -2), lua_tonumber(L, -1));
    return 0;
}

static int l_Node_setColor(lua_State *L)
{
    Node *self = static_cast<Node*>(lua_touserdata(L, -2));
    Color3B *color = static_cast<Color3B*>(lua_touserdata(L, -1));
    self->setColor(*color);
    return 0;
}

static int l_Node_setOpacity(lua_State *L)
{
    // node, opacity
    Node *node = static_cast<Node*>(lua_touserdata(L, -2));
    node->setOpacity(lua_tointeger(L, -1));
    return 0;
}

static int l_Node_schedule(lua_State *L)
{

    return 0;
}

static int l_create_class_Node(lua_State *L)
{
    lua_newtable(L);

    lua_pushliteral(L, "create");
    lua_pushcfunction(L, l_Node_static_create);
    lua_rawset(L, -3);

    lua_pushliteral(L, "addChild");
    lua_pushcfunction(L, l_Node_addChild);
    lua_rawset(L, -3);

    lua_pushliteral(L, "removeChild");
    lua_pushcfunction(L, l_Node_removeChild);
    lua_rawset(L, -3);

    lua_pushliteral(L, "setPosition");
    lua_pushcfunction(L, l_Node_setPosition);
    lua_rawset(L, -3);

    lua_pushliteral(L, "setColor");
    lua_pushcfunction(L, l_Node_setColor);
    lua_rawset(L, -3);

    lua_pushliteral(L, "setOpacity");
    lua_pushcfunction(L, l_Node_setOpacity);
    lua_rawset(L, -3);

    lua_pushliteral(L, "schedule");
    lua_pushcfunction(L, l_Node_schedule);
    lua_rawset(L, -3);

    // lua_pushliteral(L, "__index");
    // lua_pushvalue(L, -2);
    // lua_rawset(L, -3);

    return 1;
}

// Sprite

static int l_Sprite_static_create(lua_State *L)
{
    const char *filename = lua_tostring(L, -1);
    Sprite *sprite = Sprite::create(filename);
    return l_push_userdata(L, sprite, "cc.Sprite");
}

static int l_create_class_Sprite(lua_State *L)
{
    lua_newtable(L);

    lua_pushliteral(L, "create");
    lua_pushcfunction(L, l_Sprite_static_create);
    lua_rawset(L, -3);

    // lua_pushliteral(L, "__index");
    // lua_pushvalue(L, -2);
    // lua_rawset(L, -3);

    return 1;
}

// Layer

static int l_Layer_static_create(lua_State *L)
{
    Layer *layer = Layer::create();
    return l_push_userdata(L, layer, "cc.Layer");
}

static int l_create_class_Layer(lua_State *L)
{
    lua_newtable(L);

    lua_pushliteral(L, "create");
    lua_pushcfunction(L, l_Layer_static_create);
    lua_rawset(L, -3);

    // lua_pushliteral(L, "__index");
    // lua_pushvalue(L, -2);
    // lua_rawset(L, -3);

    return 1;
}

static int l_LayerColor_static_create(lua_State *L)
{
    LayerColor *layer;
    int top = lua_gettop(L);
    switch (top) {
        case 3:
        {
            Color4B *color = static_cast<Color4B*>(lua_touserdata(L, -3));
            float width = lua_tonumber(L, -2);
            float height = lua_tonumber(L, -1);
            layer = LayerColor::create(*color, width, height);
            break;
        }

        case 1:
        {
            Color4B *color = static_cast<Color4B*>(lua_touserdata(L, -1));
            layer = LayerColor::create(*color);
            break;
        }

        default:
            layer = LayerColor::create();
    }
    return l_push_userdata(L, layer, "cc.LayerColor");
}

static int l_create_class_LayerColor(lua_State *L)
{
    lua_newtable(L);

    lua_pushliteral(L, "create");
    lua_pushcfunction(L, l_LayerColor_static_create);
    lua_rawset(L, -3);

    // lua_pushliteral(L, "__index");
    // lua_pushvalue(L, -2);
    // lua_rawset(L, -3);

    return 1;
}

// Label

static int l_Label_static_createWithSystemFont(lua_State *L)
{
    const char *text = lua_tostring(L, -3);
    const char *font = lua_tostring(L, -2);
    float fontsize = lua_tonumber(L, -1);

    CCLOG("text = %s, font = %s, fontsize = %0.2f", text, font, fontsize);

    Label *label = Label::createWithSystemFont(text, font, fontsize);
    return l_push_userdata(L, label, "cc.Label");
}

static int l_Label_setString(lua_State *L)
{
    Label *label = static_cast<Label*>(lua_touserdata(L, -2));
    const char *text = lua_tostring(L, -1);
    label->setString(text);
    return 0;
}

static int l_create_class_Label(lua_State *L)
{
    lua_newtable(L);

    lua_pushliteral(L, "createWithSystemFont");
    lua_pushcfunction(L, l_Label_static_createWithSystemFont);
    lua_rawset(L, -3);

    lua_pushliteral(L, "setString");
    lua_pushcfunction(L, l_Label_setString);
    lua_rawset(L, -3);

    // lua_pushliteral(L, "__index");
    // lua_pushvalue(L, -2);
    // lua_rawset(L, -3);

    return 1;
}

// Vec2

static int l_Vec2_static_new(lua_State *L)
{
    Vec2 *vec2;
    if (lua_gettop(L) == 2) {
        float x = lua_tonumber(L, -2);
        float y = lua_tonumber(L, -1);
        vec2 = new Vec2(x, y);
    } else {
        vec2 = new Vec2();
    }

    l_push_userdata(L, vec2, "cc.Vec2");
    return 1;
}

static int l_Vec2_meta_gc(lua_State *L)
{
    Vec2 *vec2 = static_cast<Vec2*>(lua_touserdata(L, -1));
    delete vec2;
    return 0;
}

static int l_Vec2_getprop_x(lua_State *L)
{
    // L: self key
    Vec2 *vec2 = static_cast<Vec2*>(lua_touserdata(L, -2));
    lua_pushnumber(L, vec2->x);
    return 1;
}

static int l_Vec2_setprop_x(lua_State *L)
{
    // L: self key value
    Vec2 *vec2 = static_cast<Vec2*>(lua_touserdata(L, -3));
    vec2->x = lua_tonumber(L, -1);
    return 0;
}

static int l_Vec2_getprop_y(lua_State *L)
{
    // L: self key
    Vec2 *vec2 = static_cast<Vec2*>(lua_touserdata(L, -2));
    lua_pushnumber(L, vec2->y);
    return 1;
}

static int l_Vec2_setprop_y(lua_State *L)
{
    // L: self key value
    Vec2 *vec2 = static_cast<Vec2*>(lua_touserdata(L, -3));
    vec2->y = lua_tonumber(L, -1);
    return 0;
}

static int l_create_class_Vec2(lua_State *L)
{
    lua_newtable(L);

    lua_pushliteral(L, "new");
    lua_pushcfunction(L, l_Vec2_static_new);
    lua_rawset(L, -3);

    // __index
    lua_newtable(L);                            // L: cls getters
    // x
    lua_pushliteral(L, "x");                    // L: cls getters key
    lua_pushvalue(L, -2);                       // L: cls getters key __index
    lua_pushcclosure(L, &l_Vec2_getprop_x, 1);  // L: cls getters key closure
    lua_rawset(L, -3);                          // L: cls getters
    // y
    lua_pushliteral(L, "y");
    lua_pushvalue(L, -2);
    lua_pushcclosure(L, &l_Vec2_getprop_y, 1);
    lua_rawset(L, -3);                          // L: cls getters
    // set __index
    lua_pushcclosure(L, &l_meta_index, 1);      // L: cls __index
    lua_pushliteral(L, "__index");              // L: cls __index key
    lua_insert(L, -2);                          // L: cls key __index
    lua_rawset(L, -3);                          // L: cls

    // __newindex
    lua_newtable(L);                            // L: cls setters
    // x
    lua_pushliteral(L, "x");                    // L: cls setters key
    lua_pushvalue(L, -2);                       // L: cls setters key __index
    lua_pushcclosure(L, &l_Vec2_setprop_x, 1);  // L: cls setters key closure
    lua_rawset(L, -3);                          // L: cls setters
    // y
    lua_pushliteral(L, "y");
    lua_pushvalue(L, -2);
    lua_pushcclosure(L, &l_Vec2_setprop_y, 1);
    lua_rawset(L, -3);                          // L: cls setters
    // set __index
    lua_pushcclosure(L, &l_meta_newindex, 1);   // L: cls __newindex
    lua_pushliteral(L, "__newindex");           // L: cls __index key
    lua_insert(L, -2);                          // L: cls key __newindex
    lua_rawset(L, -3);                          // L: cls

    // __gc
    lua_pushliteral(L, "__gc");
    lua_pushcfunction(L, &l_Vec2_meta_gc);
    lua_rawset(L, -3);

    // lua_pushliteral(L, "__index");
    // lua_pushvalue(L, -2);
    // lua_rawset(L, -3);

    return 1;
}

// Size

static int l_Size_static_new(lua_State *L)
{
    Size *size;
    if (lua_gettop(L) == 2) {
        float width = lua_tonumber(L, -2);
        float height = lua_tonumber(L, -1);
        size = new Size(width, height);
    } else {
        size = new Size();
    }

    l_push_userdata(L, size, "cc.Size");
    return 1;
}

static int l_Size_meta_gc(lua_State *L)
{
    Size *size = static_cast<Size*>(lua_touserdata(L, -1));
    delete size;
    return 0;
}

static int l_Size_getprop_width(lua_State *L)
{
    // L: self key
    Size *size = static_cast<Size*>(lua_touserdata(L, -2));
    lua_pushnumber(L, size->width);
    return 1;
}

static int l_Size_setprop_width(lua_State *L)
{
    // L: self key value
    Size *size = static_cast<Size*>(lua_touserdata(L, -3));
    size->width = lua_tonumber(L, -1);
    return 0;
}

static int l_Size_getprop_height(lua_State *L)
{
    // L: self key
    Size *size = static_cast<Size*>(lua_touserdata(L, -2));
    lua_pushnumber(L, size->height);
    return 1;
}

static int l_Size_setprop_height(lua_State *L)
{
    // L: self key value
    Size *size = static_cast<Size*>(lua_touserdata(L, -3));
    size->height = lua_tonumber(L, -1);
    return 0;
}

static int l_create_class_Size(lua_State *L)
{
    lua_newtable(L);

    lua_pushliteral(L, "new");
    lua_pushcfunction(L, l_Size_static_new);
    lua_rawset(L, -3);

    // __index
    lua_newtable(L);                            // L: cls getters
    // width
    lua_pushliteral(L, "width");                // L: cls getters key
    lua_pushvalue(L, -2);                       // L: cls getters key __index
    lua_pushcclosure(L, &l_Size_getprop_width, 1);  // L: cls getters key closure
    lua_rawset(L, -3);                          // L: cls getters
    // height
    lua_pushliteral(L, "height");
    lua_pushvalue(L, -2);
    lua_pushcclosure(L, &l_Size_getprop_height, 1);
    lua_rawset(L, -3);                          // L: cls getters
    // set __index
    lua_pushcclosure(L, &l_meta_index, 1);      // L: cls __index
    lua_pushliteral(L, "__index");              // L: cls __index key
    lua_insert(L, -2);                          // L: cls key __index
    lua_rawset(L, -3);                          // L: cls

    // __newindex
    lua_newtable(L);                            // L: cls setters
    // width
    lua_pushliteral(L, "width");                    // L: cls setters key
    lua_pushvalue(L, -2);                       // L: cls setters key __index
    lua_pushcclosure(L, &l_Size_setprop_width, 1);  // L: cls setters key closure
    lua_rawset(L, -3);                          // L: cls setters
    // height
    lua_pushliteral(L, "height");
    lua_pushvalue(L, -2);
    lua_pushcclosure(L, &l_Size_setprop_height, 1);
    lua_rawset(L, -3);                          // L: cls setters
    // set __index
    lua_pushcclosure(L, &l_meta_newindex, 1);   // L: cls __newindex
    lua_pushliteral(L, "__newindex");           // L: cls __index key
    lua_insert(L, -2);                          // L: cls key __newindex
    lua_rawset(L, -3);                          // L: cls

    // __gc
    lua_pushliteral(L, "__gc");
    lua_pushcfunction(L, &l_Size_meta_gc);
    lua_rawset(L, -3);

    // lua_pushliteral(L, "__index");
    // lua_pushvalue(L, -2);
    // lua_rawset(L, -3);

    return 1;
}

// Color4B

static int l_Color4B_static_new(lua_State *L)
{
    Color4B *color;
    if (lua_gettop(L) == 4) {
        GLubyte r = lua_tointeger(L, -4);
        GLubyte g = lua_tointeger(L, -3);
        GLubyte b = lua_tointeger(L, -2);
        GLubyte a = lua_tointeger(L, -1);
        color = new Color4B(r, g, b, a);
    } else {
        color = new Color4B();
    }

    l_push_userdata(L, color, "cc.Color4B");
    return 1;
}

static int l_Color4B_meta_gc(lua_State *L)
{
    Color4B *color = static_cast<Color4B*>(lua_touserdata(L, -1));
    delete color;
    return 0;
}

static int l_Color4B_getprop_r(lua_State *L)
{
    // L: self key
    Color4B *color = static_cast<Color4B*>(lua_touserdata(L, -2));
    lua_pushinteger(L, color->r);
    return 1;
}

static int l_Color4B_setprop_r(lua_State *L)
{
    // L: self key value
    Color4B *color = static_cast<Color4B*>(lua_touserdata(L, -3));
    color->r = lua_tonumber(L, -1);
    return 0;
}

static int l_Color4B_getprop_g(lua_State *L)
{
    // L: self key
    Color4B *color = static_cast<Color4B*>(lua_touserdata(L, -2));
    lua_pushinteger(L, color->g);
    return 1;
}

static int l_Color4B_setprop_g(lua_State *L)
{
    // L: self key value
    Color4B *color = static_cast<Color4B*>(lua_touserdata(L, -3));
    color->g = lua_tonumber(L, -1);
    return 0;
}

static int l_Color4B_getprop_b(lua_State *L)
{
    // L: self key
    Color4B *color = static_cast<Color4B*>(lua_touserdata(L, -2));
    lua_pushinteger(L, color->b);
    return 1;
}

static int l_Color4B_setprop_b(lua_State *L)
{
    // L: self key value
    Color4B *color = static_cast<Color4B*>(lua_touserdata(L, -3));
    color->b = lua_tonumber(L, -1);
    return 0;
}

static int l_Color4B_getprop_a(lua_State *L)
{
    // L: self key
    Color4B *color = static_cast<Color4B*>(lua_touserdata(L, -2));
    lua_pushinteger(L, color->a);
    return 1;
}

static int l_Color4B_setprop_a(lua_State *L)
{
    // L: self key value
    Color4B *color = static_cast<Color4B*>(lua_touserdata(L, -3));
    color->a = lua_tonumber(L, -1);
    return 0;
}

static int l_create_class_Color4B(lua_State *L)
{
    lua_newtable(L);

    lua_pushliteral(L, "new");
    lua_pushcfunction(L, l_Color4B_static_new);
    lua_rawset(L, -3);

    // __index
    lua_newtable(L);
    // r
    lua_pushliteral(L, "r");
    lua_pushvalue(L, -2);
    lua_pushcclosure(L, &l_Color4B_getprop_r, 1);
    lua_rawset(L, -3);
    // g
    lua_pushliteral(L, "g");
    lua_pushvalue(L, -2);
    lua_pushcclosure(L, &l_Color4B_getprop_g, 1);
    lua_rawset(L, -3);
    // b
    lua_pushliteral(L, "b");
    lua_pushvalue(L, -2);
    lua_pushcclosure(L, &l_Color4B_getprop_b, 1);
    lua_rawset(L, -3);
    // a
    lua_pushliteral(L, "a");
    lua_pushvalue(L, -2);
    lua_pushcclosure(L, &l_Color4B_getprop_a, 1);
    lua_rawset(L, -3);
    // set __index
    lua_pushcclosure(L, &l_meta_index, 1);      // L: cls __index
    lua_pushliteral(L, "__index");              // L: cls __index key
    lua_insert(L, -2);                          // L: cls key __index
    lua_rawset(L, -3);                          // L: cls

    // __newindex
    lua_newtable(L);                            // L: cls setters
    // r
    lua_pushliteral(L, "r");
    lua_pushvalue(L, -2);
    lua_pushcclosure(L, &l_Color4B_setprop_r, 1);
    lua_rawset(L, -3);
    // g
    lua_pushliteral(L, "g");
    lua_pushvalue(L, -2);
    lua_pushcclosure(L, &l_Color4B_setprop_g, 1);
    lua_rawset(L, -3);
    // b
    lua_pushliteral(L, "b");
    lua_pushvalue(L, -2);
    lua_pushcclosure(L, &l_Color4B_setprop_b, 1);
    lua_rawset(L, -3);
    // a
    lua_pushliteral(L, "a");
    lua_pushvalue(L, -2);
    lua_pushcclosure(L, &l_Color4B_setprop_a, 1);
    lua_rawset(L, -3);
    // set __index
    lua_pushcclosure(L, &l_meta_newindex, 1);   // L: cls __newindex
    lua_pushliteral(L, "__newindex");           // L: cls __index key
    lua_insert(L, -2);                          // L: cls key __newindex
    lua_rawset(L, -3);                          // L: cls

    // __gc
    lua_pushliteral(L, "__gc");
    lua_pushcfunction(L, &l_Color4B_meta_gc);
    lua_rawset(L, -3);

    // lua_pushliteral(L, "__index");
    // lua_pushvalue(L, -2);
    // lua_rawset(L, -3);

    return 1;
}

// Color3B

static int l_Color3B_static_new(lua_State *L)
{
    Color3B *color;
    if (lua_gettop(L) == 3) {
        GLubyte r = lua_tointeger(L, -3);
        GLubyte g = lua_tointeger(L, -2);
        GLubyte b = lua_tointeger(L, -1);
        color = new Color3B(r, g, b);
    } else {
        color = new Color3B();
    }

    l_push_userdata(L, color, "cc.Color3B");
    return 1;
}

static int l_Color3B_meta_gc(lua_State *L)
{
    Color3B *color = static_cast<Color3B*>(lua_touserdata(L, -1));
    delete color;
    return 0;
}

static int l_Color3B_getprop_r(lua_State *L)
{
    // L: self key
    Color3B *color = static_cast<Color3B*>(lua_touserdata(L, -2));
    lua_pushinteger(L, color->r);
    return 1;
}

static int l_Color3B_setprop_r(lua_State *L)
{
    // L: self key value
    Color3B *color = static_cast<Color3B*>(lua_touserdata(L, -3));
    color->r = lua_tonumber(L, -1);
    return 0;
}

static int l_Color3B_getprop_g(lua_State *L)
{
    // L: self key
    Color3B *color = static_cast<Color3B*>(lua_touserdata(L, -2));
    lua_pushinteger(L, color->g);
    return 1;
}

static int l_Color3B_setprop_g(lua_State *L)
{
    // L: self key value
    Color3B *color = static_cast<Color3B*>(lua_touserdata(L, -3));
    color->g = lua_tonumber(L, -1);
    return 0;
}

static int l_Color3B_getprop_b(lua_State *L)
{
    // L: self key
    Color3B *color = static_cast<Color3B*>(lua_touserdata(L, -2));
    lua_pushinteger(L, color->b);
    return 1;
}

static int l_Color3B_setprop_b(lua_State *L)
{
    // L: self key value
    Color3B *color = static_cast<Color3B*>(lua_touserdata(L, -3));
    color->b = lua_tonumber(L, -1);
    return 0;
}

static int l_create_class_Color3B(lua_State *L)
{
    lua_newtable(L);

    lua_pushliteral(L, "new");
    lua_pushcfunction(L, l_Color3B_static_new);
    lua_rawset(L, -3);

    // __index
    lua_newtable(L);
    // r
    lua_pushliteral(L, "r");
    lua_pushvalue(L, -2);
    lua_pushcclosure(L, &l_Color3B_getprop_r, 1);
    lua_rawset(L, -3);
    // g
    lua_pushliteral(L, "g");
    lua_pushvalue(L, -2);
    lua_pushcclosure(L, &l_Color3B_getprop_g, 1);
    lua_rawset(L, -3);
    // b
    lua_pushliteral(L, "b");
    lua_pushvalue(L, -2);
    lua_pushcclosure(L, &l_Color3B_getprop_b, 1);
    lua_rawset(L, -3);
    // set __index
    lua_pushcclosure(L, &l_meta_index, 1);      // L: cls __index
    lua_pushliteral(L, "__index");              // L: cls __index key
    lua_insert(L, -2);                          // L: cls key __index
    lua_rawset(L, -3);                          // L: cls

    // __newindex
    lua_newtable(L);                            // L: cls setters
    // r
    lua_pushliteral(L, "r");
    lua_pushvalue(L, -2);
    lua_pushcclosure(L, &l_Color3B_setprop_r, 1);
    lua_rawset(L, -3);
    // g
    lua_pushliteral(L, "g");
    lua_pushvalue(L, -2);
    lua_pushcclosure(L, &l_Color3B_setprop_g, 1);
    lua_rawset(L, -3);
    // b
    lua_pushliteral(L, "b");
    lua_pushvalue(L, -2);
    lua_pushcclosure(L, &l_Color3B_setprop_b, 1);
    lua_rawset(L, -3);
    // set __index
    lua_pushcclosure(L, &l_meta_newindex, 1);   // L: cls __newindex
    lua_pushliteral(L, "__newindex");           // L: cls __index key
    lua_insert(L, -2);                          // L: cls key __newindex
    lua_rawset(L, -3);                          // L: cls

    // __gc
    lua_pushliteral(L, "__gc");
    lua_pushcfunction(L, &l_Color3B_meta_gc);
    lua_rawset(L, -3);

    // lua_pushliteral(L, "__index");
    // lua_pushvalue(L, -2);
    // lua_rawset(L, -3);

    return 1;
}

//

int l_create_namespace_cc(lua_State *L)
{
    // create namespace cc
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "cc");

    // add classes
    lua_pushliteral(L, "Ref");
    l_register_class(L, "cc.Ref", &l_create_class_Ref);
    lua_rawset(L, -3);

    lua_pushliteral(L, "Node");
    l_register_class(L, "cc.Node", &l_create_class_Node);
    lua_rawset(L, -3);

    lua_pushliteral(L, "Sprite");
    l_register_class(L, "cc.Sprite", &l_create_class_Sprite);
    lua_rawset(L, -3);

    lua_pushliteral(L, "Layer");
    l_register_class(L, "cc.Layer", &l_create_class_Layer);
    lua_rawset(L, -3);

    lua_pushliteral(L, "LayerColor");
    l_register_class(L, "cc.LayerColor", &l_create_class_LayerColor);
    lua_rawset(L, -3);

    lua_pushliteral(L, "Label");
    l_register_class(L, "cc.Label", &l_create_class_Label);
    lua_rawset(L, -3);

    lua_pushliteral(L, "Vec2");
    l_register_class(L, "cc.Vec2", &l_create_class_Vec2);
    lua_rawset(L, -3);

    lua_pushliteral(L, "Size");
    l_register_class(L, "cc.Size", &l_create_class_Size);
    lua_rawset(L, -3);

    lua_pushliteral(L, "Color4B");
    l_register_class(L, "cc.Color4B", &l_create_class_Color4B);
    lua_rawset(L, -3);

    lua_pushliteral(L, "Color3B");
    l_register_class(L, "cc.Color3B", &l_create_class_Color3B);
    lua_rawset(L, -3);

    // set base classes
    l_set_base_class(L, "cc.Node", "cc.Ref");
    l_set_base_class(L, "cc.Sprite", "cc.Node");

    l_set_base_class(L, "cc.Layer", "cc.Node");
    l_set_base_class(L, "cc.LayerColor", "cc.Layer");

    l_set_base_class(L, "cc.Label", "cc.Node");

    return 0;
}

int l_push_userdata(lua_State *L, void *p, const char *name)
{
    lua_pushlightuserdata(L, p);
    lua_newtable(L);
    lua_pushliteral(L, "__index");
    l_get_class(L, name);
    lua_rawset(L, -3);
    lua_setmetatable(L, -2);
    return 1;
}
