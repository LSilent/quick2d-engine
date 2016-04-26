#ifndef __WELCOME_SCENE_H__
#define __WELCOME_SCENE_H__

#include "cocos2d.h"

USING_NS_CC;

class Welcome : public Layer
{
public:
    static Scene *createScene();

    virtual bool init();
    CREATE_FUNC(Welcome);
};

#endif // __WELCOME_SCENE_H__
