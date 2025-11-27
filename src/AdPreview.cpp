#include "AdPreview.hpp"

#include <Advertisements.hpp>
#include <Geode/Geode.hpp>
#include <argon/argon.hpp>

#include "ReportPopup.hpp"

using namespace geode::prelude;
using namespace geode::utils;
using namespace ads;

AdPreview* AdPreview::create(int adId, int levelId, std::string userId, AdType type, int viewCount, int clickCount) {
    auto ret = new AdPreview();
    ret->m_adId = adId;
    ret->m_levelId = levelId;
    ret->m_userId = userId;
    ret->m_type = type;
    ret->m_viewCount = viewCount;
    ret->m_clickCount = clickCount;

    // @geode-ignore(unknown-resource)
    if (ret && ret->initAnchored(250.f, 200.f, "geode.loader/GE_square03.png")) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
};

bool AdPreview::setup() {
    setTitle("AD ID: " + numToString(m_adId));
    auto levelIdLabel = CCLabelBMFont::create(("Level ID: " + numToString(m_levelId)).c_str(), "bigFont.fnt");
    levelIdLabel->setID("level-id-label");
    levelIdLabel->setPosition({ m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height - 40 });
    levelIdLabel->setScale(0.5f);
    m_mainLayer->addChild(levelIdLabel);

    auto menu = CCMenu::create();
    menu->setPosition({ 0.f, 0.f });

    auto playAdLevelSprite = CCSprite::createWithSpriteFrameName("GJ_playBtn2_001.png");
    auto playAdLevelButton = CCMenuItemSpriteExtra::create(playAdLevelSprite, this, menu_selector(AdPreview::onPlayButton));

    playAdLevelButton->setPosition({ m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height / 2 });
    m_mainLayer->addChild(menu);
    menu->addChild(playAdLevelButton);

    // view and click counts
    auto viewCountLabel = CCLabelBMFont::create(("Views: " + numToString(m_viewCount)).c_str(), "goldFont.fnt");
    viewCountLabel->setID("view-count-label");
    viewCountLabel->setColor({ 255, 125, 0 });
    viewCountLabel->setPosition({ m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height / 2 - 55 });
    viewCountLabel->setScale(0.7f);
    m_mainLayer->addChild(viewCountLabel);

    auto clickCountLabel = CCLabelBMFont::create(("Clicks: " + numToString(m_clickCount)).c_str(), "goldFont.fnt");
    clickCountLabel->setID("click-count-label");
    clickCountLabel->setColor({ 0, 175, 255 });
    clickCountLabel->setPosition({ m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height / 2 - 75 });
    clickCountLabel->setScale(0.7f);
    m_mainLayer->addChild(clickCountLabel);

    // report button
    auto reportSprite = CCSprite::createWithSpriteFrameName("GJ_reportBtn_001.png");
    auto reportButton = CCMenuItemSpriteExtra::create(reportSprite, this, menu_selector(AdPreview::onReportButton));
    reportButton->setPosition({ 0, 0 });
    menu->addChild(reportButton);

    return true;
};

void AdPreview::onReportButton(CCObject* sender) {
    auto reportPopup = ReportPopup::create(m_adId, m_levelId, m_userId, "");
    reportPopup->show();
}

void AdPreview::onPlayButton(CCObject* sender) {
    // stop player if they have too many scenes loaded
    if (CCDirector::sharedDirector()->sceneCount() >= 10 && Mod::get()->getSettingValue<bool>("ads_disable_scene_limit_protection") == false) {
        geode::createQuickPopup(
            "Stop right there!",
            "You have <cr>too many scenes loaded</c> because you opening too many ads. This will cause your game to be <cr>unstable.</c>\n<cy>Do you want to return to the Main Menu?</c>",
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
                    this->registerClick(m_adId, m_userId);
                    auto searchStr = std::to_string(m_levelId);
                    auto scene = LevelBrowserLayer::scene(GJSearchObject::create(SearchType::Search, searchStr));
                    CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, scene));
                }
            });
    } else {
        // close popup
        this->onClose(sender);
        this->registerClick(m_adId, m_userId);
        auto searchStr = std::to_string(m_levelId);
        auto scene = LevelBrowserLayer::scene(GJSearchObject::create(SearchType::Search, searchStr));
        CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, scene));
    }
};

void AdPreview::registerClick(int adId, std::string userId) {
    log::debug("Sending click tracking request for ad_id={}, user_id={}", adId, userId);

    // get argon token yum
    auto res = argon::startAuth([this, adId, userId](Result<std::string> res) {
        if (!res) {
            log::warn("Auth failed: {}", res.unwrapErr());
            Notification::create("Failed to authenticate with Argon", NotificationIcon::Error)
                ->show();
            return;
        };

        auto token = std::move(res).unwrap();
        Mod::get()->setSavedValue<std::string>("argon_token", token);
        log::debug("Token: {}", token);

        auto clickRequest = web::WebRequest();
        clickRequest.userAgent("PlayerAdvertisements/1.0");
        clickRequest.timeout(std::chrono::seconds(15));
        clickRequest.header("Content-Type", "application/json");

        matjson::Value jsonBody = matjson::Value::object();
        jsonBody["ad_id"] = adId;
        jsonBody["user_id"] = userId;
        jsonBody["authtoken"] = token;
        jsonBody["account_id"] = GJAccountManager::sharedState()->m_accountID;

        clickRequest.bodyJSON(jsonBody);
        auto clickTask = clickRequest.post("https://ads.arcticwoof.xyz/api/click");

        EventListener<web::WebTask> clickListener;
        clickListener.bind([this](web::WebTask::Event* e) {
            if (auto res = e->getValue()) {
                if (res->ok()) {
                    log::info("Click pass ad_id={}, user_id={}", m_adId, m_userId);
                } else {
                    log::error("Click failed for ad_id={}, user_id={}: (code: {})", m_adId, m_userId, res->code());
                }
            } }); }, [](argon::AuthProgress progress) { log::info("Auth progress: {}", argon::authProgressToString(progress)); });

            if (!res) {
                log::warn("Failed to start auth attempt: {}", res.unwrapErr());
                Notification::create("Failed to start argon auth", NotificationIcon::Error)
                    ->show();
            };
}