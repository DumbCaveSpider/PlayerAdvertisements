#include <Geode/Geode.hpp>
#include <Advertisements.hpp>
#include <Geode/modify/LevelBrowserLayer.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(MyLevelBrowserLayer, LevelBrowserLayer)
{
    bool init(GJSearchObject* searchObj)
    {
        if (!LevelBrowserLayer::init(searchObj))
            return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        // banner ad at the top
        auto adBanner = Advertisement::create();
        if (adBanner)
        {
            adBanner->setID("advertisement-menu");
            this->addChild(adBanner, 2);
            adBanner->setType(AdType::Banner);
            adBanner->setPosition({winSize.width / 5.5f, winSize.height - 50.f});
            adBanner->loadRandom();
        }

        return true;
    }
};