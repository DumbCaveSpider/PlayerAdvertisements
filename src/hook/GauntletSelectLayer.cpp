#include <Geode/Geode.hpp>
#include <Advertisements.hpp>
#include <Geode/modify/GauntletSelectLayer.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(AdsGauntletSelectLayer, GauntletSelectLayer) {
    bool init(int p0) {
        if (!GauntletSelectLayer::init(p0)) return false;

        if (Mod::get()->getSettingValue<bool>("GauntletSelectLayer")) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();

            // banner at the bottom center
            if (auto adBanner = Advertisement::create()) {
                adBanner->setID("banner"_spr);
                adBanner->setType(AdType::Banner);
                adBanner->setPosition({ winSize.width / 2.f, 30.f });

                this->addChild(adBanner, 1);

                adBanner->loadRandom();
            };
        };

        return true;
    };
};