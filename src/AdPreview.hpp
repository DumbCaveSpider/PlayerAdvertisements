#include <Geode/Geode.hpp>
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace ads;

class AdPreview : public Popup<> {
protected:
    int m_adId = 0;
    int m_levelId = 0;
    std::string m_userId = "";
    AdType m_type = AdType::Banner;
    int m_viewCount = 0;
    int m_clickCount = 0;
    EventListener<web::WebTask> m_announcementListener;

    bool setup() override;
    void onPlayButton(CCObject* sender);
    void onReportButton(CCObject* sender);
    void onAnnouncementButton(CCObject* sender);
    void registerClick(int adId, std::string_view userId);

public:
    static AdPreview* create(int adId, int levelId, std::string_view userId, AdType type, int viewCount, int clickCount);
};