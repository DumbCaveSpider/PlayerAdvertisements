#include "AdPreview.hpp"

#include <Advertisements.hpp>
#include <Geode/Geode.hpp>
#include <Geode/utils/async.hpp>
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
    async::TaskHolder<web::WebResponse> m_announcementListener;
    async::TaskHolder<web::WebResponse> m_clickListener;

    std::string m_pendingKey;
    int m_pendingLevelId = -1;
    float m_pendingTimeout = 0.0f;
    LoadingSpinner* m_pendingSpinner = nullptr;
    CCMenu* m_levelsMenu = nullptr;
};

AdPreview::AdPreview() {
    m_impl = std::make_unique<Impl>();
};

AdPreview::~AdPreview() {};

bool AdPreview::init() {
    if (!Popup::init(250.f, 200.f, "geode.loader/GE_square03.png")) return false;
    
    setTitle("Ad ID: " + numToString(m_impl->m_adId));
    auto levelIdLabel = CCLabelBMFont::create(("Level ID: " + numToString(m_impl->m_levelId)).c_str(), "bigFont.fnt");
    levelIdLabel->setID("level-id-label");
    levelIdLabel->setPosition({ m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height - 40 });
    levelIdLabel->setScale(0.5f);
    m_mainLayer->addChild(levelIdLabel);

    auto playAdLevelSprite = CCSprite::createWithSpriteFrameName("GJ_playBtn2_001.png");
    auto playAdLevelBtn = CCMenuItemSpriteExtra::create(playAdLevelSprite, this, menu_selector(AdPreview::onPlayButton));
    playAdLevelBtn->setID("play-btn");
    // tag the play button with the level id so the scheduler can restore it later
    playAdLevelBtn->setTag(m_impl->m_levelId);
    playAdLevelBtn->setPosition({ m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height / 2 });
    // store the menu for spinner placement / restoring state
    m_buttonMenu->addChild(playAdLevelBtn);
    m_impl->m_levelsMenu = m_buttonMenu;

    // view and click counts
    auto viewCountLabel = CCLabelBMFont::create(("Views: " + numToString(m_impl->m_viewCount)).c_str(), "goldFont.fnt");
    viewCountLabel->setID("view-count-label");
    viewCountLabel->setColor({ 255, 125, 0 });
    viewCountLabel->setPosition({ m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height / 2 - 55 });
    viewCountLabel->setScale(0.7f);
    m_mainLayer->addChild(viewCountLabel);

    auto clickCountLabel = CCLabelBMFont::create(("Clicks: " + numToString(m_impl->m_clickCount)).c_str(), "goldFont.fnt");
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
    m_buttonMenu->addChild(reportBtn);

    // @geode-ignore(unknown-resource)
    auto announcementBtnSprite = CCSprite::createWithSpriteFrameName("geode.loader/news.png");
    auto announcementBtn = CCMenuItemSpriteExtra::create(
        announcementBtnSprite,
        this,
        menu_selector(AdPreview::onAnnouncementButton));
    announcementBtn->setID("latest-announcement-btn");
    announcementBtn->setPosition({ m_mainLayer->getContentSize().width - 17, 18 });
    m_buttonMenu->addChild(announcementBtn);

    this->scheduleUpdate();

    return true;
};

void AdPreview::onReportButton(CCObject* sender) {
    auto reportPopup = ReportPopup::create(m_impl->m_adId, m_impl->m_levelId, m_impl->m_userId, "");
    reportPopup->show();
};

void AdPreview::onAnnouncementButton(CCObject* sender) {
    // fetch from /api/announcement
    auto request = web::WebRequest();
    request.userAgent("PlayerAdvertisements/1.0");
    request.timeout(std::chrono::seconds(15));
    request.header("Content-Type", "application/json");

    async::spawn(
        request.get("https://ads.arcticwoof.xyz/api/announcement"),
        [this](web::WebResponse res) {
            if (res.ok()) {
                auto data = res.json();
                if (!data.isOk()) {
                    log::error("Failed to parse announcement JSON");
                    return;
                };

                auto const val = data.unwrap();
                std::string const title = val.contains("title") && val["title"].asString().isOk()
                    ? val["title"].asString().unwrap()
                    : "Announcement";
                std::string const content = val.contains("content") && val["content"].asString().isOk()
                    ? val["content"].asString().unwrap()
                    : "";

                if (auto popup = MDPopup::create(title.c_str(), content.c_str(), "Close")) popup->show();
            } else {
                log::error("Failed to fetch announcement: (code: {})", res.code());
                Notification::create("Failed to fetch announcement", NotificationIcon::Error)
                    ->show();
            }
        }
    );
};

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
                };
            });

        return;
    };

    if (PlayLayer::get()) {
        geode::createQuickPopup(
            "Warning",
            "You are already inside of a level, attempt to play another level before closing the current level may <cr>crash your game</c>.\n<cy>Do you still want to proceed?</c>",
            "Cancel", "Proceed",
            [this, sender](auto, bool btn) {
                if (btn) {
                    auto menuItem = typeinfo_cast<CCMenuItemSpriteExtra*>(sender);
                    this->registerClick(m_impl->m_adId, m_impl->m_userId, menuItem);
                    this->tryOpenOrFetchLevel(menuItem, m_impl->m_levelId);
                };
            });
    } else {
        auto menuItem = typeinfo_cast<CCMenuItemSpriteExtra*>(sender);
        this->registerClick(m_impl->m_adId, m_impl->m_userId, menuItem);
        this->tryOpenOrFetchLevel(menuItem, m_impl->m_levelId);
    };
};

void AdPreview::registerClick(int adId, std::string_view userId, CCMenuItemSpriteExtra* menuItem) {
    log::debug("Sending click tracking request for ad_id={}, user_id={}", adId, userId);

    // disable the clicked menu item immediately
    if (menuItem) {
        menuItem->setEnabled(false);
    }

    // get argon token yum
    argon::AuthOptions opts;
    opts.progress = [](argon::AuthProgress progress) { log::debug("Auth progress: {}", argon::authProgressToString(progress)); };

    async::spawn(
        argon::startAuth(std::move(opts)),
        [this, adId, userId](geode::Result<std::string> res) {
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

            async::spawn(
                clickRequest.post("https://ads.arcticwoof.xyz/api/click"),
                [this, adId, userId](web::WebResponse res) {
                    if (res.ok()) {
                        log::info("Click passed ad_id={}, user_id={}", adId, userId);
                    } else {
                        log::error("Click failed with code {} for ad_id={}, user_id={}: {}", res.code(), adId, userId, res.errorMessage());
                    }

                    log::debug("Click request completed for ad_id={}, user_id={}", adId, userId);
                }
            );
            log::debug("Sent click tracking request for ad_id={}, user_id={}", adId, userId);
        }
    );
};

// open LevelInfo if stored otherwise prepare pending state and request
void AdPreview::tryOpenOrFetchLevel(CCMenuItemSpriteExtra* menuItem, int levelId) {
    if (!menuItem) return;

    auto searchObj = GJSearchObject::create(SearchType::Search, numToString(levelId));
    auto key = std::string(searchObj->getKey());
    auto glm = GameLevelManager::sharedState();

    // check stored cache first
    auto stored = glm->getStoredOnlineLevels(key.c_str());
    if (stored && stored->count() > 0) {
        auto level = static_cast<GJGameLevel*>(stored->objectAtIndex(0));
        if (level && level->m_levelID == levelId) {
            this->onClose(nullptr);
            auto scene = LevelInfoLayer::scene(level, false);
            auto transitionFade = CCTransitionFade::create(0.5f, scene);
            CCDirector::sharedDirector()->pushScene(transitionFade);
            return;
        }
    }

    // prepare pending state
    m_impl->m_pendingKey = key;
    m_impl->m_pendingLevelId = levelId;
    m_impl->m_pendingTimeout = 10.0f;  // seconds

    // show spinner on the clicked button and disable it
    if (m_impl->m_pendingSpinner) {
        m_impl->m_pendingSpinner->removeFromParent();
        m_impl->m_pendingSpinner = nullptr;
    }
    auto spinner = LoadingSpinner::create(100.f);
    if (spinner) {
        spinner->setPosition(menuItem->getPosition());
        spinner->setVisible(true);
        if (m_impl->m_levelsMenu) m_impl->m_levelsMenu->addChild(spinner);
        m_impl->m_pendingSpinner = spinner;
    }
    glm->getOnlineLevels(searchObj);
}

void AdPreview::update(float dt) {
    if (!m_impl->m_pendingKey.empty()) {
        auto glm = GameLevelManager::sharedState();
        auto stored = glm->getStoredOnlineLevels(m_impl->m_pendingKey.c_str());
        if (stored && stored->count() > 0) {
            auto level = static_cast<GJGameLevel*>(stored->objectAtIndex(0));
            if (level && level->m_levelID == m_impl->m_pendingLevelId) {
                // open level info
                this->onClose(nullptr);
                auto scene = LevelInfoLayer::scene(level, false);
                auto transitionFade = CCTransitionFade::create(0.5f, scene);
                CCDirector::sharedDirector()->pushScene(transitionFade);
                // cleanup pending state and spinner
                if (m_impl->m_pendingSpinner) {
                    m_impl->m_pendingSpinner->removeFromParent();
                    m_impl->m_pendingSpinner = nullptr;
                }
                m_impl->m_pendingKey.clear();
                m_impl->m_pendingLevelId = -1;
                m_impl->m_pendingTimeout = 0.0;
                return;
            }
        }

        m_impl->m_pendingTimeout -= dt;
        if (m_impl->m_pendingTimeout <= 0.0) {
            if (m_impl->m_pendingSpinner) {
                m_impl->m_pendingSpinner->removeFromParent();
                m_impl->m_pendingSpinner = nullptr;
            }
            Notification::create("Level not found", NotificationIcon::Warning)->show();
            m_impl->m_pendingKey.clear();
            m_impl->m_pendingLevelId = -1;
            m_impl->m_pendingTimeout = 0.0;
        }
    }
}

AdPreview* AdPreview::create(int adId, int levelId, std::string_view userId, AdType type, int viewCount, int clickCount) {
    auto ret = new AdPreview();
    ret->m_impl->m_adId = adId;
    ret->m_impl->m_levelId = levelId;
    ret->m_impl->m_userId = userId;
    ret->m_impl->m_type = type;
    ret->m_impl->m_viewCount = viewCount;
    ret->m_impl->m_clickCount = clickCount;

    // @geode-ignore(unknown-resource)
    if (ret->init()) {
        ret->autorelease();
        return ret;
    };

    delete ret;
    return nullptr;
};