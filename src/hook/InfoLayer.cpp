#include <Geode/Geode.hpp>
#include <Geode/modify/InfoLayer.hpp>
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(InfoLayer) {
    bool init(GJGameLevel * p0, GJUserScore * p1, GJLevelList * p2) {
        if (!InfoLayer::init(p0, p1, p2)) return false;

        if (Mod::get()->getSettingValue<bool>("InfoLayer")) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();

            // skyscraper ad on the right side
            if (auto adSkyscraper = Advertisement::create()) {
                adSkyscraper->setID("skyscraper-right"_spr);
                m_mainLayer->addChild(adSkyscraper);
                adSkyscraper->setType(AdType::Skyscraper);
                adSkyscraper->setPosition({ winSize.width - 30.f, winSize.height / 2.f });
                adSkyscraper->loadRandom();
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

        return true;
    };
};