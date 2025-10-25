#include <Geode/Geode.hpp>
#include <Advertisements.hpp>
#include <Geode/modify/LeaderboardsLayer.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(MyLeaderboardsLayer, LeaderboardsLayer)
{
    bool init(LeaderboardState layer)
    {
        if (!LeaderboardsLayer::init(layer))
            return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        // banner at the bottom center
        auto adBanner = Advertisement::create();
        if (adBanner)
        {
            adBanner->setID("advertisement-leaderboards-bottom");
            this->addChild(adBanner);
            adBanner->setType(AdType::Banner);
            adBanner->setPosition({winSize.width / 2.f, 10.f});
            adBanner->loadRandom();
        }
        return true;
    }
};