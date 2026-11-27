#pragma once

#include <Geode/Geode.hpp>

struct Session {
    int64_t time = 0;
    int attempts = 0;
    double playTime = 0.0;
    int best = -1;
    int oldBest = 0;
    int newBest = 0;
    bool completed = false;
    int deathPercent = -1;
    int deathCount = 0;
};

template <>
struct matjson::Serialize<Session> {
    static geode::Result<Session> fromJson(matjson::Value const& value) {
        Session session;
        session.time = value["time"].as<int64_t>().unwrapOr(0);
        session.attempts = value["attempts"].as<int>().unwrapOr(0);
        session.playTime = value["play_time"].as<double>().unwrapOr(0.0);
        session.best = value["best"].as<int>().unwrapOr(-1);
        session.oldBest = value["old_best"].as<int>().unwrapOr(0);
        session.newBest = value["new_best"].as<int>().unwrapOr(0);
        session.completed = value["completed"].as<bool>().unwrapOr(false);
        session.deathPercent = value["death_percent"].as<int>().unwrapOr(-1);
        session.deathCount = value["death_count"].as<int>().unwrapOr(0);
        return geode::Ok(session);
    }

    static matjson::Value toJson(Session const& session) {
        return matjson::makeObject({
            { "time", session.time },
            { "attempts", session.attempts },
            { "play_time", session.playTime },
            { "best", session.best },
            { "old_best", session.oldBest },
            { "new_best", session.newBest },
            { "completed", session.completed },
            { "death_percent", session.deathPercent },
            { "death_count", session.deathCount },
        });
    }
};

std::vector<Session> loadSessions(int levelID);
void saveSession(int levelID, Session const& session);
std::string formatTime(double seconds);
