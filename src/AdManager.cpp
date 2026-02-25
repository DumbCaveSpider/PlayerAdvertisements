#include "AdManager.hpp"

#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/utils/async.hpp>
#include "AdNode.hpp"

using namespace geode::prelude;

class AdManager::Impl final {
public:
    matjson::Value adsData;
    matjson::Value userData;

    // stats
    int totalViews = 0;
    int totalClicks = 0;
    int adCount = 0;
    int globalTotalViews = 0;
    int globalTotalClicks = 0;
    int globalAdCount = 0;

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
};

AdManager::AdManager() : m_impl(std::make_unique<Impl>()) {};
AdManager::~AdManager() {};

bool AdManager::init() {
    if (!Popup::init(450.f, 280.f)) return false;

    setTitle("Advertisement Manager");

    auto const winSize = CCDirector::sharedDirector()->getWinSize();

    // get the user id
    std::string const userID = Mod::get()->getSettingValue<std::string>("user-id");
    if (userID == "") {
        this->onClose(nullptr);
        createQuickPopup(
            "No User ID Set",
            "You have not set a User ID yet.\n<cy>Do you want to open the Advertisement Manager and Mod Options?</c>",
            "No", "Yes",
            [](auto, bool ok) {
                if (ok) {
                    openSettingsPopup(getMod());
                    Notification::create("Opening Advertisement Manager", NotificationIcon::Info)->show();
                    web::openLinkInBrowser("https://ads.arcticwoof.xyz/");
                };
            }
        );

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
                onGlobalStatsFetchComplete(res);
            } else {
                log::error("Global stats request failed with status code: {}", res.code());
            }
        }
    );

    // @geode-ignore(unknown-resource)
    auto bg1 = CCScale9Sprite::create("geode.loader/inverseborder.png");
    bg1->setPosition({ 115.f, winSize.height / 2 - 30.f });
    bg1->setContentSize({ 200.f, 200.f });

    m_mainLayer->addChild(bg1, 5);

    // create scroll layer for ads
    m_impl->adsScrollLayer = ScrollLayer::create({ 200.f, 199.f }, true, true);
    m_impl->adsScrollLayer->setPosition({ 0, 1 });

    bg1->addChild(m_impl->adsScrollLayer, -1);

    // @geode-ignore(unknown-resource)
    auto bg2 = CCScale9Sprite::create("geode.loader/inverseborder.png");
    bg2->setPosition({ m_mainLayer->getContentSize().width - 115.f, winSize.height / 2.f + 30.f + 2.f });
    bg2->setContentSize({ 200.f, 75.f });

    m_mainLayer->addChild(bg2, 5);

    // @geode-ignore(unknown-resource)
    auto bg3 = CCScale9Sprite::create("geode.loader/inverseborder.png");
    bg3->setPosition({ m_mainLayer->getContentSize().width - 115.f, winSize.height / 2.f - 80.f });
    bg3->setContentSize({ 200.f, 100.f });

    m_mainLayer->addChild(bg3, 5);

    // title label at the top of each of the backgrounds
    auto titleLabel = CCLabelBMFont::create(fmt::format("Your Advertisements ({})", m_impl->adCount).c_str(), "goldFont.fnt");
    titleLabel->setPosition({ bg1->getContentSize().width / 2.f, bg1->getContentSize().height + 10.f });
    titleLabel->setScale(0.4f);

    m_impl->titleLabel = titleLabel;

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
    m_impl->viewsLabel = CCLabelBMFont::create("Total Views: -", "bigFont.fnt");
    m_impl->viewsLabel->setPosition({ bg2->getContentSize().width / 2.f, bg2->getContentSize().height - 20.f });
    m_impl->viewsLabel->setScale(0.5f);

    bg2->addChild(m_impl->viewsLabel);

    m_impl->clicksLabel = CCLabelBMFont::create("Total Clicks: -", "bigFont.fnt");
    m_impl->clicksLabel->setPosition({ bg2->getContentSize().width / 2.f, bg2->getContentSize().height - 50.f });
    m_impl->clicksLabel->setScale(0.5f);

    bg2->addChild(m_impl->clicksLabel);

    // global stats labels on the third background
    m_impl->globalViewsLabel = CCLabelBMFont::create("Views: -", "bigFont.fnt");
    m_impl->globalViewsLabel->setPosition({ bg3->getContentSize().width / 2.f, bg3->getContentSize().height - 20.f });
    m_impl->globalViewsLabel->setScale(0.5f);

    bg3->addChild(m_impl->globalViewsLabel);

    m_impl->globalClicksLabel = CCLabelBMFont::create("Clicks: -", "bigFont.fnt");
    m_impl->globalClicksLabel->setPosition({ bg3->getContentSize().width / 2.f, bg3->getContentSize().height - 50.f });
    m_impl->globalClicksLabel->setScale(0.5f);

    bg3->addChild(m_impl->globalClicksLabel);

    m_impl->globalAdCountLabel = CCLabelBMFont::create("Active Ads: -", "bigFont.fnt");
    m_impl->globalAdCountLabel->setPosition({ bg3->getContentSize().width / 2.f, bg3->getContentSize().height - 80.f });
    m_impl->globalAdCountLabel->setScale(0.5f);

    bg3->addChild(m_impl->globalAdCountLabel);

    // button to the website at the bottom center of the main layer popup
    auto webBtn = ButtonSprite::create("Manage Ads");
    auto webBtnMenu = CCMenuItemSpriteExtra::create(webBtn, this, menu_selector(AdManager::onWebButton));

    auto modSettingsBtn = CCMenuItemSpriteExtra::create(
        CircleButtonSprite::createWithSpriteFrameName(
            // @geode-ignore(unknown-resource)
            "geode.loader/settings.png",
            1.f,
            CircleBaseColor::Green,
            CircleBaseSize::Small
        ),
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
        2
    );
    kofiBtnSprite->setScale(0.5f);

    auto kofiBtn = CCMenuItemSpriteExtra::create(
        kofiBtnSprite,
        this,
        menu_selector(AdManager::onKofiButton)
    );
    kofiBtn->setID("kofi-btn");
    kofiBtn->setPosition({ discordBtn->getPositionX() + 25.f, discordBtn->getPositionY() });

    auto announcementBtn = CCMenuItemSpriteExtra::create(
        CircleButtonSprite::create(
            // @geode-ignore(unknown-resource)
            CCSprite::createWithSpriteFrameName("geode.loader/news.png"),
            CircleBaseColor::Green,
            CircleBaseSize::Small
        ),
        this,
        menu_selector(AdManager::onAnnouncement)
    );
    announcementBtn->setID("latest-announcement-btn");
    announcementBtn->setPosition({ m_mainLayer->getContentSize().width, 0 });

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

                if (auto popup = MDPopup::create(title.c_str(), content.c_str(), "Close")) popup->show();

            } else {
                log::error("Failed to fetch announcement: (code: {})", res.code());
                Notification::create("Failed to fetch announcement", NotificationIcon::Error)->show();
            };
        }
    );
};

void AdManager::onDiscordButton(CCObject* sender) {
    createQuickPopup(
        "Join our Discord?",
        "You will be redirected to <co>Cheeseworks's Support Server</c>.\n<cy>Would you like to proceed?</c>",
        "Cancel", "Proceed",
        [](auto, bool ok) {
            if (ok) {
                utils::web::openLinkInBrowser("https://www.dsc.gg/cheeseworks");
                Notification::create("Opening in your browser", NotificationIcon::Info, 1.f)->show();
            };
        }
    );
};

void AdManager::onWebButton(CCObject* sender) {
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
};

void AdManager::onModSettingsButton(CCObject* sender) {
    openSettingsPopup(getMod());
};

void AdManager::populateAdsScrollLayer() {
    if (!m_impl->adsScrollLayer) return;

    auto adsArray = m_impl->adsData.asArray();
    if (!adsArray.isOk()) return;

    auto ads = adsArray.unwrap();

    m_impl->adCount = 0;

    auto layout = SimpleAxisLayout::create(Axis::Column);
    layout->setGap(5.f);
    layout->setMainAxisScaling(AxisScaling::Fit);
    m_impl->adsScrollLayer->m_contentLayer->setLayout(layout);

    if (m_impl->adsScrollLayer->m_contentLayer) m_impl->adsScrollLayer->m_contentLayer->removeAllChildrenWithCleanup(true);

    for (const auto& adValue : ads) {
        if (auto node = AdNode::create(adValue)) {
            m_impl->adsScrollLayer->m_contentLayer->addChild(node);
            m_impl->adCount++;
        };
    };

    m_impl->adsScrollLayer->m_contentLayer->updateLayout();
    m_impl->adsScrollLayer->scrollToTop();

    // Update the title label with the correct ad count
    if (m_impl->titleLabel) m_impl->titleLabel->setString(fmt::format("Your Advertisements ({})", m_impl->adCount).c_str());
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
            m_impl->adsData = jsonValue["ads"];
            auto adsArray = m_impl->adsData.asArray();
            if (adsArray.isOk()) {
                log::info("Fetched {} ads", adsArray.unwrap().size());
                populateAdsScrollLayer();
            } else {
                log::info("Fetched 0 ads");
            };
        };

        if (jsonValue.contains("user")) {
            m_impl->userData = jsonValue["user"];
            auto username = m_impl->userData["username"].asString();
            if (username) log::info("Fetched user data for: {}", username.unwrap());

            if (m_impl->userData.contains("total_views")) {
                auto totalViews = m_impl->userData["total_views"].asInt();
                if (totalViews) {
                    m_impl->totalViews = totalViews.unwrap();
                    log::info("Total Views: {}", m_impl->totalViews);
                };
            };

            if (m_impl->userData.contains("total_clicks")) {
                auto totalClicks = m_impl->userData["total_clicks"].asInt();
                if (totalClicks) {
                    m_impl->totalClicks = totalClicks.unwrap();
                    log::info("Total Clicks: {}", m_impl->totalClicks);
                };
            };

            // Update labels after fetching
            if (m_impl->viewsLabel) m_impl->viewsLabel->setString(fmt::format("Total Views: {}", m_impl->totalViews).c_str());
            if (m_impl->clicksLabel) m_impl->clicksLabel->setString(fmt::format("Total Clicks: {}", m_impl->totalClicks).c_str());
        };
    } else {
        log::error("Request failed with status code: {}", res.code());
        this->onClose(nullptr);
        createQuickPopup(
            "Something went wrong",
            "Either the provided User ID is <cr>incorrect</c> or the Advertisement Manager is <co>not responding</c>.\n<cy>Do you want to open the Advertisement Manager and Mod Options?</c>",
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

void AdManager::onGlobalStatsFetchComplete(web::WebResponse const& res) {
    if (res.ok()) {
        auto jsonStr = res.string().unwrapOr("");

        auto json = matjson::parse(jsonStr);
        if (!json.isOk()) {
            log::error("Failed to parse global stats JSON");
            return;
        };

        auto jsonValue = json.unwrap();

        if (jsonValue.contains("total_views")) {
            auto totalViews = jsonValue["total_views"].asInt();

            if (totalViews.isOk()) {
                m_impl->globalTotalViews = totalViews.unwrap();
                log::info("Global Total Views: {}", m_impl->globalTotalViews);
            };
        };

        if (jsonValue.contains("total_clicks")) {
            auto totalClicks = jsonValue["total_clicks"].asInt();

            if (totalClicks.isOk()) {
                m_impl->globalTotalClicks = totalClicks.unwrap();
                log::info("Global Total Clicks: {}", m_impl->globalTotalClicks);
            };
        };

        if (jsonValue.contains("ad_count")) {
            auto adCount = jsonValue["ad_count"].asInt();

            if (adCount.isOk()) {
                m_impl->globalAdCount = adCount.unwrap();
                log::info("Global Ad Count: {}", m_impl->globalAdCount);
            };
        };

        // Update labels with global stats
        if (m_impl->globalViewsLabel) m_impl->globalViewsLabel->setString(fmt::format("Views: {}", m_impl->globalTotalViews).c_str());
        if (m_impl->globalClicksLabel) m_impl->globalClicksLabel->setString(fmt::format("Clicks: {}", m_impl->globalTotalClicks).c_str());
        if (m_impl->globalAdCountLabel) m_impl->globalAdCountLabel->setString(fmt::format("Active Ads: {}", m_impl->globalAdCount).c_str());
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