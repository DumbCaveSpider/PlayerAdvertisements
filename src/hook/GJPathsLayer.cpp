#include <Geode/Geode.hpp>
#include <Geode/modify/GJPathsLayer.hpp>
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(GJPathsLayer) {
    bool init() {
        if (!GJPathsLayer::init())
            return false;
        if (Mod::get()->getSettingValue<bool>("GJPathsLayer"))
        {
        auto winSize = CCDirector::sharedDirector()->getWinSize();

        // square ad at the left side
        auto adBanner = Advertisement::create();
        if (adBanner) {
            adBanner->setID("advertisement-menu");
            this->addChild(adBanner);
            adBanner->setType(AdType::Skyscraper);
            adBanner->setPosition({ 30.f, winSize.height / 2.f });
            adBanner->loadRandom();
        }

        // square ad at the right side
        auto adBannerRight = Advertisement::create();
        if (adBannerRight) {
            adBannerRight->setID("advertisement-menu-right");
            this->addChild(adBannerRight);
            adBannerRight->setType(AdType::Skyscraper);
            adBannerRight->setPosition({ winSize.width - 30.f, winSize.height / 2.f });
            adBannerRight->loadRandom();
        }
    }
        return true;
    }
};