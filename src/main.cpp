#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include "AdManager.hpp"
#include <Advertisements.hpp>
#include <argon/argon.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(AdsMenuLayer, MenuLayer)
{
    bool init()
    {
        if (!MenuLayer::init())
            return false;

        // get argon token yum
        auto res = argon::startAuth([](Result<std::string> res)
                                    {
        if (!res) {
            log::warn("Auth failed: {}", res.unwrapErr());
            Notification::create("Failed to authenticate with Argon.", NotificationIcon::Error)
                ->show();
            return;
        }

        auto token = std::move(res).unwrap();
        Mod::get()->setSavedValue<std::string>("argon_token", token);
        log::debug("Token: {}", token); }, [](argon::AuthProgress progress)
                                    { log::info("Auth progress: {}", argon::authProgressToString(progress)); });

        if (!res)
        {
            log::warn("Failed to start auth attempt: {}", res.unwrapErr());
            Notification::create("Failed to start argon auth. Your views/clicks won't be tracked.", NotificationIcon::Error)
                ->show();
        }

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        if (Mod::get()->getSettingValue<bool>("MenuLayer"))
        {
            // banner ad at the center
            auto adBanner = Advertisement::create();
            if (adBanner)
            {
                adBanner->setID("advertisement-menu");
                this->addChild(adBanner);
                adBanner->setType(AdType::Banner);
                adBanner->setPosition({winSize.width / 2.f, winSize.height / 2.f - 70.f});
                adBanner->loadRandom();
            }
        }
        // ad button in the bottom menu
        if (auto bottomMenu = this->getChildByID("bottom-menu"))
        {
            auto sprite = CCSprite::create("adIcon.png"_spr);
            auto adButton = CircleButtonSprite::create(
                sprite,
                CircleBaseColor::Green,
                CircleBaseSize::MediumAlt);

            auto popupButton = CCMenuItemSpriteExtra::create(
                adButton,
                this,
                menu_selector(AdsMenuLayer::onAdClicked));

            if (auto menu = typeinfo_cast<CCMenu *>(bottomMenu))
            {
                menu->addChild(popupButton);
                menu->updateLayout();
            }
        }

        return true;
    };

    void onAdClicked(CCObject *sender)
    {
        if (auto popup = AdManager::create())
            popup->show();
    }
};