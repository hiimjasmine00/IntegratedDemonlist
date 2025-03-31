#include "IDPackCell.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/GameStatsManager.hpp>
#include <Geode/binding/GJSearchObject.hpp>
#include <Geode/binding/LevelBrowserLayer.hpp>
#include <Geode/Enums.hpp>
#include <Geode/utils/cocos.hpp>
#include <Geode/utils/ranges.hpp>
#include <Geode/utils/string.hpp>

using namespace geode::prelude;

IDPackCell* IDPackCell::create(const std::string& name, double points, const std::vector<int>& levels) {
    auto ret = new IDPackCell();
    if (ret->init(name, points, levels)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool IDPackCell::init(const std::string& name, double points, const std::vector<int>& levels) {
    if (!CCLayer::init()) return false;

    setID("IDPackCell");
    auto winSize = CCDirector::get()->getWinSize();

    auto difficultySprite = CCSprite::createWithSpriteFrameName("difficulty_10_btn_001.png");
    difficultySprite->setPosition({ 31.0f, 50.0f });
    difficultySprite->setScale(1.1f);
    difficultySprite->setID("difficulty-sprite");
    addChild(difficultySprite, 2);

    auto nameLabel = CCLabelBMFont::create(name.c_str(), "bigFont.fnt");
    nameLabel->setPosition({ 162.0f, 80.0f });
    nameLabel->limitLabelWidth(205.0f, 0.9f, 0.0f);
    nameLabel->setID("name-label");
    addChild(nameLabel);

    auto viewSprite = ButtonSprite::create("View", 50, 0, 0.6f, false, "bigFont.fnt", "GJ_button_01.png", 50.0f);
    auto viewMenu = CCMenu::create();
    auto viewButton = CCMenuItemExt::createSpriteExtra(viewSprite, [this, levels](auto) {
        CCDirector::get()->pushScene(CCTransitionFade::create(0.5f, LevelBrowserLayer::scene(GJSearchObject::create(SearchType::MapPackOnClick,
            ranges::reduce<std::string>(levels, [](std::string& str, int level) { str += (str.empty() ? "" : ",") + std::to_string(level); })))));
    });
    viewButton->setID("view-button");
    viewMenu->addChild(viewButton);
    viewMenu->setPosition({ 347.0f - viewSprite->getContentWidth() / 2.0f, 50.0f });
    viewMenu->setID("view-menu");
    addChild(viewMenu);

    auto progressBackground = CCSprite::create("GJ_progressBar_001.png");
    progressBackground->setColor({ 0, 0, 0 });
    progressBackground->setOpacity(125);
    progressBackground->setScaleX(0.6f);
    progressBackground->setScaleY(0.8f);
    progressBackground->setPosition({ 164.0f, 48.0f });
    progressBackground->setID("progress-background");
    addChild(progressBackground, 3);

    auto progressBar = CCSprite::create("GJ_progressBar_001.png");
    progressBar->setColor({ 184, 0, 0 });
    progressBar->setScaleX(0.985f);
    progressBar->setScaleY(0.83f);
    progressBar->setAnchorPoint({ 0.0f, 0.5f });
    auto rect = progressBar->getTextureRect();
    progressBar->setPosition({ rect.size.width * 0.0075f, progressBackground->getContentHeight() / 2.0f });
    auto gsm = GameStatsManager::get();
    auto completed = std::ranges::count_if(levels, [gsm](int level) { return gsm->hasCompletedOnlineLevel(level); });
    progressBar->setTextureRect({ rect.origin.x, rect.origin.y, rect.size.width * (completed / (float)levels.size()), rect.size.height });
    progressBar->setID("progress-bar");
    progressBackground->addChild(progressBar, 1);

    auto progressLabel = CCLabelBMFont::create(fmt::format("{}/{}", completed, levels.size()).c_str(), "bigFont.fnt");
    progressLabel->setPosition({ 164.0f, 48.0f });
    progressLabel->setScale(0.5f);
    progressLabel->setID("progress-label");
    addChild(progressLabel, 4);

    auto pointsLabel = CCLabelBMFont::create(fmt::format("{} Points", points).c_str(), "bigFont.fnt");
    pointsLabel->setPosition({ 164.0f, 20.0f });
    pointsLabel->setScale(0.7f);
    pointsLabel->setColor({ 255, 255, (uint8_t)(255 - (completed >= levels.size()) * 205) });
    pointsLabel->setID("points-label");
    addChild(pointsLabel, 1);

    if (completed >= levels.size()) {
        auto completedSprite = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
        completedSprite->setPosition({ 250.0f, 49.0f });
        completedSprite->setID("completed-sprite");
        addChild(completedSprite, 5);
    }

    return true;
}
