
#include "BenchmarkCppScene.h"
#include "WelcomeScene.h"

#include <math.h>

USING_NS_CC;

Scene* BenchmarkCpp::createScene()
{
    auto scene = Scene::create();
    scene->addChild(BenchmarkCpp::create());
    return scene;
}

bool BenchmarkCpp::init()
{
    if (!Layer::init()) {
        return false;
    }

    auto menu = Menu::create();
    menu->addChild(MenuItemLabel::create(Label::createWithSystemFont("Back to Main Menu", "sans", 32), [](Ref*) {
        Director::getInstance()->replaceScene(Welcome::createScene());
    }));
    menu->alignItemsVertically();
    this->addChild(menu, 99999);

    // ----

    _viewsize = Director::getInstance()->getOpenGLView()->getFrameSize();

    _starsLayer = Node::create();
    this->addChild(_starsLayer);

    auto layerColor = LayerColor::create(Color4B(200, 200, 200, 200), 200, 40);
    layerColor->setPosition(_viewsize.width / 2 - 100, _viewsize.height - 40);
    this->addChild(layerColor);
    _starsLabel = Label::createWithSystemFont("0 stars", "sans", 24);
    _starsLabel->setAlignment(TextHAlignment::CENTER);
    _starsLabel->setColor(Color3B(0, 0, 0));
    _starsLabel->setPosition(100, 20);
    layerColor->addChild(_starsLabel);

    _offsetCount = 60;
    for (int i = 0; i < _offsetCount; ++i) {
        _offsets.push_back(Vec2(sinf(i * 6.0f * M_PI / 180.0f) * 4.0f,
                                cosf(i * 6.0f * M_PI / 180.0f) * 4.0f));
    }

    _maxStars = 20000;
    _starsCountOffset = 1000;
    _steps = _stepsCount = 180;

    this->scheduleUpdate();

    return true;
}

void BenchmarkCpp::update(float dt)
{
    _steps++;
    if (_steps >= _stepsCount) {
        if (_starsCountOffset > 0) {
            _addStars(_starsCountOffset);
        } else {
            _removeStars(- _starsCountOffset);
        }
        _steps = 0;
        _starsLabel->setString(std::to_string(_stars.size()));
    }

    for (auto it = _stars.begin(); it != _stars.end(); ++it) {
        _updateStar(*it);
    }
}

void BenchmarkCpp::_addStars(int count)
{
    for (int i = 0; i < count; ++i) {
        Star star;
        star.sprite = Sprite::create("star.png");
        star.pos = Vec2(random<float>(0, _viewsize.width), random<float>(0, _viewsize.height));
        int index = random<int>(0, _offsetCount);
        star.offsetIndex = index;
        star.opacity = random<GLubyte>(0, 255);
        star.opacityOffset = 1;

        star.sprite->setPosition(star.pos + _offsets[index]);
        star.sprite->setOpacity(star.opacity);
        _starsLayer->addChild(star.sprite);

        _stars.push_back(star);
    }

    if (_stars.size() >= _maxStars) {
        _starsCountOffset = -_starsCountOffset;
    }
}

void BenchmarkCpp::_removeStars(int count)
{
    while (count > 0 && _stars.size() > 0) {
        (*_stars.rbegin()).sprite->removeFromParentAndCleanup(true);
        _stars.pop_back();
        count--;
    }

    if (_stars.size() <= 0) {
        _starsCountOffset = -_starsCountOffset;
    }
}

void BenchmarkCpp::_updateStar(Star &star)
{
    star.offsetIndex++;
    star.offsetIndex %= _offsetCount;
    int index = star.offsetIndex;

    star.opacity += star.opacityOffset;
    if (star.opacity > 255) {
        star.opacity = 255;
        star.opacityOffset = -star.opacityOffset;
    } else if (star.opacity < 0) {
        star.opacity = 0;
        star.opacityOffset = -star.opacityOffset;
    }

    star.sprite->setPosition(star.pos + _offsets[index]);
    star.sprite->setOpacity(star.opacity);
}
