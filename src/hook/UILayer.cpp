#include <Geode/Geode.hpp>
#include <Geode/modify/UILayer.hpp>
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(UILayer) {
    bool init(GJBaseGameLayer * p0) {
        if (!UILayer::init(p0))
            return false;

        // dont show ads ingame if the setting is false
        if (Mod::get()->getSettingValue<bool>("show-in-game")) {

            log::info("showing ads ingame");

            auto winSize = CCDirector::sharedDirector()->getWinSize();

            // banner ad at the top
            auto adBanner = Advertisement::create();
            if (adBanner) {
                adBanner->setID("advertisement-menu");
                this->addChild(adBanner, -5);
                adBanner->setType(AdType::Banner);
                adBanner->setPosition({ winSize.width / 2.f, winSize.height - 30.f });
                adBanner->loadRandom();
            }

            // banner at the bottom center
            auto adBannerBottom = Advertisement::create();
            if (adBannerBottom) {
                adBannerBottom->setID("advertisement-leaderboards-bottom");
                this->addChild(adBannerBottom, -5);
                adBannerBottom->setType(AdType::Banner);
                adBannerBottom->setPosition({ winSize.width / 2.f, 30.f });
                adBannerBottom->loadRandom();
            }

            // skyscraper ad on the right side
            auto adSkyscraper = Advertisement::create();
            if (adSkyscraper) {
                adSkyscraper->setID("advertisement-menu-skyscraper");
                this->addChild(adSkyscraper, -5);
                adSkyscraper->setType(AdType::Skyscraper);
                adSkyscraper->setPosition({ winSize.width - 30.f, winSize.height / 2.f });
                adSkyscraper->loadRandom();
            }
            // skyscraper ad on the left side
            auto adSkyscraperLeft = Advertisement::create();
            if (adSkyscraperLeft) {
                adSkyscraperLeft->setID("advertisement-menu-skyscraper-left");
                this->addChild(adSkyscraperLeft, -5);
                adSkyscraperLeft->setType(AdType::Skyscraper);
                adSkyscraperLeft->setPosition({ 30.f, winSize.height / 2.f });
                adSkyscraperLeft->loadRandom();
            }
        }

        return true;
    }
};