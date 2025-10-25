#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include "AdManager.hpp"
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(AdsMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init())
            return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        // banner ad at the center
        auto adBanner = Advertisement::create();
        if (adBanner) {
            adBanner->setID("advertisement-menu");
            this->addChild(adBanner);
            adBanner->setType(AdType::Banner);
            adBanner->setPosition({ winSize.width / 2.f, winSize.height / 2.f - 70.f });
            adBanner->loadRandom();
        }

        // ad button in the bottom menu
        if (auto bottomMenu = this->getChildByID("bottom-menu")) {
            auto sprite = CCSprite::create("adIcon.png"_spr);
            auto adButton = CircleButtonSprite::create(
                sprite,
                CircleBaseColor::Green,
                CircleBaseSize::MediumAlt);

            auto popupButton = CCMenuItemSpriteExtra::create(
                adButton,
                this,
                menu_selector(AdsMenuLayer::onAdClicked));

            if (auto menu = typeinfo_cast<CCMenu*>(bottomMenu)) {
                menu->addChild(popupButton);
                menu->updateLayout();
            }
        }

        return true;
    };

    void onAdClicked(CCObject * sender) {
        if (auto popup = AdManager::create())
            popup->show();
    }
};