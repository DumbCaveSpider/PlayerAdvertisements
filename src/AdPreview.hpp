#include <Geode/Geode.hpp>
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace ads;

class AdPreview : public Popup<>
{
protected:
    int m_adId = 0;
    int m_levelId = 0;
    int m_userId = 0;
    AdType m_type = AdType::Banner;
    
    bool setup() override;

public:
    static AdPreview *create(int adId, int levelId, int userId, AdType type);
};