#include "IDListLayer.hpp"
#include "../IntegratedDemonlist.hpp"
#include <Geode/binding/CustomListView.hpp>
#include <Geode/binding/GameLevelManager.hpp>
#include <Geode/binding/GJListLayer.hpp>
#include <Geode/binding/GJSearchObject.hpp>
#include <Geode/binding/InfoAlertButton.hpp>
#include <Geode/binding/LoadingCircle.hpp>
#include <Geode/binding/SetIDPopup.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/utils/ranges.hpp>
#include <random>

using namespace geode::prelude;

IDListLayer* IDListLayer::create() {
    auto ret = new IDListLayer();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

CCScene* IDListLayer::scene() {
    auto ret = CCScene::create();
    ret->addChild(IDListLayer::create());
    return ret;
}

bool IDListLayer::init() {
    if (!CCLayer::init()) return false;

    setID("IDListLayer");
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

    m_list = GJListLayer::create(CustomListView::create(CCArray::create(), BoomListType::Level, 190.0f, 356.0f),
        pemonlistEnabled ? "Pemonlist" : "All Rated Extreme Demons List", { 0, 0, 0, 180 }, 356.0f, 220.0f, 0);
    m_list->setZOrder(2);
    m_list->setPosition(winSize / 2.0f - m_list->getContentSize() / 2.0f);
    m_list->setID("GJListLayer");
    addChild(m_list);

    addSearchBar();

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

    m_leftButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_arrow_03_001.png", 1.0f, [this](auto) { page(m_page - 1); });
    m_leftButton->setPosition({ 24.0f, winSize.height / 2.0f });
    m_leftButton->setID("prev-page-button");
    menu->addChild(m_leftButton);

    auto rightBtnSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    rightBtnSpr->setFlipX(true);
    m_rightButton = CCMenuItemExt::createSpriteExtra(rightBtnSpr, [this](auto) { page(m_page + 1); });
    m_rightButton->setPosition({ winSize.width - 24.0f, winSize.height / 2.0f });
    m_rightButton->setID("next-page-button");
    menu->addChild(m_rightButton);

    m_infoButton = InfoAlertButton::create(pemonlistEnabled ? "Pemonlist" : "All Rated Extreme Demons List",
        pemonlistEnabled ? pemonlistInfo : aredlInfo, 1.0f);
    m_infoButton->setPosition({ 30.0f, 30.0f });
    m_infoButton->setID("info-button");
    menu->addChild(m_infoButton, 2);

    auto refreshBtnSpr = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
    auto refreshButton = CCMenuItemExt::createSpriteExtra(refreshBtnSpr, [this](auto) {
        showLoading();
        if (pemonlistEnabled) IntegratedDemonlist::loadPemonlist(&m_pemonlistListener, &m_pemonlistOkListener,
            [this] { populateList(m_query); }, failure(true));
        else IntegratedDemonlist::loadAREDL(&m_aredlListener, &m_aredlOkListener, [this] { populateList(m_query); }, failure(false));
    });
    refreshButton->setPosition({ winSize.width - refreshBtnSpr->getContentWidth() / 2.0f - 4.0f, refreshBtnSpr->getContentHeight() / 2.0f + 4.0f });
    refreshButton->setID("refresh-button");
    menu->addChild(refreshButton, 2);

    m_starToggle = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_starsIcon_001.png", 1.1f, [this](auto) {
        if (!pemonlistEnabled) return;
        pemonlistEnabled = false;
        m_starToggle->setColor({ 255, 255, 255 });
        m_moonToggle->setColor({ 125, 125, 125 });
        showLoading();
        if (auto listTitle = static_cast<CCLabelBMFont*>(m_list->getChildByID("title"))) {
            listTitle->setString("All Rated Extreme Demons List");
            listTitle->limitLabelWidth(280.0f, 0.8f, 0.0f);
        }
        m_infoButton->m_title = "All Rated Extreme Demons List";
        m_infoButton->m_description = aredlInfo;
        m_fullSearchResults.clear();
        if (IntegratedDemonlist::aredlLoaded) page(0);
        else IntegratedDemonlist::loadAREDL(&m_aredlListener, &m_aredlOkListener, [this] { page(0); }, failure(false));
    });
    m_starToggle->setPosition({ 30.0f, 60.0f });
    m_starToggle->setColor(pemonlistEnabled ? ccColor3B { 125, 125, 125 } : ccColor3B { 255, 255, 255 });
    m_starToggle->setID("aredl-button");
    menu->addChild(m_starToggle, 2);

    m_moonToggle = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_moonsIcon_001.png", 1.1f, [this](auto) {
        if (pemonlistEnabled) return;
        pemonlistEnabled = true;
        m_starToggle->setColor({ 125, 125, 125 });
        m_moonToggle->setColor({ 255, 255, 255 });
        showLoading();
        if (auto listTitle = static_cast<CCLabelBMFont*>(m_list->getChildByID("title"))) {
            listTitle->setString("Pemonlist");
            listTitle->limitLabelWidth(280.0f, 0.8f, 0.0f);
        }
        m_infoButton->m_title = "Pemonlist";
        m_infoButton->m_description = pemonlistInfo;
        m_fullSearchResults.clear();
        if (IntegratedDemonlist::pemonlistLoaded) page(0);
        else IntegratedDemonlist::loadPemonlist(&m_pemonlistListener, &m_pemonlistOkListener, [this] { page(0); }, failure(true));
    });
    m_moonToggle->setPosition({ 60.0f, 60.0f });
    m_moonToggle->setColor(pemonlistEnabled ? ccColor3B { 255, 255, 255 } : ccColor3B { 125, 125, 125 });
    m_moonToggle->setID("pemonlist-button");
    menu->addChild(m_moonToggle, 2);

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
    // Sprite by Cvolton
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
    m_lastButton = CCMenuItemExt::createSpriteExtra(lastArrow, [this](auto) { page((m_fullSearchResults.size() - 1) / 10); });
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
    m_firstButton = CCMenuItemExt::createSpriteExtra(firstArrow, [this](auto) { page(0); });
    m_firstButton->setPosition({ 21.5f, m_lastButton->getPositionY() });
    m_firstButton->setID("first-button");
    menu->addChild(m_firstButton);

    m_loadingCircle = LoadingCircle::create();
    m_loadingCircle->setParentLayer(this);
    m_loadingCircle->retain();
    m_loadingCircle->show();
    m_loadingCircle->setID("loading-circle");

    showLoading();
    setKeypadEnabled(true);
    setKeyboardEnabled(true);

    if (pemonlistEnabled && IntegratedDemonlist::pemonlistLoaded) populateList("");
    else if (pemonlistEnabled) IntegratedDemonlist::loadPemonlist(&m_pemonlistListener, &m_pemonlistOkListener,
        [this] { populateList(""); }, failure(true));
    else if (IntegratedDemonlist::aredlLoaded) populateList("");
    else IntegratedDemonlist::loadAREDL(&m_aredlListener, &m_aredlOkListener, [this] { populateList(""); }, failure(false));

    return true;
}

std::function<void(int)> IDListLayer::failure(bool platformer) {
    return [this, platformer](int code) {
        FLAlertLayer::create(
            fmt::format("Load Failed ({})", code).c_str(),
            platformer ? "Failed to load Pemonlist. Please try again later." : "Failed to load AREDL. Please try again later.",
            "OK"
        )->show();
        m_loadingCircle->setVisible(false);
    };
}

void IDListLayer::addSearchBar() {
    auto winSize = CCDirector::get()->getWinSize();

    m_searchBarMenu = CCMenu::create();
    m_searchBarMenu->setContentSize({ 356.0f, 30.0f });
    m_searchBarMenu->setPosition({ 0.0f, 190.0f });
    m_searchBarMenu->setID("search-bar-menu");
    m_list->addChild(m_searchBarMenu);

    auto searchBackground = CCLayerColor::create({ 194, 114, 62, 255 }, 356.0f, 30.0f);
    searchBackground->setID("search-bar-background");
    m_searchBarMenu->addChild(searchBackground);

    if (!m_query.empty()) {
        auto searchButton = CCMenuItemExt::createSpriteExtraWithFilename("ID_findBtnOn_001.png"_spr, 0.7f, [this](auto) { search(); });
        searchButton->setPosition({ 337.0f, 15.0f });
        searchButton->setID("search-button");
        m_searchBarMenu->addChild(searchButton);
    } else {
        auto searchButton = CCMenuItemExt::createSpriteExtraWithFrameName("gj_findBtn_001.png", 0.7f, [this](auto) { search(); });
        searchButton->setPosition({ 337.0f, 15.0f });
        searchButton->setID("search-button");
        m_searchBarMenu->addChild(searchButton);
    }

    m_searchBar = TextInput::create(413.3f, "Search Demons...");
    m_searchBar->setCommonFilter(CommonFilter::Any);
    m_searchBar->setPosition({ 165.0f, 15.0f });
    m_searchBar->setTextAlign(TextInputAlign::Left);
    m_searchBar->getInputNode()->setLabelPlaceholderScale(0.53f);
    m_searchBar->getInputNode()->setMaxLabelScale(0.53f);
    m_searchBar->setScale(0.75f);
    m_searchBar->setCallback([this](const std::string& text) { m_searchBarText = text; });
    m_searchBar->setID("search-bar");
    m_searchBarMenu->addChild(m_searchBar);
}

void IDListLayer::showLoading() {
    m_pageLabel->setString(std::to_string(m_page + 1).c_str());
    m_loadingCircle->setVisible(true);
    m_list->m_listView->setVisible(false);
    m_searchBarMenu->setVisible(false);
    m_countLabel->setVisible(false);
    m_leftButton->setVisible(false);
    m_rightButton->setVisible(false);
    m_firstButton->setVisible(false);
    m_lastButton->setVisible(false);
    m_pageButton->setVisible(false);
    m_randomButton->setVisible(false);
}

void IDListLayer::populateList(const std::string& query) {
    m_fullSearchResults = ranges::reduce<std::vector<std::string>>(pemonlistEnabled ? IntegratedDemonlist::pemonlist : IntegratedDemonlist::aredl,
        [&query](std::vector<std::string>& acc, const IDListDemon& level) {
            if (!query.empty() && !string::contains(string::toLower(level.name), string::toLower(query))) return;
            acc.push_back(std::to_string(level.id));
        });

    m_query = query;

    if (m_fullSearchResults.empty()) {
        loadLevelsFinished(CCArray::create(), "", 0);
        m_countLabel->setString("");
    }
    else {
        auto glm = GameLevelManager::get();
        glm->m_levelManagerDelegate = this;
        auto searchObject = GJSearchObject::create(SearchType::Type19, string::join(std::vector<std::string>(
            m_fullSearchResults.begin() + m_page * 10,
            m_fullSearchResults.begin() + std::min((int)m_fullSearchResults.size(), (m_page + 1) * 10)
        ), ","));
        std::string key = searchObject->getKey();
        if (auto storedLevels = glm->getStoredOnlineLevels(key.substr(std::max(0, (int)key.size() - 256)).c_str())) {
            loadLevelsFinished(storedLevels, "", 0);
            setupPageInfo("", "");
        }
        else glm->getOnlineLevels(searchObject);
    }
}

void IDListLayer::loadLevelsFinished(CCArray* levels, const char*, int) {
    auto winSize = CCDirector::get()->getWinSize();
    if (m_list->getParent() == this) removeChild(m_list);
    m_list = GJListLayer::create(CustomListView::create(levels, BoomListType::Level, 190.0f, 356.0f),
        pemonlistEnabled ? "Pemonlist" : "All Rated Extreme Demons List", { 0, 0, 0, 180 }, 356.0f, 220.0f, 0);
    m_list->setZOrder(2);
    m_list->setPosition(winSize / 2.0f - m_list->getContentSize() / 2.0f);
    m_list->setID("GJListLayer");
    addChild(m_list);
    addSearchBar();
    m_searchBar->setString(m_searchBarText);
    m_countLabel->setVisible(true);
    m_loadingCircle->setVisible(false);
    if (m_fullSearchResults.size() > 10) {
        auto maxPage = (m_fullSearchResults.size() - 1) / 10;
        m_leftButton->setVisible(m_page > 0);
        m_rightButton->setVisible(m_page < maxPage);
        m_firstButton->setVisible(m_page > 0);
        m_lastButton->setVisible(m_page < maxPage);
        m_pageButton->setVisible(true);
        m_randomButton->setVisible(true);
    }
}

void IDListLayer::loadLevelsFailed(const char*, int) {
    m_searchBarMenu->setVisible(true);
    m_countLabel->setVisible(true);
    m_loadingCircle->setVisible(false);
    FLAlertLayer::create("Load Failed", "Failed to load levels. Please try again later.", "OK")->show();
}

void IDListLayer::setupPageInfo(gd::string, const char*) {
    m_countLabel->setString(fmt::format("{} to {} of {}", m_page * 10 + 1,
        std::min((int)m_fullSearchResults.size(), (m_page + 1) * 10), m_fullSearchResults.size()).c_str());
    m_countLabel->limitLabelWidth(100.0f, 0.6f, 0.0f);
}

void IDListLayer::search() {
    if (m_query != m_searchBarText) {
        showLoading();
        if (pemonlistEnabled) IntegratedDemonlist::loadPemonlist(&m_pemonlistListener, &m_pemonlistOkListener, [this] {
            m_page = 0;
            populateList(m_searchBarText);
        }, failure(true));
        else IntegratedDemonlist::loadAREDL(&m_aredlListener, &m_aredlOkListener, [this] {
            m_page = 0;
            populateList(m_searchBarText);
        }, failure(false));
    }
}

void IDListLayer::page(int page) {
    auto maxPage = (m_fullSearchResults.size() + 9) / 10;
    m_page = maxPage > 0 ? (maxPage + (page % maxPage)) % maxPage : 0;
    showLoading();
    populateList(m_query);
}

void IDListLayer::keyDown(enumKeyCodes key) {
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

void IDListLayer::keyBackClicked() {
    CCDirector::get()->popSceneWithTransition(0.5f, kPopTransitionFade);
}

void IDListLayer::setIDPopupClosed(SetIDPopup*, int page) {
    m_page = std::min(std::max(page - 1, 0), ((int)m_fullSearchResults.size() - 1) / 10);
    showLoading();
    populateList(m_query);
}

IDListLayer::~IDListLayer() {
    CC_SAFE_RELEASE(m_loadingCircle);
    auto glm = GameLevelManager::get();
    if (glm->m_levelManagerDelegate == this) glm->m_levelManagerDelegate = nullptr;
}
