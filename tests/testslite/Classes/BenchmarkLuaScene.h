#ifndef __BENCHMARK_LUA_SCENE_H__
#define __BENCHMARK_LUA_SCENE_H__

#include "cocos2d.h"
//#include "kaguya/kaguya.hpp"
#include "lua.hpp"

USING_NS_CC;

class BenchmarkLua : public Layer
{
public:
    static Scene *createScene();
    virtual ~BenchmarkLua();

    virtual bool init();
    CREATE_FUNC(BenchmarkLua);

private:
//    kaguya::State _lua;
    lua_State *_luaState;
};

#endif // __BENCHMARK_LUA_SCENE_H__
