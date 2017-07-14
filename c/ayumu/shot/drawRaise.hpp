/*
 drawRaise.hpp
 Katsuki Ohto
 */

// デジタルカーリング
//（主に）自分の石をセンターに押し込む

#ifndef DCURLING_SHOT_DRAWRAISE_HPP_
#define DCURLING_SHOT_DRAWRAISE_HPP_

#include "shotGenerator.hpp"

namespace DigitalCurling{
    
    struct DrawRaise : public ShotGenerator{
        
        constexpr static int number()noexcept{ return Standard::DRAWRAISE; }
        constexpr static int dimension()noexcept{ return 2; }
        constexpr static int flexibility()noexcept{ return 0;}
        constexpr static bool hasContact()noexcept{ return true; }
        constexpr static fpn_t baseSpeed()noexcept{ return FV_TEESHOT; }
        
        template<class board_t, class vmove_t>
        double estimate(const board_t& bd, uint32_t stNum, const vmove_t& vmv)const{
            return 1.0;
        }
        
        template<class board0_t, class board1_t, class vmove_t>
        static double evaluate(const board0_t& pre, const board1_t& post, uint32_t stNum, const vmove_t& vmv, const ContactTree& ct = 0){
            // 目的の石を出来る限りティーに近づければ高得点
            const uint32_t obj = vmv.getNum0();
            return exp(-calcDistance2Tee(post.stone(obj)));
        }
        
        template<class move_t, class pos_t>
        void genReal1(move_t *const pmv, const pos_t& pos){
            fPosXY<> dst;
            fpn_t r = calcDistanceTee(pos);
            fpn_t rate = 1 + 2 * FR_STONE_RAD / r; // 石の直径分ティーより遠くする倍率
            dst.set(FX_TEE + (pos.x - FX_TEE) * rate, FY_TEE + (pos.y - FY_TEE) * rate);
            FastSimulator::rotateToPassPointF(pmv, dst, baseSpeed());
        }
        
        template<class move_t, class board_t, class vmove_t>
        void realize(move_t *const pmv, const board_t& bd, const vmove_t& vmv){
            pmv->setSpin(vmv.getSpin());
            return genReal1(pmv, bd.stone(vmv.getNum0()));
        }
        
        template<class board_t, class dice_t>
        void setSampleState(board_t *const pbd, dice_t *const pdice){
            fPosXY<> pos;
            do{
                fpn_t dr = (FR_HOUSE_RAD + 5 * FR_STONE_RAD) * pdice->drand();
                fpn_t dth = M_PI + M_PI / 6 * (pdice->drand() * 2 - 1);
                pos.x = FX_TEE + dr * sin(dth);
                pos.y = FY_TEE + dr * cos(dth);
            }while((pos.x < FX_TEE - FR_HOUSE_RAD * 0.6) || (FX_TEE + FR_HOUSE_RAD * 0.6 < pos.x));
            pbd->pushStone(WHITE, pos);
        }
    };
}

#endif //DCURLING_SHOT_DRAWRAISE_HPP_
