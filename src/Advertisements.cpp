#include <Advertisements.hpp>

#include <fmt/core.h>

#include <Geode/Geode.hpp>

using namespace geode::prelude;

CCSize Advertisements::getAdSize(AdType type) {
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

void Advertisements::getRandomAd(AdType type, std::function<void(Ad)> onComplete) {
    EventListener<web::WebTask> listener;

    listener.bind([onComplete](web::WebTask::Event* e) {
        if (web::WebResponse* res = e->getValue()) {
            if (res->ok()) {
                GEODE_UNWRAP_INTO(auto json, res->json());

                unsigned int id = json["ad_id"].asInt().unwrapOrDefault();
                std::string_view image = json["image_url"].asString().unwrapOrDefault();
                int level = json["level_id"].asInt().unwrapOrDefault();
                int type = json["type"].asInt().unwrapOrDefault();

                Ad advert(id, image, level, static_cast<AdType>(type));
                onComplete(advert);
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

    auto request = web::WebRequest();
    request.userAgent("PlayerAdvertisements/1.0");
    request.timeout(std::chrono::seconds(15));
    request.param("type", static_cast<int>(type));
    listener.setFilter(request.get("https://ads.arcticwoof.xyz/api/ad"));
};

void Advertisements::getAdByID(int id, std::function<void(Ad)> onComplete) {
    return; // finish later
};

LazySprite* Advertisements::loadAdImage(Ad ad) {
    auto adSprite = LazySprite::create(getAdSize(ad.type), true);

    adSprite->setLoadCallback([adSprite](Result<> res) {
        if (res.isOk()) {
            log::info("Ad loaded successfully");
        } else {
            log::error("Failed to load ad: {}", res.unwrapErr());
            adSprite->removeMeAndCleanup();
        };
                              });

    adSprite->loadFromUrl(std::string(ad.image).c_str(), CCImage::kFmtUnKnown, true);
    return adSprite;
};