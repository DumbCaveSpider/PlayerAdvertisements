#include "../AdManager.hpp"

#include "../AdNode.hpp"

#include <Geode/Geode.hpp>

#include <Geode/ui/GeodeUI.hpp>
#include <Geode/ui/Button.hpp>

#include <Geode/utils/async.hpp>

using namespace geode::prelude;

class AdManager::Impl final {
public:
    matjson::Value adsData;
    matjson::Value userData;

    // stats
    unsigned int totalViews = 0;
    unsigned int totalClicks = 0;
    unsigned int adCount = 0;
    unsigned int globalTotalViews = 0;
    unsigned int globalTotalClicks = 0;
    unsigned int globalAdCount = 0;

    CCLabelBMFont* viewsLabel = nullptr;
    CCLabelBMFont* clicksLabel = nullptr;
    CCLabelBMFont* titleLabel = nullptr;
    CCLabelBMFont* globalViewsLabel = nullptr;
    CCLabelBMFont* globalClicksLabel = nullptr;
    CCLabelBMFont* globalAdCountLabel = nullptr;

    ScrollLayer* adsScrollLayer = nullptr;

    // pending fetch state
    std::string pendingKey;
    int pendingLevelId = -1;
    float pendingTimeout = 0.0f;
    LoadingSpinner* pendingSpinner = nullptr;

    void setupAdsList() {
        if (adsScrollLayer) {
            auto adsArray = adsData.asArray();
            if (!adsArray.isOk()) return;

            if (adsScrollLayer->m_contentLayer) adsScrollLayer->m_contentLayer->removeAllChildrenWithCleanup(true);

            for (const auto& adValue : adsArray.unwrap()) {
                if (auto node = AdNode::create(adValue, adsScrollLayer->getScaledContentWidth())) adsScrollLayer->m_contentLayer->addChild(node);
            };

            adsScrollLayer->m_contentLayer->updateLayout();
            adsScrollLayer->scrollToTop();

            // Update the title label with the correct ad count
            if (titleLabel) titleLabel->setString(fmt::format("Your Advertisements ({})", adsScrollLayer->m_contentLayer->getChildrenCount()).c_str());
        } else {
            log::error("Ads list not found");
        };
    };

    void onFetchComplete(web::WebResponse const& res) {
        if (res.ok()) {
            auto json = res.json().unwrapOrDefault();

            if (json.contains("ads")) {
                adsData = json["ads"];
                auto adsArray = adsData.asArray();
                if (adsArray.isOk()) {
                    log::info("Fetched {} ads", adsArray.unwrap().size());
                    setupAdsList();
                } else {
                    log::info("Fetched 0 ads");
                };
            };

            if (json.contains("user")) {
                userData = json["user"];
                auto username = userData["username"].asString();
                if (username.isOk()) log::info("Fetched user data for: {}", username.unwrapOrDefault());

                if (userData.contains("total_views")) {
                    auto totalViewsStat = userData["total_views"].asUInt();

                    if (totalViewsStat.isOk()) {
                        totalViews = totalViewsStat.unwrapOrDefault();
                        log::info("Total Views: {}", totalViewsStat);
                    };
                };

                if (userData.contains("total_clicks")) {
                    auto totalClicksStat = userData["total_clicks"].asUInt();

                    if (totalClicksStat.isOk()) {
                        totalClicks = totalClicksStat.unwrapOrDefault();
                        log::info("Total Clicks: {}", totalClicksStat);
                    };
                };

                // Update labels after fetching
                if (viewsLabel) viewsLabel->setString(fmt::format("Total Views: {}", totalViews).c_str());
                if (clicksLabel) clicksLabel->setString(fmt::format("Total Clicks: {}", totalClicks).c_str());
            };
        } else {
            log::error("Request failed with status code: {}", res.code());

            createQuickPopup(
                "Something went wrong",
                "Either the provided User ID is <cr>incorrect</c> or the Advertisement Manager is <co>not responding</c>.\n<cy>Do you want to open the Advertisement Manager and mod settings?</c>",
                "No", "Yes",
                [](auto, bool ok) {
                    if (ok) {
                        openSettingsPopup(getMod());
                        Notification::create("Opening Advertisement Manager", NotificationIcon::Info)->show();
                        web::openLinkInBrowser("https://ads.arcticwoof.xyz/");
                    }
                }
            );
        };
    };

    void onGlobalStatsFetchComplete(web::WebResponse const& res) {
        if (res.ok()) {
            auto json = res.json().unwrapOrDefault();

            if (json.contains("total_views")) {
                auto totalViews = json["total_views"].asUInt();

                if (totalViews.isOk()) {
                    globalTotalViews = totalViews.unwrapOrDefault();
                    log::info("Global Total Views: {}", globalTotalViews);
                };
            };

            if (json.contains("total_clicks")) {
                auto totalClicks = json["total_clicks"].asUInt();

                if (totalClicks.isOk()) {
                    globalTotalClicks = totalClicks.unwrapOrDefault();
                    log::info("Global Total Clicks: {}", globalTotalClicks);
                };
            };

            if (json.contains("ad_count")) {
                auto adCount = json["ad_count"].asUInt();

                if (adCount.isOk()) {
                    globalAdCount = adCount.unwrapOrDefault();
                    log::info("Global Ad Count: {}", globalAdCount);
                };
            };

            // Update labels with global stats
            if (globalViewsLabel) globalViewsLabel->setString(fmt::format("Views: {}", globalTotalViews).c_str());
            if (globalClicksLabel) globalClicksLabel->setString(fmt::format("Clicks: {}", globalTotalClicks).c_str());
            if (globalAdCountLabel) globalAdCountLabel->setString(fmt::format("Active Ads: {}", globalAdCount).c_str());
        } else {
            log::error("Global stats request failed with status code: {}", res.code());
        };
    };
};

AdManager::AdManager() : m_impl(std::make_unique<Impl>()) {};
AdManager::~AdManager() {};

bool AdManager::init() {
    if (!Popup::init(450.f, 280.f)) return false;

    setID("manager"_spr);
    setTitle("Advertisement Manager");

    // fetch user ads and data
    std::string urlStr = "https://ads.arcticwoof.xyz/users/fetch?id=";

    urlStr += Mod::get()->getSettingValue<std::string>("user-id");

    auto req = web::WebRequest();
    req.header("User-Agent", "PlayerAdvertisements/1.0");

    async::spawn(
        req.get(urlStr),
        [this](web::WebResponse res) {
            if (res.ok()) {
                if (auto impl = m_impl.get()) impl->onFetchComplete(res);
            } else {
                log::error("Request failed with status code: {}", res.code());
                Notification::create("Advertisement data fetch failed", NotificationIcon::Error)->show();
            };
        }
    );

    // fetch global stats
    auto globalStatsReq = web::WebRequest();
    globalStatsReq.header("User-Agent", "PlayerAdvertisements/1.0");

    async::spawn(
        globalStatsReq.get("https://ads.arcticwoof.xyz/stats/global"),
        [this](web::WebResponse res) {
            if (res.ok()) {
                if (auto impl = m_impl.get()) impl->onGlobalStatsFetchComplete(res);
            } else {
                log::error("Global stats request failed with status code: {}", res.code());
            };
        }
    );

    // @geode-ignore(unknown-resource)
    auto adsBg = NineSlice::create("geode.loader/inverseborder.png");
    adsBg->setID("ads-list-container");
    adsBg->setPosition({ 115.f, (m_mainLayer->getScaledContentHeight() / 2.f) - 10.f });
    adsBg->setContentSize({ 200.f, 200.f });

    m_mainLayer->addChild(adsBg, 5);

    // create scroll layer for ads
    m_impl->adsScrollLayer = ScrollLayer::create(adsBg->getScaledContentSize() - 5.f, true, true);
    m_impl->adsScrollLayer->setID("ads-list");
    m_impl->adsScrollLayer->setPosition(adsBg->getScaledContentSize() / 2.f);
    m_impl->adsScrollLayer->ignoreAnchorPointForPosition(false);

    auto layout = ColumnLayout::create()
        ->setGap(5.f)
        ->setAxisAlignment(AxisAlignment::End)
        ->setAutoGrowAxis(m_impl->adsScrollLayer->getScaledContentHeight());

    m_impl->adsScrollLayer->m_contentLayer->setLayout(layout);

    adsBg->addChild(m_impl->adsScrollLayer, 1);

    // title label at the top of each of the backgrounds
    auto titleLabel = CCLabelBMFont::create(fmt::format("Your Advertisements ({})", m_impl->adCount).c_str(), "goldFont.fnt");
    titleLabel->setID("ads-title-label");
    titleLabel->setPosition({ adsBg->getScaledContentWidth() / 2.f, adsBg->getScaledContentHeight() + 10.f });
    titleLabel->setScale(0.4f);

    m_impl->titleLabel = titleLabel;

    adsBg->addChild(titleLabel);

    // @geode-ignore(unknown-resource)
    auto statsBg = NineSlice::create("geode.loader/inverseborder.png");
    statsBg->setID("stats-container");
    statsBg->setPosition({ m_mainLayer->getScaledContentWidth() - 115.f, (m_mainLayer->getScaledContentHeight() / 2.f) + 52.f });
    statsBg->setContentSize({ 200.f, 75.f });

    m_mainLayer->addChild(statsBg, 5);

    auto titleLabel2 = CCLabelBMFont::create("Your Statistics", "goldFont.fnt");
    titleLabel2->setID("stats-label");
    titleLabel2->setPosition({ statsBg->getScaledContentWidth() / 2.f, statsBg->getScaledContentHeight() + 10.f });
    titleLabel2->setScale(0.4f);

    statsBg->addChild(titleLabel2);

    // total views and clicks labels on the right background
    m_impl->viewsLabel = CCLabelBMFont::create("Total Views: -", "bigFont.fnt");
    m_impl->viewsLabel->setID("views-label");
    m_impl->viewsLabel->setPosition({ statsBg->getScaledContentWidth() / 2.f, statsBg->getScaledContentHeight() - 20.f });
    m_impl->viewsLabel->setScale(0.5f);

    statsBg->addChild(m_impl->viewsLabel);

    m_impl->clicksLabel = CCLabelBMFont::create("Total Clicks: -", "bigFont.fnt");
    m_impl->clicksLabel->setID("clicks-label");
    m_impl->clicksLabel->setPosition({ statsBg->getScaledContentWidth() / 2.f, statsBg->getScaledContentHeight() - 50.f });
    m_impl->clicksLabel->setScale(0.5f);

    statsBg->addChild(m_impl->clicksLabel);

    // @geode-ignore(unknown-resource)
    auto globalBg = NineSlice::create("geode.loader/inverseborder.png");
    globalBg->setID("global-stats-container");
    globalBg->setPosition({ m_mainLayer->getScaledContentWidth() - 115.f, (m_mainLayer->getScaledContentHeight() / 2.f) - 60.f });
    globalBg->setContentSize({ 200.f, 100.f });

    m_mainLayer->addChild(globalBg, 5);

    auto titleLabel3 = CCLabelBMFont::create("Global Statistics", "goldFont.fnt");
    titleLabel3->setID("global-stats-label");
    titleLabel3->setPosition({ globalBg->getScaledContentWidth() / 2.f, globalBg->getScaledContentHeight() + 10.f });
    titleLabel3->setScale(0.4f);

    globalBg->addChild(titleLabel3);

    // global stats labels on the third background
    m_impl->globalViewsLabel = CCLabelBMFont::create("Views: -", "bigFont.fnt");
    m_impl->globalViewsLabel->setID("global-views-label");
    m_impl->globalViewsLabel->setPosition({ globalBg->getScaledContentWidth() / 2.f, globalBg->getScaledContentHeight() - 20.f });
    m_impl->globalViewsLabel->setScale(0.5f);

    globalBg->addChild(m_impl->globalViewsLabel);

    m_impl->globalClicksLabel = CCLabelBMFont::create("Clicks: -", "bigFont.fnt");
    m_impl->globalClicksLabel->setID("global-clicks-label");
    m_impl->globalClicksLabel->setPosition({ globalBg->getScaledContentWidth() / 2.f, globalBg->getScaledContentHeight() - 50.f });
    m_impl->globalClicksLabel->setScale(0.5f);

    globalBg->addChild(m_impl->globalClicksLabel);

    m_impl->globalAdCountLabel = CCLabelBMFont::create("Active Ads: -", "bigFont.fnt");
    m_impl->globalAdCountLabel->setID("global-ad-count-label");
    m_impl->globalAdCountLabel->setPosition({ globalBg->getScaledContentWidth() / 2.f, globalBg->getScaledContentHeight() - 80.f });
    m_impl->globalAdCountLabel->setScale(0.5f);

    globalBg->addChild(m_impl->globalAdCountLabel);

    // button to the website at the bottom center of the main layer popup
    auto webBtnMenu = Button::createWithNode(
        ButtonSprite::create("Manage Ads"),
        [](auto) {
            createQuickPopup(
                "Manage Advertisements?",
                "You will be redirected to <cp>GD Ads Manager</c>.\n<cy>Would you like to proceed?</c>",
                "Cancel", "Proceed",
                [](auto, bool ok) {
                    if (ok) {
                        utils::web::openLinkInBrowser("https://ads.arcticwoof.xyz/");
                        Notification::create("Opening in your browser", NotificationIcon::Info, 1.f)->show();
                    };
                }
            );
        }
    );
    webBtnMenu->setID("ads-manager-btn");
    webBtnMenu->setPosition({ m_buttonMenu->getScaledContentWidth() / 2, 0.f });

    auto modSettingsBtn = Button::createWithNode(
        CircleButtonSprite::createWithSpriteFrameName(
            // @geode-ignore(unknown-resource)
            "geode.loader/settings.png",
            1.f,
            CircleBaseColor::Green,
            CircleBaseSize::Small
        ),
        [](auto) {
            openSettingsPopup(Mod::get());
        }
    );
    modSettingsBtn->setID("mod-settings-btn");
    modSettingsBtn->setScale(0.75f);
    modSettingsBtn->setPosition(m_mainLayer->getContentSize());

    auto announcementBtn = Button::createWithNode(
        CircleButtonSprite::createWithSpriteFrameName(
            // @geode-ignore(unknown-resource)
            "geode.loader/news.png",
            0.75f,
            CircleBaseColor::Green,
            CircleBaseSize::Small
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

                        auto val = data.unwrapOrDefault();

                        std::string title = val.contains("title") && val["title"].asString().isOk()
                            ? val["title"].asString().unwrapOrDefault()
                            : "Announcement";

                        std::string content = val.contains("content") && val["content"].asString().isOk()
                            ? val["content"].asString().unwrapOrDefault()
                            : "";

                        if (auto popup = MDPopup::create(title.c_str(), content.c_str(), "Close")) popup->show();

                    } else {
                        log::error("Failed to fetch announcement: (code: {})", res.code());
                        Notification::create("Failed to fetch announcement", NotificationIcon::Error)->show();
                    };
                }
            );
        }
    );
    announcementBtn->setID("latest-announcement-btn");
    announcementBtn->setScale(0.875f);
    announcementBtn->setPosition({ m_mainLayer->getScaledContentWidth(), 0 });

    auto discordBtn = Button::createWithSpriteFrameName(
        "gj_discordIcon_001.png",
        [this](auto) {
            createQuickPopup(
                "Join our Discord?",
                "You will be redirected to <cd>Cheeseworks's support Discord server</c>.\n<cy>Would you like to proceed?</c>",
                "Cancel", "Proceed",
                [](auto, bool ok) {
                    if (ok) {
                        utils::web::openLinkInBrowser("https://www.dsc.gg/cheeseworks");
                        Notification::create("Opening in your browser", NotificationIcon::Info, 1.f)->show();
                    };
                }
            );
        }
    );
    discordBtn->setID("discord-btn");
    discordBtn->setScale(0.7f);
    discordBtn->setPosition({ 15.f, 15.f });

    auto kofiBtn = Button::createWithNode(
        BasedButtonSprite::create(
            CCSprite::createWithSpriteFrameName("kofiIcon.png"_spr),
            BaseType::Account,
            0,
            2
        ),
        [](auto) {
            createQuickPopup(
                "Support on Ko-fi",
                "Would you like to explore our <cp>Ko-fi</c> shop?\n<cg>You can buy upgrades for your ads there.</c>",
                "Cancel", "Proceed",
                [](auto, bool ok) {
                    if (ok) {
                        utils::web::openLinkInBrowser("https://ko-fi.com/playerads");
                        Notification::create("Opening in your browser", NotificationIcon::Info, 1.f)->show();
                    };
                }
            );
        }
    );
    kofiBtn->setID("kofi-btn");
    kofiBtn->setScale(0.5f);
    kofiBtn->setPosition({ discordBtn->getPositionX() + 25.f, discordBtn->getPositionY() });

    m_mainLayer->addChild(webBtnMenu);
    m_mainLayer->addChild(modSettingsBtn);
    m_mainLayer->addChild(announcementBtn);
    m_mainLayer->addChild(discordBtn);
    m_mainLayer->addChild(kofiBtn);

    this->scheduleUpdate();

    return true;
};

void AdManager::update(float dt) {
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
                m_impl->pendingTimeout = 0.0f;

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
            m_impl->pendingTimeout = 0.0f;
        };
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