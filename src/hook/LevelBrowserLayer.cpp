#include <Geode/Geode.hpp>
#include <Advertisements.hpp>
#include <Geode/modify/LevelBrowserLayer.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(LevelBrowserLayer)
{
    bool init(GJSearchObject *searchObj)
    {
        if (!LevelBrowserLayer::init(searchObj))
            return false;
        if (Mod::get()->getSettingValue<bool>("LevelBrowserLayer"))
        {
            auto winSize = CCDirector::sharedDirector()->getWinSize();

            // banner ad at the top
            auto adBanner = Advertisement::create();
            if (adBanner)
            {
                adBanner->setID("advertisement-menu");
                this->addChild(adBanner, 2);
                adBanner->setType(AdType::Banner);
                adBanner->setPosition({winSize.width / 2.f, winSize.height - 30.f});
                adBanner->loadRandom();
            }
        }
        return true;
    }
};