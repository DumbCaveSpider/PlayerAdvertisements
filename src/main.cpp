#include <PlayerAdsUtils/Include.hpp>

#include "ui/AdManager.hpp"

#include <Geode/Geode.hpp>

#include <Geode/ui/GeodeUI.hpp>
#include <Geode/ui/Button.hpp>

#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(AdsMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        auto const winSize = CCDirector::sharedDirector()->getWinSize();

        if (Mod::get()->getSettingValue<bool>("MenuLayer")) {
            // banner ad at the center
            if (auto adBanner = Advertisement::create()) {
                adBanner->setID("banner"_spr);
                adBanner->setPosition({ winSize.width / 2.f, winSize.height / 2.f - 70.f });

                this->addChild(adBanner);

                adBanner->loadRandom();
            };
        };

        // ad button in the bottom menu
        if (auto bottomMenu = this->getChildByID("bottom-menu")) {
            auto popupButton = Button::createWithNode(
                CircleButtonSprite::createWithSpriteFrameName(
                    "adIcon.png"_spr,
                    0.875f,
                    CircleBaseColor::Green,
                    CircleBaseSize::MediumAlt
                ),
                [userId = Mod::get()->getSettingValue<std::string>("user-id")](auto) {
                    if (userId.empty()) {
                        createQuickPopup(
                            "No User ID Set",
                            "You have not set a User ID yet.\n<cy>Do you want to open the Advertisement Manager and mod settings?</c>",
                            "No", "Yes",
                            [](auto, bool ok) {
                                if (ok) {
                                    openSettingsPopup(getMod());
                                    Notification::create("Opening Advertisement Manager", NotificationIcon::Info)->show();
                                    web::openLinkInBrowser("https://ads.arcticwoof.xyz/");
                                };
                            }
                        );
                    } else {
                        if (auto popup = AdManager::create()) popup->show();
                    };
                }
            );

            bottomMenu->addChild(popupButton);
            bottomMenu->updateLayout();
        };

        return true;
    };
};