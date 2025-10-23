#include <Geode/binding/LevelManagerDelegate.hpp>
#include <Geode/binding/SetIDPopupDelegate.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/utils/web.hpp>

class IDListLayer : public cocos2d::CCLayer, SetIDPopupDelegate, LevelManagerDelegate {
public:
    static IDListLayer* create();
    static cocos2d::CCScene* scene();

    void search();
    void page(int);
    void keyDown(cocos2d::enumKeyCodes) override;
    void keyBackClicked() override;

    ~IDListLayer() override;
protected:
    geode::EventListener<geode::utils::web::WebTask> m_aredlListener;
    geode::EventListener<geode::utils::web::WebTask> m_pemonlistListener;
    GJListLayer* m_list;
    cocos2d::CCLabelBMFont* m_listLabel;
    LoadingCircle* m_loadingCircle;
    cocos2d::CCMenu* m_searchBarMenu;
    geode::TextInput* m_searchBar;
    cocos2d::CCLabelBMFont* m_countLabel;
    cocos2d::CCLabelBMFont* m_pageLabel;
    InfoAlertButton* m_infoButton;
    CCMenuItemSpriteExtra* m_backButton;
    CCMenuItemSpriteExtra* m_leftButton;
    CCMenuItemSpriteExtra* m_rightButton;
    CCMenuItemSpriteExtra* m_pageButton;
    CCMenuItemSpriteExtra* m_randomButton;
    CCMenuItemSpriteExtra* m_firstButton;
    CCMenuItemSpriteExtra* m_lastButton;
    CCMenuItemSpriteExtra* m_starToggle;
    CCMenuItemSpriteExtra* m_moonToggle;
    CCMenuItemSpriteExtra* m_searchButton;
    int m_page = 0;
    std::string m_query;
    std::vector<std::string> m_fullSearchResults;
    std::function<void(int)> m_aredlFailure;
    std::function<void(int)> m_pemonlistFailure;

    bool init() override;
    void showLoading();
    void populateList(const std::string& query);
    void loadLevelsFinished(cocos2d::CCArray* levels, const char* key, int) override;
    void loadLevelsFailed(const char* key, int) override;
    void setupPageInfo(gd::string, const char*) override;
    void setIDPopupClosed(SetIDPopup*, int) override;
};
