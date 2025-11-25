#include <Geode/Geode.hpp>
#include <Advertisements.hpp>
#include <Geode/modify/LeaderboardsLayer.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(LeaderboardsLayer) {
    bool init(LeaderboardState layer) {
        if (!LeaderboardsLayer::init(layer))
            return false;
        if (Mod::get()->getSettingValue<bool>("LeaderboardsLayer")) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();

            // banner at the bottom center
            auto adBanner = Advertisement::create();
            if (adBanner) {
                adBanner->setID("advertisement-leaderboards-bottom");
                this->addChild(adBanner, 1);
                adBanner->setType(AdType::Banner);
                adBanner->setPosition({ winSize.width / 2.f, 30.f });
                adBanner->loadRandom();
            }
        }
        return true;
    }
};