#include <Geode/Geode.hpp>
#include <Geode/modify/ChallengesPage.hpp>
#include "AdManager.hpp"
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(ChallengesPage) {
    bool init() {
        if (!ChallengesPage::init())
            return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        // banner ad at the top
        auto adBanner = Advertisement::create();
        if (adBanner) {
            adBanner->setID("advertisement-menu");
            this->addChild(adBanner);
            adBanner->setType(AdType::Banner);
            adBanner->setPosition({ winSize.width / 2.f, winSize.height - 30.f });
            adBanner->loadRandom();
        }
        return true;
    }
};