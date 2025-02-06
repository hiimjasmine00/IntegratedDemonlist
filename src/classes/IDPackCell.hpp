#include "../IntegratedDemonlist.hpp"
#include <cocos2d.h>

class IDPackCell : public cocos2d::CCLayer {
public:
    static IDPackCell* create(const IDDemonPack&);
protected:
    bool init(const IDDemonPack&);
};
