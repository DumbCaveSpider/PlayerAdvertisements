#include <Geode/Geode.hpp>
#include <Geode/modify/EndLevelLayer.hpp>
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(AdsEndLevelLayer, EndLevelLayer) {
    void customSetup() override {
        EndLevelLayer::customSetup();

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        if (Mod::get()->getSettingValue<bool>("EndLevelLayer")) {
            // banner ad at the top
            if (auto adBanner = Advertisement::create()) {
                adBanner->setID("banner"_spr);
                adBanner->setType(AdType::Banner);
                adBanner->setPosition({ winSize.width / 2.f, winSize.height - 30.f });

                m_mainLayer->addChild(adBanner);

                adBanner->loadRandom();
            };

            // skyscraper ad on the right side
            if (auto adSkyscraperRight = Advertisement::create()) {
                adSkyscraperRight->setID("skyscraper-right"_spr);
                adSkyscraperRight->setType(AdType::Skyscraper);
                adSkyscraperRight->setPosition({ winSize.width - 30.f, winSize.height / 2.f });

                m_mainLayer->addChild(adSkyscraperRight);

                adSkyscraperRight->loadRandom();
            };

            // skyscraper ad on the left side
            if (auto adSkyscraperLeft = Advertisement::create()) {
                adSkyscraperLeft->setID("skyscraper-left"_spr);
                adSkyscraperLeft->setType(AdType::Skyscraper);
                adSkyscraperLeft->setPosition({ 30.f, winSize.height / 2.f });

                m_mainLayer->addChild(adSkyscraperLeft);

                adSkyscraperLeft->loadRandom();
            };
        };
    };
};