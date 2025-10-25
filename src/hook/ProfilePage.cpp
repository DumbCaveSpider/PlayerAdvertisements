#include <Geode/Geode.hpp>
#include <Geode/modify/ProfilePage.hpp>
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(ProfilePage)
{
    bool init(int p0, bool p1)
    {
        if (!ProfilePage::init(p0, p1))
            return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        // skyscraper ad on the right side
        auto adSkyscraper = Advertisement::create();
        if (adSkyscraper)
        {
            adSkyscraper->setID("advertisement-menu-skyscraper");
            m_mainLayer->addChild(adSkyscraper);
            adSkyscraper->setType(AdType::Skyscraper);
            adSkyscraper->setPosition({winSize.width - 30.f, winSize.height / 2.f});
            adSkyscraper->loadRandom();
        }
        // skyscraper ad on the left side
        auto adSkyscraperLeft = Advertisement::create();
        if (adSkyscraperLeft)
        {
            adSkyscraperLeft->setID("advertisement-menu-skyscraper-left");
            m_mainLayer->addChild(adSkyscraperLeft);
            adSkyscraperLeft->setType(AdType::Skyscraper);
            adSkyscraperLeft->setPosition({30.f, winSize.height / 2.f});
            adSkyscraperLeft->loadRandom();
        }

        return true;
    }
};