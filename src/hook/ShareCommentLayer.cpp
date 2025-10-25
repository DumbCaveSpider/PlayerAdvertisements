#include <Geode/Geode.hpp>
#include <Geode/modify/ShareCommentLayer.hpp>
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(ShareCommentLayer)
{
    bool init(gd::string title, int charLimit, CommentType type, int ID, gd::string desc)
    {
        if (!ShareCommentLayer::init(title, charLimit, type, ID, desc))
            return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        // square ad left
        auto adSquareLeft = Advertisement::create();
        if (adSquareLeft)
        {
            adSquareLeft->setID("advertisement-menu");
            m_mainLayer->addChild(adSquareLeft);
            adSquareLeft->setType(AdType::Square);
            adSquareLeft->setPosition({winSize.width / 2.f - 140.f, winSize.height / 2.f - 70.f});
            adSquareLeft->loadRandom();
        }

        // square ad center
        auto adSquareCenter = Advertisement::create();
        if (adSquareCenter)
        {
            adSquareCenter->setID("advertisement-menu");
            m_mainLayer->addChild(adSquareCenter);
            adSquareCenter->setType(AdType::Square);
            adSquareCenter->setPosition({winSize.width / 2.f, winSize.height / 2.f - 70.f});
            adSquareCenter->loadRandom();
        }

        // square ad right
        auto adSquareRight = Advertisement::create();
        if (adSquareRight)
        {
            adSquareRight->setID("advertisement-menu");
            m_mainLayer->addChild(adSquareRight);
            adSquareRight->setType(AdType::Square);
            adSquareRight->setPosition({winSize.width / 2.f + 140.f, winSize.height / 2.f - 70.f});
            adSquareRight->loadRandom();
        }

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