#include <Geode/Geode.hpp>
#include <Geode/modify/EndLevelLayer.hpp>
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(AdsEndLevelLayer, EndLevelLayer) {
    void customSetup() override {
        EndLevelLayer::customSetup();

        auto const winSize = CCDirector::sharedDirector()->getWinSize();

        if (Mod::get()->getSettingValue<bool>("EndLevelLayer")) {
            // banner ad at the top
            if (auto adBanner = Advertisement::create(AdType::Banner)) {
                adBanner->setID("banner"_spr);
                adBanner->setPosition({ winSize.width / 2.f, winSize.height - 30.f });

                m_mainLayer->addChild(adBanner);

                adBanner->loadRandom();
            };

            // skyscraper ad on the right side
            if (auto adSkyscraperRight = Advertisement::create(AdType::Skyscraper)) {
                adSkyscraperRight->setID("skyscraper-right"_spr);
                adSkyscraperRight->setPosition({ winSize.width - 30.f, winSize.height / 2.f });

                m_mainLayer->addChild(adSkyscraperRight);

                adSkyscraperRight->loadRandom();
            };

            // skyscraper ad on the left side
            if (auto adSkyscraperLeft = Advertisement::create(AdType::Skyscraper)) {
                adSkyscraperLeft->setID("skyscraper-left"_spr);
                adSkyscraperLeft->setPosition({ 30.f, winSize.height / 2.f });

                m_mainLayer->addChild(adSkyscraperLeft);

                adSkyscraperLeft->loadRandom();
            };
        };
    };
};