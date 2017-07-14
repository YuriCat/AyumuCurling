/*
 logic.hpp
 Katsuki Ohto
 */

// デジタルカーリング
// 判定

#ifndef DCURLING_AYUMU_LOGIC_LOGIC_HPP_
#define DCURLING_AYUMU_LOGIC_LOGIC_HPP_

#include "../../dc.hpp"

namespace DigitalCurling{
    
    // 絶対位置を離散化
    template<class stone_t>
    int getPositionIndex(const stone_t& st){
        if(!isInActiveZone(st)){
            return 0;
        }else{
            if(st.y > FY_TEE){
                if(isInHouse(st)){
                    int i = 1;
                    if(st.getR() <= (FR_HOUSE_RAD + FR_STONE_RAD) / 3){
                        i += 2;
                    }
                    if(fabs(st.th) > M_PI / 6){
                        i += 1;
                    }
                    return i;
                }else{
                    return 15;
                }
            }else{
                if(isInHouse(st)){
                    int i = 5;
                    if(st.getR() > (FR_HOUSE_RAD + FR_STONE_RAD) / 3){
                        i += 2;
                        if(st.getR() > 2 * (FR_HOUSE_RAD + FR_STONE_RAD) / 3){
                            i += 2;
                        }
                    }
                    if(fabs(st.th) < 5 * M_PI / 6){
                        i += 1;
                    }
                    return i;
                }else{
                    int i = 11;
                    if(st.y < FY_TEE - 2 * FR_HOUSE_RAD){
                        i += 2;
                    }
                    if(fabs(st.x) < (FR_HOUSE_RAD + FR_STONE_RAD) * sin(M_PI / 6)){
                        i += 1;
                    }
                    return i;
                }
            }
        }
        UNREACHABLE;
    }
    
    // 相対位置を離散化
    // 0 ~ 15 の相対位置またばどれにも該当しない(-1)を返す
    template<class stone_t, class relStone_t>
    int labelRelPos(const stone_t& base, const stone_t& obj, const relStone_t& rel){
        if(base.tr >= obj.tr){ // objが手前側
            int bs;
            fpn_t th0 = obj.tth - base.tth;
            
            const fpn_t objRHL[2] = {obj.hitLimit(Spin::RIGHT, 0), obj.hitLimit(Spin::RIGHT, 1)};
            const fpn_t baseRHL[2] = {base.hitLimit(Spin::RIGHT, 0), base.hitLimit(Spin::RIGHT, 1)};
            const fpn_t objLHL[2] = {obj.hitLimit(Spin::LEFT, 0), obj.hitLimit(Spin::LEFT, 1)};
            const fpn_t baseLHL[2] = {base.hitLimit(Spin::LEFT, 0), base.hitLimit(Spin::LEFT, 1)};
            
            //cerr << objRHL[0] << " " << objRHL[1] << endl;
            //cerr << objLHL[0] << " " << objLHL[1] << endl;
            //cerr << baseRHL[0] << " " << baseRHL[1] << endl;
            //cerr << baseLHL[0] << " " << baseLHL[1] << endl;
            
            // 右スピンとの関係
            int rs, ls;
            if(objRHL[1] < (baseRHL[0] + baseRHL[1]) / 2){ // 左側ヒットのとき関係?
                if(objRHL[1] < baseRHL[0]){ // 左端にかすっても当たらない
                    return -1;
                }else{
                    rs = 0;
                }
            }else{
                if(objRHL[0] > baseRHL[1]){ // 右端にかすっても当たらない
                    rs = 2;
                }else{
                    rs = 1;
                }
            }
            // 左スピンとの関係
            if(objLHL[0] < (baseLHL[0] + baseLHL[1]) / 2){ // 左側ヒットのとき関係?
                if(objLHL[1] < baseLHL[0]){ // 左端にかすっても当たらない
                    ls = 2;
                }else{
                    ls = 1;
                }
            }else{
                if(objLHL[0] > baseLHL[1]){ // 右端にかすっても当たらない
                    return -1;
                }else{
                    ls = 0;
                }
            }
            /*
             switch(rs * 3 + ls){
             case 0 * 3 + 0: bs = 2; break;
             case 0 * 3 + 1: bs = 2; break;
             case 0 * 3 + 2: bs = 0; break;
             case 1 * 3 + 0: bs = 2; break;
             case 1 * 3 + 1: bs = 2; break;
             case 1 * 3 + 2: bs = 1; break;
             case 2 * 3 + 0: bs = 4; break;
             case 2 * 3 + 1: bs = 3; break;
             case 2 * 3 + 2: return -1; break;
             default: UNREACHABLE; break;
             }
             switch(rs * 3 + ls){
             case 0 * 3 + 0: bs = 2; break;
             case 0 * 3 + 1: bs = 1; break;
             case 0 * 3 + 2: bs = 0; break;
             case 1 * 3 + 0: bs = 3; break;
             case 1 * 3 + 1: bs = 2; break;
             case 1 * 3 + 2: bs = 1; break;
             case 2 * 3 + 0: bs = 4; break;
             case 2 * 3 + 1: bs = 3; break;
             case 2 * 3 + 2: return -1; break;
             default: UNREACHABLE; break;
             }*/
            if(rs == 2 && ls == 2){return -1;}
            bs = (int)(!(rs & ls)) + (unsigned int)(rel.r > 4 * FR_STONE_RAD) * 2 + (unsigned int)(rel.th > 0) * 4;
            return bs;
        }else{ // objが後方
            fpn_t ath = fabs(rel.th);
            if(rel.r <= 3 * FR_STONE_RAD){ // 近距離
                return 8 + ((unsigned int)(ath > M_PI / 6)) + ((unsigned int)(rel.th > 0)) * 4;
            }else{ // 遠距離
                if(ath > M_PI / 4){
                    return -1;
                }
                return 10 + ((unsigned int)(ath > M_PI / 6)) + ((unsigned int)(rel.th > 0)) * 4;
            }
        }
        UNREACHABLE;
    }
    
    template<class pos0_t, class pos1_t>
    bool isFrontGuard(const pos0_t& ref, const pos1_t& guard, int spin){
        // 判定基準:ある石を狙って、触れる位置に投げられた場合に当たる可能性のあるゾーンに石があるかどうか
        constexpr fpn_t bure = 0.36603324;
        if (guard.getY() < ref.getY()){
            fpn_t minX, maxX;
            if (spin == Spin::RIGHT){
                minX = ref.getX() - bure - 4 * FR_STONE_RAD;
                maxX = ref.getX() + 4 * FR_STONE_RAD;
            }
            else{
                minX = ref.getX() + bure - 4 * FR_STONE_RAD;
                maxX = ref.getX() + 4 * FR_STONE_RAD;
            }
            if (minX <= guard.getX() && guard.getX() <= maxX){
                return true;
            }
        }
        return false;
    }
    
    template<class pos0_t, class pos1_t>
    bool isBackGuard(const pos0_t& ref, const pos1_t& guard){
        // 判定基準:ある石の後方かつハウスから1一個分までの間にある
        if (guard.getY() > ref.getY()){
            if (guard.getR() < FR_HOUSE_RAD + 3 * FR_STONE_RAD){
                return true;
            }
        }
        return false;
    }
    
    template<class pos0_t, class pos1_t>
    bool isTailStone(const pos0_t& ref, const pos1_t& tail){
        // 判定基準:ある石の後方、近辺かつハウス内にある
        if (tail.getY() > ref.getY()){
            if (isInHouse(tail) && calcDistance2(ref, tail) < pow(FR_TAIL_RADIUS_MAX, 2)){
                return true;
            }
        }
        return false;
    }
    
    template<class pos0_t,class pos1_t>
    bool isRelativeFreeDrawPosition(const pos0_t& ref, const pos1_t& stone,int spin){
        // 判定基準:前方ガードが無く、後方一定距離までに石が無い
        constexpr fpn_t bure = 0.36603324;
        if (stone.getY() < ref.getY()){
            return true;
        }else{
            fpn_t minX, maxX;
            if (spin == Spin::RIGHT){
                minX = ref.getX() - bure - 4 * FR_STONE_RAD;
                maxX = ref.getX() + 4 * FR_STONE_RAD;
            }
            else{
                minX = ref.getX() + bure - 4 * FR_STONE_RAD;
                maxX = ref.getX() + 4 * FR_STONE_RAD;
            }
            if (minX <= stone.getX() && stone.getX() <= maxX){
                return true;
            }
        }
        return false;
    }
    
    template<class pos_t,class board_t>
    bool isFreeDrawPosition(const pos_t& ref, const board_t& bd, int spin){
        // 判定基準:前方ガードが無く、後方一定距離までに石が無い
        constexpr fpn_t bure = 0.36603324;
        
        if(0 <= searchStone(bd, [&ref, spin](const auto& st)->bool{
            return !isRelativeFreeDrawPosition(ref, st, spin);
        })){return false;}
        return true;
    }
}

#endif // DCURLING_AYUMU_LOGIC_LOGIC_HPP_
