#include "AdManager.hpp"

#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>

using namespace geode::prelude;

bool AdManager::setup() {
    setTitle("Advertisement Manager");
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    // get the user id
    std::string userID = Mod::get()->getSettingValue<std::string>("user-id");
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
    m_listener.bind([this](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            onFetchComplete(e);
        } else if (e->isCancelled()) {
            log::error("Request was cancelled");
            Notification::create("Advertisement data fetch was cancelled!", NotificationIcon::Warning)->show();
        }; });
        m_listener.setFilter(req.get(url));

        // fetch global stats
        auto globalStatsReq = web::WebRequest();
        globalStatsReq.header("User-Agent", "PlayerAdvertisements/1.0");
        m_globalStatsListener.bind([this](web::WebTask::Event* e) {
            if (auto res = e->getValue()) {
                onGlobalStatsFetchComplete(e);
            } else if (e->isCancelled()) {
                log::error("Global stats request was cancelled");
            }; });
            m_globalStatsListener.setFilter(globalStatsReq.get("https://ads.arcticwoof.xyz/stats/global"));

            // add a background on the left side
            // @geode-ignore(unknown-resource)
            auto bg1 = CCScale9Sprite::create("geode.loader/inverseborder.png");
            bg1->setPosition({ 115.f, winSize.height / 2 - 30.f });
            bg1->setContentSize({ 200.f, 200.f });
            m_mainLayer->addChild(bg1, 5);

            // create scroll layer for ads
            m_adsScrollLayer = ScrollLayer::create({ 200.f, 199.f }, true, true);
            m_adsScrollLayer->setPosition({ 0, 1 });
            bg1->addChild(m_adsScrollLayer, -1);

            // add a background on the right side
            // @geode-ignore(unknown-resource)
            auto bg2 = CCScale9Sprite::create("geode.loader/inverseborder.png");
            bg2->setPosition({ m_mainLayer->getContentSize().width - 115.f, winSize.height / 2 + 30.f + 2.f });
            bg2->setContentSize({ 200.f, 75.f });
            m_mainLayer->addChild(bg2, 5);

            // @geode-ignore(unknown-resource)
            auto bg3 = CCScale9Sprite::create("geode.loader/inverseborder.png");
            bg3->setPosition({ m_mainLayer->getContentSize().width - 115.f, winSize.height / 2 - 80.f });
            bg3->setContentSize({ 200.f, 100.f });
            m_mainLayer->addChild(bg3, 5);

            // title label at the top of each of the backgrounds
            auto titleLabel = CCLabelBMFont::create(fmt::format("Your Advertisements ({})", m_adCount).c_str(), "goldFont.fnt");
            titleLabel->setPosition({ bg1->getContentSize().width / 2, bg1->getContentSize().height + 10 });
            titleLabel->setScale(0.4f);
            m_titleLabel = titleLabel;
            bg1->addChild(titleLabel);

            auto titleLabel2 = CCLabelBMFont::create("Your Statistics", "goldFont.fnt");
            titleLabel2->setPosition({ bg2->getContentSize().width / 2, bg2->getContentSize().height + 10 });
            titleLabel2->setScale(0.4f);
            bg2->addChild(titleLabel2);

            auto titleLabel3 = CCLabelBMFont::create("Global Statistics", "goldFont.fnt");
            titleLabel3->setPosition({ bg3->getContentSize().width / 2, bg3->getContentSize().height + 10 });
            titleLabel3->setScale(0.4f);
            bg3->addChild(titleLabel3);

            // total views and clicks labels on the right background
            m_viewsLabel = CCLabelBMFont::create("Total Views: -", "bigFont.fnt");
            m_viewsLabel->setPosition({ bg2->getContentSize().width / 2, bg2->getContentSize().height - 20.f });
            m_viewsLabel->setScale(0.5f);
            bg2->addChild(m_viewsLabel);

            m_clicksLabel = CCLabelBMFont::create("Total Clicks: -", "bigFont.fnt");
            m_clicksLabel->setPosition({ bg2->getContentSize().width / 2, bg2->getContentSize().height - 50.f });
            m_clicksLabel->setScale(0.5f);
            bg2->addChild(m_clicksLabel);

            // global stats labels on the third background
            m_globalViewsLabel = CCLabelBMFont::create("Views: -", "bigFont.fnt");
            m_globalViewsLabel->setPosition({ bg3->getContentSize().width / 2, bg3->getContentSize().height - 20.f });
            m_globalViewsLabel->setScale(0.5f);
            bg3->addChild(m_globalViewsLabel);

            m_globalClicksLabel = CCLabelBMFont::create("Clicks: -", "bigFont.fnt");
            m_globalClicksLabel->setPosition({ bg3->getContentSize().width / 2, bg3->getContentSize().height - 50.f });
            m_globalClicksLabel->setScale(0.5f);
            bg3->addChild(m_globalClicksLabel);

            m_globalAdCountLabel = CCLabelBMFont::create("Active Ads: -", "bigFont.fnt");
            m_globalAdCountLabel->setPosition({ bg3->getContentSize().width / 2, bg3->getContentSize().height - 80.f });
            m_globalAdCountLabel->setScale(0.5f);
            bg3->addChild(m_globalAdCountLabel);

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

            return true;
}

AdManager* AdManager::create() {
    auto ret = new AdManager();

    if (ret && ret->initAnchored(450.f, 280.f)) {
        ret->autorelease();
        return ret;
    };

    CC_SAFE_DELETE(ret);
    return nullptr;
};

void AdManager::onKofiButton(CCObject* sender) {
    geode::createQuickPopup(
        "Support on Ko-fi",
        "Would you like to explore our <cp>Ko-fi</c> shop? You can buy upgrades for your ads through here.",
        "Cancel", "Proceed",
        [](auto, bool ok) {
            if (ok) {
                geode::utils::web::openLinkInBrowser("https://ko-fi.com/playerads");
                Notification::create("Opening in your browser", NotificationIcon::Info, 1.f)->show();
            };
        });
};

void AdManager::onAnnouncement(CCObject* sender) {
    // fetch from /api/announcement
    auto request = web::WebRequest();
    request.userAgent("PlayerAdvertisements/1.0");
    request.timeout(std::chrono::seconds(15));
    request.header("Content-Type", "application/json");

    auto task = request.get("https://ads.arcticwoof.xyz/api/announcement");
    m_announcementListener.bind([this](web::WebTask::Event* e) {
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
    m_announcementListener.setFilter(task);
};

void AdManager::onDiscordButton(CCObject* sender) {
    geode::createQuickPopup(
        "Join our Discord?",
        "You will be redirected to <cp>ArcticWoof's Discord Server</c>.\n<cy>Would you like to open the link?</c>",
        "Cancel", "Proceed",
        [](auto, bool ok) {
            if (ok) {
                geode::utils::web::openLinkInBrowser("https://discord.gg/gXcppxTNxC");
                Notification::create("Opening in your browser", NotificationIcon::Info, 1.f)->show();
            };
        });
};

void AdManager::onWebButton(CCObject* sender) {
    geode::createQuickPopup(
        "Manage Advertisements?",
        "You will be redirected to <cp>GD Advertisement Manager</c>.\n<cy>Would you like to open the link?</c>",
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
    auto menuItem = static_cast<CCMenuItemSpriteExtra*>(sender);
    int levelId = menuItem->getTag();

    auto searchStr = std::to_string(levelId);

    if (PlayLayer::get()) {
        geode::createQuickPopup(
            "Warning",
            "You are already inside of a level, attempt to play another level before closing the current level will <cr>crash your game</c>.\n<cy>Do you still want to proceed?</c>",
            "Cancel", "Proceed",
            [this, sender, searchStr](auto, bool btn) {
                if (btn) {
                    auto scene = LevelBrowserLayer::scene(GJSearchObject::create(SearchType::Search, searchStr));
                    CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, scene));
                };
            });
    } else {
        auto scene = LevelBrowserLayer::scene(GJSearchObject::create(SearchType::Search, searchStr));
        CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, scene));
    };
};

void AdManager::populateAdsScrollLayer() {
    if (!m_adsScrollLayer)
        return;

    auto adsArray = m_adsData.asArray();
    if (!adsArray.isOk())
        return;

    auto ads = adsArray.unwrap();

    m_adCount = 0;

    auto layout = SimpleAxisLayout::create(Axis::Column);
    layout->setGap(5.f);
    layout->setMainAxisScaling(AxisScaling::Fit);
    m_adsScrollLayer->m_contentLayer->setLayout(layout);

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

        m_adsScrollLayer->m_contentLayer->addChild(adContainer);
        m_adCount++;
    };

    m_adsScrollLayer->m_contentLayer->updateLayout();
    m_adsScrollLayer->scrollToTop();

    // Update the title label with the correct ad count
    if (m_titleLabel) m_titleLabel->setString(fmt::format("Your Advertisements ({})", m_adCount).c_str());
};

void AdManager::onFetchComplete(web::WebTask::Event* event) {
    if (auto res = event->getValue()) {
        if (res->ok()) {
            auto jsonStr = res->string().unwrapOr("");

            auto json = matjson::parse(jsonStr);
            if (!json.isOk()) {
                log::error("Failed to parse JSON");
                return;
            }

            auto jsonValue = json.unwrap();

            if (jsonValue.contains("ads")) {
                m_adsData = jsonValue["ads"];
                auto adsArray = m_adsData.asArray();
                if (adsArray.isOk()) {
                    log::info("Fetched {} ads", adsArray.unwrap().size());
                    populateAdsScrollLayer();
                } else {
                    log::info("Fetched 0 ads");
                }
            }

            if (jsonValue.contains("user")) {
                m_userData = jsonValue["user"];
                auto username = m_userData["username"].asString();
                if (username) {
                    log::info("Fetched user data for: {}", username.unwrap());
                }
                if (m_userData.contains("total_views")) {
                    auto totalViews = m_userData["total_views"].asInt();
                    if (totalViews) {
                        m_totalViews = totalViews.unwrap();
                        log::info("Total Views: {}", m_totalViews);
                    }
                }
                if (m_userData.contains("total_clicks")) {
                    auto totalClicks = m_userData["total_clicks"].asInt();
                    if (totalClicks) {
                        m_totalClicks = totalClicks.unwrap();
                        log::info("Total Clicks: {}", m_totalClicks);
                    }
                }
                // Update labels after fetching
                if (m_viewsLabel)
                    m_viewsLabel->setString(fmt::format("Total Views: {}", m_totalViews).c_str());
                if (m_clicksLabel)
                    m_clicksLabel->setString(fmt::format("Total Clicks: {}", m_totalClicks).c_str());
            }
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
                    }
                });
        }
    }
}

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
                    m_globalTotalViews = totalViews.unwrap();
                    log::info("Global Total Views: {}", m_globalTotalViews);
                }
            }

            if (jsonValue.contains("total_clicks")) {
                auto totalClicks = jsonValue["total_clicks"].asInt();
                if (totalClicks) {
                    m_globalTotalClicks = totalClicks.unwrap();
                    log::info("Global Total Clicks: {}", m_globalTotalClicks);
                }
            }

            if (jsonValue.contains("ad_count")) {
                auto adCount = jsonValue["ad_count"].asInt();
                if (adCount) {
                    m_globalAdCount = adCount.unwrap();
                    log::info("Global Ad Count: {}", m_globalAdCount);
                }
            }

            // Update labels with global stats
            if (m_globalViewsLabel)
                m_globalViewsLabel->setString(fmt::format("Views: {}", m_globalTotalViews).c_str());
            if (m_globalClicksLabel)
                m_globalClicksLabel->setString(fmt::format("Clicks: {}", m_globalTotalClicks).c_str());
            if (m_globalAdCountLabel)
                m_globalAdCountLabel->setString(fmt::format("Active Ads: {}", m_globalAdCount).c_str());
        } else {
            log::error("Global stats request failed with status code: {}", res->code());
        }
    }
}
