/*
 takeOut.hpp
 Katsuki Ohto
 */

// デジタルカーリング
// フロントガードやバックガードを避けてテイクアウト

#ifndef DCURLING_SHOT_TAKEOUT_HPP_
#define DCURLING_SHOT_TAKEOUT_HPP_

#include "../ayumu_dc.hpp"
#include "params.h"
#include "shotGenerator.hpp"

namespace DigitalCurling{
    struct TakeOut : public ShotGenerator{
        
        constexpr static int number()noexcept{ return Standard::TAKEOUT; }
        constexpr static int dimension()noexcept{ return 1; }
        constexpr static int flexibility()noexcept{ return 1; }
        constexpr static bool hasContact()noexcept{ return true; }
        constexpr static fpn_t baseSpeed()noexcept{ return 32.5; }
        
        template<class board_t, class vmove_t>
        static double estimate(const board_t&, uint32_t, const vmove_t&){
            return 0;
        }
        
        template<class board0_t, class board1_t, class vmove_t>
        static double evaluate(const board0_t& pre, const board1_t& post, uint32_t stNum, const vmove_t& vmv, const ContactTree& ct = 0){
            // テイクした石が残っているかどうかで判定
            uint32_t obj = vmv.getNum0();
            return (!post.sb().test(obj) || !isInActiveZone(post.stone(obj))) ? 1.0 : ((!isInHouse(post.stone(obj))) ? 0.9 : 0.0);
        }
        
        template<class move_t, class board_t, class vmove_t>
        double realize(move_t *const pmv, const board_t& bd, const vmove_t& vmv){
            pmv->setSpin(vmv.getSpin());
            return genReal1(pmv, bd, bd.stone(vmv.getNum0()));
        }
        
        template<class move_t, class board_t, class pos_t>
        double generateFreeSpin1(move_t *const pmv, const board_t& bd, const pos_t& pos);
        
        template<class move_t, class board_t, class pos_t>
        double genReal1(move_t *const pmv, const board_t&, const pos_t& pos);
        
        template<class board_t, class dice_t>
        void setSampleState(board_t *const, dice_t *const);
        
        void init(){};
        
        TakeOut(){}
        ~TakeOut(){}
    };
    
    /*
     template<class move_t,class board_t, class pos_t>
     double TakeOut::generateFreeSpin1(move_t *const pmv, const board_t& bd, const pos_t& pos){
     
     //石投げる点からの位置
     fpn_t tr = pos.tr;
     fpn_t tth = pos.tth;
     
     fpn_t length[2] = {-1, -1};
     std::array<double, 2> maxZone[2];
     
     //右回転と左回転の良い方を選択
     for(int s=0; s<2; ++s){
     //この回転での角度設定
     fpn_t left,right;
     left = hitLimitTable.at<0>(tr);
     right = hitLimitTable.at<1>(tr);
     if(s != Spin::RIGHT){
     std::swap(right, left);
     right = -right;left = -left;
     }
     
     //cerr<<left<<","<<right<<endl;
     
     LineZone<double> zone(tth + left, tth + right);//ヒット出来るゾーン
     
     //cerr<<zone.toString()<<endl;
     
     zone.compress(0.8);//テイク出来そうなゾーン
     
     //front guard
     iterate(pos.frontGuard(s), [&bd, &zone, left, right, s](int idx)->void{
     fpn_t trFG = calcDistanceThrow(bd.stone(idx));
     fpn_t tthFG = calcRelativeAngleThrow(bd.stone(idx));
     fpn_t leftFG,rightFG;
     
     leftFG = hitLimitTable.at<0>(trFG);
     rightFG = hitLimitTable.at<1>(trFG);
     if(s != Spin::RIGHT){
     std::swap(rightFG, leftFG);
     rightFG = -rightFG;leftFG = -leftFG;
     }
     
     zone.limit_out(tthFG + leftFG, tthFG + rightFG);
     });
     
     //back guard
     
     //集計
     if(zone.any()){
     maxZone[s] = zone.searchMaxZone();
     length[s] = std::get<1>(maxZone[s]) - std::get<0>(maxZone[s]);
     //cerr<<std::get<0>(maxZone[s])<<endl;
     //cerr<<std::get<1>(maxZone[s])<<endl;
     //cerr<<OutSpin(s)<<" "<<zone.toString()<<" length = "<<length[s]<<endl;
     }
     }
     //getchar();
     
     if(length[0] < 0 && length[1] < 0){ return -1; }
     
     fpn_t maxLength;
     if(length[0]>length[1]){
     pmv->setPCS(baseSpeed(), (maxZone[0][0]+maxZone[0][1])/2, 0);
     maxLength=length[0];
     }else{
     pmv->setPCS(baseSpeed(), (maxZone[1][0]+maxZone[1][1])/2, 1);
     maxLength=length[1];
     }
     //cerr<<*pmv<<" length = "<<maxLength<<endl;
     
     //成功率を計算
     return 0.5;
     }
     */
    
    template<class move_t,class board_t,class pos_t>
    double TakeOut::genReal1(move_t *const pmv, const board_t& bd, const pos_t& pos){
        
        Spin s = pmv->getSpin();
        
        //石投げる点からの位置
        fpn_t tr = pos.tr;
        fpn_t tth = pos.tth;
        fpn_t left = hitLimitTable.at<0>(tr);
        fpn_t right = hitLimitTable.at<1>(tr);
        
        if(isRight(s)){
            left += tth; right += tth;
            for(BitSet16 bs = pos.frontGuard(s); bs.any(); bs.pop_lsb()){
                int idx = bs.bsf();
                fpn_t trFG = calcDistanceThrow(bd.stone(idx));
                fpn_t tthFG = calcRelativeAngleThrow(bd.stone(idx));
                
                fpn_t leftFG = hitLimitTable.at<0>(trFG) + tthFG;
                fpn_t rightFG = hitLimitTable.at<1>(trFG) + tthFG;
                
                if(rightFG < left){
                }else if(rightFG >= right){
                    if(leftFG <= left){
                        genHit(pmv, pos, baseSpeed()); return -1;
                    }else{
                        right = leftFG;
                    }
                }else{
                    left = rightFG;
                }
            }
            pmv->setPCS(baseSpeed(), (left + right) / 2 , s);
        }else{
            left -= tth; right -= tth;
            for(BitSet16 bs = pos.frontGuard(s); bs.any(); bs.pop_lsb()){
                int idx = bs.bsf();
                fpn_t trFG = calcDistanceThrow(bd.stone(idx));
                fpn_t tthFG = calcRelativeAngleThrow(bd.stone(idx));
                
                fpn_t leftFG = hitLimitTable.at<0>(trFG) - tthFG;
                fpn_t rightFG = hitLimitTable.at<1>(trFG) - tthFG;
                
                if(rightFG < left){
                }else if(rightFG >= right){
                    if(leftFG <= left){
                        genHit(pmv, pos, baseSpeed()); return -1;
                    }else{
                        right = leftFG;
                    }
                }else{
                    left = rightFG;
                }
            }
            pmv->setPCS(baseSpeed(), (left + right) / (-2) , s);
        }
        // 成功率を計算
        return 0.5;
    }
    
    template<class board_t, class dice_t>
    void TakeOut::setSampleState(board_t *const pbd, dice_t *const pdice){
        
        //put 1 stone in House
        fPosXY<> pos;
        locateInHouse(&pos, pdice);
        
        fpn_t dr = pdice->drand() * 5 + 1.5;
        fpn_t r = calcDistance(pos, FPOSXY_THROW);
        
        fpn_t ran0, ran1;
        
        NormalDistribution<double> norm(0, 0.4);
        norm.rand(&ran0, &ran1, pdice);
        
        pbd->pushStone(BLACK, pos);
        pbd->pushStone(BLACK,
                       fPosXY<>(FX_THROW + (pos.x - FX_THROW) * (r - dr) / r + ran0,
                                FY_THROW + (pos.y - FY_THROW) * (r - dr) / r + ran1)
                       );
    }
}

#endif // DCURLING_SHOT_TAKEOUT_HPP_