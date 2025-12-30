#include <Geode/Geode.hpp>
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace ads;

class ReportPopup : public Popup<> {
protected:
    int m_adId = 0;
    int m_levelId = 0;
    std::string m_userId = "";
    std::string m_description = "";
    EventListener<web::WebTask> m_listener;

    bool setup() override;
    void onSubmitButton(CCObject* sender);

public:
    static ReportPopup* create(int adId, int levelId, std::string_view userId, std::string_view description);
};