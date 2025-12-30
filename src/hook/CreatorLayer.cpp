#include <Geode/Geode.hpp>
#include <Advertisements.hpp>
#include <Geode/modify/CreatorLayer.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(AdsCreatorLayer, CreatorLayer) {
    bool init() {
        if (!CreatorLayer::init()) return false;

        if (Mod::get()->getSettingValue<bool>("CreatorLayer")) {
            auto const winSize = CCDirector::sharedDirector()->getWinSize();

            // banner ad at the top
            if (auto adBanner = Advertisement::create()) {
                adBanner->setID("banner"_spr);
                adBanner->setType(AdType::Banner);
                adBanner->setPosition({ winSize.width / 2.f, winSize.height - 30.f });

                this->addChild(adBanner);

                adBanner->loadRandom();
            };
        };

        return true;
    };
};