#include <Geode/Geode.hpp>
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace ads;

class ReportPopup : public Popup<> {
private:
    class Impl;
    std::unique_ptr<Impl> m_impl;

protected:
    ReportPopup();
    virtual ~ReportPopup();

    bool setup() override;
    void onSubmitButton(CCObject* sender);

public:
    static ReportPopup* create(int adId, int levelId, std::string_view userId, std::string_view description);
};