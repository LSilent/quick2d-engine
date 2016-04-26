
#include "WelcomeScene.h"
#include "BenchmarkCppScene.h"
#include "BenchmarkLuaScene.h"

USING_NS_CC;

Scene* Welcome::createScene()
{
    auto scene = Scene::create();
    scene->addChild(Welcome::create());
    return scene;
}

bool Welcome::init()
{
    if (!Layer::init()) {
        return false;
    }

    auto menu = Menu::create();

    menu->addChild(MenuItemLabel::create(Label::createWithSystemFont("Benchmark C++", "sans", 32), [](Ref*) {
        Director::getInstance()->replaceScene(BenchmarkCpp::createScene());
    }));

    menu->addChild(MenuItemLabel::create(Label::createWithSystemFont("Benchmark Lua", "sans", 32), [](Ref*) {
        Director::getInstance()->replaceScene(BenchmarkLua::createScene());
    }));

    auto size = Director::getInstance()->getOpenGLView()->getFrameSize();
    menu->setPosition(Vec2(size.width / 2, size.height / 2));
    menu->alignItemsVertically();
    this->addChild(menu);
    
    return true;
}
