#include "ReportPopup.hpp"

#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <argon/argon.hpp>

using namespace geode::prelude;
using namespace geode::utils;

ReportPopup* ReportPopup::create(int adId, int levelId, std::string userId, std::string description) {
    auto ret = new ReportPopup();
    ret->m_adId = adId;
    ret->m_levelId = levelId;
    ret->m_userId = userId;
    ret->m_description = description;

    // @geode-ignore(unknown-resource)
    if (ret && ret->initAnchored(340.f, 120.f, "geode.loader/GE_square03.png")) {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};

bool ReportPopup::setup() {
    setTitle("Report AD ID: " + numToString(m_adId));
    auto descriptionInput = TextInput::create(300.f, "Report Reason...", "bigFont.fnt");
    descriptionInput->setID("description-input");
    descriptionInput->setPosition({ m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height / 2 + 10 });
    if (!m_description.empty())
        descriptionInput->setString(m_description);
    m_mainLayer->addChild(descriptionInput);

    auto menu = CCMenu::create();
    menu->setPosition({ m_mainLayer->getContentSize().width / 2, 0.f });
    auto submitButtonSprite = ButtonSprite::create("Submit Report", 0, false, "goldFont.fnt", "GJ_button_01.png", 0.f, 1.f);
    auto submitButton = CCMenuItemSpriteExtra::create(submitButtonSprite, this, menu_selector(ReportPopup::onSubmitButton));
    menu->addChild(submitButton);
    m_mainLayer->addChild(menu);

    // info1 label
    auto infoLabel1 = CCLabelBMFont::create("Make sure to report this advertisement if you believe it violates the submission rules.", "chatFont.fnt");
    infoLabel1->setScale(0.6f);
    infoLabel1->setPosition({ m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height / 2 - 20 });
    m_mainLayer->addChild(infoLabel1);

    // info2 label
    auto infoLabel2 = CCLabelBMFont::create("Multiple false reports will lead to your account getting blacklisted.", "chatFont.fnt");
    infoLabel2->setScale(0.6f);
    infoLabel2->setColor({ 255, 100, 100 });
    infoLabel2->setPosition({ m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height / 2 - 30 });
    m_mainLayer->addChild(infoLabel2);

    return true;
};

void ReportPopup::onSubmitButton(CCObject* sender) {
    auto descriptionInput = static_cast<TextInput*>(m_mainLayer->getChildByID("description-input"));
    std::string desc;
    if (descriptionInput)
        desc = descriptionInput->getString();
    else
        desc = m_description;

    if (desc.length() < 10) {
        Notification::create("Report reason is too short", NotificationIcon::Warning)->show();
        return;
    };

    // argon token thing
    auto res = argon::startAuth([this, desc](Result<std::string> res) {
        if (!res) {
            log::warn("Auth failed: {}", res.unwrapErr());
            Notification::create("Failed to authenticate with Argon", NotificationIcon::Error)
                ->show();
            return;
        };

        auto token = std::move(res).unwrap();
        Mod::get()->setSavedValue<std::string>("argon_token", token);
        log::debug("Token: {}", token);

        auto reportReq = web::WebRequest();
        reportReq.userAgent("PlayerAdvertisements/1.0");
        reportReq.timeout(std::chrono::seconds(15));
        reportReq.header("Content-Type", "application/json");

        matjson::Value body = matjson::Value::object();
        body["ad_id"] = m_adId;
        body["account_id"] = GJAccountManager::sharedState()->m_accountID;
        body["description"] = desc;
        body["authtoken"] = token;
        reportReq.bodyJSON(body);

        m_listener.bind([this](web::WebTask::Event* e) {
            if (auto res = e->getValue()) {
                if (res->ok()) {
                    Notification::create("Report Sent", NotificationIcon::Success)->show();
                    this->onClose(nullptr);
                } else {
                    Notification::create("Failed to send report", NotificationIcon::Warning)->show();
                };
            } else if (e->isCancelled()) {
                log::error("Report request was cancelled");
            };
                        });
        m_listener.setFilter(reportReq.post("https://ads.arcticwoof.xyz/api/report")); }, [](argon::AuthProgress progress) { log::info("Auth progress: {}", argon::authProgressToString(progress)); });

    if (!res) {
        log::warn("Failed to start auth attempt: {}", res.unwrapErr());
        Notification::create("Failed to start argon auth", NotificationIcon::Error)
            ->show();
    };
};