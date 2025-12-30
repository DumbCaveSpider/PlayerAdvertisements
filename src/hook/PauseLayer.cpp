#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include "AdManager.hpp"
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(AdsPauseLayer, PauseLayer) {
    void customSetup() override {
        PauseLayer::customSetup();

        if (Mod::get()->getSettingValue<bool>("PauseLayer")) {
            // get level name label
            auto levelName = static_cast<CCLabelBMFont*>(getChildByID("level-name"));
            levelName->setVisible(false);

            // get the practice mode positions

            auto practiceTitle = static_cast<CCLabelBMFont*>(getChildByID("practice-mode-label"));
            auto practiceProgress = static_cast<CCLabelBMFont*>(getChildByID("practice-progress-label"));
            auto practiceBar = static_cast<CCSprite*>(getChildByID("practice-progress-bar"));

            // move the normal mode title and program bar down
            auto normalTitle = static_cast<CCLabelBMFont*>(getChildByID("normal-mode-label"));
            auto normalProgress = static_cast<CCLabelBMFont*>(getChildByID("normal-progress-label"));
            auto normalBar = static_cast<CCSprite*>(getChildByID("normal-progress-bar"));

            // position the y values
            if (practiceTitle && normalTitle)
                normalTitle->setPositionY(practiceTitle->getPositionY());
            if (practiceProgress && normalProgress)
                normalProgress->setPositionY(practiceProgress->getPositionY());
            if (practiceBar && normalBar)
                normalBar->setPositionY(practiceBar->getPositionY());

            // make all practice mode invisible by default
            if (practiceTitle) practiceTitle->setVisible(false);
            if (practiceProgress) practiceProgress->setVisible(false);
            if (practiceBar) practiceBar->setVisible(false);

            // player is practice mode, show practice mode elements
            if (GJBaseGameLayer::get()->m_isPracticeMode) {
                // set all practice mode elements to visible
                if (practiceTitle) practiceTitle->setVisible(true);
                if (practiceProgress) practiceProgress->setVisible(true);
                if (practiceBar) practiceBar->setVisible(true);

                // hide normal mode elements
                if (normalTitle) normalTitle->setVisible(false);
                if (normalProgress) normalProgress->setVisible(false);
                if (normalBar) normalBar->setVisible(false);
            };

            auto const winSize = CCDirector::get()->getWinSize();

            // insert ad banner (722x84)
            if (auto adBanner = Advertisement::create()) {
                adBanner->setID("banner"_spr);
                adBanner->setType(AdType::Banner);
                adBanner->setPosition({ winSize.width / 2.f, winSize.height - 50.f });

                this->addChild(adBanner, 100);

                adBanner->loadRandom();
            };
        };

        // im confused
        // add a button on the side on the menu
        if (auto rightButtonMenu = getChildByID("right-button-menu")) {
            auto adButton = CircleButtonSprite::create(
                CCSprite::create("adIcon.png"_spr),
                CircleBaseColor::Green,
                CircleBaseSize::Medium);

            auto popupButton = CCMenuItemSpriteExtra::create(
                adButton,
                this,
                menu_selector(AdsPauseLayer::onAdClicked));

            if (auto menu = typeinfo_cast<CCMenu*>(rightButtonMenu)) {
                menu->addChild(popupButton);
                menu->updateLayout();
            };
        };
    };

    void onAdClicked(CCObject * sender) {
        if (auto popup = AdManager::create()) popup->show();
    };
};