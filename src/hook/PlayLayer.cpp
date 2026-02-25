#include <Advertisements.hpp>
#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;
using namespace ads;

class $modify(AdsPlayLayer, PlayLayer) {
    struct Fields {
        Ref<Advertisement> bannerTop = nullptr;
        Ref<Advertisement> bannerBottom = nullptr;
        Ref<Advertisement> skyscraperRight = nullptr;
        Ref<Advertisement> skyscraperLeft = nullptr;
    };

    void setupHasCompleted() {
        // dont show ads ingame if the setting is false
        if (Mod::get()->getSettingValue<bool>("PlayLayer")) {
            auto const winSize = CCDirector::sharedDirector()->getWinSize();

            // banner ad at the top
            if (!m_fields->bannerTop) {
                m_fields->bannerTop = Advertisement::create(AdType::Banner);
                m_fields->bannerTop->setID("banner-top"_spr);
                m_fields->bannerTop->setPosition({ winSize.width / 2.f, winSize.height - 30.f });

                m_uiLayer->addChild(m_fields->bannerTop);

                m_fields->bannerTop->loadRandom();
            };

            // banner at the bottom
            if (!m_fields->bannerBottom) {
                m_fields->bannerBottom = Advertisement::create(AdType::Banner);
                m_fields->bannerBottom->setID("banner-bottom"_spr);
                m_fields->bannerBottom->setPosition({ winSize.width / 2.f, 30.f });

                m_uiLayer->addChild(m_fields->bannerBottom);

                m_fields->bannerBottom->loadRandom();
            };

            // skyscraper ad on the right
            if (!m_fields->skyscraperRight) {
                m_fields->skyscraperRight = Advertisement::create(AdType::Skyscraper);
                m_fields->skyscraperRight->setID("skyscraper-right"_spr);
                m_fields->skyscraperRight->setPosition({ winSize.width - 30.f, winSize.height / 2.f });

                m_uiLayer->addChild(m_fields->skyscraperRight);

                m_fields->skyscraperRight->loadRandom();
            };

            // skyscraper ad on the left
            if (!m_fields->skyscraperLeft) {
                m_fields->skyscraperLeft = Advertisement::create(AdType::Skyscraper);
                m_fields->skyscraperLeft->setID("skyscraper-left"_spr);
                m_fields->skyscraperLeft->setPosition({ 30.f, winSize.height / 2.f });

                m_uiLayer->addChild(m_fields->skyscraperLeft);

                m_fields->skyscraperLeft->loadRandom();
            };

            log::info("setting up scheduler for auto ad refresh");
            this->schedule(schedule_selector(AdsPlayLayer::schedReload), 12.5f);
        };

        PlayLayer::setupHasCompleted();
    };

    void fullReset() {
        // reload all ads lulz
        this->reloadAllAds();
        PlayLayer::fullReset();
    };

    void resetLevelFromStart() {
        // reload all ads lulz
        this->reloadAllAds();
        PlayLayer::resetLevelFromStart();
    };

    void destroyPlayer(PlayerObject * player, GameObject * object) {
        PlayLayer::destroyPlayer(player, object);
        if (player->m_isDead) this->reloadAllAds();
    };

    void reloadAllAds() {
        if (m_fields->bannerTop) m_fields->bannerTop->loadRandom();
        if (m_fields->bannerBottom) m_fields->bannerBottom->loadRandom();
        if (m_fields->skyscraperRight) m_fields->skyscraperRight->loadRandom();
        if (m_fields->skyscraperLeft) m_fields->skyscraperLeft->loadRandom();

        log::info("All ads are now reloading");
    };

    void schedReload(float dt) {
        log::debug("reloading ads after {}s...", dt);
        this->reloadAllAds();
    };
};