#include "AdPreview.hpp"
#include <Geode/Geode.hpp>
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace ads;

AdPreview *AdPreview::create(int adId, int levelId, int userId, AdType type)
{
    auto ret = new AdPreview();
    ret->m_adId = adId;
    ret->m_levelId = levelId;
    ret->m_userId = userId;
    ret->m_type = type;
    
    if (ret && ret->initAnchored(300.f, 200.f))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
};

bool AdPreview::setup()
{
    setTitle("Ad Preview");
    
    // Display ad info in a label
    std::string info = fmt::format("Ad ID: {}\nLevel ID: {}\nUser ID: {}\nType: {}", m_adId, m_levelId, m_userId, static_cast<int>(m_type));
    auto label = CCLabelBMFont::create(info.c_str(), "bigFont.fnt");
    if (label) {
        label->setAnchorPoint({0.5f, 0.5f});
        label->setPosition({m_size.width / 2.f, m_size.height / 2.f});
        label->setScale(0.4f);
        m_mainLayer->addChild(label);
    }
    return true;
};