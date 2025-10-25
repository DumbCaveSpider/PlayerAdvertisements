#include <Geode/Geode.hpp>
#include <Geode/modify/GauntletLayer.hpp>
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(GauntletLayer) {
    bool init(GauntletType p0) {
        if (!GauntletLayer::init(p0))
            return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        // skyscraper ad on the right side
        auto adSkyscraper = Advertisement::create();
        if (adSkyscraper) {
            adSkyscraper->setID("advertisement-menu-skyscraper");
            this->addChild(adSkyscraper);
            adSkyscraper->setType(AdType::Skyscraper);
            adSkyscraper->setPosition({winSize.width - 30.f, winSize.height / 2.f});
            adSkyscraper->loadRandom();
        }
        // skyscraper ad on the left side
        auto adSkyscraperLeft = Advertisement::create();
        if (adSkyscraperLeft) {
            adSkyscraperLeft->setID("advertisement-menu-skyscraper-left");
            this->addChild(adSkyscraperLeft);
            adSkyscraperLeft->setType(AdType::Skyscraper);
            adSkyscraperLeft->setPosition({30.f, winSize.height / 2.f});
            adSkyscraperLeft->loadRandom();
        }

        return true;
    }
};