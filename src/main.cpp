#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include "AdManager.hpp"
#include <Advertisements.hpp>
#include <argon/argon.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(AdsMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        // // get argon token yum
        // auto res = argon::startAuth([](Result<std::string> res) {
        //     if (!res) {
        //         log::warn("Auth failed: {}", res.unwrapErr());
        //         Notification::create("Failed to authenticate with Argon", NotificationIcon::Error)
        //             ->show();
        //         return;
        //     };

        //     auto token = std::move(res).unwrap();
        //     Mod::get()->setSavedValue<std::string>("argon_token", token);
        //     log::debug("Token: {}", token); }, [](argon::AuthProgress progress) { log::debug("Auth progress: {}", argon::authProgressToString(progress)); });

        // if (!res) {
        //     log::warn("Failed to start auth attempt: {}", res.unwrapErr());
        //     Notification::create("Failed to start argon auth", NotificationIcon::Error)
        //         ->show();
        // };

        // argon but with async spawn
        async::spawn(
            argon::startAuth(),
            [this](geode::Result<std::string> res) {
                if (res.isOk()) {
                    auto token = std::move(res).unwrap();
                    Mod::get()->setSavedValue<std::string>("argon_token", token);
                    log::debug("Token: {}", token);
                } else {
                    log::warn("Auth failed: {}", res.unwrapErr());
                    Notification::create("Failed to authenticate with Argon", NotificationIcon::Error)
                        ->show();
                }
            }
        );

        auto const winSize = CCDirector::sharedDirector()->getWinSize();
        if (Mod::get()->getSettingValue<bool>("MenuLayer")) {
            // banner ad at the center
            if (auto adBanner = Advertisement::create()) {
                adBanner->setID("banner"_spr);
                adBanner->setType(AdType::Banner);
                adBanner->setPosition({ winSize.width / 2.f, winSize.height / 2.f - 70.f });

                this->addChild(adBanner);

                adBanner->loadRandom();
            };
        };

        // ad button in the bottom menu
        if (auto bottomMenu = this->getChildByID("bottom-menu")) {
            auto adButton = CircleButtonSprite::create(
                CCSprite::create("adIcon.png"_spr),
                CircleBaseColor::Green,
                CircleBaseSize::MediumAlt
            );

            auto popupButton = CCMenuItemSpriteExtra::create(
                adButton,
                this,
                menu_selector(AdsMenuLayer::onAdClicked));

            if (auto menu = typeinfo_cast<CCMenu*>(bottomMenu)) {
                menu->addChild(popupButton);
                menu->updateLayout();
            };
        };

        return true;
    };

    void onAdClicked(CCObject * sender) {
        if (auto popup = AdManager::create()) popup->show();
    };
};