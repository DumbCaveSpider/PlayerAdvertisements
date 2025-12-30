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
            if (auto adBanner = Advertisement::create()) {
                adBanner->setID("banner"_spr);
                adBanner->setType(AdType::Banner);
                adBanner->setPosition({ winSize.width / 2.f, winSize.height - 30.f });

                m_mainLayer->addChild(adBanner, 2);

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
            // check if hide-dropdown-menu exists, then dont show the left ad
            if (!m_mainLayer->getChildByID("hide-dropdown-menu")) {
                if (auto adSkyscraperLeft = Advertisement::create()) {
                    adSkyscraperLeft->setID("advertisement-menu-skyscraper-left");
                    adSkyscraperLeft->setType(AdType::Skyscraper);
                    adSkyscraperLeft->setPosition({ 30.f, winSize.height / 2.f });

                    m_mainLayer->addChild(adSkyscraperLeft);

                    adSkyscraperLeft->loadRandom();
                };
            };
        };

        return true;
    };
};