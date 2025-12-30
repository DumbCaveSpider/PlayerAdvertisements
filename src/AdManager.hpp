#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class AdManager : public Popup<> {
private:
    class Impl;
    std::unique_ptr<Impl> m_impl;

protected:
    AdManager();
    virtual ~AdManager();

    bool setup() override;

public:
    static AdManager* create();
    void onDiscordButton(CCObject* sender);
    void onKofiButton(CCObject* sender);
    void onWebButton(CCObject* sender);
    void onFetchComplete(web::WebTask::Event* event);
    void onGlobalStatsFetchComplete(web::WebTask::Event* event);
    void onModSettingsButton(CCObject* sender);
    void onPlayButton(CCObject* sender);
    void onAnnouncement(CCObject* sender);
    void populateAdsScrollLayer();

    int m_currentPlayLevelId = 0;
};