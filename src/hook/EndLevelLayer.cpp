#include <Geode/Geode.hpp>
#include <Geode/modify/EndLevelLayer.hpp>
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(EndLevelLayer) {
    void customSetup() override {
        EndLevelLayer::customSetup();

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        if (Mod::get()->getSettingValue<bool>("EndLevelLayer")) {
            // banner ad at the top
            auto adBanner = Advertisement::create();
            if (adBanner) {
                adBanner->setID("advertisement-menu");
                m_mainLayer->addChild(adBanner);
                adBanner->setType(AdType::Banner);
                adBanner->setPosition({ winSize.width / 2.f, winSize.height - 30.f });
                adBanner->loadRandom();
            }

            // skyscraper ad on the right side
            auto adSkyscraper = Advertisement::create();
            if (adSkyscraper) {
                adSkyscraper->setID("advertisement-menu-skyscraper");
                m_mainLayer->addChild(adSkyscraper);
                adSkyscraper->setType(AdType::Skyscraper);
                adSkyscraper->setPosition({ winSize.width - 30.f, winSize.height / 2.f });
                adSkyscraper->loadRandom();
            }
            // skyscraper ad on the left side
            auto adSkyscraperLeft = Advertisement::create();
            if (adSkyscraperLeft) {
                adSkyscraperLeft->setID("advertisement-menu-skyscraper-left");
                m_mainLayer->addChild(adSkyscraperLeft);
                adSkyscraperLeft->setType(AdType::Skyscraper);
                adSkyscraperLeft->setPosition({ 30.f, winSize.height / 2.f });
                adSkyscraperLeft->loadRandom();
            }
        }
    }
};