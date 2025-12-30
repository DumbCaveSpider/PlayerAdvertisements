#include <Geode/Geode.hpp>
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace ads;

class AdPreview : public Popup<> {
private:
    class Impl;
    std::unique_ptr<Impl> m_impl;

protected:
AdPreview();
virtual ~AdPreview();

bool setup() override;
    void onPlayButton(CCObject* sender);
    void onReportButton(CCObject* sender);
    void onAnnouncementButton(CCObject* sender);
    void registerClick(int adId, std::string_view userId);

public:
    static AdPreview* create(int adId, int levelId, std::string_view userId, AdType type, int viewCount, int clickCount);
};