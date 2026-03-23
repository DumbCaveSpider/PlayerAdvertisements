#include <Advertisements.hpp>

#include "ui/AdPreview.hpp"

#include <argon/argon.hpp>

#include <fmt/core.h>

#include <Geode/Geode.hpp>

#include <Geode/ui/Button.hpp>

#include <Geode/utils/async.hpp>

using namespace geode::prelude;
using namespace geode::utils;
using namespace ads;

namespace ads {
    namespace particles {
        constexpr const char* banner = "1,2065,2,4515,3,855,155,1,156,20,145,20a-1a1a0.3a15a90a0a20a0a100a25a0a25a0a0a0a0a10a5a0a180a1a0a1a0a1a0a1a0a5a0a180a0a1a0a1a0a1a0a1a0a0a1a1a0a0a0a0a0a0a0a0a2a1a0a0a0a41a0a0a0a0a0a0a0a0a0a0a0a0a0a0;";
        constexpr const char* square = "1,2065,2,4515,3,855,155,1,156,20,145,20a-1a1a0.3a15a90a0a20a0a50a50a0a25a0a0a0a0a10a5a0a180a1a0a1a0a1a0a1a0a5a0a180a0a1a0a1a0a1a0a1a0a0a1a1a0a0a0a0a0a0a0a0a2a1a0a0a0a41a0a0a0a0a0a0a0a0a0a0a0a0a0a0;";
        constexpr const char* skyscraper = "1,2065,2,4515,3,855,155,1,156,20,145,20a-1a1a0.3a15a90a0a20a0a25a100a0a25a0a0a0a0a10a5a0a180a1a0a1a0a1a0a1a0a5a0a180a0a1a0a1a0a1a0a1a0a0a1a1a0a0a0a0a0a0a0a0a2a1a0a0a0a41a0a0a0a0a0a0a0a0a0a0a0a0a0a0;";
    };

    constexpr CCSize getAdSize(AdType type) noexcept {
        auto const banner = CCSize(364.f, 45.f);
        auto const square = CCSize(122.6f, 122.6f);
        auto const skyscraper = CCSize(41.f, 314.f);

        CCSize contentSize = banner;

        switch (type) {
            default: [[fallthrough]];

            case AdType::Banner: contentSize = banner; break;
            case AdType::Square: contentSize = square; break;
            case AdType::Skyscraper: contentSize = skyscraper; break;
        };

        return contentSize;
    };

    constexpr const char* getParticlesForAdType(AdType type) noexcept {
        switch (type) {
            default: [[fallthrough]];

            case AdType::Banner: return particles::banner;
            case AdType::Square: return particles::square;
            case AdType::Skyscraper: return particles::skyscraper;
        };
    };

    class Advertisement::Impl final {
    public:
        async::TaskHolder<web::WebResponse> adListener;

        Ad ad = Ad();
        AdType type = AdType::Banner;

        Button* adButton = nullptr;
        LazySprite* adSprite = nullptr;
        CCSprite* adIcon = nullptr;

        bool hasLoaded = false;
        bool loadRandom = false;

        int loadId = 0;

        std::string token;

        async::TaskHolder<web::WebResponse> viewListener;
    };

    Advertisement::Advertisement() : m_impl(std::make_unique<Impl>()) {};
    Advertisement::~Advertisement() {};

    bool Advertisement::init(AdType type) {
        m_impl->type = type;

        if (!CCNode::init()) return false;

        setAnchorPoint({0.5, 0.5});
        setContentSize(getAdSize(type));

        return true;
    };

    void Advertisement::onEnter() {
        CCNode::onEnter();
        log::trace("advertisement appeared on scene");
        reloadType();
    };

    void Advertisement::reload() {
        this->removeAllChildrenWithCleanup(true);

        if (!m_impl->adSprite) {
            log::warn("ad sprite is null");
            return;
        };

        log::info("Reloading advertisement");

        m_impl->adButton = Button::createWithNode(
            m_impl->adSprite,
            [this](auto) {
                auto const& ad = m_impl->ad;
                if (ad.id == 0) {
                    log::warn("Ad not loaded yet or ad ID is invalid");
                    Notification::create("Invalid Ad", NotificationIcon::Error)->show();
                    return;
                };

                log::info("Opening AdPreview popup: ad_id={}, level_id={}, user_id={}, type={}", ad.id, ad.level, ad.user, static_cast<int>(ad.type));
                if (auto popup = AdPreview::create(ad.id, ad.level, ad.user, ad.type, ad.viewCount, ad.clickCount)) {
                    popup->show();
                } else {
                    log::error("Failed to create AdPreview popup");
                };
            });
        m_impl->adButton->setPosition(getScaledContentSize() / 2.f);

        if (m_impl->adButton) {
            this->addChild(m_impl->adButton, 1);
            log::info("Advertisement button created and added to menu");
        } else {
            log::error("Failed to create button");
        };
    };

    void Advertisement::reloadType() {
        this->setContentSize(getAdSize(m_impl->type));

        m_impl->adSprite = LazySprite::create(getScaledContentSize(), true);
        if (!m_impl->adSprite) {
            log::error("Failed to create LazySprite");
            return;
        };

        log::info("Created LazySprite with size: {}x{}", getScaledContentSize().width, getScaledContentSize().height);

        m_impl->adSprite->retain();
        m_impl->adSprite->setPosition({getScaledContentWidth() / 2.f, getScaledContentHeight() / 2.f});
        m_impl->adSprite->setVisible(true);

        log::info("setting up callbacks");

        async::spawn(
            argon::startAuth(),
            [self = WeakRef(this)](geode::Result<std::string> res) {
                if (auto s = self.lock()) {
                    if (res.isOk()) {
                        s->m_impl->token = std::move(res).unwrap();
                    } else {
                        log::warn("Auth failed: {}", res.unwrapErr());
                    };
                };
            });

        // prepare request for ad data
        auto req = web::WebRequest();
        req.userAgent("PlayerAdvertisements/1.0");
        req.header("Content-Type", "application/json");
        req.timeout(std::chrono::seconds(15));
        req.param("type", static_cast<int>(m_impl->type));

        req.onProgress([](web::WebProgress const& progress) {
            log::trace("ad progress: {}", progress.downloadProgress().value_or(0.f));
        });

        reload();

        m_impl->adListener.spawn(
            req.get("https://ads.arcticwoof.xyz/api/ad"),
            [self = WeakRef(this)](web::WebResponse res) {
                if (auto s = self.lock()) s->handleAdResponse(res);
            });

        m_impl->adSprite->setLoadCallback([self = WeakRef(this)](Result<> res) {
            if (auto s = self.lock()) {
                if (res.isOk()) {
                    log::info("Ad image loaded successfully");

                    // add the adIcon at the bottom right of the ad button
                    s->m_impl->adIcon = CCSprite::createWithSpriteFrameName("adIcon.png"_spr);
                    s->m_impl->adIcon->setAnchorPoint({0.f, 0.f});
                    s->m_impl->adIcon->setPosition({3.f, 3.f});
                    s->m_impl->adIcon->setScale(0.25f);
                    s->m_impl->adIcon->setOpacity(100);

                    if (s->m_impl->adButton) s->m_impl->adButton->addChild(s->m_impl->adIcon, 9);

                    if (!s->m_impl->adSprite) {
                        log::warn("Load callback: ad sprite is null");
                        return;
                    };

                    s->m_impl->adSprite->setAnchorPoint({0.5, 0.5});

                    float posx = s->m_impl->adSprite->getScaledContentWidth() * s->m_impl->adSprite->getScale() / 2.f;
                    float posy = s->m_impl->adSprite->getScaledContentHeight() * s->m_impl->adSprite->getScale() / 2.f;

                    if (s->m_impl->adButton) {
                        posx = s->m_impl->adButton->getScaledContentWidth() / 2.f;
                        posy = s->m_impl->adButton->getScaledContentHeight() / 2.f;
                    };

                    s->m_impl->adSprite->setPosition({posx, posy});
                    s->m_impl->adSprite->setVisible(true);

                    auto const natural = s->m_impl->adSprite->getContentSize();
                    if (natural.width <= 0.f || natural.height <= 0.f) {
                        log::warn("Ad sprite has invalid natural size ({}x{})", natural.width, natural.height);
                    } else {
                        // try to determine target size from adButton if available, otherwise use sprite's container size
                        CCSize target = s->m_impl->adSprite->getContentSize();
                        if (s->m_impl->adButton) target = s->m_impl->adButton->getContentSize();

                        float sx = target.width / natural.width;
                        float sy = target.height / natural.height;

                        float scale = std::min(sx, sy);

                        s->m_impl->adSprite->setScale(scale);
                        log::info("Scaled ad sprite by {} to fit target {}x{} (natural {}x{})", scale, target.width, target.height, natural.width, natural.height);
                    };

                    if (s->m_impl->ad.glowLevel > 0) {
                        auto const size = s->m_impl->adSprite->getScaledContentSize();

                        auto featuredStar = CCSprite::createWithSpriteFrameName("featuredIcon.png"_spr);
                        featuredStar->setAnchorPoint({1.f, 0.f});
                        featuredStar->setScale(0.35f);

                        float xpos = 3.f;
                        if (s->m_impl->adButton) xpos = s->m_impl->adButton->getScaledContentWidth() - 3.f;

                        featuredStar->setPosition({xpos, 4.25f});
                        featuredStar->setOpacity(200);
                        featuredStar->setColor({255, 255, 255});

                        if (s->m_impl->adButton) s->m_impl->adButton->addChild(featuredStar, 9);

                        auto glowNode = NineSlice::create("glow.png"_spr);
                        glowNode->setContentSize(size);
                        glowNode->setAnchorPoint({0.5, 0.5});

                        if (s->m_impl->adButton) glowNode->setPosition(s->m_impl->adButton->getContentSize() / 2);

                        auto particles = GameToolbox::particleFromString(getParticlesForAdType(s->m_impl->ad.type), CCParticleSystemQuad::create(), false);
                        particles->setScale(1.25f);
                        particles->setAnchorPoint({0.5, 0.5});
                        particles->setPosition(glowNode->getPosition());
                        particles->setEmissionRate(2500.f);
                        particles->setTotalParticles(125);

                        auto tag = CCLabelBMFont::create("Featured", "bigFont.fnt");
                        tag->setScale(0.375f);
                        tag->setAnchorPoint({1, 0});
                        tag->setAlignment(kCCTextAlignmentRight);

                        float tagx = 12.f;
                        if (s->m_impl->adButton) tagx = s->m_impl->adButton->getScaledContentWidth() - 12.f;

                        tag->setPosition({tagx, 3.f});
                        tag->setOpacity(200);

                        if (s->m_impl->ad.type == AdType::Skyscraper) tag->setVisible(false);

                        switch (s->m_impl->ad.glowLevel) {
                            case 1: {
                                glowNode->setOpacity(175);
                                glowNode->setColor({250, 250, 75});
                                glowNode->setContentSize({size.width + 6.25f, size.height + 6.25f});
                                particles->setStartColorVar({250, 250, 75, 255});
                                tag->setColor({250, 250, 75});

                                if (featuredStar) featuredStar->setColor({250, 250, 75});
                            } break;

                            case 2: {
                                glowNode->setOpacity(200);
                                glowNode->setColor({50, 250, 250});
                                glowNode->setContentSize({size.width + 7.5f, size.height + 7.5f});
                                particles->setStartColorVar({50, 250, 250, 255});
                                tag->setColor({50, 250, 250});

                                if (featuredStar) featuredStar->setColor({50, 250, 250});
                            } break;

                            case 3: {
                                glowNode->setOpacity(250);
                                glowNode->setColor({255, 125, 175});
                                glowNode->setContentSize({size.width + 8.75f, size.height + 8.75f});
                                particles->setStartColorVar({255, 125, 175, 255});
                                tag->setColor({255, 125, 175});

                                if (featuredStar) featuredStar->setColor({255, 125, 175});
                            } break;

                            default: {
                                if (glowNode) glowNode->removeMeAndCleanup();
                                if (particles) particles->removeMeAndCleanup();
                                if (tag) tag->removeMeAndCleanup();
                            } break;
                        };

                        if (glowNode) {
                            glowNode->setContentSize({glowNode->getScaledContentWidth() * 2.5f, glowNode->getScaledContentHeight() * 2.5f});
                            glowNode->setScale(glowNode->getScale() / 2.5f);

                            if (s->m_impl->adButton) s->m_impl->adButton->addChild(glowNode, -5);

                            if (s->m_impl->ad.type != AdType::Skyscraper) {
                                if (particles) {
                                    if (s->m_impl->adButton && s->m_impl->adButton->getParent()) s->m_impl->adButton->getParent()->addChild(particles, 2);
                                };
                            };

                            if (tag && s->m_impl->adButton) s->m_impl->adButton->addChild(tag, 9);
                        };
                    };
                } else if (res.isErr()) {
                    log::error("Failed to load ad image: {}", res.unwrapErr());

                    if (s->m_impl->adSprite) s->m_impl->adSprite->setVisible(false);
                    if (s->m_impl->adButton) s->m_impl->adButton->setEnabled(false);
                } else {
                    log::error("Unknown error loading ad image");
                };
            };
        });
    };

    void Advertisement::handleAdResponse(web::WebResponse const& res) {
        if (res.ok()) {
            auto jsonRes = res.json();
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

            m_impl->ad = Ad(id, image, level, type, user, view, click, glow);
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
            viewBody["authtoken"] = m_impl->token;
            viewBody["account_id"] = GJAccountManager::sharedState()->m_accountID;

            viewRequest.bodyJSON(viewBody);

            m_impl->viewListener.spawn(viewRequest.post("https://ads.arcticwoof.xyz/api/view"), [this, id, user](web::WebResponse res) {
                if (res.ok()) {
                    log::info("View passed ad_id={}, user_id={}", id, user);
                } else {
                    log::error("View failed with code {} for ad_id={}, user_id={}: {}", res.code(), id, user, res.errorMessage());
                };

                log::debug("View request completed for ad_id={}, user_id={}", id, user);
            });
            log::debug("Sent view tracking request for ad_id={}, user_id={}", id, user);

            if (m_impl->adSprite && !m_impl->ad.image.empty()) {
                log::info("Loading ad image from URL: {}", m_impl->ad.image);
                m_impl->adSprite->loadFromUrl(m_impl->ad.image.c_str(), CCImage::kFmtUnKnown);
            } else if (m_impl->ad.image.empty()) {
                log::warn("Ad image URL is empty, skipping image load");
            } else {
                log::warn("Ad sprite missing when trying to load image");
            };
        } else {
            log::error("Failed to fetch ad: HTTP {}", res.code());
        };
    };

    void Advertisement::setType(AdType type) {
        m_impl->type = type;
        reloadType();
    };

    void Advertisement::loadRandom() {
        reloadType();  // refresh any existing nodes

        log::debug("Preparing request for random advertisement...");

        auto request = web::WebRequest();
        request.userAgent("PlayerAdvertisements/1.0");
        request.timeout(std::chrono::seconds(15));
        request.param("type", static_cast<int>(m_impl->type));

        m_impl->adListener.spawn(
            request.get("https://ads.arcticwoof.xyz/api/ad"),
            [self = WeakRef(this)](web::WebResponse res) {
                if (auto s = self.lock()) s->handleAdResponse(res);
            });

        m_impl->hasLoaded = true;
        m_impl->loadRandom = true;

        log::info("Sent request for random advertisement");
    };

    void Advertisement::load(int id) {
        reloadType();  // refresh any existing nodes

        log::debug("Preparing request for advertisement of ID {}...", id);

        auto request = web::WebRequest();
        request.userAgent("PlayerAdvertisements/1.0");
        request.timeout(std::chrono::seconds(15));
        request.param("id", id);

        m_impl->adListener.spawn(
            request.get("https://ads.arcticwoof.xyz/api/ad/get"),
            [self = WeakRef(this)](web::WebResponse res) {
                if (auto s = self.lock()) s->handleAdResponse(res);
            });

        m_impl->hasLoaded = true;
        m_impl->loadRandom = false;
        m_impl->loadId = id;

        log::info("Sent request for advertisement of ID {}", id);
    };

    LazySprite* Advertisement::getAdSprite() const {
        return m_impl->adSprite;
    };

    Advertisement* Advertisement::create(AdType type) {
        auto ret = new Advertisement();
        if (ret->init(type)) {
            ret->autorelease();
            return ret;
        };

        delete ret;
        return nullptr;
    };
};