#include <Geode/Geode.hpp>
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace ads;

class AdPreview : public Popup {
private:
    class Impl;
    std::unique_ptr<Impl> m_impl;

    // pending fetch state used by the scheduler
    std::string m_pendingKey;
    int m_pendingLevelId = -1;
    float m_pendingTimeout = 0.0f;
    CCMenu* m_levelsMenu = nullptr;

protected:
    AdPreview();
    virtual ~AdPreview();

    bool init() override;

    void onPlayButton(CCObject* sender);
    void onReportButton(CCObject* sender);
    void onAnnouncementButton(CCObject* sender);
    void registerClick(int adId, std::string_view userId, CCMenuItemSpriteExtra* menuItem);
    void tryOpenOrFetchLevel(CCMenuItemSpriteExtra* menuItem, int levelId);

    void update(float dt) override;


public:
    static AdPreview* create(int adId, int levelId, std::string_view userId, AdType type, int viewCount, int clickCount);
};