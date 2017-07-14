/*
 peel.hpp
 Katsuki Ohto
 */

// デジタルカーリング
// peel shot

#ifndef DCURLING_SHOT_PEEL_HPP_
#define DCURLING_SHOT_PEEL_HPP_

#include "shotGenerator.hpp"

namespace DigitalCurling{
    
    //double peelTThTable[64];
    
    struct Peel : public ShotGenerator{
        
        constexpr static int number()noexcept{ return Standard::PEEL; }
        constexpr static int dimension()noexcept{ return 1; }
        constexpr static int flexibility()noexcept{ return 1; }
        constexpr static bool hasContact()noexcept{ return true; }
        constexpr static fpn_t baseSpeed()noexcept{ return FV_MAX - 0.3; }
        
        template<class board_t, class vmove_t>
        static double estimate(const board_t& bd, uint32_t stNum, const vmove_t& vmv){
            // アイデア...ハウスの向こう側の境界線との距離が小さい方が成功しやすい
            const uint32_t obj = vmv.getNum0();
            const auto& st = bd.stone(obj);
            fpn_t r;
            if(st.y > FY_TEE){ // ティーより向こう側
                if(isInHouse(st)){ // ハウス内
                    r = FR_IN_HOUSE - st.getR();
                }else{ // ハウス外
                    return 1.0;
                }
            }else{ // ティーよりこちら側
                r = XYtoR(FR_IN_HOUSE - fabs(st.x - FX_TEE), st.y - FY_TEE);
            }
            return exp(-r / FR_IN_HOUSE);
        }
        
        template<class board0_t, class board1_t, class vmove_t>
        static double evaluate(const board0_t& pre, const board1_t& post, uint32_t stNum, const vmove_t& vmv, const ContactTree& ct = 0){

            const uint32_t obj = vmv.getNum0();
            const double level = vmv.getNum1() / 15.0;
            double minEv, maxEv;
            double ev;
            
            if(post.sb().test(obj) && isInHouse(post.stone(obj))){
                minEv = 0;
                maxEv = 0.5 - 0.5 * level;
            }else{
                minEv = 0.5 + (0.8 - 0.5) * level;
                maxEv = 1;
            }
            if(post.sb().test(stNum) && isInHouse(post.stone(stNum))){
                ev = minEv;
            }else{
                ev = maxEv;
            }
            return ev;
        }
        
        template<class move_t, class board_t, class vmove_t>
        static double realize(move_t *const pmv, const board_t& bd, const vmove_t& vmv){
            pmv->setSpin(vmv.getSpin());
            return genReal1(pmv, bd.stone(vmv.getNum0()));
        }
        
        template<class pos_t, class move_t>
        static double genReal1(move_t *const pmv, const pos_t& pos){
            DERR << shotString[number()] << " - Stone : " << pos << endl;
            /*constexpr fpn_t k = 1 / sqrt(2);
            fPosXY<> dpos;
            if(!pmv->isLeftSpin()){
                dpos.set(pos.getX() - k * FR_STONE_RAD, pos.getY());
            }else{
                dpos.set(pos.getX() + k * FR_STONE_RAD, pos.getY());
            }
            FastSimulator::rotateToPassPointF(pmv, dpos, baseSpeed());*/
            fPosXY<> dpos;
            constexpr fpn_t d = 2 * FR_STONE_RAD;
            if(!pmv->isLeftSpin()){
                const fpn_t th = -2.525 + calcRelativeAngleThrow(pos);
                dpos.set(pos.getX() + d * sin(th), pos.getY() + d * cos(th));
            }else{
                const fpn_t th = +2.525 - calcRelativeAngleThrow(pos);
                dpos.set(pos.getX() - d * sin(th), pos.getY() + d * cos(th));
            }
            FastSimulator::rotateToPassPointF(pmv, dpos, baseSpeed());
            
            return 0;
        }
        
        template<class board_t, class dice_t>
        static void setSampleState(board_t *const pbd, dice_t *const pdice){
            pushStonesInHouse(pbd, BLACK, 1, pdice);
        }
    };
    
    int initPeelTable(){
        /*constexpr fpn_t FY_MIN = FY_PA_MIN - FR_STONE_RAD;
        constexpr fpn_t FY_MAX = FY_PA_MAX + FR_STONE_RAD;
        constexpr fpn_t fv = Peel::baseSpeed();
        for(int i = 0; i < 64; ++i){
            
            fPosXY<> bp(FX_TEE, FY_MIN + i * (FY_MAX - FY_MIN) / 64);
            
            BiSolver solver(-M_PI, -M_PI / 2);
            
            for(int t = 0; t < 64; ++t){
                fpn_t val = solver.play();
                fMoveXY<> mv[2];
                fPosXY<> p;
                mv[0].setSpin(Spin::RIGHT); mv[1].setSpin(Spin::LEFT);
                p.setPCS(FR_STONE_RAD * 2, val - 0.0001);
                FastSimulator::rotateToPassPointF(&mv[0], bp + p, Peel::baseSpeed());
                p.setPCS(FR_STONE_RAD * 2, val + 0.0001);
                FastSimulator::rotateToPassPointF(&mv[1], bp + p, Peel::baseSpeed());
                
                fpn_t dist[2];
                for(int d = 0; d < 2; ++d){
                    MiniBoard bd;
                    bd.locateStone(TURN_BEFORE_LAST, bp);
                    makeMoveNoRand<0>(&bd, TURN_LAST, mv[d]);
                    dist[d] = bd.stone(TURN_BEFORE_LAST).x - bd.stone(TURN_LAST).x;
                }
                
                solver.feed(dist[1] - dist[0] < 0);
            }
            
            peelTThTable[i] = solver.answer();
            
            cerr << peelTThTable[i] << endl;
        }*/
        return 0;
    }
}

#endif // DCURLING_SHOT_PEEL_HPP_
