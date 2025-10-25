#include <Geode/Geode.hpp>
#include <Geode/modify/GJDropDownLayer.hpp>
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(GJDropDownLayer) {
    bool init(const char* p0, float p1, bool p2) {
        if (!GJDropDownLayer::init(p0, p1, p2))
            return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        // banner ad at the top
        auto adBanner = Advertisement::create();
        if (adBanner) {
            adBanner->setID("advertisement-menu");
            this->addChild(adBanner, 2);
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

        return true;
    }
};