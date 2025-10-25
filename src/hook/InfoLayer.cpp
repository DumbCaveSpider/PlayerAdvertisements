#include <Geode/Geode.hpp>
#include <Geode/modify/InfoLayer.hpp>
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(InfoLayer)
{
    bool init(GJGameLevel *p0, GJUserScore *p1, GJLevelList *p2)
    {
        if (!InfoLayer::init(p0, p1, p2))
            return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        // skyscraper ad on the right side
        auto adSkyscraper = Advertisement::create();
        if (adSkyscraper)
        {
            adSkyscraper->setID("advertisement-menu-skyscraper");
            m_mainLayer->addChild(adSkyscraper);
            adSkyscraper->setType(AdType::Skyscraper);
            adSkyscraper->setPosition({winSize.width - 50.f, 2.f});
            adSkyscraper->loadRandom();
        }
        // skyscraper ad on the left side
        auto adSkyscraperLeft = Advertisement::create();
        if (adSkyscraperLeft)
        {
            adSkyscraperLeft->setID("advertisement-menu-skyscraper-left");
            m_mainLayer->addChild(adSkyscraperLeft);
            adSkyscraperLeft->setType(AdType::Skyscraper);
            adSkyscraperLeft->setPosition({10.f, 2.f});
            adSkyscraperLeft->loadRandom();
        }

        return true;
    }
};