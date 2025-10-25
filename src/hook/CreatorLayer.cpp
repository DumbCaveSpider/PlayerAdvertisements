#include <Geode/Geode.hpp>
#include <Advertisements.hpp>
#include <Geode/modify/CreatorLayer.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(CreatorLayer)
{
    bool init()
    {
        if (!CreatorLayer::init())
            return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        // banner ad at the top
        auto adBanner = Advertisement::create();
        if (adBanner)
        {
            adBanner->setID("advertisement-menu");
            this->addChild(adBanner);
            adBanner->setType(AdType::Banner);
            adBanner->setPosition({winSize.width / 2.f, winSize.height - 30.f});
            adBanner->loadRandom();
        }
        return true;
    }
};