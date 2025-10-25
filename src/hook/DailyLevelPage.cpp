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
            m_mainLayer->addChild(adBanner, 8);
            adBanner->setType(AdType::Banner);
            adBanner->setPosition({ winSize.width / 2.f, 70.f });
            adBanner->loadRandom();
        }
        return true;
    }
};