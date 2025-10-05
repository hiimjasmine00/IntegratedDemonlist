#include "../IntegratedDemonlist.hpp"
#include <Geode/binding/SetIDPopupDelegate.hpp>
#include <Geode/ui/TextInput.hpp>

class IDPackLayer : public cocos2d::CCLayer, SetIDPopupDelegate {
public:
    static IDPackLayer* create();
    static cocos2d::CCScene* scene();

    void search();
    void page(int);
    void keyDown(cocos2d::enumKeyCodes) override;
    void keyBackClicked() override;
protected:
    geode::EventListener<geode::utils::web::WebTask> m_aredlListener;
    GJListLayer* m_list;
    cocos2d::CCLabelBMFont* m_listLabel;
    LoadingCircle* m_loadingCircle;
    cocos2d::CCMenu* m_searchBarMenu;
    geode::TextInput* m_searchBar;
    cocos2d::CCLabelBMFont* m_countLabel;
    cocos2d::CCLabelBMFont* m_pageLabel;
    CCMenuItemSpriteExtra* m_backButton;
    CCMenuItemSpriteExtra* m_leftButton;
    CCMenuItemSpriteExtra* m_rightButton;
    CCMenuItemSpriteExtra* m_pageButton;
    CCMenuItemSpriteExtra* m_randomButton;
    CCMenuItemSpriteExtra* m_firstButton;
    CCMenuItemSpriteExtra* m_lastButton;
    CCMenuItemSpriteExtra* m_searchButton;
    int m_page = 0;
    std::string m_query;
    std::vector<IDDemonPack> m_fullSearchResults;

    bool init() override;
    std::function<void(int)> failure();
    void showLoading();
    void populateList(const std::string& query);
    void setIDPopupClosed(SetIDPopup*, int) override;
};
