#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class AdManager final : public Popup {
private:
    class Impl;
    std::unique_ptr<Impl> m_impl;

protected:
    AdManager();
    ~AdManager();

    bool init() override;

    void update(float dt) override;

public:
    static AdManager* create();
};