#include "AdPreview.hpp"

#include <Advertisements.hpp>
#include <Geode/Geode.hpp>
#include <argon/argon.hpp>

#include "ReportPopup.hpp"

using namespace geode::prelude;
using namespace geode::utils;
using namespace ads;

class AdPreview::Impl final {
public:
    int m_adId = 0;
    int m_levelId = 0;
    std::string m_userId = "";
    AdType m_type = AdType::Banner;
    int m_viewCount = 0;
    int m_clickCount = 0;
    EventListener<web::WebTask> m_announcementListener;
    EventListener<web::WebTask> m_clickListener;
};

AdPreview::AdPreview() {
    m_impl = std::make_unique<Impl>();
};

AdPreview::~AdPreview() {};

bool AdPreview::setup() {
    setTitle("Ad ID: " + numToString(m_impl->m_adId));
    auto levelIdLabel = CCLabelBMFont::create(("Level ID: " + numToString(m_impl->m_levelId)).data(), "bigFont.fnt");
    levelIdLabel->setID("level-id-label");
    levelIdLabel->setPosition({ m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height - 40 });
    levelIdLabel->setScale(0.5f);
    m_mainLayer->addChild(levelIdLabel);

    auto menu = CCMenu::create();
    menu->setPosition({ 0.f, 0.f });

    auto playAdLevelSprite = CCSprite::createWithSpriteFrameName("GJ_playBtn2_001.png");
    auto playAdLevelBtn = CCMenuItemSpriteExtra::create(playAdLevelSprite, this, menu_selector(AdPreview::onPlayButton));
    playAdLevelBtn->setID("play-btn");
    playAdLevelBtn->setPosition({ m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height / 2 });
    m_mainLayer->addChild(menu);
    menu->addChild(playAdLevelBtn);

    // view and click counts
    auto viewCountLabel = CCLabelBMFont::create(("Views: " + numToString(m_impl->m_viewCount)).data(), "goldFont.fnt");
    viewCountLabel->setID("view-count-label");
    viewCountLabel->setColor({ 255, 125, 0 });
    viewCountLabel->setPosition({ m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height / 2 - 55 });
    viewCountLabel->setScale(0.7f);
    m_mainLayer->addChild(viewCountLabel);

    auto clickCountLabel = CCLabelBMFont::create(("Clicks: " + numToString(m_impl->m_clickCount)).data(), "goldFont.fnt");
    clickCountLabel->setID("click-count-label");
    clickCountLabel->setColor({ 0, 175, 255 });
    clickCountLabel->setPosition({ m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height / 2 - 75 });
    clickCountLabel->setScale(0.7f);
    m_mainLayer->addChild(clickCountLabel);

    // report button
    auto reportSprite = CCSprite::createWithSpriteFrameName("GJ_reportBtn_001.png");
    auto reportBtn = CCMenuItemSpriteExtra::create(reportSprite, this, menu_selector(AdPreview::onReportButton));
    reportBtn->setID("report-ad-btn");
    reportBtn->setPosition({ 0, 0 });
    menu->addChild(reportBtn);

    // @geode-ignore(unknown-resource)
    auto announcementBtnSprite = CCSprite::createWithSpriteFrameName("geode.loader/news.png");
    auto announcementBtn = CCMenuItemSpriteExtra::create(
        announcementBtnSprite,
        this,
        menu_selector(AdPreview::onAnnouncementButton));
    announcementBtn->setID("latest-announcement-btn");
    announcementBtn->setPosition({ m_mainLayer->getContentSize().width - 17, 18 });
    menu->addChild(announcementBtn);

    return true;
};

void AdPreview::onReportButton(CCObject* sender) {
    auto reportPopup = ReportPopup::create(m_impl->m_adId, m_impl->m_levelId, m_impl->m_userId, "");
    reportPopup->show();
}

void AdPreview::onAnnouncementButton(CCObject* sender) {
    // fetch from /api/announcement
    auto request = web::WebRequest();
    request.userAgent("PlayerAdvertisements/1.0");
    request.timeout(std::chrono::seconds(15));
    request.header("Content-Type", "application/json");

    auto task = request.get("https://ads.arcticwoof.xyz/api/announcement");
    m_impl->m_announcementListener.bind([this](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (res->ok()) {
                auto data = res->json();
                if (!data.isOk()) {
                    log::error("Failed to parse announcement JSON");
                    return;
                }
                auto val = data.unwrap();
                std::string title = val.contains("title") && val["title"].asString().isOk()
                    ? val["title"].asString().unwrap()
                    : "Announcement";
                std::string content = val.contains("content") && val["content"].asString().isOk()
                    ? val["content"].asString().unwrap()
                    : "";

                if (auto popup = geode::MDPopup::create(title.data(), content.data(), "Close")) {
                    popup->show();
                }
            } else {
                log::error("Failed to fetch announcement: (code: {})", res->code());
                Notification::create("Failed to fetch announcement", NotificationIcon::Error)
                    ->show();
            }
        }
                                        });
    m_impl->m_announcementListener.setFilter(task);
}

void AdPreview::onPlayButton(CCObject* sender) {
    // stop player if they have too many scenes loaded
    if (CCDirector::sharedDirector()->sceneCount() >= 10 && Mod::get()->getSettingValue<bool>("scene-protection") == false) {
        geode::createQuickPopup(
            "Stop right there!",
            "You have <cr>too many scenes loaded</c> because you're opening too many ads. This may cause your game to become <cr>unstable</c>.\n<cy>Would you like to return to the main menu?</c>",
            "Cancel", "Yes", [](auto, bool ok) {
                if (ok) {
                    // pop to root scene
                    CCDirector::sharedDirector()->popToRootScene();
                }
            });
        return;
    }

    if (PlayLayer::get()) {
        geode::createQuickPopup(
            "Warning",
            "You are already inside of a level, attempt to play another level before closing the current level will <cr>crash your game</c>.\n<cy>Do you still want to proceed?</c>",
            "Cancel", "Proceed",
            [this, sender](auto, bool btn) {
                if (btn) {
                    // close popup
                    this->onClose(sender);
                    this->registerClick(m_impl->m_adId, m_impl->m_userId);
                    auto searchStr = std::to_string(m_impl->m_levelId);
                    auto scene = LevelBrowserLayer::scene(GJSearchObject::create(SearchType::Search, searchStr));
                    CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, scene));
                }
            });
    } else {
        // close popup
        this->onClose(sender);
        this->registerClick(m_impl->m_adId, m_impl->m_userId);
        auto searchStr = std::to_string(m_impl->m_levelId);
        auto scene = LevelBrowserLayer::scene(GJSearchObject::create(SearchType::Search, searchStr));
        CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, scene));
    }
};

void AdPreview::registerClick(int adId, std::string_view userId) {
    log::debug("Sending click tracking request for ad_id={}, user_id={}", adId, userId);

    // get argon token yum
    auto res = argon::startAuth([this, adId, userId](Result<std::string> res) {
        if (res.isErr()) {
            log::warn("Auth failed: {}", res.unwrapErr());
            Notification::create("Failed to authenticate with Argon", NotificationIcon::Error)
                ->show();
            return;
        };

        auto token = std::move(res).unwrapOrDefault();
        Mod::get()->setSavedValue<std::string>("argon_token", token);
        log::debug("Token: {}", token);

        log::debug("Sending click tracking request for ad_id={}, user_id={}", adId, userId);
        auto clickRequest = web::WebRequest();
        clickRequest.userAgent("PlayerAdvertisements/1.0");
        clickRequest.header("Content-Type", "application/json");
        clickRequest.timeout(std::chrono::seconds(15));

        matjson::Value clickBody = matjson::Value::object();
        clickBody["ad_id"] = adId;
        clickBody["authtoken"] = token;
        clickBody["account_id"] = GJAccountManager::sharedState()->m_accountID;

        clickRequest.bodyJSON(clickBody);

        m_impl->m_clickListener.bind([this, adId, userId](web::WebTask::Event* e) {
            if (auto res = e->getValue()) {
                if (res->ok()) {
                    log::info("Click passed ad_id={}, user_id={}", adId, userId);
                } else {
                    log::error("Click failed with code {} for ad_id={}, user_id={}: {}", res->code(), adId, userId, res->errorMessage());
                };

                log::debug("Click request completed for ad_id={}, user_id={}", adId, userId);
            } else if (e->isCancelled()) {
                log::error("Click request failed for ad_id={}, user_id={}", adId, userId);
            };
                                     });
        m_impl->m_clickListener.setFilter(clickRequest.post("https://ads.arcticwoof.xyz/api/click"));
        log::debug("Sent click tracking request for ad_id={}, user_id={}", adId, userId);
                                },
                                [](argon::AuthProgress progress) {
                                    log::debug("Auth progress: {}", argon::authProgressToString(progress));
                                });

    if (!res) {
        log::warn("Failed to start auth attempt: {}", res.unwrapErr());
        Notification::create("Failed to start argon auth", NotificationIcon::Error)
            ->show();
    };
};

AdPreview* AdPreview::create(int adId, int levelId, std::string_view userId, AdType type, int viewCount, int clickCount) {
    auto ret = new AdPreview();
    ret->m_impl->m_adId = adId;
    ret->m_impl->m_levelId = levelId;
    ret->m_impl->m_userId = userId;
    ret->m_impl->m_type = type;
    ret->m_impl->m_viewCount = viewCount;
    ret->m_impl->m_clickCount = clickCount;

    // @geode-ignore(unknown-resource)
    if (ret->initAnchored(250.f, 200.f, "geode.loader/GE_square03.png")) {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};