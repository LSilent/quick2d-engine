#ifndef __BENCHMARK_LUA_SCENE_H__
#define __BENCHMARK_LUA_SCENE_H__

#include "cocos2d.h"
#include "kaguya/kaguya.hpp"

USING_NS_CC;

class BenchmarkLua : public Layer
{
public:
    static Scene *createScene();

    virtual bool init();
    CREATE_FUNC(BenchmarkLua);

private:
    kaguya::State _lua;
};

#endif // __BENCHMARK_LUA_SCENE_H__
