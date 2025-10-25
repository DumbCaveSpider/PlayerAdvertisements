#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include "AdManager.hpp"
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(AdsPauseLayer, PauseLayer)
{
    void customSetup() override
    {
        PauseLayer::customSetup();

        // get level name label
        auto levelName = static_cast<CCLabelBMFont *>(getChildByID("level-name"));
        levelName->setVisible(false);

        // get the practice mode positions

        auto practiceTitle = static_cast<CCLabelBMFont *>(getChildByID("practice-mode-label"));
        auto practiceProgress = static_cast<CCLabelBMFont *>(getChildByID("practice-progress-label"));
        auto practiceBar = static_cast<CCSprite *>(getChildByID("practice-progress-bar"));

        // move the normal mode title and program bar down
        auto normalTitle = static_cast<CCLabelBMFont *>(getChildByID("normal-mode-label"));
        auto normalProgress = static_cast<CCLabelBMFont *>(getChildByID("normal-progress-label"));
        auto normalBar = static_cast<CCSprite *>(getChildByID("normal-progress-bar"));

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
        if (GJBaseGameLayer::get()->m_isPracticeMode)
        {
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
        auto adBanner = Advertisement::create();
        auto winSize = CCDirector::get()->getWinSize();
        if (adBanner)
        {
            adBanner->setID("advertisement-menu");
            this->addChild(adBanner, 100);
            adBanner->setType(AdType::Banner);
            adBanner->setPosition({winSize.width / 2.f, winSize.height - 50.f});
            adBanner->loadRandom();
        }

        // im confused
        // add a button on the side on the menu
        auto rightButtonMenu = getChildByID("right-button-menu");
        auto sprite = CCSprite::create("adIcon.png"_spr);
        if (rightButtonMenu)
        {
            auto adButton = CircleButtonSprite::create(
                sprite,
                CircleBaseColor::Green,
                CircleBaseSize::Medium);

            auto popupButton = CCMenuItemSpriteExtra::create(
                adButton,
                this,
                menu_selector(AdsPauseLayer::onAdClicked));

            if (auto menu = typeinfo_cast<CCMenu *>(rightButtonMenu))
            {
                menu->addChild(popupButton);
                menu->updateLayout();
            }
        }
    }

    void onAdClicked(CCObject *sender)
    {
        if (auto popup = AdManager::create())
            popup->show();
    }
};