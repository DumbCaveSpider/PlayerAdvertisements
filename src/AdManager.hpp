#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class AdManager : public Popup<> {
protected:
    bool setup() override;
    matjson::Value m_adsData;
    matjson::Value m_userData;
    EventListener<web::WebTask> m_listener;
    // stats
    int m_totalViews = 0;
    int m_totalClicks = 0;
    CCLabelBMFont* m_viewsLabel = nullptr;
    CCLabelBMFont* m_clicksLabel = nullptr;

public:
    static AdManager* create();
    void onWebButton(CCObject* sender);
    void onFetchComplete(web::WebTask::Event* event);
};