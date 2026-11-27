#include "SessionsPopup.hpp"
#include <Geode/ui/NineSlice.hpp>
#include <Geode/ui/ScrollLayer.hpp>

using namespace geode::prelude;

constexpr float kCellWidth = 340.f;
constexpr float kCellHeight = 40.f;
constexpr float kListHeight = 190.f;

static std::string count(size_t amount, std::string_view word) {
    return fmt::format("{} {}{}", amount, word, amount == 1 ? "" : "s");
}

SessionsPopup* SessionsPopup::create(GJGameLevel* level) {
    auto ret = new SessionsPopup();
    if (ret->init(level)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool SessionsPopup::init(GJGameLevel* level) {
    if (!Popup::init(380.f, 280.f)) return false;
    setID("sessions-popup"_spr);
    setTitle("Sessions");

    auto sessions = loadSessions(level->m_levelID.value());
    if (sessions.empty()) {
        auto label = Label::create("No sessions yet", "bigFont.fnt");
        label->setID("empty-label");
        label->setScale(.5f);
        m_mainLayer->addChildAtPosition(label, Anchor::Center);
        return true;
    }

    auto attempts = 0;
    auto playTime = 0.0;
    for (auto& session : sessions) {
        attempts += session.attempts;
        playTime += session.playTime;
    }

    auto summary = Label::create(
        fmt::format(
            "{}, {}, {} played",
            count(sessions.size(), "session"),
            count(attempts, "attempt"),
            formatTime(playTime)
        ),
        "goldFont.fnt"
    );
    summary->setID("summary-label");
    summary->setLimitLabelSize({ kCellWidth, 20.f }, .5f);
    m_mainLayer->addChildAtPosition(summary, Anchor::Top, { 0.f, -46.f });

    auto list = ScrollLayer::create({ kCellWidth, kListHeight });
    list->setID("session-list");
    list->m_contentLayer->setLayout(ScrollLayer::createDefaultListLayout(4.f));

    for (auto it = sessions.rbegin(); it != sessions.rend(); ++it) {
        auto cell = createCell(*it);
        cell->setID(fmt::format("session-{}", it->time));
        list->m_contentLayer->addChild(cell);
    }
    list->m_contentLayer->updateLayout();
    list->scrollToTop();

    m_mainLayer->addChildAtPosition(list, Anchor::Center, { 0.f, -22.f });
    return true;
}

CCNode* SessionsPopup::createCell(Session const& session) {
    auto cell = CCNode::create();
    cell->setContentSize({ kCellWidth, kCellHeight });

    auto bg = NineSlice::create("square02b_001.png", { 0, 0, 80, 80 });
    bg->setID("background");
    bg->setScaleMultiplier(.5f);
    bg->setColor({ 0, 0, 0 });
    bg->setOpacity(90);
    bg->setAnchorPoint({ 0.f, 0.f });
    bg->setContentSize({ kCellWidth, kCellHeight });
    cell->addChild(bg, -1);

    auto date = Label::create(fmt::format("{:%b %d, %H:%M}", geode::localtime(session.time)), "goldFont.fnt");
    date->setID("date-label");
    date->setScale(.5f);
    date->setAnchorPoint({ 0.f, .5f });
    cell->addChildAtPosition(date, Anchor::Left, { 10.f, 8.f });

    auto details = fmt::format("{}, {}", count(session.attempts, "attempt"), formatTime(session.playTime));
    if (session.deathCount > 1) details += fmt::format(", most deaths at {}%", session.deathPercent);

    auto info = Label::create(details, "bigFont.fnt");
    info->setID("info-label");
    info->setLimitLabelSize({ 240.f, 12.f }, .35f);
    info->setAnchorPoint({ 0.f, .5f });
    cell->addChildAtPosition(info, Anchor::Left, { 10.f, -9.f });

    auto newBest = session.newBest > session.oldBest;
    std::string tag;
    ccColor3B color = { 200, 200, 200 };
    if (session.completed) {
        tag = "Completed";
        color = { 255, 210, 60 };
    } else if (newBest) {
        tag = "New best";
        color = { 120, 255, 80 };
    } else if (session.best >= 0) {
        tag = "Best run";
    }
    if (tag.empty()) return cell;

    auto tagLabel = Label::create(tag, "bigFont.fnt");
    tagLabel->setID("tag-label");
    tagLabel->setScale(.3f);
    tagLabel->setColor(color);
    tagLabel->setAnchorPoint({ 1.f, .5f });
    cell->addChildAtPosition(tagLabel, Anchor::Right, { -10.f, session.best >= 0 ? -10.f : 0.f });

    if (session.best >= 0) {
        auto percent = Label::create(fmt::format("{}%", session.best), "bigFont.fnt");
        percent->setID("percent-label");
        percent->setScale(.55f);
        percent->setAnchorPoint({ 1.f, .5f });
        if (session.completed || newBest) percent->setColor(color);
        cell->addChildAtPosition(percent, Anchor::Right, { -10.f, 6.f });
    }

    return cell;
}
