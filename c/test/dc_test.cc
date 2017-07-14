/*
 dc_test.cc
 Katsuki Ohto
 */

// カーリング基本演算のテスト
#include "../dc.hpp"
#include "../ayumu/ayumu_dc.hpp"

using namespace std;
using namespace DigitalCurling;

namespace DigitalCurling{
        
    // サンプル
    using PB = tuple<array<double, 2>, bool>;
    const vector<PB> freeGuardPositionSample = {
        PB({FX_TEE, FY_TEE}, false),
        PB({FX_TEE - 0.5, FY_TEE - 0.5}, false),
        PB({FX_TEE - 1, FY_TEE - 3}, true)
    };
    
    XorShift64 dice;
    DSFMT ddice;
    
    int testPosition(){
        // 座標テスト
        
        // フリーガードゾーン判定
        for(const auto& p : freeGuardPositionSample){
            
            const fpn_t x = get<0>(p)[0], y = get<0>(p)[1];
            const fPosXY<> pos(x, y);
            const bool ans = get<1>(p);
            
            if(isInFreeGuardZone(x, y) != ans){
                cerr << "free guard zone judge failed." << pos << endl; return -1;
            }
            if(!isInHouse(x, y) && isInFreeGuardZone<1, 0>(x, y) != ans){
                cerr << "free guard zone judge failed." << pos << endl; return -1;
            }
            if(isInPlayArea(x, y) && isInFreeGuardZone<0, 1>(x, y) != ans){
                cerr << "free guard zone judge failed." << pos << endl; return -1;
            }
            if(!isInHouse(x, y) && isInPlayArea(x, y) && isInFreeGuardZone<1, 1>(x, y) != ans){
                cerr << "free guard zone judge failed." << pos << endl; return -1;
            }
            if(isInFreeGuardZone(pos) != ans){
                cerr << "free guard zone judge failed." << pos << endl; return -1;
            }
            if(!isInHouse(pos) && isInFreeGuardZone<1, 0>(pos) != ans){
                cerr << "free guard zone judge failed." << pos << endl; return -1;
            }
            if(isInPlayArea(pos) && isInFreeGuardZone<0, 1>(pos) != ans){
                cerr << "free guard zone judge failed." << pos << endl; return -1;
            }
            if(!isInHouse(pos) && isInPlayArea(pos) && isInFreeGuardZone<1, 1>(pos) != ans){
                cerr << "free guard zone judge failed." << pos << endl; return -1;
            }
        }
        
        return 0;
    }
    
    int testOrderBits(){
        // オーダーのテスト
        // 中心から順に並べたときに正しく順序がとれているか
        return 0;
    }
}


int main(int argc, char *argv[]){

    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    
    std::random_device seed;

    dice.srand(seed() * (unsigned int)time(NULL));
    ddice.srand(seed() * (unsigned int)time(NULL));
    
    if(testPosition()){
        cerr << "failed position test." << endl;
    }
    cerr << "passed position test." << endl;
    
    return 0;
}