#include <Geode/Geode.hpp>
#include <Geode/modify/GJPathsLayer.hpp>
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(GJPathsLayer) {
    bool init() {
        if (!GJPathsLayer::init()) return false;

        if (Mod::get()->getSettingValue<bool>("GJPathsLayer")) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();

            // square ad at the left side
            if (auto adBannerLeft = Advertisement::create()) {
                adBannerLeft->setID("banner-left"_spr);
                adBannerLeft->setType(AdType::Skyscraper);
                adBannerLeft->setPosition({ 30.f, winSize.height / 2.f });

                this->addChild(adBannerLeft);

                adBannerLeft->loadRandom();
            };

            // square ad at the right side
            if (auto adBannerRight = Advertisement::create()) {
                adBannerRight->setID("banner-right"_spr);
                adBannerRight->setType(AdType::Skyscraper);
                adBannerRight->setPosition({ winSize.width - 30.f, winSize.height / 2.f });

                this->addChild(adBannerRight);

                adBannerRight->loadRandom();
            };
        };

        return true;
    };
};