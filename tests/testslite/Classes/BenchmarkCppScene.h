#ifndef __BENCHMARK_CPP_SCENE_H__
#define __BENCHMARK_CPP_SCENE_H__

#include "cocos2d.h"

#include <vector>

USING_NS_CC;

typedef struct {
    Sprite *sprite;
    Vec2 pos;
    int opacity;
    int opacityOffset;
    int offsetIndex;
} Star;

class BenchmarkCpp : public Layer
{
public:
    static Scene *createScene();

    virtual bool init();
    CREATE_FUNC(BenchmarkCpp);

    virtual void update(float dt);

private:
    Size _viewsize;
    Node *_starsLayer;
    Label *_starsLabel;

    int _offsetCount;
    std::vector<Vec2> _offsets;

    int _maxStars;
    int _starsCountOffset;
    int _stepsCount;
    int _steps;
    std::vector<Star> _stars;

    void _addStars(int count);
    void _removeStars(int count);
    void _updateStar(Star &star);
};

#endif // __BENCHMARK_CPP_SCENE_H__
