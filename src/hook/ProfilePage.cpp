#include <Geode/Geode.hpp>
#include <Geode/modify/ProfilePage.hpp>
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(AdsProfilePage, ProfilePage) {
    bool init(int p0, bool p1) {
        if (!ProfilePage::init(p0, p1)) return false;

        if (Mod::get()->getSettingValue<bool>("ProfilePage")) {
            auto const winSize = CCDirector::sharedDirector()->getWinSize();

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

        return true;
    };
};