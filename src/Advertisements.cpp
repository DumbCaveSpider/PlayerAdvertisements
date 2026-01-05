#include <algorithm>
#include <fmt/core.h>
#include <Geode/Geode.hpp>
#include <argon/argon.hpp>

#include <AdPreview.hpp>
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace geode::utils;
using namespace ads;

namespace ads {
    namespace particles {
        constexpr const char* banner = "1,2065,2,4515,3,855,155,1,156,20,145,20a-1a1a0.3a15a90a0a20a0a100a25a0a25a0a0a0a0a10a5a0a180a1a0a1a0a1a0a1a0a5a0a180a0a1a0a1a0a1a0a1a0a0a1a1a0a0a0a0a0a0a0a0a2a1a0a0a0a41a0a0a0a0a0a0a0a0a0a0a0a0a0a0;";
        constexpr const char* square = "1,2065,2,4515,3,855,155,1,156,20,145,20a-1a1a0.3a15a90a0a20a0a50a50a0a25a0a0a0a0a10a5a0a180a1a0a1a0a1a0a1a0a5a0a180a0a1a0a1a0a1a0a1a0a0a1a1a0a0a0a0a0a0a0a0a2a1a0a0a0a41a0a0a0a0a0a0a0a0a0a0a0a0a0a0;";
        constexpr const char* skyscraper = "1,2065,2,4515,3,855,155,1,156,20,145,20a-1a1a0.3a15a90a0a20a0a25a100a0a25a0a0a0a0a10a5a0a180a1a0a1a0a1a0a1a0a5a0a180a0a1a0a1a0a1a0a1a0a0a1a1a0a0a0a0a0a0a0a0a2a1a0a0a0a41a0a0a0a0a0a0a0a0a0a0a0a0a0a0;";
    };

    CCSize const getAdSize(AdType type) {
        auto const banner = CCSize(364.f, 45.f);
        auto const square = CCSize(122.6f, 122.6f);
        auto const skyscraper = CCSize(41.f, 314.f);

        CCSize contentSize = banner;

        switch (type) {
        case AdType::Banner:
            contentSize = banner;
            break;
        case AdType::Square:
            contentSize = square;
            break;
        case AdType::Skyscraper:
            contentSize = skyscraper;
            break;

        default:
            contentSize = banner;
            break;
        };

        return contentSize;
    };

    constexpr const char* getParticlesForAdType(AdType type) {
        switch (type) {
        case AdType::Banner:
            return particles::banner;

        case AdType::Square:
            return particles::square;

        case AdType::Skyscraper:
            return particles::skyscraper;

        default:
            return particles::square;
        };
    };

    class Advertisement::Impl final {
    public:
        EventListener<web::WebTask> m_adListener;

        Ad m_ad = Ad();
        AdType m_type = AdType::Banner;

        CCMenuItemSpriteExtra* m_adButton = nullptr;
        Ref<LazySprite> m_adSprite = nullptr;
        CCSprite* m_adIcon = nullptr;

        bool m_hasLoaded = false;
        bool m_loadRandom = false;
        int m_loadId = 0;
        bool m_isInScene = false;

        std::string m_token;

        EventListener<web::WebTask> m_viewListener;
    };

    Advertisement::Advertisement() {
        m_impl = std::make_unique<Impl>();
    };

    Advertisement::~Advertisement() {
        if (m_impl && m_impl->m_adSprite) m_impl->m_adSprite->release();
    };

    bool Advertisement::init() {
        if (CCMenu::init()) {
            setAnchorPoint({ 0.5, 0.5 });
            return true;
        } else {
            return false;
        };
    };

    void Advertisement::onEnter() {
        CCMenu::onEnter();
        m_impl->m_isInScene = true;
        if (m_impl->m_hasLoaded) {
            log::info("reloading new random advertisement");
            reloadType();
            loadRandom();
        };
    };

    void Advertisement::onExit() {
        m_impl->m_isInScene = false;
        CCMenu::onExit();
    };

    void Advertisement::activate(CCObject*) {
        auto const& ad = m_impl->m_ad;
        if (ad.id == 0) {
            log::warn("Ad not loaded yet or invalid ad ID");
            Notification::create("Invalid Ad", NotificationIcon::Error)->show();
            return;
        };

        log::info("Opening AdPreview popup: ad_id={}, level_id={}, user_id={}, type={}", ad.id, ad.level, ad.user, static_cast<int>(ad.type));
        if (auto popup = AdPreview::create(ad.id, ad.level, ad.user, ad.type, ad.viewCount, ad.clickCount)) {
            popup->show();
        } else {
            log::error("Failed to create AdPreview popup");
        };
    };

    void Advertisement::reload() {
        this->removeAllChildrenWithCleanup(true);

        if (!m_impl->m_adSprite) {
            log::warn("ad sprite is null");
            return;
        };

        log::info("Reloading advertisement");

        m_impl->m_adButton = CCMenuItemSpriteExtra::create(
            m_impl->m_adSprite,
            this,
            menu_selector(Advertisement::activate)
        );

        // m_impl->m_adButton->setPosition({getScaledContentWidth() / 2.f, getScaledContentHeight() / 2.f});

        if (m_impl->m_adButton) {
            this->addChild(m_impl->m_adButton, 1);
            log::info("Advertisement button created and added to menu");
        } else {
            log::error("Failed to create CCMenuItemSpriteExtra");
        };
    };

    void Advertisement::reloadType() {
        if (m_impl->m_adButton) {
            m_impl->m_adButton->removeMeAndCleanup();
            m_impl->m_adButton = nullptr;
        };

        if (m_impl->m_adSprite) {
            m_impl->m_adSprite->removeMeAndCleanup();
            m_impl->m_adSprite->release();
            m_impl->m_adSprite = nullptr;
        };

        setScaledContentSize(getAdSize(m_impl->m_type));

        m_impl->m_adSprite = LazySprite::create(getScaledContentSize(), true);
        if (!m_impl->m_adSprite) {
            log::error("Failed to create LazySprite");
            return;
        };

        log::info("Created LazySprite with size: {}x{}", getScaledContentSize().width, getScaledContentSize().height);

        m_impl->m_adSprite->setID("ad");
        m_impl->m_adSprite->retain();
        m_impl->m_adSprite->setAnchorPoint({ 0.5f, 0.5f });
        // m_impl->m_adSprite->setPosition({getScaledContentWidth() / 2.f, getScaledContentHeight() / 2.f});
        m_impl->m_adSprite->setVisible(true);

        log::info("LazySprite configured - setting up callbacks");

        // get argon token yum
        auto res = argon::startAuth([this](Result<std::string> res) {
            if (!res) {
                log::warn("Auth failed: {}", res.unwrapErr());
                Notification::create("Failed to authenticate with Argon", NotificationIcon::Error)
                    ->show();
                return;
            };

            auto token = std::move(res).unwrap();
            log::debug("Token: {}", token);
            m_impl->m_token = token;
                                    },
                                    [](argon::AuthProgress progress) {
                                        log::debug("Auth progress: {}", argon::authProgressToString(progress));
                                    });

        if (!res) {
            log::warn("Failed to start auth attempt: {}", res.unwrapErr());
            Notification::create("Ad View invalid: Failed auth", NotificationIcon::Error)
                ->show();
        };

        m_impl->m_adListener.bind([this](web::WebTask::Event* e) {
            if (!m_impl) {
                log::error("m_impl is null in ad listener callback");
                return;
            };

            if (auto res = e->getValue()) {
                if (res->ok()) {
                    auto jsonRes = res->json();
                    if (!jsonRes) {
                        log::error("Failed to parse ad JSON");
                        return;
                    };

                    auto json = jsonRes.unwrapOrDefault();

                    auto id = json["ad_id"].asInt().unwrapOrDefault();
                    auto image = json["image_url"].asString().unwrapOrDefault();
                    auto level = json["level_id"].asInt().unwrapOrDefault();
                    auto user = json["user_id"].asString().unwrapOrDefault();
                    auto type = static_cast<AdType>(json["type"].asInt().unwrapOrDefault());
                    auto view = json["views"].asInt().unwrapOrDefault();
                    auto click = json["clicks"].asInt().unwrapOrDefault();
                    auto glow = json["glow"].asInt().unwrapOrDefault();

                    m_impl->m_ad = Ad(id, image, level, type, user, view, click, glow);
                    log::debug("Ad metadata set inside listener: ad_id={} level_id={} user_id={} type={}", id, level, user, static_cast<int>(type));
                    log::debug("Ad view count: {}, click count: {}", view, click);
                    log::debug("Ad glow level: {}", glow);

                    log::debug("Sending view tracking request for ad_id={}, user_id={}", id, user);
                    auto viewRequest = web::WebRequest();
                    viewRequest.userAgent("PlayerAdvertisements/1.0");
                    viewRequest.header("Content-Type", "application/json");
                    viewRequest.timeout(std::chrono::seconds(15));

                    matjson::Value viewBody = matjson::Value::object();
                    viewBody["ad_id"] = id;
                    viewBody["authtoken"] = m_impl->m_token;
                    viewBody["account_id"] = GJAccountManager::sharedState()->m_accountID;

                    viewRequest.bodyJSON(viewBody);

                    m_impl->m_viewListener.bind([this, id, user](web::WebTask::Event* e) {
                        if (auto res = e->getValue()) {
                            if (res->ok()) {
                                log::info("View passed ad_id={}, user_id={}", id, user);
                            } else {
                                log::error("View failed with code {} for ad_id={}, user_id={}: {}", res->code(), id, user, res->errorMessage());
                            };

                            log::debug("View request completed for ad_id={}, user_id={}", id, user);
                        } else if (e->isCancelled()) {
                            log::error("View request failed for ad_id={}, user_id={}", id, user);
                        };
                                                });
                    m_impl->m_viewListener.setFilter(viewRequest.post("https://ads.arcticwoof.xyz/api/view"));
                    log::debug("Sent view tracking request for ad_id={}, user_id={}", id, user);

                    if (m_impl->m_adSprite) {
                        log::info("Loading ad image from URL: {}", m_impl->m_ad.image);
                        m_impl->m_adSprite->loadFromUrl(m_impl->m_ad.image.c_str(), CCImage::kFmtUnKnown);
                    } else {
                        log::warn("Ad sprite missing when trying to load image");
                    };
                } else {
                    log::error("Failed to fetch ad: HTTP {}", res->code());
                };
            } else if (auto p = e->getProgress()) {
                log::debug("ad progress: {}", p->downloadProgress().value_or(0.f));
            } else if (e->isCancelled()) {
                log::error("Ad web request failed");
            } else {
                log::error("Unknown ad web request error");
            }; });

            m_impl->m_adSprite->setLoadCallback([this](Result<> res) {
                if (!m_impl) {
                    log::error("m_impl is null in load callback");
                    return;
                };

                if (res.isOk()) {
                    log::info("Ad image loaded successfully");
                    // add the adIcon at the bottom right of the ad button
                    if (!m_impl->m_adIcon) {
                        m_impl->m_adIcon = CCSprite::create("adIcon.png"_spr);
                        if (m_impl->m_adIcon) {
                            m_impl->m_adIcon->setAnchorPoint({ 0.f, 0.f });
                            m_impl->m_adIcon->setPosition({ 3.f, 3.f });
                            m_impl->m_adIcon->setScale(0.25f);
                            m_impl->m_adIcon->setOpacity(100);

                            m_impl->m_adButton->addChild(m_impl->m_adIcon, 9);
                        } else {
                            log::error("Failed to create ad icon sprite");
                        };
                    };

                    if (!m_impl->m_adSprite) {
                        log::warn("Load callback: ad sprite is null");
                        return;
                    };

                    m_impl->m_adSprite->setAnchorPoint({ 0.5f, 0.5f });
                    //m_impl->m_adSprite->setPosition({ getScaledContentWidth() / 2.f, getScaledContentHeight() / 2.f });
                    m_impl->m_adSprite->setVisible(true);

                    auto const natural = m_impl->m_adSprite->getContentSize();
                    if (natural.width <= 0.f || natural.height <= 0.f) {
                        log::warn("Ad sprite has invalid natural size ({}x{})", natural.width, natural.height);
                    } else {
                        auto const target = getScaledContentSize();

                        float sx = target.width / natural.width;
                        float sy = target.height / natural.height;

                        float scale = std::min(sx, sy);

                        m_impl->m_adSprite->setScale(scale);
                        log::info("Scaled ad sprite by {} to fit target {}x{} (natural {}x{})", scale, target.width, target.height, natural.width, natural.height);
                    };

                    if (m_impl->m_ad.glowLevel > 0) {
                        auto const size = m_impl->m_adSprite->getScaledContentSize();

                        auto glowNode = CCScale9Sprite::create("glow.png"_spr);
                        glowNode->setContentSize(size);
                        glowNode->setAnchorPoint({ 0.5, 0.5 });
                        glowNode->setPosition(m_impl->m_adButton->getPosition());

                        auto particles = GameToolbox::particleFromString(getParticlesForAdType(m_impl->m_ad.type), CCParticleSystemQuad::create(), false);
                        particles->setScale(1.25f);
                        particles->setAnchorPoint({ 0.5, 0.5 });
                        particles->setPosition(glowNode->getPosition());
                        particles->setTotalParticles(125);
                        particles->setEmissionRate(25.f);

                        auto tag = CCLabelBMFont::create("Featured", "bigFont.fnt");
                        tag->setScale(0.375f);
                        tag->setAnchorPoint({ 1, 0 });
                        tag->setAlignment(kCCTextAlignmentRight);
                        tag->setPosition({ this->getScaledContentWidth() - 3.f, 3.f });
                        tag->setOpacity(200);

                        switch (m_impl->m_ad.glowLevel) {
                        case 1:
                            glowNode->setOpacity(200);
                            glowNode->setColor({ 250, 250, 75 });
                            glowNode->setContentSize({ size.width + 3.75f, size.height + 3.75f });
                            particles->setStartColorVar({ 250, 250, 75, 255 });
                            tag->setColor({ 250, 250, 75 });
                            break;

                        case 2:
                            glowNode->setOpacity(125);
                            glowNode->setColor({ 50, 250, 250 });
                            glowNode->setContentSize({ size.width + 6.25f, size.height + 6.25f });
                            particles->setStartColorVar({ 50, 250, 250, 255 });
                            tag->setColor({ 50, 250, 250 });
                            break;

                        case 3:
                            glowNode->setOpacity(100);
                            glowNode->setColor({ 255, 75, 150 });
                            glowNode->setContentSize({ size.width + 8.75f, size.height + 8.75f });
                            particles->setStartColorVar({ 255, 75, 150, 255 });
                            tag->setColor({ 255, 75, 150 });
                            break;

                        default:
                            glowNode->removeMeAndCleanup();
                            particles->removeMeAndCleanup();
                            tag->removeMeAndCleanup();
                            break;
                        };

                        if (glowNode) {
                            glowNode->setContentSize({ glowNode->getScaledContentWidth() * 2.5f, glowNode->getScaledContentHeight() * 2.5f });
                            glowNode->setScale(glowNode->getScale() / 2.5f);

                            this->addChild(glowNode, 0);
                            if (particles) this->addChild(particles, 2);
                            if (tag) m_impl->m_adButton->addChild(tag, 9);
                        };
                    };

                    // if (m_impl->m_adButton) {
                    //     m_impl->m_adButton->setPosition({ getScaledContentWidth() / 2.f, getScaledContentHeight() / 2.f });
                    // }

                } else if (res.isErr()) {
                    log::error("Failed to load ad image: {}", res.unwrapErr());
                    if (m_impl && m_impl->m_adSprite) {
                        m_impl->m_adSprite->initWithSpriteFrameName("squareTemp.png"_spr);
                    };
                } else {
                    log::error("Unknown error loading ad image");
                } });

                reload();
    };

    void Advertisement::setType(AdType type) {
        m_impl->m_type = type;
        reloadType();
    };

    void Advertisement::loadRandom() {
        reloadType(); // refresh any existing nodes

        log::debug("Preparing request for random advertisement...");
        auto request = web::WebRequest();
        request.userAgent("PlayerAdvertisements/1.0");
        request.timeout(std::chrono::seconds(15));
        request.param("type", static_cast<int>(m_impl->m_type));
        m_impl->m_adListener.setFilter(request.get("https://ads.arcticwoof.xyz/api/ad"));
        m_impl->m_hasLoaded = true;
        m_impl->m_loadRandom = true;
        log::info("Sent request for random advertisement");
    };

    void Advertisement::load(int id) {
        reloadType(); // refresh any existing nodes

        log::debug("Preparing request for advertisement of ID {}...", id);
        auto request = web::WebRequest();
        request.userAgent("PlayerAdvertisements/1.0");
        request.timeout(std::chrono::seconds(15));
        request.param("id", id);
        m_impl->m_adListener.setFilter(request.get("https://ads.arcticwoof.xyz/api/ad/get"));
        m_impl->m_hasLoaded = true;
        m_impl->m_loadRandom = false;
        m_impl->m_loadId = id;
        log::info("Sent request for advertisement of ID {}", id);
    };

    LazySprite* Advertisement::getAdSprite() const {
        return m_impl->m_adSprite;
    };

    Advertisement* Advertisement::create() {
        auto ret = new Advertisement();
        if (ret->init()) {
            ret->autorelease();
            return ret;
        };

        CC_SAFE_DELETE(ret);
        return nullptr;
    };
};