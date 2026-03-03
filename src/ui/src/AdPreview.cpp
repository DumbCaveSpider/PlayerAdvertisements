#include "../AdPreview.hpp"

#include "../ReportPopup.hpp"

#include <Advertisements.hpp>

#include <argon/argon.hpp>

#include <Geode/Geode.hpp>

#include <Geode/ui/Button.hpp>

#include <Geode/utils/async.hpp>

using namespace geode::prelude;
using namespace geode::utils;

using namespace ads;

class AdPreview::Impl final {
public:
    unsigned int adId = 0;
    int levelId = 0;
    std::string userId = "";
    AdType type = AdType::Banner;
    unsigned int viewCount = 0;
    unsigned int clickCount = 0;

    async::TaskHolder<web::WebResponse> announcementListener;
    async::TaskHolder<web::WebResponse> clickListener;

    std::string pendingKey;
    int pendingLevelId = -1;
    float pendingTimeout = 0.0f;
    LoadingSpinner* pendingSpinner = nullptr;
    bool hasClicked = false;
};

AdPreview::AdPreview() : m_impl(std::make_unique<Impl>()) {};
AdPreview::~AdPreview() {};

bool AdPreview::init(unsigned int adId, int levelId, std::string userId, AdType type, unsigned int viewCount, unsigned int clickCount) {
    m_impl->adId = adId;
    m_impl->levelId = levelId;
    m_impl->userId = std::move(userId);
    m_impl->type = type;
    m_impl->viewCount = viewCount;
    m_impl->clickCount = clickCount;

    // @geode-ignore(unknown-resource)
    if (!Popup::init(250.f, 200.f, "geode.loader/GE_square03.png")) return false;

    setID("preview"_spr);
    setTitle("Ad ID: " + numToString(m_impl->adId));

    auto levelIdLabel = CCLabelBMFont::create(
        ("Level ID: " + numToString(m_impl->levelId)).c_str(),
        "bigFont.fnt"
    );
    levelIdLabel->setID("level-id-label");
    levelIdLabel->setPosition({ m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height - 40 });
    levelIdLabel->setScale(0.5f);

    m_mainLayer->addChild(levelIdLabel);

    auto playAdLevelSprite = CCSprite::createWithSpriteFrameName("GJ_playBtn2_001.png");
    auto playAdLevelBtn = CCMenuItemSpriteExtra::create(
        playAdLevelSprite,
        this,
        menu_selector(AdPreview::onPlayButton)
    );
    playAdLevelBtn->setID("play-btn");
    playAdLevelBtn->setTag(m_impl->levelId); // tag the play button with the level id so the scheduler can restore it later
    playAdLevelBtn->setPosition({ m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height / 2 });

    // store the menu for spinner placement / restoring state
    m_buttonMenu->addChild(playAdLevelBtn);

    // view and click counts
    auto viewCountLabel = CCLabelBMFont::create(
        ("Views: " + numToString(m_impl->viewCount)).c_str(),
        "goldFont.fnt"
    );
    viewCountLabel->setID("view-count-label");
    viewCountLabel->setColor({ 255, 125, 0 });
    viewCountLabel->setPosition({ m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height / 2 - 55 });
    viewCountLabel->setScale(0.7f);

    m_mainLayer->addChild(viewCountLabel);

    auto clickCountLabel = CCLabelBMFont::create(
        ("Clicks: " + numToString(m_impl->clickCount)).c_str(),
        "goldFont.fnt"
    );
    clickCountLabel->setID("click-count-label");
    clickCountLabel->setColor({ 0, 175, 255 });
    clickCountLabel->setPosition({ m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height / 2 - 75 });
    clickCountLabel->setScale(0.7f);

    m_mainLayer->addChild(clickCountLabel);

    // report button
    auto reportBtn = Button::createWithSpriteFrameName(
        "GJ_reportBtn_001.png",
        [this](auto) {
            if (auto reportPopup = ReportPopup::create(m_impl->adId, m_impl->levelId, m_impl->userId, "")) reportPopup->show();
        }
    );
    reportBtn->setID("report-ad-btn");
    reportBtn->setScale(0.875f);
    reportBtn->setPosition({ 0, 0 });

    m_mainLayer->addChild(reportBtn);

    auto announcementBtn = Button::createWithNode(
        CircleButtonSprite::createWithSpriteFrameName(
            // @geode-ignore(unknown-resource)
            "geode.loader/news.png",
            0.75f,
            CircleBaseColor::Green,
            CircleBaseSize::Medium
        ),
        [](auto) {
            // fetch from /api/announcement
            auto request = web::WebRequest();
            request.userAgent("PlayerAdvertisements/1.0");
            request.timeout(std::chrono::seconds(15));
            request.header("Content-Type", "application/json");

            async::spawn(
                request.get("https://ads.arcticwoof.xyz/api/announcement"),
                [](web::WebResponse res) {
                    if (res.ok()) {
                        auto data = res.json();
                        if (!data.isOk()) {
                            log::error("Failed to parse announcement JSON");
                            return;
                        };

                        auto const val = data.unwrap();

                        std::string const title =
                            val.contains("title") && val["title"].asString().isOk()
                            ? val["title"].asString().unwrap()
                            : "Announcement";

                        std::string const content =
                            val.contains("content") && val["content"].asString().isOk()
                            ? val["content"].asString().unwrap()
                            : "";

                        if (auto popup = MDPopup::create(title.c_str(), content.c_str(), "Close")) popup->show();
                    } else {
                        log::error("Failed to fetch announcement: (code: {})", res.code());
                        Notification::create("Failed to fetch announcement",
                            NotificationIcon::Error)
                            ->show();
                    };
                }
            );
        }
    );
    announcementBtn->setID("latest-announcement-btn");
    announcementBtn->setScale(0.875f);
    announcementBtn->setPosition({ m_mainLayer->getContentSize().width, 0 });

    m_mainLayer->addChild(announcementBtn);

    this->scheduleUpdate();

    return true;
};

void AdPreview::onPlayButton(CCObject* sender) {
    // stop player if they have too many scenes loaded
    if (CCDirector::sharedDirector()->sceneCount() >= 10 &&
        Mod::get()->getSettingValue<bool>("scene-protection") == false) {
        createQuickPopup(
            "Stop right there!",
            "You have <cr>too many scenes loaded</c> because you're opening too "
            "many ads. This may cause your game to become "
            "<cr>unstable</c>.\n<cy>Would you like to return to the main menu?</c>",
            "Cancel", "Yes", [](auto, bool ok) {
                if (ok) {
                    // pop to root scene
                    CCDirector::sharedDirector()->popToRootScene();
                };
            });

        return;
    };

    if (PlayLayer::get()) {
        createQuickPopup(
            "Warning",
            "You are already inside of a level, attempt to play another level "
            "before closing the current level may <cr>crash your game</c>.\n<cy>Do "
            "you still want to proceed?</c>",
            "Cancel", "Proceed", [this, sender](auto, bool btn) {
                if (btn) {
                    auto menuItem = typeinfo_cast<CCMenuItemSpriteExtra*>(sender);
                    this->registerClick(m_impl->adId, m_impl->userId, menuItem);
                    this->tryOpenOrFetchLevel(menuItem, m_impl->levelId);
                };
            });
    } else {
        auto menuItem = typeinfo_cast<CCMenuItemSpriteExtra*>(sender);
        if (!m_impl->hasClicked) {
            m_impl->hasClicked = true;
            this->registerClick(m_impl->adId, m_impl->userId, menuItem);
            log::debug("click registered for ad_id={}, user_id={}", m_impl->adId,
                m_impl->userId);
            this->tryOpenOrFetchLevel(menuItem, m_impl->levelId);
        } else {
            log::debug("click already registered for ad_id={}, user_id={}",
                m_impl->adId, m_impl->userId);
            this->tryOpenOrFetchLevel(menuItem, m_impl->levelId);
        };
    };
};

void AdPreview::registerClick(unsigned int adId, std::string_view userId,
    CCMenuItemSpriteExtra* menuItem) {
    log::debug("Sending click tracking request for ad_id={}, user_id={}", adId,
        userId);

    // get argon token yum
    auto accountData = argon::getGameAccountData();

    async::spawn(
        argon::startAuth(std::move(accountData)),
        [this, adId, userId](Result<std::string> res) {
            if (res.isErr()) {
                log::warn("Auth failed: {}", res.unwrapErr());
                Notification::create("Failed to authorize with Argon",
                    NotificationIcon::Error)
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
                        log::error(
                            "Click failed with code {} for ad_id={}, user_id={}: {}",
                            res.code(), adId, userId, res.errorMessage()
                        );
                    };

                    log::debug("Click request completed for ad_id={}, user_id={}", adId, userId);
                });
            log::debug("Sent click tracking request for ad_id={}, user_id={}", adId, userId);
        });
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
            auto scene = LevelInfoLayer::scene(level, false);
            auto transitionFade = CCTransitionFade::create(0.5f, scene);
            CCDirector::sharedDirector()->pushScene(transitionFade);
            return;
        };
    };

    // prepare pending state
    m_impl->pendingKey = key;
    m_impl->pendingLevelId = levelId;
    m_impl->pendingTimeout = 10.0f; // seconds

    // show spinner on the clicked button and disable it
    if (m_impl->pendingSpinner) {
        m_impl->pendingSpinner->removeFromParent();
        m_impl->pendingSpinner = nullptr;
    };

    if (auto spinner = LoadingSpinner::create(100.f)) {
        spinner->setPosition(menuItem->getPosition());
        spinner->setVisible(true);

        m_buttonMenu->addChild(spinner);

        m_impl->pendingSpinner = spinner;
    };

    glm->getOnlineLevels(searchObj);
};

void AdPreview::update(float dt) {
    if (!m_impl->pendingKey.empty()) {
        auto glm = GameLevelManager::sharedState();
        auto stored = glm->getStoredOnlineLevels(m_impl->pendingKey.c_str());

        if (stored && stored->count() > 0) {
            auto level = typeinfo_cast<GJGameLevel*>(stored->objectAtIndex(0));

            if (level && level->m_levelID == m_impl->pendingLevelId) {
                // open level info
                auto scene = LevelInfoLayer::scene(level, false);
                auto transitionFade = CCTransitionFade::create(0.5f, scene);

                CCDirector::sharedDirector()->pushScene(transitionFade);

                // cleanup pending state and spinner
                if (m_impl->pendingSpinner) {
                    m_impl->pendingSpinner->removeFromParent();
                    m_impl->pendingSpinner = nullptr;
                };

                m_impl->pendingKey.clear();
                m_impl->pendingLevelId = -1;
                m_impl->pendingTimeout = 0.0;

                return;
            };
        };

        m_impl->pendingTimeout -= dt;
        if (m_impl->pendingTimeout <= 0.0) {
            if (m_impl->pendingSpinner) {
                m_impl->pendingSpinner->removeFromParent();
                m_impl->pendingSpinner = nullptr;
            };

            Notification::create("Level not found", NotificationIcon::Warning)->show();

            m_impl->pendingKey.clear();
            m_impl->pendingLevelId = -1;
            m_impl->pendingTimeout = 0.0;
        };
    };
};

AdPreview* AdPreview::create(unsigned int adId, int levelId, std::string userId, AdType type, unsigned int viewCount, unsigned int clickCount) {
    auto ret = new AdPreview();

    if (ret->init(adId, levelId, userId, type, viewCount, clickCount)) {
        ret->autorelease();
        return ret;
    };

    delete ret;
    return nullptr;
};