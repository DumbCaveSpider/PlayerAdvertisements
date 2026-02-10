#include "ReportPopup.hpp"
#include "Geode/ui/MDTextArea.hpp"

#include <Geode/Geode.hpp>
#include <Geode/utils/async.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <argon/argon.hpp>

using namespace geode::prelude;
using namespace geode::utils;

class ReportPopup::Impl final {
public:
    int m_adId = 0;
    int m_levelId = 0;
    std::string m_userId = "";
    std::string m_description = "";
    async::TaskHolder<web::WebResponse> m_listener;
};

ReportPopup::ReportPopup() {
    m_impl = std::make_unique<Impl>();
};

ReportPopup::~ReportPopup() {};

bool ReportPopup::init() {
    if (!Popup::init(300.f, 200.f, "geode.loader/GE_square03.png")) return false;
    setTitle("Report AD ID: " + numToString(m_impl->m_adId));
    auto descriptionInput = TextInput::create(260.f, "Report Reason...", "chatFont.fnt");
    descriptionInput->setID("description-input");
    descriptionInput->setPosition({ m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height - 50 });
    if (!m_impl->m_description.empty()) descriptionInput->setString(m_impl->m_description);
    m_mainLayer->addChild(descriptionInput);

    auto menu = CCMenu::create();
    menu->setPosition({ m_mainLayer->getContentSize().width / 2, 0.f });
    auto submitButtonSprite = ButtonSprite::create("Submit Report", 0, false, "goldFont.fnt", "GJ_button_01.png", 0.f, 1.f);
    auto submitButton = CCMenuItemSpriteExtra::create(submitButtonSprite, this, menu_selector(ReportPopup::onSubmitButton));
    menu->addChild(submitButton);
    m_mainLayer->addChild(menu);

    auto textArea = MDTextArea::create(
        "Make sure to report this advertisement if you believe it violates the submission rules.\n\n"
        "<cr>Multiple false reports will lead to your account getting blacklisted.</c>",
        {260.f, 100.f}
    );
    textArea->setPosition({ m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height / 2 - 25 });
    m_mainLayer->addChild(textArea);

    return true;
};

void ReportPopup::onSubmitButton(CCObject* sender) {
    auto descriptionInput = typeinfo_cast<TextInput*>(m_mainLayer->getChildByID("description-input"));
    auto upopup = UploadActionPopup::create(nullptr, "Submitting Report...");
    upopup->show();
    std::string desc;
    if (descriptionInput)
        desc = descriptionInput->getString();
    else
        desc = m_impl->m_description;

    if (desc.length() < 10) {
        upopup->showFailMessage("Report reason is too short");
        return;
    };

    // argon token thing
    auto accountData = argon::getGameAccountData();

    async::spawn(
        argon::startAuth(std::move(accountData)),
        [this, desc, upopup](geode::Result<std::string> res) {
            if (!res) {
                log::warn("Auth failed: {}", res.unwrapErr());
                upopup->showFailMessage("Failed to authenticate with Argon");
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
            body["ad_id"] = m_impl->m_adId;
            body["account_id"] = GJAccountManager::sharedState()->m_accountID;
            body["description"] = desc;
            body["authtoken"] = token;
            reportReq.bodyJSON(body);

            async::spawn(
                reportReq.post("https://ads.arcticwoof.xyz/api/report"),
                [this, upopup](web::WebResponse res) {
                    if (res.ok()) {
                        this->onClose(nullptr);
                        upopup->showSuccessMessage("Report submitted successfully");
                    } else {
                        upopup->showFailMessage(res.code() == 403 ? "You've been banned from reporting ads" : "Failed to send report");
                    }
                }
            );
        }
    );
};

ReportPopup* ReportPopup::create(int adId, int levelId, std::string_view userId, std::string_view description) {
    auto ret = new ReportPopup();
    ret->m_impl->m_adId = adId;
    ret->m_impl->m_levelId = levelId;
    ret->m_impl->m_userId = userId;
    ret->m_impl->m_description = description;

    // @geode-ignore(unknown-resource)
    if (ret->init()) {
        ret->autorelease();
        return ret;
    };

    delete ret;
    return nullptr;
};