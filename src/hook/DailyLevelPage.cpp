#include <Geode/Geode.hpp>
#include <Geode/modify/DailyLevelPage.hpp>
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(DailyLevelPage) {
    bool init(GJTimedLevelType levelType) {
        if (!DailyLevelPage::init(levelType))
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