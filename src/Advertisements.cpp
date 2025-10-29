#include <Advertisements.hpp>
#include <AdPreview.hpp>

#include <fmt/core.h>

#include <Geode/Geode.hpp>
#include <algorithm>

using namespace geode::prelude;
using namespace geode::utils;
using namespace ads;

namespace ads
{
    CCSize getAdSize(AdType type)
    {
        auto banner = CCSize(364.f, 45.f);
        auto square = CCSize(122.6f, 122.6f);
        auto skyscraper = CCSize(41.f, 314.f);

        CCSize contentSize = banner;

        switch (type)
        {
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

    class Advertisement::Impl final
    {
    public:
        EventListener<web::WebTask> m_adListener;

        Ad m_ad = Ad();
        AdType m_type = AdType::Banner;

        CCMenuItemSpriteExtra *m_adButton = nullptr;
        LazySprite *m_adSprite = nullptr;
        CCSprite *m_adIcon = nullptr;
    };

    Advertisement::Advertisement()
    {
        m_impl = std::make_unique<Impl>();
    };

    Advertisement::~Advertisement()
    {
        if (m_impl && m_impl->m_adSprite)
        {
            m_impl->m_adSprite->release();
        }
    };

    bool Advertisement::init()
    {
        if (CCMenu::init())
        {
            setAnchorPoint({0.5, 0.5});
            return true;
        }
        else
        {
            return false;
        };
    };

    void Advertisement::activate(CCObject *)
    {
        auto &ad = m_impl->m_ad;
        if (ad.id == 0)
        {
            log::warn("Ad not loaded yet or invalid ad ID");
            Notification::create("Invalid Ad", NotificationIcon::Error)->show();
            return;
        }
        log::info("Opening AdPreview popup: ad_id={}, level_id={}, user_id={}, type={}", ad.id, ad.level, ad.user, static_cast<int>(ad.type));
        if (auto popup = AdPreview::create(ad.id, ad.level, ad.user, ad.type, ad.viewCount, ad.clickCount))
        {
            popup->show();
        }
        else
        {
            log::error("Failed to create AdPreview popup");
        }
    }

    void Advertisement::reload()
    {
        if (m_impl->m_adButton)
        {
            m_impl->m_adButton->removeMeAndCleanup();
            m_impl->m_adButton = nullptr;
        };

        if (!m_impl->m_adSprite)
        {
            log::warn("Cannot reload advertisement: ad sprite is null");
            return;
        }

        log::info("Reloading advertisement - creating button with sprite");

        m_impl->m_adButton = CCMenuItemSpriteExtra::create(
            m_impl->m_adSprite,
            this,
            menu_selector(Advertisement::activate));

        // m_impl->m_adButton->setPosition({getScaledContentWidth() / 2.f, getScaledContentHeight() / 2.f});

        if (m_impl->m_adButton)
        {
            this->addChild(m_impl->m_adButton);
            log::info("Advertisement button created and added to menu");
        }
        else
        {
            log::error("Failed to create CCMenuItemSpriteExtra");
        }
    };

    void Advertisement::reloadType()
    {
        if (m_impl->m_adButton)
        {
            m_impl->m_adButton->removeMeAndCleanup();
            m_impl->m_adButton = nullptr;
        };

        if (m_impl->m_adSprite)
        {
            m_impl->m_adSprite->removeMeAndCleanup();
            m_impl->m_adSprite->release();
            m_impl->m_adSprite = nullptr;
        };

        setScaledContentSize(getAdSize(m_impl->m_type));

        m_impl->m_adSprite = LazySprite::create(getScaledContentSize(), true);
        if (!m_impl->m_adSprite)
        {
            log::error("Failed to create LazySprite");
            return;
        }

        log::info("Created LazySprite with size: {}x{}", getScaledContentSize().width, getScaledContentSize().height);

        m_impl->m_adSprite->setID("ad");
        m_impl->m_adSprite->retain();
        m_impl->m_adSprite->setAnchorPoint({0.5f, 0.5f});
        // m_impl->m_adSprite->setPosition({getScaledContentWidth() / 2.f, getScaledContentHeight() / 2.f});
        m_impl->m_adSprite->setVisible(true);

        log::info("LazySprite configured - setting up callbacks");

        m_impl->m_adListener.bind([this](web::WebTask::Event *e)
                                  {
            if (!m_impl) {
                log::error("m_impl is null in ad listener callback");
                return;
            }

            if (auto res = e->getValue()) {
                if (res->ok()) {
                    auto jsonRes = res->json();
                    if (!jsonRes) {
                        log::error("Failed to parse ad JSON");
                        return;
                    }
                    auto json = jsonRes.unwrap();

                    auto id = json["ad_id"].asInt().unwrapOrDefault();
                    auto image = json["image_url"].asString().unwrapOrDefault();
                    auto level = json["level_id"].asInt().unwrapOrDefault();
                    auto user = json["user_id"].asString().unwrapOrDefault();
                    auto type = static_cast<AdType>(json["type"].asInt().unwrapOrDefault());
                    auto view = json["view_count"].asInt().unwrapOrDefault();
                    auto click = json["click_count"].asInt().unwrapOrDefault();

                    m_impl->m_ad = Ad(id, image, level, type, user, view, click);
                    log::info("Ad metadata set inside listener: ad_id={} level_id={} user_id={} type={}", id, level, user, static_cast<int>(type));
                    log::info("Ad view count: {}, click count: {}", view, click);

                    log::debug("Sending view tracking request for ad_id={}, user_id={}", id, user);
                    auto viewRequest = web::WebRequest();
                    viewRequest.userAgent("PlayerAdvertisements/1.0");
                    viewRequest.timeout(std::chrono::seconds(15));
                    viewRequest.header("Content-Type", "application/json");

                    matjson::Value viewBody = matjson::Value::object();
                    viewBody["ad_id"] = id;
                    viewBody["user_id"] = user;

                    viewRequest.bodyJSON(viewBody);
                    (void)viewRequest.post("https://ads.arcticwoof.xyz/api/view"); // those reviewing, all im doing is post the request to the api and dont return anything, this is why lol
                    log::info("Sent view tracking request for ad_id={}, user_id={}", id, user);

                    if (m_impl->m_adSprite) {
                        log::info("Loading ad image from URL: {}", m_impl->m_ad.image);
                        m_impl->m_adSprite->loadFromUrl(m_impl->m_ad.image.c_str(), cocos2d::CCImage::kFmtUnKnown, true);
                    } else {
                        log::warn("Ad sprite missing when trying to load image");
                    }
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
        m_impl->m_adSprite->setLoadCallback([this](Result<> res)
                                            {
                if (!m_impl) {
                    log::error("m_impl is null in load callback");
                    return;
                }

                if (res.isOk()) {
                    log::info("Ad image loaded successfully");
                    // add the adIcon at the bottom right of the ad button
                    if (!m_impl->m_adIcon) {
                        m_impl->m_adIcon = CCSprite::create("adIcon.png"_spr);
                        if (m_impl->m_adIcon) {
                            m_impl->m_adIcon->setAnchorPoint({ 0.f, 0.f });
                            m_impl->m_adIcon->setPosition({3.f, 3.f});
                            m_impl->m_adButton->addChild(m_impl->m_adIcon);
                            m_impl->m_adIcon->setScale(0.25f);
                            m_impl->m_adIcon->setOpacity(100);
                        } else {
                            log::error("Failed to create ad icon sprite");
                        }
                    }
                    if (!m_impl->m_adSprite) {
                        log::warn("Load callback: ad sprite is null");
                        return;
                    }

                    m_impl->m_adSprite->setAnchorPoint({ 0.5f, 0.5f });
                    //m_impl->m_adSprite->setPosition({ getScaledContentWidth() / 2.f, getScaledContentHeight() / 2.f });
                    m_impl->m_adSprite->setVisible(true);

                    auto natural = m_impl->m_adSprite->getContentSize();
                    if (natural.width <= 0.f || natural.height <= 0.f) {
                        log::warn("Ad sprite has invalid natural size ({}x{})", natural.width, natural.height);
                    } else {
                        auto target = getScaledContentSize();
                        float sx = target.width / natural.width;
                        float sy = target.height / natural.height;
                        float scale = std::min(sx, sy);
                        m_impl->m_adSprite->setScale(scale);
                        log::info("Scaled ad sprite by {} to fit target {}x{} (natural {}x{})", scale, target.width, target.height, natural.width, natural.height);
                    }

                    // if (m_impl->m_adButton) {
                    //     m_impl->m_adButton->setPosition({ getScaledContentWidth() / 2.f, getScaledContentHeight() / 2.f });
                    // }

                } else if (res.isErr()) {
                    log::error("Failed to load ad image: {}", res.unwrapErr());
                    if (m_impl && m_impl->m_adSprite) {
                        m_impl->m_adSprite->initWithSpriteFrameName("squareTemp.png"_spr);
                    }
                } else {
                    log::error("Unknown error loading ad image");
                } });

        reload();
    };

    void Advertisement::setType(AdType type)
    {
        m_impl->m_type = type;
        reloadType();
    };

    void Advertisement::loadRandom()
    {
        log::debug("Preparing request for random advertisement...");
        auto request = web::WebRequest();
        request.userAgent("PlayerAdvertisements/1.0");
        request.timeout(std::chrono::seconds(15));
        request.param("type", static_cast<int>(m_impl->m_type));
        m_impl->m_adListener.setFilter(request.get("https://ads.arcticwoof.xyz/api/ad"));
        log::info("Sent request for random advertisement");
    };

    void Advertisement::load(int id)
    {
        log::debug("Preparing request for advertisement of ID {}...", id);
        auto request = web::WebRequest();
        request.userAgent("PlayerAdvertisements/1.0");
        request.timeout(std::chrono::seconds(15));
        request.param("id", id);
        m_impl->m_adListener.setFilter(request.get("https://ads.arcticwoof.xyz/api/ad/get"));
        log::info("Sent request for advertisement of ID {}", id);
    };

    LazySprite *Advertisement::getAdSprite() const
    {
        return m_impl->m_adSprite;
    };

    Advertisement *Advertisement::create()
    {
        auto ret = new Advertisement();

        if (ret && ret->init())
        {
            ret->autorelease();
            return ret;
        };

        CC_SAFE_DELETE(ret);
        return nullptr;
    };
};