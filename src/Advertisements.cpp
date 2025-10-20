#include <Advertisements.hpp>

#include <fmt/core.h>

#include <Geode/Geode.hpp>

using namespace geode::prelude;
using namespace ads;

namespace ads {
    CCSize getAdSize(AdType type) {
        auto banner = CCSize({ 1456.f, 180.f });
        auto square = CCSize({ 1456.f, 1456.f });
        auto skyscraper = CCSize({ 180.f, 1456.f });

        CCSize contentSize = banner;

        switch (type) {
        case Banner:
            contentSize = banner;
            break;
        case Square:
            contentSize = square;
            break;
        case Skyscraper:
            contentSize = skyscraper;
            break;

        default:
            contentSize = banner;
            break;
        };

        return contentSize;
    };

    class Advertisement::Impl final {
    public:
        EventListener<web::WebTask> m_adListener;

        Ad m_ad = Ad();
        AdType m_type = AdType::Banner;

        CCMenuItemSpriteExtra* m_adButton = nullptr;
        LazySprite* m_adSprite = nullptr;
    };

    Advertisement::Advertisement() {
        m_impl = std::make_unique<Impl>();
    };

    Advertisement::~Advertisement() {};

    bool Advertisement::init() {
        if (CCMenu::init()) {
            m_impl->m_adListener.bind([=](web::WebTask::Event* e) {
                if (web::WebResponse* res = e->getValue()) {
                    if (res->ok()) {
                        GEODE_UNWRAP_INTO(auto json, res->json());

                        auto id = json["ad_id"].asInt().unwrapOrDefault();
                        auto image = json["image_url"].asString().unwrapOrDefault();
                        auto level = json["level_id"].asInt().unwrapOrDefault();
                        auto type = static_cast<AdType>(json["type"].asInt().unwrapOrDefault());

                        m_impl->m_ad = Ad(id, image, level, type);
                        m_impl->m_adSprite->loadFromUrl(m_impl->m_ad.image.c_str(), cocos2d::CCImage::kFmtUnKnown, true);
                    } else {
                        log::error("Failed to fetch ad: HTTP {}", res->code());
                    };
                } else if (web::WebProgress* p = e->getProgress()) {
                    log::debug("ad progress: {}", (float)p->downloadProgress().value_or(0.f));
                } else if (e->isCancelled()) {
                    log::error("Ad web request failed");
                } else {
                    log::error("Unknown ad web request error");
                };
                                      });

            setAnchorPoint({ 0.5, 0.5 });

            reloadType();

            return true;
        } else {
            return false;
        };
    };

    void Advertisement::activate(CCObject*) {
        if (m_impl->m_ad.level > 0) {
            log::info("Activating ad for level ID {}", m_impl->m_ad.level);
            Notification::create(fmt::format("Loading level ID {}...", m_impl->m_ad.level), NotificationIcon::Loading, 1.25f)->show();
        } else {
            log::warn("Ad has no associated level ID");
        };
    };

    void Advertisement::reload() {
        if (m_impl->m_adButton) {
            m_impl->m_adButton->removeMeAndCleanup();
            m_impl->m_adButton = nullptr;
        };

        m_impl->m_adButton = CCMenuItemSpriteExtra::create(
            m_impl->m_adSprite,
            this,
            menu_selector(Advertisement::activate)
        );
        m_impl->m_adButton->setPosition({ getScaledContentWidth() / 2.f, getScaledContentHeight() / 2.f });
        this->addChild(m_impl->m_adButton);
    };

    void Advertisement::reloadType() {
        if (m_impl->m_adButton) {
            m_impl->m_adButton->removeMeAndCleanup();
            m_impl->m_adButton = nullptr;
        }
        if (m_impl->m_adSprite) {
            m_impl->m_adSprite->removeMeAndCleanup();
            m_impl->m_adSprite = nullptr;
        }

        setScaledContentSize(getAdSize(m_impl->m_type));

        m_impl->m_adSprite = LazySprite::create(getScaledContentSize(), true);
        m_impl->m_adSprite->setID("ad");
        m_impl->m_adSprite->setAnchorPoint({ 0.5f, 0.5f });
        m_impl->m_adSprite->setPosition({ getScaledContentWidth() / 2.f, getScaledContentHeight() / 2.f });

        reload();
    };

    void Advertisement::setType(AdType type) {
        m_impl->m_type = type;
        reloadType();
    };

    void Advertisement::loadRandom() {
        log::debug("Preparing request for random advertisement...");
        auto request = web::WebRequest();
        request.userAgent("PlayerAdvertisements/1.0");
        request.timeout(std::chrono::seconds(15));
        request.param("type", static_cast<int>(m_impl->m_type));
        m_impl->m_adListener.setFilter(request.get("https://ads.arcticwoof.xyz/api/ad"));
        log::info("Sent request for random advertisement");
    };

    void Advertisement::load(int id) {
        log::debug("Preparing request for advertisement of ID {}...", id);
        auto request = web::WebRequest();
        request.userAgent("PlayerAdvertisements/1.0");
        request.timeout(std::chrono::seconds(15));
        request.param("id", id);
        m_impl->m_adListener.setFilter(request.get("https://ads.arcticwoof.xyz/api/ad/get"));
        log::info("Sent request for advertisement of ID {}", id);
    };

    LazySprite* Advertisement::getAdSprite() const {
        return m_impl->m_adSprite;
    };

    Advertisement* Advertisement::create() {
        auto ret = new Advertisement();

        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        };

        CC_SAFE_DELETE(ret);
        return nullptr;
    };
};