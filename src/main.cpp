#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include "AdManager.hpp"
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(MyPauseLayer, PauseLayer) {
    void customSetup() override {
        PauseLayer::customSetup();

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
        if (practiceTitle)
            practiceTitle->setVisible(false);
        if (practiceProgress)
            practiceProgress->setVisible(false);
        if (practiceBar)
            practiceBar->setVisible(false);

        // player is practice mode, show practice mode elements
        if (GJBaseGameLayer::get()->m_isPracticeMode) {
            // set all practice mode elements to visible
            if (practiceTitle)
                practiceTitle->setVisible(true);
            if (practiceProgress)
                practiceProgress->setVisible(true);
            if (practiceBar)
                practiceBar->setVisible(true);
            // hide normal mode elements
            if (normalTitle)
                normalTitle->setVisible(false);
            if (normalProgress)
                normalProgress->setVisible(false);
            if (normalBar)
                normalBar->setVisible(false);
        }
        // insert ad banner (722x84)
        CCMenu* adMenu = nullptr;
        if (auto existing = this->getChildByID("ad-menu")) {
            adMenu = typeinfo_cast<CCMenu*>(existing);
        }
        if (!adMenu) {
            adMenu = CCMenu::create();
            adMenu->setID("ad-menu");
            adMenu->setPosition({ 0.f, 0.f });
            this->addChild(adMenu);
        }

        auto adManager = Advertisements::create();

        // fetch a random ad and load its image into the banner
        adManager->getRandomAd(Banner, [&](Ad ad) {
            log::debug("Fetching ad id {} from url: {}", ad.id, ad.image);

            if (LazySprite* sprite = adManager->loadAdImage(ad)) {
                sprite->setPosition({ 0, 0 });
                log::debug("Loaded ad from url: {}", ad.image);

                auto adButton = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(MyPauseLayer::onAdClicked));
                adButton->setPosition(levelName->getPosition());
                adMenu->addChild(adButton);
                sprite->setPosition(adButton->getPosition());
            } else {
                log::error("Failed to create LazySprite for ad id {}", ad.id);
            };
                               });

        // im confused
        // add a button on the side on the menu
        auto rightButtonMenu = getChildByID("right-button-menu");
        if (rightButtonMenu) {
            auto adButton = CircleButtonSprite::createWithSpriteFrameName(
                "GJ_freeStuffBtn_001.png",
                0.875f,
                CircleBaseColor::Green,
                CircleBaseSize::Medium);

            auto popupButton = CCMenuItemSpriteExtra::create(
                adButton,
                this,
                menu_selector(MyPauseLayer::onAdClicked));

            if (auto menu = typeinfo_cast<CCMenu*>(rightButtonMenu)) {
                menu->addChild(popupButton);
                menu->updateLayout();
            }
        }
    }

    void onAdClicked(CCObject * sender) {
        if (auto popup = AdManager::create())
            popup->show();
    }
};