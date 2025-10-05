#include <cocos2d.h>

class IDPackCell : public cocos2d::CCLayer {
public:
    static IDPackCell* create(const std::string&, double, const std::vector<int>&, const std::string&);

    void draw() override;
protected:
    cocos2d::CCSprite* m_background;
    std::vector<cocos2d::ccColor4F> m_colors;
    int m_colorMode = 0;

    bool init(const std::string&, double, const std::vector<int>&, const std::string&);
};
