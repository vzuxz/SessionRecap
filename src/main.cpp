#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/ui/PopupManager.hpp>

using namespace geode::prelude;

static std::string formatTime(double seconds) {
    auto total = static_cast<int>(seconds);
    auto h = total / 3600;
    auto m = total % 3600 / 60;
    auto s = total % 60;

    if (h > 0) return fmt::format("{}h {:02}m", h, m);
    if (m > 0) return fmt::format("{}m {:02}s", m, s);
    return fmt::format("{}s", s);
}

class $modify(RecapPlayLayer, PlayLayer) {
    struct Fields {
        double m_playTime = 0.0;
        int m_startBest = 0;
        int m_sessionBest = -1;
        bool m_completed = false;
        bool m_diedThisAttempt = false;
        std::map<int, int> m_deaths;
    };

    bool init(GJGameLevel* level, bool dontCreateObjects) {
        if (!PlayLayer::init(level, dontCreateObjects)) return false;
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
        showRecap();
        PlayLayer::onQuit();
    }

    void showRecap() {
        auto fields = m_fields.self();
        if (m_attempts < Mod::get()->getSettingValue<int64_t>("min-attempts")) return;

        auto text = fmt::format(
            "<cy>{}</c> attempts in <cy>{}</c>",
            m_attempts,
            formatTime(fields->m_playTime)
        );

        auto best = m_level->m_normalPercent.value();
        if (fields->m_completed) {
            text += "\n<cg>Completed!</c>";
        } else if (best > fields->m_startBest) {
            text += fmt::format("\nNew best: <cg>{}%</c> (was {}%)", best, fields->m_startBest);
        } else if (fields->m_sessionBest >= 0) {
            text += fmt::format("\nBest run: <cg>{}%</c>", fields->m_sessionBest);
        }

        auto worst = std::max_element(fields->m_deaths.begin(), fields->m_deaths.end(), [](auto& a, auto& b) {
            return a.second < b.second;
        });
        if (worst != fields->m_deaths.end() && worst->second > 1) {
            text += fmt::format("\nMost deaths: <cr>{}%</c> ({}x)", worst->first, worst->second);
        }

        auto popup = PopupManager::get().alert("Session Recap", text);
        popup.setPersistent();
        popup.showQueue();
    }
};
