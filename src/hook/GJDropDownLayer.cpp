#include <Advertisements.hpp>
#include <Geode/Geode.hpp>
#include <Geode/modify/GJDropDownLayer.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(AdsGJDropDownLayer, GJDropDownLayer) {
    bool init(const char* p0, float p1, bool p2) {
        if (!GJDropDownLayer::init(p0, p1, p2)) return false;

        if (Mod::get()->getSettingValue<bool>("GJDropDownLayer")) {
            auto const winSize = CCDirector::sharedDirector()->getWinSize();

            // banner ad at the top
            if (auto adBanner = Advertisement::create(AdType::Banner)) {
                adBanner->setID("banner"_spr);
                adBanner->setPosition({ winSize.width / 2.f, winSize.height - 30.f });

                m_mainLayer->addChild(adBanner, 2);

                adBanner->loadRandom();
            };

            // check if hide-dropdown-menu exists, then dont show the left and right ad
            if (!m_mainLayer->getChildByID("hide-dropdown-menu")) {
                // skyscraper ad on the left side
                if (auto adSkyscraperLeft = Advertisement::create(AdType::Skyscraper)) {
                    adSkyscraperLeft->setID("advertisement-menu-skyscraper-left");
                    adSkyscraperLeft->setPosition({ 30.f, winSize.height / 2.f });

                    m_mainLayer->addChild(adSkyscraperLeft);

                    adSkyscraperLeft->loadRandom();
                };

                // skyscraper ad on the right side
                if (auto adSkyscraperRight = Advertisement::create(AdType::Skyscraper)) {
                    adSkyscraperRight->setID("skyscraper-right"_spr);
                    adSkyscraperRight->setPosition({ winSize.width - 30.f, winSize.height / 2.f });

                    m_mainLayer->addChild(adSkyscraperRight);

                    adSkyscraperRight->loadRandom();
                };
            };
        };

        return true;
    };
};