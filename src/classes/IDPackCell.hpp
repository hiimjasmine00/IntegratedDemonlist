#include <cocos2d.h>

class IDPackCell : public cocos2d::CCLayer {
public:
    static IDPackCell* create(const std::string&, double, const std::vector<int>&);
protected:
    bool init(const std::string&, double, const std::vector<int>&);
};
