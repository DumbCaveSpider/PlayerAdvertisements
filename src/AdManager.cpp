#include <Geode/Geode.hpp>
#include "AdManager.hpp"

using namespace geode::prelude;

bool AdManager::setup()
{
    setTitle("Advertisement Manager");
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    // add a background on the left side
    auto bg1 = CCScale9Sprite::create("square02_001.png");
    bg1->setPosition({winSize.width / 6, winSize.height / 2 - 30.f});
    bg1->setContentSize({200.f, 220.f});
    bg1->setOpacity(50);
    m_mainLayer->addChild(bg1);

    // add a background on the right side
    auto bg2 = CCScale9Sprite::create("square02_001.png");
    bg2->setPosition({winSize.width / 6 * 3, winSize.height / 2 - 30.f});
    bg2->setContentSize({200.f, 220.f});
    bg2->setOpacity(50);
    m_mainLayer->addChild(bg2);

    return true;
}

AdManager *AdManager::create()
{
    auto ret = new AdManager();
    if (ret && ret->initAnchored(450.f, 280.f))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}
