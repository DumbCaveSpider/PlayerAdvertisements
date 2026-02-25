#include <Geode/Geode.hpp>
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace ads;

class AdPreview final : public Popup {
private:
    class Impl;
    std::unique_ptr<Impl> m_impl;

protected:
    AdPreview();
    ~AdPreview();

    bool init(unsigned int adId, int levelId, std::string userId, AdType type, int viewCount, int clickCount);

    void onPlayButton(CCObject* sender);
    void onReportButton(CCObject* sender);
    void onAnnouncementButton(CCObject* sender);
    void registerClick(unsigned int adId, std::string_view userId, CCMenuItemSpriteExtra* menuItem);
    void tryOpenOrFetchLevel(CCMenuItemSpriteExtra* menuItem, int levelId);

    void update(float dt) override;


public:
    static AdPreview* create(unsigned int adId, int levelId, std::string userId, AdType type, int viewCount, int clickCount);
};