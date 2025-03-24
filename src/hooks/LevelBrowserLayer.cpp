#include "../classes/IDPackLayer.hpp"
#include <Geode/binding/GJSearchObject.hpp>
#include <Geode/modify/LevelBrowserLayer.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>

using namespace geode::prelude;

class $modify(IDLevelBrowserLayer, LevelBrowserLayer) {
    bool init(GJSearchObject* object) {
        if (!LevelBrowserLayer::init(object)) return false;

        if (object->m_searchType != SearchType::MapPack) return true;

        auto winSize = CCDirector::get()->getWinSize();
        auto demonlistButtonSprite = CircleButtonSprite::createWithSprite("ID_demonBtn_001.png"_spr);
        demonlistButtonSprite->getTopNode()->setScale(1.0f);
        auto demonlistButton = CCMenuItemSpriteExtra::create(demonlistButtonSprite, this, menu_selector(IDLevelBrowserLayer::onDemonlistPacks));
        demonlistButton->setID("demonlist-button"_spr);
        auto y = demonlistButtonSprite->getContentHeight() / 2.0f + 4.0f;
        auto menu = CCMenu::create();
        menu->addChild(demonlistButton);
        menu->setPosition({ winSize.width - y, y });
        menu->setID("demonlist-menu"_spr);
        addChild(menu, 2);

        return true;
    }

    void onDemonlistPacks(CCObject* sender) {
        CCDirector::get()->pushScene(CCTransitionFade::create(0.5f, IDPackLayer::scene()));
    }
};
