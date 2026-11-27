#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <asp/time/SystemTime.hpp>
#include "Session.hpp"
#include "SessionsPopup.hpp"

using namespace geode::prelude;

class $modify(RecapPlayLayer, PlayLayer) {
    struct Fields {
        int64_t m_startTime = 0;
        double m_playTime = 0.0;
        int m_startBest = 0;
        int m_sessionBest = -1;
        bool m_completed = false;
        bool m_diedThisAttempt = false;
        std::map<int, int> m_deaths;
    };

    bool init(GJGameLevel* level, bool dontCreateObjects) {
        if (!PlayLayer::init(level, dontCreateObjects)) return false;
        m_fields->m_startTime = asp::time::SystemTime::now().to_time_t();
        m_fields->m_startBest = level->m_normalPercent.value();
        return true;
    }

    bool isRealRun() {
        return !m_isPracticeMode && !m_isTestMode;
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);
        if (m_isPaused || m_hasCompletedLevel || m_player1->m_isDead) return;
        m_fields->m_playTime += dt;
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        m_fields->m_diedThisAttempt = false;
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        PlayLayer::destroyPlayer(player, object);

        auto fields = m_fields.self();
        if (!player->m_isDead || fields->m_diedThisAttempt) return;
        fields->m_diedThisAttempt = true;

        if (!isRealRun() || m_isPlatformer) return;
        auto percent = getCurrentPercentInt();
        fields->m_deaths[percent]++;
        fields->m_sessionBest = std::max(fields->m_sessionBest, percent);
    }

    void levelComplete() {
        PlayLayer::levelComplete();
        if (!isRealRun()) return;
        m_fields->m_completed = true;
        if (!m_isPlatformer) m_fields->m_sessionBest = 100;
    }

    void onQuit() {
        recordSession();
        PlayLayer::onQuit();
    }

    void recordSession() {
        if (m_level->m_levelType == GJLevelType::Editor || m_level->m_levelType == GJLevelType::Main) return;

        auto levelID = m_level->m_levelID.value();
        auto fields = m_fields.self();
        if (levelID <= 0 || fields->m_playTime < 1.0) return;

        Session session;
        session.time = fields->m_startTime;
        session.attempts = m_attempts;
        session.playTime = fields->m_playTime;
        session.best = fields->m_sessionBest;
        session.oldBest = fields->m_startBest;
        session.newBest = m_level->m_normalPercent.value();
        session.completed = fields->m_completed;

        auto worst = std::max_element(fields->m_deaths.begin(), fields->m_deaths.end(), [](auto& a, auto& b) {
            return a.second < b.second;
        });
        if (worst != fields->m_deaths.end()) {
            session.deathPercent = worst->first;
            session.deathCount = worst->second;
        }

        saveSession(levelID, session);
    }
};

class $modify(LevelInfoLayer) {
    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;

        auto menu = getChildByID("left-side-menu");
        if (!menu) return true;

        auto sprite = CircleButtonSprite::createWithSpriteFrameName("GJ_timeIcon_001.png");
        if (auto copy = menu->getChildByID("copy-button")) {
            sprite->setScale(copy->getContentHeight() / sprite->getContentHeight());
        }

        auto button = CCMenuItemExt::createSpriteExtra(sprite, [this](auto) {
            if (auto popup = SessionsPopup::create(m_level)) popup->show();
        });
        button->setID("sessions-button"_spr);
        menu->addChild(button);
        menu->updateLayout();
        return true;
    }
};
