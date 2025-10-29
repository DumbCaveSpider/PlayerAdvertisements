#include "AdPreview.hpp"
#include <Geode/Geode.hpp>
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace geode::utils;
using namespace ads;

AdPreview *AdPreview::create(int adId, int levelId, std::string userId, AdType type, int viewCount, int clickCount)
{
    auto ret = new AdPreview();
    ret->m_adId = adId;
    ret->m_levelId = levelId;
    ret->m_userId = userId;
    ret->m_type = type;
    ret->m_viewCount = viewCount;
    ret->m_clickCount = clickCount;

    // @geode-ignore(unknown-resource)
    if (ret && ret->initAnchored(250.f, 200.f, "geode.loader/GE_square03.png"))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
};

bool AdPreview::setup()
{
    setTitle("AD ID: " + numToString(m_adId));
    auto levelIdLabel = CCLabelBMFont::create(("Level ID: " + numToString(m_levelId)).c_str(), "bigFont.fnt");
    levelIdLabel->setID("level-id-label");
    levelIdLabel->setPosition({m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height - 40});
    levelIdLabel->setScale(0.5f);
    m_mainLayer->addChild(levelIdLabel);

    auto menu = CCMenu::create();
    menu->setPosition({0.f, 0.f});

    auto playAdLevelSprite = CCSprite::createWithSpriteFrameName("GJ_playBtn2_001.png");
    auto playAdLevelButton = CCMenuItemSpriteExtra::create(playAdLevelSprite, this, menu_selector(AdPreview::onPlayButton));

    playAdLevelButton->setPosition({m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height / 2});
    m_mainLayer->addChild(menu);
    menu->addChild(playAdLevelButton);

    // view and click counts
    auto viewCountLabel = CCLabelBMFont::create(("Views: " + numToString(m_viewCount)).c_str(), "goldFont.fnt");
    viewCountLabel->setID("view-count-label");
    viewCountLabel->setColor({255, 125, 0});
    viewCountLabel->setPosition({m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height / 2 - 55});
    viewCountLabel->setScale(0.7f);
    m_mainLayer->addChild(viewCountLabel);

    auto clickCountLabel = CCLabelBMFont::create(("Clicks: " + numToString(m_clickCount)).c_str(), "goldFont.fnt");
    clickCountLabel->setID("click-count-label");
    clickCountLabel->setColor({0, 175, 255});
    clickCountLabel->setPosition({m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height / 2 - 75});
    clickCountLabel->setScale(0.7f);
    m_mainLayer->addChild(clickCountLabel);

    // report button
    auto reportSprite = CCSprite::createWithSpriteFrameName("GJ_reportBtn_001.png");
    auto reportButton = CCMenuItemSpriteExtra::create(reportSprite, this, menu_selector(AdPreview::onReportButton));
    reportButton->setPosition({0, 0});
    menu->addChild(reportButton);

    return true;
};

void AdPreview::onReportButton(CCObject *sender)
{
    geode::createQuickPopup(
        "Report Ad",
        "To <cr>report</c> this advertisement, join the <cb>Discord server</c> and contact an <cy>admin</c> with the User ID: <cg>" + numToString(m_userId) + "</c>\n<cy>Do you wish to join the Discord server?</c>",
        "Cancel", "Join",
        [this, sender](auto, bool btn)
        {
            if (btn)
            {
                // open discord link
                web::openLinkInBrowser("https://discord.com/invite/gXcppxTNxC");
                // copy the user id to clipboard
                clipboard::write(numToString(m_userId));
                Notification::create("User ID copied to clipboard", NotificationIcon::Success)->show();
            }
        });
};

void AdPreview::onPlayButton(CCObject *sender)
{

    if (PlayLayer::get())
    {
        geode::createQuickPopup(
            "Warning",
            "You are already inside of a level, attempt to play another level before closing the current level will <cr>crash your game</c>.\n<cy>Do you still want to proceed?</c>",
            "Cancel", "Proceed",
            [this, sender](auto, bool btn)
            {
                if (btn)
                {
                    // close popup
                    this->onClose(sender);

                    log::info("opening level id {} from ad id {}", m_levelId, m_adId);
                    log::debug("Sending click tracking request for ad_id={}, user_id={}", m_adId, m_userId);
                    auto clickRequest = web::WebRequest();
                    clickRequest.userAgent("PlayerAdvertisements/1.0");
                    clickRequest.timeout(std::chrono::seconds(15));
                    clickRequest.header("Content-Type", "application/json");

                    matjson::Value jsonBody = matjson::Value::object();
                    jsonBody["ad_id"] = m_adId;
                    jsonBody["user_id"] = m_userId;

                    clickRequest.bodyJSON(jsonBody);
                    clickRequest.param("authtoken", Mod::get()->getSavedValue<std::string>("argon_token"));
                    clickRequest.param("account_id", GJAccountManager::sharedState()->m_accountID);
                    auto clickTask = clickRequest.post("https://ads.arcticwoof.xyz/api/click");

                    EventListener<web::WebTask> clickListener;
                    clickListener.bind([this](web::WebTask::Event* e) {
                        if (auto res = e->getValue()) {
                            if (res->ok()) {
                                log::info("Click pass ad_id={}, user_id={}", m_adId, m_userId);
                            } else {
                                log::error("Click failed for ad_id={}, user_id={}: HTTP {}", m_adId, m_userId, res->code());
                            }
                        }
                    });

                    auto searchStr = std::to_string(m_levelId);
                    auto scene = LevelBrowserLayer::scene(GJSearchObject::create(SearchType::Search, searchStr));
                    CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, scene));
                }
            });
    }
    else
    {
        // close popup
        this->onClose(sender);

        log::info("opening level id {} from ad id {}", m_levelId, m_adId);
        log::debug("Sending click tracking request for ad_id={}, user_id={}", m_adId, m_userId);
        auto clickRequest = web::WebRequest();
        clickRequest.userAgent("PlayerAdvertisements/1.0");
        clickRequest.timeout(std::chrono::seconds(15));
        clickRequest.header("Content-Type", "application/json");

        matjson::Value jsonBody = matjson::Value::object();
        jsonBody["ad_id"] = m_adId;
        jsonBody["user_id"] = m_userId;

        clickRequest.bodyJSON(jsonBody);
        (void)clickRequest.post("https://ads.arcticwoof.xyz/api/click");
        log::info("Sent click tracking request for ad_id={}, user_id={}", m_adId, m_userId);

        auto searchStr = std::to_string(m_levelId);
        auto scene = LevelBrowserLayer::scene(GJSearchObject::create(SearchType::Search, searchStr));
        CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, scene));
    }
};