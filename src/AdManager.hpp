#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class AdManager final : public Popup {
private:
    class Impl;
    std::unique_ptr<Impl> m_impl;

protected:
    AdManager();
    virtual ~AdManager();

    bool init() override;

    void onDiscordButton(CCObject* sender);
    void onKofiButton(CCObject* sender);
    void onWebButton(CCObject* sender);
    void onFetchComplete(web::WebResponse const& res);
    void onGlobalStatsFetchComplete(web::WebResponse const& res);
    void onModSettingsButton(CCObject* sender);
    void onAnnouncement(CCObject* sender);
    void populateAdsScrollLayer();
    void tryOpenOrFetchLevel(CCMenuItemSpriteExtra* menuItem, int levelId);

    void update(float dt) override;

public:
    void onPlayButton(CCObject* sender);
    static AdManager* create();
};