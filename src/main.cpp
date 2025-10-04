#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include "AdManager.hpp"

using namespace geode::prelude;

class $modify(MyPauseLayer, PauseLayer)
{

    void customSetup()
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
        CCMenu *adMenu = nullptr;
        if (auto existing = this->getChildByID("ad-menu"))
        {
            adMenu = typeinfo_cast<CCMenu *>(existing);
        }
        if (!adMenu)
        {
            adMenu = CCMenu::create();
            adMenu->setID("ad-menu");
            adMenu->setPosition({0.f, 0.f});
            this->addChild(adMenu);
        }
        // add funny banner
        auto adBanner = LazySprite::create({1456.f, 180.f}, true);
        auto tempBanner = CCSprite::create("bannerTemp.png"_spr);
        tempBanner->setVisible(false);
        auto adButton = CCMenuItemSpriteExtra::create(tempBanner, this, menu_selector(MyPauseLayer::onAdClicked));
        adBanner->loadFromUrl("https://john.citrons.xyz/static/img/merrybot-2.png"); // insert like a api url that serves ads here

        if (levelName)
        {
            adButton->setPosition({levelName->getPositionX(), levelName->getPositionY()});
        }

        // banner to the menu
        adMenu->addChild(adBanner);
        adMenu->addChild(adButton);

        // child the real ad
        adBanner->setPosition(adButton->getPosition());

        // add a button on the side on the menu
        auto rightButtonMenu = getChildByID("right-button-menu");
        if (rightButtonMenu)
        {
            auto adButton = CircleButtonSprite::createWithSpriteFrameName(
                "GJ_freeStuffBtn_001.png",
                0.875f,
                CircleBaseColor::Green,
                CircleBaseSize::Medium);

            auto popupButton = CCMenuItemSpriteExtra::create(
                adButton,
                this,
                menu_selector(MyPauseLayer::onAdClicked));

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