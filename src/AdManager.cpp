#include "AdManager.hpp"

#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/utils/async.hpp>
#include "AdNode.hpp"

using namespace geode::prelude;

class AdManager::Impl final {
public:
    matjson::Value m_adsData;
    matjson::Value m_userData;

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

bool AdManager::init() {
    if (!Popup::init(450.f, 280.f)) {
        return false;
    }
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
    async::spawn(
        req.get(url),
        [this](web::WebResponse res) {
            if (res.ok()) {
                onFetchComplete(res);
            } else {
                log::error("Request failed with status code: {}", res.code());
                Notification::create("Advertisement data fetch failed", NotificationIcon::Error)->show();
            }
        }
    );

    // fetch global stats
    auto globalStatsReq = web::WebRequest();
    globalStatsReq.header("User-Agent", "PlayerAdvertisements/1.0");
    async::spawn(
        globalStatsReq.get("https://ads.arcticwoof.xyz/stats/global"),
        [this](web::WebResponse res) {
            if (res.ok()) {
                onGlobalStatsFetchComplete(res);
            } else {
                log::error("Global stats request failed with status code: {}", res.code());
            }
        }
    );

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
    auto webBtn = ButtonSprite::create("Manage Ads");
    auto webBtnMenu = CCMenuItemSpriteExtra::create(webBtn, this, menu_selector(AdManager::onWebButton));

    // button to open mod settings
    auto modSettingsBtnSprite = CircleButtonSprite::createWithSpriteFrameName(
        // @geode-ignore(unknown-resource)
        "geode.loader/settings.png",
        1.f,
        CircleBaseColor::Green,
        CircleBaseSize::Small);

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
    auto announcementBtnSprite = CircleButtonSprite::create(CCSprite::createWithSpriteFrameName("geode.loader/news.png"), CircleBaseColor::Green, CircleBaseSize::Small);
    auto announcementBtn = CCMenuItemSpriteExtra::create(
        announcementBtnSprite,
        this,
        menu_selector(AdManager::onAnnouncement));
    announcementBtn->setID("latest-announcement-btn");
    announcementBtn->setPosition({ m_mainLayer->getContentSize().width, 0});

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

    async::spawn(
        request.get("https://ads.arcticwoof.xyz/api/announcement"),
        [this](web::WebResponse res) {
            if (res.ok()) {
                auto data = res.json();
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
                log::error("Failed to fetch announcement: (code: {})", res.code());
                Notification::create("Failed to fetch announcement", NotificationIcon::Error)
                    ->show();
            }
        }
    );
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

    if (m_impl->m_adsScrollLayer->m_contentLayer) {
        m_impl->m_adsScrollLayer->m_contentLayer->removeAllChildrenWithCleanup(true);
    }

    for (const auto& adValue : ads) {
        auto node = AdNode::create(adValue, this);
        if (node) {
            m_impl->m_adsScrollLayer->m_contentLayer->addChild(node);
            m_impl->m_adCount++;
        }
    }

    m_impl->m_adsScrollLayer->m_contentLayer->updateLayout();
    m_impl->m_adsScrollLayer->scrollToTop();

    // Update the title label with the correct ad count
    if (m_impl->m_titleLabel) m_impl->m_titleLabel->setString(fmt::format("Your Advertisements ({})", m_impl->m_adCount).c_str());
};

void AdManager::onFetchComplete(web::WebResponse const& res) {
    if (res.ok()) {
            auto jsonStr = res.string().unwrapOr("");

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
            log::error("Request failed with status code: {}", res.code());
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
                    }
                });
    }
}

void AdManager::onGlobalStatsFetchComplete(web::WebResponse const& res) {
    if (res.ok()) {
        auto jsonStr = res.string().unwrapOr("");

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
        log::error("Global stats request failed with status code: {}", res.code());
    };
};

AdManager* AdManager::create() {
    auto ret = new AdManager();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    };

    delete ret;
    return nullptr;
};