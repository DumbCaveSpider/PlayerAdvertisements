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

        if (Mod::get()->getSettingValue<bool>("ChallengesPage")) {

            auto winSize = CCDirector::sharedDirector()->getWinSize();

            // banner ad at the top
            auto adBanner = Advertisement::create();
            if (adBanner) {
                adBanner->setID("advertisement-menu");
                m_mainLayer->addChild(adBanner, 20);
                adBanner->setType(AdType::Banner);
                adBanner->setPosition({ winSize.width / 2.f, 30.f });
                adBanner->loadRandom();
            }
        }
        return true;
    }
};