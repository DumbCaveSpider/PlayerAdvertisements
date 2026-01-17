#include "AdManager.hpp"

#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>

using namespace geode::prelude;

class AdManager::Impl final {
public:
    matjson::Value m_adsData;
    matjson::Value m_userData;

    EventListener<web::WebTask> m_listener;
    EventListener<web::WebTask> m_globalStatsListener;
    EventListener<web::WebTask> m_announcementListener;

    // stats
    int m_totalViews = 0;
    int m_totalClicks = 0;
    int m_adCount = 0;
    int m_globalTotalViews = 0;
    int m_globalTotalClicks = 0;
    int m_globalAdCount = 0;

    CCLabelBMFont* m_viewsLabel = nullptr;
    CCLabelBMFont* m_clicksLabel = nullptr;
    CCLabelBMFont* m_titleLabel = nullptr;
    CCLabelBMFont* m_globalViewsLabel = nullptr;
    CCLabelBMFont* m_globalClicksLabel = nullptr;
    CCLabelBMFont* m_globalAdCountLabel = nullptr;

    ScrollLayer* m_adsScrollLayer = nullptr;

    // pending fetch state
    std::string m_pendingKey;
    int m_pendingLevelId = -1;
    float m_pendingTimeout = 0.0f;
    LoadingSpinner* m_pendingSpinner = nullptr;
};

AdManager::AdManager() {
    m_impl = std::make_unique<Impl>();
};

AdManager::~AdManager() {};

bool AdManager::setup() {
    setTitle("Advertisement Manager");
    auto const winSize = CCDirector::sharedDirector()->getWinSize();

    // get the user id
    std::string const userID = Mod::get()->getSettingValue<std::string>("user-id");
    if (userID == "") {
        this->onClose(nullptr);
        geode::createQuickPopup(
            "No User ID Set",
            "You have not set a User ID yet.\n<cy>Do you want to open the Advertisement Manager and Mod Options?</c>",
            "No", "Yes",
            [](auto, bool ok) {
                if (ok) {
                    openSettingsPopup(getMod());
                    Notification::create("Opening Advertisement Manager", NotificationIcon::Info)->show();
                    web::openLinkInBrowser("https://ads.arcticwoof.xyz/");
                };
            });

        return false;
    };

    // fetch user ads and data
    std::string urlStr = "https://ads.arcticwoof.xyz/users/fetch?id=";
    urlStr += userID;
    auto url = urlStr;
    auto req = web::WebRequest();
    req.header("User-Agent", "PlayerAdvertisements/1.0");
    m_impl->m_listener.bind([this](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            onFetchComplete(e);
        } else if (e->isCancelled()) {
            log::error("Request was cancelled");
            Notification::create("Advertisement data fetch was cancelled!", NotificationIcon::Warning)->show();
        };
                            });
    m_impl->m_listener.setFilter(req.get(url));

    // fetch global stats
    auto globalStatsReq = web::WebRequest();
    globalStatsReq.header("User-Agent", "PlayerAdvertisements/1.0");
    m_impl->m_globalStatsListener.bind([this](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            onGlobalStatsFetchComplete(e);
        } else if (e->isCancelled()) {
            log::error("Global stats request was cancelled");
        };
                                       });
    m_impl->m_globalStatsListener.setFilter(globalStatsReq.get("https://ads.arcticwoof.xyz/stats/global"));

    // add a background on the left side
    auto bg1 = CCScale9Sprite::create("geode.loader/inverseborder.png");
    bg1->setPosition({ 115.f, winSize.height / 2 - 30.f });
    bg1->setContentSize({ 200.f, 200.f });
    m_mainLayer->addChild(bg1, 5);

    // create scroll layer for ads
    m_impl->m_adsScrollLayer = ScrollLayer::create({ 200.f, 199.f }, true, true);
    m_impl->m_adsScrollLayer->setPosition({ 0, 1 });
    bg1->addChild(m_impl->m_adsScrollLayer, -1);

    // add a background on the right side
    auto bg2 = CCScale9Sprite::create("geode.loader/inverseborder.png");
    bg2->setPosition({ m_mainLayer->getContentSize().width - 115.f, winSize.height / 2.f + 30.f + 2.f });
    bg2->setContentSize({ 200.f, 75.f });
    m_mainLayer->addChild(bg2, 5);

    auto bg3 = CCScale9Sprite::create("geode.loader/inverseborder.png");
    bg3->setPosition({ m_mainLayer->getContentSize().width - 115.f, winSize.height / 2.f - 80.f });
    bg3->setContentSize({ 200.f, 100.f });
    m_mainLayer->addChild(bg3, 5);

    // title label at the top of each of the backgrounds
    auto titleLabel = CCLabelBMFont::create(fmt::format("Your Advertisements ({})", m_impl->m_adCount).c_str(), "goldFont.fnt");
    titleLabel->setPosition({ bg1->getContentSize().width / 2.f, bg1->getContentSize().height + 10.f });
    titleLabel->setScale(0.4f);
    m_impl->m_titleLabel = titleLabel;
    bg1->addChild(titleLabel);

    auto titleLabel2 = CCLabelBMFont::create("Your Statistics", "goldFont.fnt");
    titleLabel2->setPosition({ bg2->getContentSize().width / 2.f, bg2->getContentSize().height + 10.f });
    titleLabel2->setScale(0.4f);
    bg2->addChild(titleLabel2);

    auto titleLabel3 = CCLabelBMFont::create("Global Statistics", "goldFont.fnt");
    titleLabel3->setPosition({ bg3->getContentSize().width / 2.f, bg3->getContentSize().height + 10.f });
    titleLabel3->setScale(0.4f);
    bg3->addChild(titleLabel3);

    // total views and clicks labels on the right background
    m_impl->m_viewsLabel = CCLabelBMFont::create("Total Views: -", "bigFont.fnt");
    m_impl->m_viewsLabel->setPosition({ bg2->getContentSize().width / 2.f, bg2->getContentSize().height - 20.f });
    m_impl->m_viewsLabel->setScale(0.5f);
    bg2->addChild(m_impl->m_viewsLabel);

    m_impl->m_clicksLabel = CCLabelBMFont::create("Total Clicks: -", "bigFont.fnt");
    m_impl->m_clicksLabel->setPosition({ bg2->getContentSize().width / 2.f, bg2->getContentSize().height - 50.f });
    m_impl->m_clicksLabel->setScale(0.5f);
    bg2->addChild(m_impl->m_clicksLabel);

    // global stats labels on the third background
    m_impl->m_globalViewsLabel = CCLabelBMFont::create("Views: -", "bigFont.fnt");
    m_impl->m_globalViewsLabel->setPosition({ bg3->getContentSize().width / 2.f, bg3->getContentSize().height - 20.f });
    m_impl->m_globalViewsLabel->setScale(0.5f);
    bg3->addChild(m_impl->m_globalViewsLabel);

    m_impl->m_globalClicksLabel = CCLabelBMFont::create("Clicks: -", "bigFont.fnt");
    m_impl->m_globalClicksLabel->setPosition({ bg3->getContentSize().width / 2.f, bg3->getContentSize().height - 50.f });
    m_impl->m_globalClicksLabel->setScale(0.5f);
    bg3->addChild(m_impl->m_globalClicksLabel);

    m_impl->m_globalAdCountLabel = CCLabelBMFont::create("Active Ads: -", "bigFont.fnt");
    m_impl->m_globalAdCountLabel->setPosition({ bg3->getContentSize().width / 2.f, bg3->getContentSize().height - 80.f });
    m_impl->m_globalAdCountLabel->setScale(0.5f);
    bg3->addChild(m_impl->m_globalAdCountLabel);

    // button to the website at the bottom center of the main layer popup
    auto webBtn = ButtonSprite::create("Manage Ads", 0, false, "goldFont.fnt", "GJ_button_01.png", 0.f, 0.8f);
    auto webBtnMenu = CCMenuItemSpriteExtra::create(webBtn, this, menu_selector(AdManager::onWebButton));

    // button to open mod settings
    auto modSettingsBtnSprite = CircleButtonSprite::createWithSpriteFrameName(
        // @geode-ignore(unknown-resource)
        "geode.loader/settings.png",
        1.f,
        CircleBaseColor::Green,
        CircleBaseSize::Medium);
    modSettingsBtnSprite->setScale(0.75f);

    auto modSettingsBtn = CCMenuItemSpriteExtra::create(
        modSettingsBtnSprite,
        this,
        menu_selector(AdManager::onModSettingsButton));
    modSettingsBtn->setID("mod-settings-btn");
    modSettingsBtn->setPosition(m_mainLayer->getContentSize());

    auto discordBtnSprite = CCSprite::createWithSpriteFrameName("gj_discordIcon_001.png");
    discordBtnSprite->setScale(0.7f);
    auto discordBtn = CCMenuItemSpriteExtra::create(
        discordBtnSprite,
        this,
        menu_selector(AdManager::onDiscordButton));
    discordBtn->setID("discord-btn");
    discordBtn->setPosition({ 15.f, 15.f });

    auto kofiBtnSprite = BasedButtonSprite::create(
        CCSprite::create("kofiLogo.png"_spr),
        BaseType::Account,
        0,
        2);
    kofiBtnSprite->setScale(0.5f);

    auto kofiBtn = CCMenuItemSpriteExtra::create(
        kofiBtnSprite,
        this,
        menu_selector(AdManager::onKofiButton));
    kofiBtn->setID("kofi-btn");
    kofiBtn->setPosition({ discordBtn->getPositionX() + 25.f, discordBtn->getPositionY() });

    // @geode-ignore(unknown-resource)
    auto announcementBtnSprite = CCSprite::createWithSpriteFrameName("geode.loader/news.png");
    auto announcementBtn = CCMenuItemSpriteExtra::create(
        announcementBtnSprite,
        this,
        menu_selector(AdManager::onAnnouncement));
    announcementBtn->setID("latest-announcement-btn");
    announcementBtn->setPosition({ m_mainLayer->getContentSize().width - 15, 18 });

    auto menu = CCMenu::create();
    menu->setPosition({ 0.f, 0.f });
    menu->setContentSize(m_mainLayer->getContentSize());

    menu->addChild(webBtnMenu);
    menu->addChild(modSettingsBtn);
    menu->addChild(discordBtn);
    menu->addChild(kofiBtn);
    menu->addChild(announcementBtn);

    webBtnMenu->setPosition({ menu->getContentSize().width / 2, 0.f });
    m_mainLayer->addChild(menu, 6);

    this->scheduleUpdate();

    return true;
};

void AdManager::onKofiButton(CCObject* sender) {
    geode::createQuickPopup(
        "Support on Ko-fi",
        "Would you like to explore our <cp>Ko-fi</c> shop?\n<cg>You can buy upgrades for your ads there.</c>",
        "Cancel", "Proceed",
        [](auto, bool ok) {
            if (ok) {
                geode::utils::web::openLinkInBrowser("https://ko-fi.com/playerads");
                Notification::create("Opening in your browser", NotificationIcon::Info, 1.f)->show();
            };
        });
};

// open LevelInfo if stored otherwise prepare pending state and request
void AdManager::tryOpenOrFetchLevel(CCMenuItemSpriteExtra* menuItem, int levelId) {
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

    auto spinner = LoadingSpinner::create(36.f);
    if (spinner) {
        spinner->setPosition(menuItem->getPosition());
        spinner->setVisible(true);
        if (menuItem->getParent()) {
            menuItem->getParent()->addChild(spinner);
        } else if (m_impl->m_adsScrollLayer && m_impl->m_adsScrollLayer->m_contentLayer) {
            m_impl->m_adsScrollLayer->m_contentLayer->addChild(spinner);
        }
        m_impl->m_pendingSpinner = spinner;
    }

    glm->getOnlineLevels(searchObj);
}

void AdManager::update(float dt) {
    if (!m_impl->m_pendingKey.empty()) {
        auto glm = GameLevelManager::sharedState();
        auto stored = glm->getStoredOnlineLevels(m_impl->m_pendingKey.c_str());
        if (stored && stored->count() > 0) {
            auto level = static_cast<GJGameLevel*>(stored->objectAtIndex(0));
            if (level && level->m_levelID == m_impl->m_pendingLevelId) {
                // open level info
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
                m_impl->m_pendingTimeout = 0.0f;
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
            m_impl->m_pendingTimeout = 0.0f;
        }
    }
}

void AdManager::onAnnouncement(CCObject* sender) {
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

                if (auto popup = geode::MDPopup::create(title.c_str(), content.c_str(), "Close")) {
                    popup->show();
                }
            } else {
                log::error("Failed to fetch announcement: (code: {})", res->code());
                Notification::create("Failed to fetch announcement", NotificationIcon::Error)
                    ->show();
            };
        };
                                        });
    m_impl->m_announcementListener.setFilter(task);
};

void AdManager::onDiscordButton(CCObject* sender) {
    geode::createQuickPopup(
        "Join our Discord?",
        "You will be redirected to <co>Cheeseworks's Support Server</c>.\n<cy>Would you like to proceed?</c>",
        "Cancel", "Proceed",
        [](auto, bool ok) {
            if (ok) {
                geode::utils::web::openLinkInBrowser("https://www.dsc.gg/cheeseworks");
                Notification::create("Opening in your browser", NotificationIcon::Info, 1.f)->show();
            };
        });
};

void AdManager::onWebButton(CCObject* sender) {
    geode::createQuickPopup(
        "Manage Advertisements?",
        "You will be redirected to <cp>GD Ads Manager</c>.\n<cy>Would you like to proceed?</c>",
        "Cancel", "Proceed",
        [](auto, bool ok) {
            if (ok) {
                geode::utils::web::openLinkInBrowser("https://ads.arcticwoof.xyz/");
                Notification::create("Opening in your browser", NotificationIcon::Info, 1.f)->show();
            };
        });
};

void AdManager::onModSettingsButton(CCObject* sender) {
    openSettingsPopup(getMod());
};

void AdManager::onPlayButton(CCObject* sender) {
    auto menuItem = typeinfo_cast<CCMenuItemSpriteExtra*>(sender);
    int levelId = menuItem ? menuItem->getTag() : -1;

    if (PlayLayer::get()) {
        geode::createQuickPopup(
            "Warning",
            "You are already inside of a level, attempt to play another level before closing the current level will <cr>crash your game</c>.\n<cy>Do you still want to proceed?</c>",
            "Cancel", "Proceed",
            [this, sender](auto, bool btn) {
                if (btn) {
                    auto menuItem = typeinfo_cast<CCMenuItemSpriteExtra*>(sender);
                    if (menuItem) this->tryOpenOrFetchLevel(menuItem, menuItem->getTag());
                };
            });
    } else {
        auto menuItemPtr = typeinfo_cast<CCMenuItemSpriteExtra*>(sender);
        if (menuItemPtr) this->tryOpenOrFetchLevel(menuItemPtr, menuItemPtr->getTag());
    };
};

void AdManager::populateAdsScrollLayer() {
    if (!m_impl->m_adsScrollLayer) return;

    auto adsArray = m_impl->m_adsData.asArray();
    if (!adsArray.isOk()) return;

    auto ads = adsArray.unwrap();

    m_impl->m_adCount = 0;

    auto layout = SimpleAxisLayout::create(Axis::Column);
    layout->setGap(5.f);
    layout->setMainAxisScaling(AxisScaling::Fit);
    m_impl->m_adsScrollLayer->m_contentLayer->setLayout(layout);

    for (const auto& adValue : ads) {
        auto adContainer = CCNode::create();
        adContainer->setContentSize({ 200.f, 85.f });
        adContainer->setAnchorPoint({ 0.5f, 0.5f });

        auto clipNode = CCClippingNode::create();
        clipNode->setContentSize({ 200.f, 85.f });
        clipNode->setAnchorPoint({ 0.0f, 0.0f });
        clipNode->setPosition({ 0.0f, 0.0f });

        auto stencil = CCScale9Sprite::create("square02_001.png");
        stencil->setContentSize({ 200.f, 85.f });
        stencil->setAnchorPoint({ 0.0f, 0.0f });
        stencil->setPosition({ 0.0f, 0.0f });
        clipNode->setStencil(stencil);
        clipNode->setAlphaThreshold(0.1f);

        // @geode-ignore(unknown-resource)
        auto bg = CCScale9Sprite::create("geode.loader/black-square.png");
        bg->setContentSize({ stencil->getContentSize() });
        bg->setAnchorPoint({ 0.0f, 0.0f });
        adContainer->addChild(bg, 1);

        auto imageUrl = adValue["image_url"].asString();
        auto lazySprite = LazySprite::create({ 200.f, 85.f });
        if (imageUrl.isOk()) lazySprite->loadFromUrl(imageUrl.unwrap(), LazySprite::Format::kFmtWebp, true);

        lazySprite->setContentSize({ 200.f, 85.f });
        lazySprite->setScale(0.55f);
        lazySprite->setPosition({ adContainer->getContentSize().width / 2, adContainer->getContentSize().height / 2 });
        clipNode->addChild(lazySprite);

        adContainer->addChild(clipNode);

        // ad data
        auto adId = adValue["ad_id"].asInt();
        auto levelId = adValue["level_id"].asInt();
        auto type = adValue["type"].asInt();
        auto viewCount = adValue["views"].asInt();
        auto clickCount = adValue["clicks"].asInt();
        auto createdAt = adValue["created_at"].asString();
        auto pending = adValue["pending"].asBool();

        // labels
        std::string adIdStr = adId.isOk() ? numToString(adId.unwrap()) : "N/A";
        auto adLabel = CCLabelBMFont::create(("Ad ID: " + adIdStr).c_str(), "goldFont.fnt");
        adLabel->setPosition({ adContainer->getContentSize().width / 2, adContainer->getContentSize().height - 10.f });
        adLabel->setAnchorPoint({ 0.5f, 0.5f });
        adLabel->setScale(0.4f);
        adContainer->addChild(adLabel, 2);

        std::string levelIdStr = levelId.isOk() ? numToString(levelId.unwrap()) : "N/A";
        auto levelLabel = CCLabelBMFont::create(("Level ID: " + levelIdStr).c_str(), "goldFont.fnt");
        levelLabel->setPosition({ adContainer->getContentSize().width / 2, adContainer->getContentSize().height - 25.f });
        levelLabel->setAnchorPoint({ 0.5f, 0.5f });
        levelLabel->setScale(0.4f);
        adContainer->addChild(levelLabel, 2);

        // views and clicks
        std::string viewsStr = viewCount.isOk() ? numToString(viewCount.unwrap()) : "0";
        auto viewsLabel = CCLabelBMFont::create(("Views: " + viewsStr).c_str(), "goldFont.fnt");
        viewsLabel->setPosition({ adContainer->getContentSize().width / 4, adContainer->getContentSize().height - 40.f });
        viewsLabel->setAnchorPoint({ 0.5f, 0.5f });
        viewsLabel->setColor({ 255, 125, 0 });
        viewsLabel->setScale(0.4f);
        adContainer->addChild(viewsLabel, 2);

        std::string clicksStr = clickCount.isOk() ? numToString(clickCount.unwrap()) : "0";
        auto clicksLabel = CCLabelBMFont::create(("Clicks: " + clicksStr).c_str(), "goldFont.fnt");
        clicksLabel->setPosition({ adContainer->getContentSize().width / 4 * 3, adContainer->getContentSize().height - 40.f });
        clicksLabel->setAnchorPoint({ 0.5f, 0.5f });
        clicksLabel->setColor({ 0, 175, 255 });
        clicksLabel->setScale(0.4f);
        adContainer->addChild(clicksLabel, 2);

        // pending label
        auto pendingLabel = CCLabelBMFont::create("Pending", "goldFont.fnt");
        pendingLabel->setPosition({ 5.f, 10.f });
        pendingLabel->setAnchorPoint({ 0.f, 0.5f });
        pendingLabel->setColor({ 255, 0, 0 });
        pendingLabel->setScale(0.3f);
        adContainer->addChild(pendingLabel, 2);

        if (pending.isOk() && !pending.unwrap()) {
            pendingLabel->setString("Approved");
            pendingLabel->setColor({ 0, 255, 0 });
        };

        // created at
        auto createdAtLabel = CCLabelBMFont::create(("Created at: " + createdAt.unwrap()).c_str(), "chatFont.fnt");
        createdAtLabel->setPosition({ adContainer->getContentSize().width / 2, 10.f });
        createdAtLabel->setAnchorPoint({ 0.5f, 0.5f });
        createdAtLabel->setScale(0.3f);
        adContainer->addChild(createdAtLabel, 2);

        // play button at the bottom right
        auto playBtnSprite = CCSprite::createWithSpriteFrameName("GJ_playBtn2_001.png");
        playBtnSprite->setScale(0.35f);
        auto playBtn = CCMenuItemSpriteExtra::create(
            playBtnSprite,
            this,
            menu_selector(AdManager::onPlayButton));
        playBtn->setID("play-btn");
        playBtn->setTag(levelId.isOk() ? levelId.unwrap() : 0);

        auto playMenu = CCMenu::create();
        playMenu->setPosition({ adContainer->getContentSize().width / 2, adContainer->getContentSize().height / 2 - 5 });
        playMenu->addChild(playBtn);
        adContainer->addChild(playMenu, 3);

        m_impl->m_adsScrollLayer->m_contentLayer->addChild(adContainer);
        m_impl->m_adCount++;
    };

    m_impl->m_adsScrollLayer->m_contentLayer->updateLayout();
    m_impl->m_adsScrollLayer->scrollToTop();

    // Update the title label with the correct ad count
    if (m_impl->m_titleLabel) m_impl->m_titleLabel->setString(fmt::format("Your Advertisements ({})", m_impl->m_adCount).c_str());
};

void AdManager::onFetchComplete(web::WebTask::Event* event) {
    if (auto res = event->getValue()) {
        if (res->ok()) {
            auto jsonStr = res->string().unwrapOr("");

            auto json = matjson::parse(jsonStr);
            if (!json.isOk()) {
                log::error("Failed to parse JSON");
                return;
            };

            auto jsonValue = json.unwrap();

            if (jsonValue.contains("ads")) {
                m_impl->m_adsData = jsonValue["ads"];
                auto adsArray = m_impl->m_adsData.asArray();
                if (adsArray.isOk()) {
                    log::info("Fetched {} ads", adsArray.unwrap().size());
                    populateAdsScrollLayer();
                } else {
                    log::info("Fetched 0 ads");
                };
            };

            if (jsonValue.contains("user")) {
                m_impl->m_userData = jsonValue["user"];
                auto username = m_impl->m_userData["username"].asString();
                if (username) log::info("Fetched user data for: {}", username.unwrap());

                if (m_impl->m_userData.contains("total_views")) {
                    auto totalViews = m_impl->m_userData["total_views"].asInt();
                    if (totalViews) {
                        m_impl->m_totalViews = totalViews.unwrap();
                        log::info("Total Views: {}", m_impl->m_totalViews);
                    };
                };

                if (m_impl->m_userData.contains("total_clicks")) {
                    auto totalClicks = m_impl->m_userData["total_clicks"].asInt();
                    if (totalClicks) {
                        m_impl->m_totalClicks = totalClicks.unwrap();
                        log::info("Total Clicks: {}", m_impl->m_totalClicks);
                    };
                };

                // Update labels after fetching
                if (m_impl->m_viewsLabel) m_impl->m_viewsLabel->setString(fmt::format("Total Views: {}", m_impl->m_totalViews).c_str());
                if (m_impl->m_clicksLabel) m_impl->m_clicksLabel->setString(fmt::format("Total Clicks: {}", m_impl->m_totalClicks).c_str());
            };
        } else {
            log::error("Request failed with status code: {}", res->code());
            this->onClose(nullptr);
            geode::createQuickPopup(
                "Something went wrong",
                "Either the provided User ID is <cr>incorrect</c> or the Advertisement Manager is <co>not responding</c>.\n<cy>Do you want to open the Advertisement Manager and Mod Options?</c>",
                "No", "Yes",
                [](auto, bool ok) {
                    if (ok) {
                        openSettingsPopup(getMod());
                        Notification::create("Opening Advertisement Manager", NotificationIcon::Info)->show();
                        web::openLinkInBrowser("https://ads.arcticwoof.xyz/");
                    };
                });
        };
    };
};

void AdManager::onGlobalStatsFetchComplete(web::WebTask::Event* event) {
    if (auto res = event->getValue()) {
        if (res->ok()) {
            auto jsonStr = res->string().unwrapOr("");

            auto json = matjson::parse(jsonStr);
            if (!json.isOk()) {
                log::error("Failed to parse global stats JSON");
                return;
            }

            auto jsonValue = json.unwrap();

            if (jsonValue.contains("total_views")) {
                auto totalViews = jsonValue["total_views"].asInt();
                if (totalViews) {
                    m_impl->m_globalTotalViews = totalViews.unwrap();
                    log::info("Global Total Views: {}", m_impl->m_globalTotalViews);
                }
            }

            if (jsonValue.contains("total_clicks")) {
                auto totalClicks = jsonValue["total_clicks"].asInt();
                if (totalClicks) {
                    m_impl->m_globalTotalClicks = totalClicks.unwrap();
                    log::info("Global Total Clicks: {}", m_impl->m_globalTotalClicks);
                }
            }

            if (jsonValue.contains("ad_count")) {
                auto adCount = jsonValue["ad_count"].asInt();
                if (adCount) {
                    m_impl->m_globalAdCount = adCount.unwrap();
                    log::info("Global Ad Count: {}", m_impl->m_globalAdCount);
                };
            };

            // Update labels with global stats
            if (m_impl->m_globalViewsLabel) m_impl->m_globalViewsLabel->setString(fmt::format("Views: {}", m_impl->m_globalTotalViews).c_str());
            if (m_impl->m_globalClicksLabel) m_impl->m_globalClicksLabel->setString(fmt::format("Clicks: {}", m_impl->m_globalTotalClicks).c_str());
            if (m_impl->m_globalAdCountLabel) m_impl->m_globalAdCountLabel->setString(fmt::format("Active Ads: {}", m_impl->m_globalAdCount).c_str());
        } else {
            log::error("Global stats request failed with status code: {}", res->code());
        };
    };
};

AdManager* AdManager::create() {
    auto ret = new AdManager();
    if (ret->initAnchored(450.f, 280.f)) {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};