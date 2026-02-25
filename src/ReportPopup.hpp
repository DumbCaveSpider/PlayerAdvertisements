#include <Geode/Geode.hpp>
#include <Advertisements.hpp>

using namespace geode::prelude;
using namespace ads;

class ReportPopup final : public Popup {
private:
    class Impl;
    std::unique_ptr<Impl> m_impl;

protected:
    ReportPopup();
    ~ReportPopup();

    bool init(unsigned int adId, int levelId, std::string userId, std::string description);
    void onSubmitButton(CCObject* sender);

public:
    static ReportPopup* create(unsigned int adId, int levelId, std::string userId, std::string description);
};