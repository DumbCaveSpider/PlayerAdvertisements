#include <Geode/Geode.hpp>
#include <Geode/modify/ChallengesPage.hpp>
#include "AdManager.hpp"
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(AdsChallengesPage, ChallengesPage) {
    bool init() {
        if (!ChallengesPage::init()) return false;

        if (Mod::get()->getSettingValue<bool>("ChallengesPage")) {
            auto const winSize = CCDirector::sharedDirector()->getWinSize();

            // banner ad at the top
            if (auto adBanner = Advertisement::create()) {
                adBanner->setID("advertisement-menu");
                adBanner->setType(AdType::Banner);
                adBanner->setPosition({ winSize.width / 2.f, 30.f });

                m_mainLayer->addChild(adBanner, 20);

                adBanner->loadRandom();
            };
        };

        return true;
    };
};