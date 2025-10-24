#include <Geode/Geode.hpp>
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace ads;

class AdPreview : public Popup<>
{
protected:
    int m_adId = 0;
    int m_levelId = 0;
    std::string m_userId = "";
    AdType m_type = AdType::Banner;
    int m_viewCount = 0;
    int m_clickCount = 0;
    
    bool setup() override;
    void onPlayButton(CCObject* sender);

public:
    static AdPreview *create(int adId, int levelId, std::string userId, AdType type, int viewCount, int clickCount);
};