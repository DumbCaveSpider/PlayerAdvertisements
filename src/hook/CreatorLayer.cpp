#include <Geode/Geode.hpp>
#include <Advertisements.hpp>
#include <Geode/modify/CreatorLayer.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(MyCreatorLayer, CreatorLayer)
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
            adBanner->setPosition({winSize.width / 5.5f, winSize.height - 50.f});
            adBanner->loadRandom();
        }
        return true;
    }
};