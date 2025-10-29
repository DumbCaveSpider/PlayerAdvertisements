#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class AdManager : public Popup<> {
protected:
    bool setup() override;
    matjson::Value m_adsData;
    matjson::Value m_userData;
    EventListener<web::WebTask> m_listener;
    EventListener<web::WebTask> m_globalStatsListener;
    // stats
    int m_totalViews = 0;
    int m_totalClicks = 0;
    int m_adCount = 0;
    int m_globalTotalViews = 0;
    int m_globalTotalClicks = 0;
    int m_globalAdCount = 0;
    CCLabelBMFont* m_viewsLabel = nullptr;
    CCLabelBMFont* m_clicksLabel = nullptr;
    CCLabelBMFont* m_titleLabel = nullptr;
    CCLabelBMFont* m_globalViewsLabel = nullptr;
    CCLabelBMFont* m_globalClicksLabel = nullptr;
    CCLabelBMFont* m_globalAdCountLabel = nullptr;
    ScrollLayer* m_adsScrollLayer = nullptr;

public:
    static AdManager* create();
    void onDiscordButton(CCObject *sender);
    void onKofiButton(CCObject *sender);
    void onWebButton(CCObject* sender);
    void onFetchComplete(web::WebTask::Event* event);
    void onGlobalStatsFetchComplete(web::WebTask::Event* event);
    void onModSettingsButton(CCObject* sender);
    void onPlayButton(CCObject* sender);
    void populateAdsScrollLayer();
    
    int m_currentPlayLevelId = 0;
};