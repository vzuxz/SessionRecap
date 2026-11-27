#include "Session.hpp"

using namespace geode::prelude;

static std::string keyFor(int levelID) {
    return fmt::format("level-{}", levelID);
}

std::vector<Session> loadSessions(int levelID) {
    return Mod::get()->getSavedValue<std::vector<Session>>(keyFor(levelID));
}

void saveSession(int levelID, Session const& session) {
    auto sessions = loadSessions(levelID);
    sessions.push_back(session);
    if (sessions.size() > 100) sessions.erase(sessions.begin());
    Mod::get()->setSavedValue(keyFor(levelID), sessions);

    auto res = Mod::get()->saveData();
    if (res.isErr()) log::warn("couldnt save sessions: {}", res.unwrapErr());
}

std::string formatTime(double seconds) {
    auto total = static_cast<int>(seconds);
    auto h = total / 3600;
    auto m = total % 3600 / 60;
    auto s = total % 60;

    if (h > 0) return fmt::format("{}h {:02}m", h, m);
    if (m > 0) return fmt::format("{}m {:02}s", m, s);
    return fmt::format("{}s", s);
}
