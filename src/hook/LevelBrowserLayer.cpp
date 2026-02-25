#include <Geode/Geode.hpp>
#include <Advertisements.hpp>
#include <Geode/modify/LevelBrowserLayer.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(AdsLevelBrowserLayer, LevelBrowserLayer) {
    bool init(GJSearchObject * searchObj) {
        if (!LevelBrowserLayer::init(searchObj)) return false;

        if (Mod::get()->getSettingValue<bool>("LevelBrowserLayer")) {
            auto const winSize = CCDirector::sharedDirector()->getWinSize();

            // banner ad at the top
            if (auto adBanner = Advertisement::create(AdType::Banner)) {
                adBanner->setID("banner"_spr);
                adBanner->setPosition({ winSize.width / 2.f, winSize.height - 30.f });

                this->addChild(adBanner, 2);

                adBanner->loadRandom();
            };
        };

        return true;
    };
};