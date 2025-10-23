#include "IDPackLayer.hpp"
#include "IDPackCell.hpp"
#include <Geode/binding/AppDelegate.hpp>
#include <Geode/binding/GJListLayer.hpp>
#include <Geode/binding/InfoAlertButton.hpp>
#include <Geode/binding/LoadingCircle.hpp>
#include <Geode/binding/SetIDPopup.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/ui/ListView.hpp>
#include <Geode/utils/ranges.hpp>
#include <random>

using namespace geode::prelude;

constexpr const char* aredlPackInfo =
    "The <cg>All Rated Extreme Demons List</c> (<cg>AREDL</c>) has <cp>packs</c> of <cr>extreme demons</c> that are <cj>related</c> in some way.\n"
    "If all levels in a pack are <cl>completed</c>, the pack can earn <cy>points</c> on <cg>aredl.net</c>.";

IDPackLayer* IDPackLayer::create() {
    auto ret = new IDPackLayer();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

CCScene* IDPackLayer::scene() {
    auto ret = CCScene::create();
    AppDelegate::get()->m_runningScene = ret;
    ret->addChild(IDPackLayer::create());
    return ret;
}

bool IDPackLayer::init() {
    if (!CCLayer::init()) return false;

    setID("IDPackLayer");
    auto winSize = CCDirector::get()->getWinSize();

    auto bg = CCSprite::create("GJ_gradientBG.png");
    bg->setAnchorPoint({ 0.0f, 0.0f });
    bg->setScaleX((winSize.width + 10.0f) / bg->getTextureRect().size.width);
    bg->setScaleY((winSize.height + 10.0f) / bg->getTextureRect().size.height);
    bg->setPosition({ -5.0f, -5.0f });
    bg->setColor({ 51, 51, 51 });
    bg->setID("background");
    addChild(bg);

    auto bottomLeftCorner = CCSprite::createWithSpriteFrameName("gauntletCorner_001.png");
    bottomLeftCorner->setPosition({ -1.0f, -1.0f });
    bottomLeftCorner->setAnchorPoint({ 0.0f, 0.0f });
    bottomLeftCorner->setID("left-corner");
    addChild(bottomLeftCorner);

    auto bottomRightCorner = CCSprite::createWithSpriteFrameName("gauntletCorner_001.png");
    bottomRightCorner->setPosition({ winSize.width + 1.0f, -1.0f });
    bottomRightCorner->setAnchorPoint({ 1.0f, 0.0f });
    bottomRightCorner->setFlipX(true);
    bottomRightCorner->setID("right-corner");
    addChild(bottomRightCorner);

    m_countLabel = CCLabelBMFont::create("", "goldFont.fnt");
    m_countLabel->setAnchorPoint({ 1.0f, 1.0f });
    m_countLabel->setScale(0.6f);
    m_countLabel->setPosition({ winSize.width - 7.0f, winSize.height - 3.0f });
    m_countLabel->setID("level-count-label");
    addChild(m_countLabel);

    m_list = GJListLayer::create(nullptr, "AREDL Packs", { 0, 0, 0, 180 }, 356.0f, 220.0f, 0);
    m_list->setPosition(winSize / 2.0f - m_list->getContentSize() / 2.0f);
    m_list->setID("GJListLayer");
    addChild(m_list, 2);

    m_searchBarMenu = CCMenu::create();
    m_searchBarMenu->setContentSize({ 356.0f, 30.0f });
    m_searchBarMenu->setPosition({ 0.0f, 190.0f });
    m_searchBarMenu->setID("search-bar-menu");
    m_list->addChild(m_searchBarMenu);

    auto searchBackground = CCLayerColor::create({ 194, 114, 62, 255 }, 356.0f, 30.0f);
    searchBackground->setID("search-bar-background");
    m_searchBarMenu->addChild(searchBackground);

    m_searchButton = CCMenuItemExt::createSpriteExtraWithFrameName("gj_findBtn_001.png", 0.7f, [this](auto) {
        search();
    });
    m_searchButton->setPosition({ 337.0f, 15.0f });
    m_searchButton->setID("search-button");
    m_searchBarMenu->addChild(m_searchButton);

    m_searchBar = TextInput::create(413.3f, "Search Packs...");
    m_searchBar->setPosition({ 165.0f, 15.0f });
    m_searchBar->setTextAlign(TextInputAlign::Left);
    auto inputNode = m_searchBar->getInputNode();
    inputNode->setLabelPlaceholderScale(0.53f);
    inputNode->setMaxLabelScale(0.53f);
    m_searchBar->setScale(0.75f);
    m_searchBar->setID("search-bar");
    m_searchBarMenu->addChild(m_searchBar);

    auto menu = CCMenu::create();
    menu->setPosition({ 0.0f, 0.0f });
    menu->setID("button-menu");
    addChild(menu);

    m_backButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_arrow_01_001.png", 1.0f, [this](auto) {
        CCDirector::get()->popSceneWithTransition(0.5f, kPopTransitionFade);
    });
    m_backButton->setPosition({ 25.0f, winSize.height - 25.0f });
    m_backButton->setID("back-button");
    menu->addChild(m_backButton);

    m_leftButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_arrow_03_001.png", 1.0f, [this](auto) {
        page(m_page - 1);
    });
    m_leftButton->setPosition({ 24.0f, winSize.height / 2.0f });
    m_leftButton->setID("prev-page-button");
    menu->addChild(m_leftButton);

    auto rightBtnSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    rightBtnSpr->setFlipX(true);
    m_rightButton = CCMenuItemExt::createSpriteExtra(rightBtnSpr, [this](auto) {
        page(m_page + 1);
    });
    m_rightButton->setPosition({ winSize.width - 24.0f, winSize.height / 2.0f });
    m_rightButton->setID("next-page-button");
    menu->addChild(m_rightButton);

    auto infoButton = InfoAlertButton::create("AREDL Packs", aredlPackInfo, 1.0f);
    infoButton->setPosition({ 30.0f, 30.0f });
    infoButton->setID("info-button");
    menu->addChild(infoButton, 2);

    m_aredlFailure = [this](int code) {
        FLAlertLayer::create(fmt::format("Load Failed ({})", code).c_str(), "Failed to load AREDL packs. Please try again later.", "OK")->show();
        m_loadingCircle->setVisible(false);
    };

    auto refreshBtnSpr = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
    auto refreshButton = CCMenuItemExt::createSpriteExtra(refreshBtnSpr, [this](auto) {
        showLoading();
        IntegratedDemonlist::loadAREDLPacks(m_aredlListener, [this] {
            populateList(m_query);
        }, m_aredlFailure);
    });
    refreshButton->setPosition(winSize.width - refreshBtnSpr->getContentWidth() / 2.0f - 4.0f, refreshBtnSpr->getContentHeight() / 2.0f + 4.0f);
    refreshButton->setID("refresh-button");
    menu->addChild(refreshButton, 2);

    auto pageBtnSpr = CCSprite::create("GJ_button_02.png");
    pageBtnSpr->setScale(0.7f);
    m_pageLabel = CCLabelBMFont::create("1", "bigFont.fnt");
    m_pageLabel->setScale(0.8f);
    m_pageLabel->setPosition(pageBtnSpr->getContentSize() / 2.0f);
    pageBtnSpr->addChild(m_pageLabel);
    m_pageButton = CCMenuItemExt::createSpriteExtra(pageBtnSpr, [this](auto) {
        auto popup = SetIDPopup::create(m_page + 1, 1, (m_fullSearchResults.size() + 9) / 10, "Go to Page", "Go", true, 1, 60.0f, false, false);
        popup->m_delegate = this;
        popup->show();
    });
    m_pageButton->setPositionY(winSize.height - 39.5f);
    m_pageButton->setID("page-button");
    menu->addChild(m_pageButton);

    m_randomButton = CCMenuItemExt::createSpriteExtraWithFilename("BI_randomBtn_001.png"_spr, 0.9f, [this](auto) {
        static std::mt19937 mt(std::random_device{}());
        page(std::uniform_int_distribution<int>(0, (m_fullSearchResults.size() - 1) / 10)(mt));
    });
    m_randomButton->setPositionY(
        m_pageButton->getPositionY() - m_pageButton->getContentHeight() / 2.0f - m_randomButton->getContentHeight() / 2.0f - 5.0f);
    m_randomButton->setID("random-button");
    menu->addChild(m_randomButton);

    auto lastArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    lastArrow->setFlipX(true);
    auto otherLastArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    otherLastArrow->setPosition(lastArrow->getContentSize() / 2.0f + CCPoint { 20.0f, 0.0f });
    otherLastArrow->setFlipX(true);
    lastArrow->addChild(otherLastArrow);
    lastArrow->setScale(0.4f);
    m_lastButton = CCMenuItemExt::createSpriteExtra(lastArrow, [this](auto) {
        page((m_fullSearchResults.size() - 1) / 10);
    });
    m_lastButton->setPositionY(
        m_randomButton->getPositionY() - m_randomButton->getContentHeight() / 2.0f - m_lastButton->getContentHeight() / 2.0f - 5.0f);
    m_lastButton->setID("last-button");
    menu->addChild(m_lastButton);

    auto x = winSize.width - m_randomButton->getContentWidth() / 2.0f - 3.0f;
    m_pageButton->setPositionX(x);
    m_randomButton->setPositionX(x);
    m_lastButton->setPositionX(x - 4.0f);

    auto firstArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    auto otherFirstArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    otherFirstArrow->setPosition(firstArrow->getContentSize() / 2.0f - CCPoint { 20.0f, 0.0f });
    firstArrow->addChild(otherFirstArrow);
    firstArrow->setScale(0.4f);
    m_firstButton = CCMenuItemExt::createSpriteExtra(firstArrow, [this](auto) {
        page(0);
    });
    m_firstButton->setPosition({ 21.5f, m_lastButton->getPositionY() });
    m_firstButton->setID("first-button");
    menu->addChild(m_firstButton);

    m_loadingCircle = LoadingCircle::create();
    m_loadingCircle->setParentLayer(this);
    m_loadingCircle->setID("loading-circle");
    m_loadingCircle->show();

    showLoading();
    setKeypadEnabled(true);
    setKeyboardEnabled(true);

    auto shaderCache = CCShaderCache::sharedShaderCache();
    if (!shaderCache->programForKey("pack-gradient"_spr)) {
        auto shader = new CCGLProgram();
        if (shader->initWithVertexShaderFilename("gradient.vert"_spr, "gradient.frag"_spr)) {
            shader->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position);
            shader->addAttribute(kCCAttributeNameColor, kCCVertexAttrib_Color);
            shader->addAttribute(kCCAttributeNameTexCoord, kCCVertexAttrib_TexCoords);
            shader->link();
            shader->updateUniforms();
            shaderCache->addProgram(shader, "pack-gradient"_spr);
            shader->release();
        }
        else {
            int length, written;
            glGetShaderiv(shader->m_uFragShader, GL_INFO_LOG_LENGTH, &length);
            if (length > 0) {
                auto log = new char[length];
                glGetShaderInfoLog(shader->m_uFragShader, length, &written, log);
                log::error("Failed to compile pack gradient shader:\n{}", log);
                delete[] log;
            }
            else {
                log::error("Failed to compile pack gradient shader");
            }
            shader->release();
            shader = nullptr;
        }
    }

    if (!IntegratedDemonlist::aredlPacks.empty()) {
        populateList("");
    }
    else {
        IntegratedDemonlist::loadAREDLPacks(m_aredlListener, [this] {
            populateList("");
        }, m_aredlFailure);
    }

    return true;
}

void IDPackLayer::showLoading() {
    m_pageLabel->setString(fmt::to_string(m_page + 1).c_str());
    m_loadingCircle->setVisible(true);
    if (auto listView = m_list->m_listView) listView->setVisible(false);
    m_searchBarMenu->setVisible(false);
    m_countLabel->setVisible(false);
    m_leftButton->setVisible(false);
    m_rightButton->setVisible(false);
    m_firstButton->setVisible(false);
    m_lastButton->setVisible(false);
    m_pageButton->setVisible(false);
    m_randomButton->setVisible(false);
}

void IDPackLayer::populateList(const std::string& query) {
    m_fullSearchResults.clear();
    auto searchSprite = static_cast<CCSprite*>(m_searchButton->getNormalImage());
    if (query.empty()) {
        m_fullSearchResults = IntegratedDemonlist::aredlPacks;
        searchSprite->setDisplayFrame(CCSpriteFrameCache::get()->spriteFrameByName("gj_findBtn_001.png"));
    }
    else {
        auto lowerQuery = string::toLower(query);
        for (auto& pack : IntegratedDemonlist::aredlPacks) {
            if (!string::toLower(pack.name).contains(lowerQuery)) continue;
            m_fullSearchResults.push_back(pack);
        }
        auto texture = CCTextureCache::get()->addImage("ID_findBtnOn_001.png"_spr, false);
        searchSprite->setDisplayFrame(CCSpriteFrame::createWithTexture(texture, { { 0.0f, 0.0f }, texture->getContentSize() }));
    }

    m_query = query;

    if (auto listView = m_list->m_listView) {
        listView->removeFromParent();
        listView->release();
    }

    auto packs = CCArray::create();
    auto start = m_page * 10;
    auto size = m_fullSearchResults.size();
    auto end = std::min<int>(size, (m_page + 1) * 10);
    auto endIt = m_fullSearchResults.begin() + end;
    for (auto it = m_fullSearchResults.begin() + start; it < endIt; ++it) {
        packs->addObject(IDPackCell::create(it->name, it->points, it->levels, it->tierName));
    }
    auto listView = ListView::create(packs, 100.0f, 356.0f, 190.0f);
    listView->retain();
    m_list->addChild(listView, 6, 9);
    m_list->m_listView = listView;

    m_searchBarMenu->setVisible(true);
    m_countLabel->setString(fmt::format("{} to {} of {}", start + 1, end, size).c_str());
    m_countLabel->limitLabelWidth(100.0f, 0.6f, 0.0f);
    m_countLabel->setVisible(true);
    m_loadingCircle->setVisible(false);
    if (size > 10) {
        auto maxPage = (size - 1) / 10;
        m_leftButton->setVisible(m_page > 0);
        m_rightButton->setVisible(m_page < maxPage);
        m_firstButton->setVisible(m_page > 0);
        m_lastButton->setVisible(m_page < maxPage);
        m_pageButton->setVisible(true);
        m_randomButton->setVisible(true);
    }
}

void IDPackLayer::search() {
    auto query = m_searchBar->getString();
    if (m_query != query) {
        showLoading();
        IntegratedDemonlist::loadAREDLPacks(m_aredlListener, [this, query] {
            m_page = 0;
            populateList(query);
        }, m_aredlFailure);
    }
}

void IDPackLayer::page(int page) {
    auto maxPage = (m_fullSearchResults.size() + 9) / 10;
    m_page = maxPage > 0 ? (maxPage + (page % maxPage)) % maxPage : 0;
    showLoading();
    populateList(m_query);
}

void IDPackLayer::keyDown(enumKeyCodes key) {
    switch (key) {
        case KEY_Left:
        case CONTROLLER_Left:
            if (m_leftButton->isVisible()) page(m_page - 1);
            break;
        case KEY_Right:
        case CONTROLLER_Right:
            if (m_rightButton->isVisible()) page(m_page + 1);
            break;
        case KEY_Enter:
            search();
            break;
        default:
            CCLayer::keyDown(key);
            break;
    }
}

void IDPackLayer::keyBackClicked() {
    CCDirector::get()->popSceneWithTransition(0.5f, kPopTransitionFade);
}

void IDPackLayer::setIDPopupClosed(SetIDPopup*, int page) {
    m_page = std::clamp<int>(page - 1, 0, (m_fullSearchResults.size() - 1) / 10);
    showLoading();
    populateList(m_query);
}
