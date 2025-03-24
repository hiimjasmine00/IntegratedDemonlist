#include <Geode/binding/SetIDPopupDelegate.hpp>
#include <Geode/binding/LevelManagerDelegate.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/utils/web.hpp>

class IDListLayer : public cocos2d::CCLayer, SetIDPopupDelegate, LevelManagerDelegate {
private:
    inline static bool PEMONLIST = false;
    inline static constexpr const char* AREDL_INFO =
        "The <cg>All Rated Extreme Demons List</c> (<cg>AREDL</c>) is an <cp>unofficial ranking</c> "
        "of all rated <cj>classic mode</c> <cr>extreme demons</c> in Geometry Dash.\n"
        "It is managed by <cy>Padahk</c> and <cy>ley</c>.";
    inline static constexpr const char* PEMONLIST_INFO =
        "The <cg>Pemonlist</c> is an <cp>unofficial ranking</c> of the top 150 <cj>platformer mode</c> <cr>demons</c> in Geometry Dash.\n"
        "It is managed by <cy>camila314</c>, <cy>Extatica</c>, <cy>IvanCrafter026</c>, <cy>Megu</c>, and <cy>Voiddle</c>.";
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
    geode::EventListener<geode::utils::web::WebTask> m_aredlOkListener;
    geode::EventListener<geode::utils::web::WebTask> m_pemonlistListener;
    geode::EventListener<geode::utils::web::WebTask> m_pemonlistOkListener;
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
    int m_page = 0;
    std::string m_query = "";
    std::string m_searchBarText = "";
    std::vector<std::string> m_fullSearchResults;

    bool init() override;
    std::function<void(int)> failure(bool);
    void addSearchBar();
    void showLoading();
    void populateList(const std::string& query);
    void loadLevelsFinished(cocos2d::CCArray* levels, const char* key, int) override;
    void loadLevelsFailed(const char* key, int) override;
    void setupPageInfo(gd::string, const char*) override;
    void setIDPopupClosed(SetIDPopup*, int) override;
};
