#include "IDPackCell.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/GameStatsManager.hpp>
#include <Geode/binding/LevelBrowserLayer.hpp>
#include <Geode/loader/Mod.hpp>
#include <jasmine/search.hpp>

using namespace geode::prelude;

std::unordered_map<std::string, std::vector<ccColor3B>> tierColors = {
    { "Iron Tier", { { 186, 169, 175 }, { 246, 246, 246 } } },
    { "Gold Tier", { { 255, 155, 92 }, { 255, 239, 128 } } },
    { "Ruby Tier", { { 219, 46, 79 }, { 248, 132, 103 } } },
    { "Sapphire Tier", { { 84, 54, 208 }, { 81, 162, 227 } } },
    { "Pearl Tier", { { 211, 142, 103 }, { 252, 222, 182 } } },
    { "Diamond Tier", { { 136, 181, 255 }, { 237, 254, 255 }, { 255, 255, 255 }, { 226, 246, 255 } } }
};

IDPackCell* IDPackCell::create(const std::string& name, double points, const std::vector<int>& levels, const std::string& tier) {
    auto ret = new IDPackCell();
    if (ret->init(name, points, levels, tier)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool IDPackCell::init(const std::string& name, double points, const std::vector<int>& levels, const std::string& tier) {
    if (!CCLayer::init()) return false;

    setID("IDPackCell");

    auto difficultySprite = CCSprite::createWithSpriteFrameName("difficulty_10_btn_001.png");
    difficultySprite->setPosition({ 31.0f, 50.0f });
    difficultySprite->setScale(1.1f);
    difficultySprite->setID("difficulty-sprite");
    addChild(difficultySprite, 2);

    if (auto shader = CCShaderCache::sharedShaderCache()->programForKey("pack-gradient"_spr)) {
        m_background = CCSprite::create("ID_background_001.png"_spr);
        m_background->setPosition({ 178.0f, 50.0f });
        m_background->setShaderProgram(shader);
        m_background->setFlipX(true);
        m_background->setID("background");
        addChild(m_background);
    }

    auto nameLabel = CCLabelBMFont::create(name.c_str(), "bigFont.fnt");
    nameLabel->setPosition({ 162.0f, 85.0f });
    nameLabel->limitLabelWidth(205.0f, 0.9f, 0.0f);
    nameLabel->setID("name-label");
    addChild(nameLabel);

    auto tierLabel = CCLabelBMFont::create(tier.c_str(), "bigFont.fnt");
    tierLabel->setPosition({ 162.0f, 65.0f });
    tierLabel->setScale(0.4f);
    tierLabel->setID("tier-label");
    addChild(tierLabel);

    auto viewSprite = ButtonSprite::create("View", 50, 0, 0.6f, false, "bigFont.fnt", "GJ_button_01.png", 50.0f);
    auto viewMenu = CCMenu::create();
    auto viewButton = CCMenuItemExt::createSpriteExtra(viewSprite, [this, levels](auto) {
        CCDirector::get()->pushScene(CCTransitionFade::create(0.5f,
            LevelBrowserLayer::scene(jasmine::search::getObject(levels, &fmt::to_string<int>))));
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
    auto total = levels.size();
    auto completed = std::ranges::count_if(levels, [gsm](int level) {
        return gsm->hasCompletedOnlineLevel(level);
    });
    rect.size.width *= (float)completed / (float)total;
    progressBar->setTextureRect(rect);
    progressBar->setID("progress-bar");
    progressBackground->addChild(progressBar, 1);

    auto progressLabel = CCLabelBMFont::create(fmt::format("{}/{}", completed, total).c_str(), "bigFont.fnt");
    progressLabel->setPosition({ 164.0f, 48.0f });
    progressLabel->setScale(0.5f);
    progressLabel->setID("progress-label");
    addChild(progressLabel, 4);

    auto pointsLabel = CCLabelBMFont::create(fmt::format("{} Points", points).c_str(), "bigFont.fnt");
    pointsLabel->setPosition({ 164.0f, 20.0f });
    pointsLabel->setScale(0.7f);
    pointsLabel->setColor(completed >= total ? ccColor3B { 255, 255, 50 } : ccColor3B { 255, 255, 255 });
    pointsLabel->setID("points-label");
    addChild(pointsLabel, 1);

    if (completed >= total) {
        auto completedSprite = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
        completedSprite->setPosition({ 250.0f, 49.0f });
        completedSprite->setID("completed-sprite");
        addChild(completedSprite, 5);
    }

    if (auto it = tierColors.find(tier); it != tierColors.end()) {
        for (auto color : it->second) {
            m_colors.emplace_back(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, 1.0f);
        }
        m_colorMode = tier == "Diamond Tier" ? 2 : 1;
    }
    else {
        m_colors.emplace_back(1.0f, 1.0f, 1.0f, 1.0f);
    }

    return true;
}

void IDPackCell::draw() {
    if (!m_background) return;

    auto shader = m_background->getShaderProgram();
    if (!shader) return;

    shader->use();
    shader->setUniformsForBuiltins();
    shader->setUniformLocationWith1i(shader->getUniformLocationForName("colorMode"), m_colorMode);
    shader->setUniformLocationWith4fv(shader->getUniformLocationForName("colors"), reinterpret_cast<float*>(m_colors.data()), m_colors.size());
}
