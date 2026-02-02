#include <Geode/Geode.hpp>
#include <Advertisements.hpp>
#include <Geode/modify/LeaderboardsLayer.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(AdsLeaderboardsLayer, LeaderboardsLayer) {
    bool init(LeaderboardType type, LeaderboardStat stat) {
        if (!LeaderboardsLayer::init(type, stat)) return false;

        if (Mod::get()->getSettingValue<bool>("LeaderboardsLayer")) {
            auto const winSize = CCDirector::sharedDirector()->getWinSize();

            // banner at the bottom center
            if (auto adBanner = Advertisement::create()) {
                adBanner->setID("advertisement-leaderboards-bottom");
                adBanner->setType(AdType::Banner);
                adBanner->setPosition({ winSize.width / 2.f, 30.f });

                this->addChild(adBanner, 1);

                adBanner->loadRandom();
            };
        };

        return true;
    };
};