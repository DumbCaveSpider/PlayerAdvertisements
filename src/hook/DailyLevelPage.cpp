#include <Geode/Geode.hpp>
#include <Geode/modify/DailyLevelPage.hpp>
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(AdsDailyLevelPage, DailyLevelPage) {
    bool init(GJTimedLevelType levelType) {
        if (!DailyLevelPage::init(levelType)) return false;

        if (Mod::get()->getSettingValue<bool>("DailyLevelPage")) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();

            // banner ad at the top
            if (auto adBanner = Advertisement::create()) {
                adBanner->setID("banner"_spr);
                adBanner->setType(AdType::Banner);
                adBanner->setPosition({ winSize.width / 2.f, 70.f });

                m_mainLayer->addChild(adBanner, 8);

                adBanner->loadRandom();
            };
        };

        return true;
    };
};