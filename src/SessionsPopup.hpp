#pragma once

#include <Geode/Geode.hpp>
#include "Session.hpp"

class SessionsPopup : public geode::Popup {
protected:
    bool init(GJGameLevel* level);
    cocos2d::CCNode* createCell(Session const& session);

public:
    static SessionsPopup* create(GJGameLevel* level);
};
