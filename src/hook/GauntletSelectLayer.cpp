#include <Geode/Geode.hpp>
#include <Advertisements.hpp>
#include <Geode/modify/GauntletSelectLayer.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(GauntletSelectLayer) {
    bool init(int p0) {
        if (!GauntletSelectLayer::init(p0))
            return false;
        if (Mod::get()->getSettingValue<bool>("GauntletSelectLayer")) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();

            // banner at the bottom center
            auto adBanner = Advertisement::create();
            if (adBanner) {
                adBanner->setID("advertisement-leaderboards-bottom");
                this->addChild(adBanner, 1);
                adBanner->setType(AdType::Banner);
                adBanner->setPosition({ winSize.width / 2.f, 30.f });
                adBanner->loadRandom();
            }
        }
        return true;
    }
};