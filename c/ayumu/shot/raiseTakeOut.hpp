/*
 raiseTakeOut.hpp
 Katsuki Ohto
 */

// デジタルカーリング
// 前の石を叩いて後ろの石をテイクアウト

#ifndef DCURLING_SHOT_RAISETAKEOUT_HPP_
#define DCURLING_SHOT_RAISETAKEOUT_HPP_

namespace DigitalCurling{
    
    struct RaiseTakeOut : public ShotGenerator{
        
        constexpr static int number()noexcept{ return Standard::RAISETAKEOUT; }
        constexpr static int dimension()noexcept{ return 2; }
        constexpr static int flexibility()noexcept{ return 0; }
        constexpr static bool hasContact()noexcept{ return true; }
        constexpr static fpn_t baseSpeed()noexcept{ return FV_MAX - 0.5; }
        
        template<class board_t, class vmove_t>
        double estimate(const board_t& bd, uint32_t stNum, const vmove_t& vmv)const{
            return 1.0;
        }
        
        template<class board0_t, class board1_t, class vmove_t>
        double evaluate(const board0_t& pre, const board1_t& post, uint32_t stNum, const vmove_t& vmv, const ContactTree& ct = 0){
            // テイクしたい方の目的の石の位置で評価
            // CT情報がある場合、直接叩くのは失敗とみなす
            const uint32_t obj = vmv.getNum1();
            if(ct[0] == obj){ return 0; } // 最初にobjに当たった
            return (!post.sb().test(obj) || !isInActiveZone(post.stone(obj))) ? 1.0 : ((!isInHouse(post.stone(obj))) ? 0.9 : 0.0);
        }
        
        template<class move_t, class pos0_t, class pos1_t>
        void genReal2(move_t *const pmv, const pos0_t& st, const pos1_t& obj){
            // st 叩く方の石, obj 出したい石
            fPosXY<> dst;
            fpn_t r = calcDistance(obj, st);
            fpn_t rate = 1 + 2 * FR_STONE_RAD / r;
            dst.set(obj.x + (st.x - obj.x) * rate, obj.y + (st.y - obj.y) * rate);
            FastSimulator::rotateToPassPointF(pmv, dst, baseSpeed());
        }
        
        template<class move_t, class board_t, class vmove_t>
        void realize(move_t *const pmv, const board_t& bd, const vmove_t& vmv){
            pmv->setSpin(vmv.getSpin());
            return genReal2(pmv, bd.stone(vmv.getNum0()), bd.stone(vmv.getNum1()));
        }
        
        template<class board_t, class dice_t>
        void setSampleState(board_t *const pbd, dice_t *const pdice){
            // ハウス内に1つ石を置く
            fPosXY<> pos0, pos1;
            locateInHouse(&pos0, pdice);
            pbd->pushStone(BLACK, pos0);
            
            // 前方に別の石を置く
            fpn_t dr = FR_STONE_RAD * 2 + FR_HOUSE_RAD * pdice->drand();
            fpn_t dth = M_PI + M_PI / 8 * (pdice->drand() * 2 - 1);
            pos1.x = pos0.x + dr * sin(dth);
            pos1.y = pos0.y + dr * cos(dth);
            pbd->pushStone(WHITE, pos1);
        }
    };
}

#endif // DCURLING_SHOT_RAISETAKEOUT_HPP_
