// デジタルカーリング
// ショットの種類クラス

#ifndef DCURLING_SHOT_SHOT_GENERATOR_HPP_
#define DCURLING_SHOT_SHOT_GENERATOR_HPP_

#include "../../dc.hpp"

namespace DigitalCurling{
    
    struct ShotGenerator{
        /*
         constexpr static int dimension()noexcept{
         return 0;
         }
         constexpr static int flexibility()noexcept{
         return 2;
         }
         constexpr static bool hasContact()noexcept{
         return false;
         }
         static std::string name()noexcept{
         return "NullStrategy";
         }
         template<class board0_t, class board1_t, class vmove_t>
         double evaluate(const board0_t&, const board1_t&, Color, const vmove_t&);
         
         template<class board_t>
         double estimate(const board_t&);
         
         template<class move_t>
         int generate0(move_t *const){
         UNREACHABLE;
         return -1;
         }
         template<class move_t,class pos_t>
         int generate1(move_t *const, const pos_t&){
         UNREACHABLE;
         return -1;
         }
         template<class move_t, class pos_t>
         int generate2(move_t *const, const pos_t&, const pos_t&){
         UNREACHABLE;
         return -1;
         }
         template<class move_t, class pos_t>
         double generate(move_t *const, const pos_t&, const std::array<int,dimension()>&){
         UNREACHABLE;
         return -1;
         }
         
         template<class move_t, class board_t>
         double generate(move_t *const, const board_t&, const std::array<int,dimension()>&){
         UNREACHABLE;
         return -1;
         }
         int choooseReferenceStone(){
         UNREACHABLE;
         return -1;
         }
         template<class pos_t>
         int choooseReferenceStone(const pos_t&){
         UNREACHABLE;
         return -1;
         }
         template<class pos_t>
         int choooseReferenceStone(const pos_t&, const pos_t&){
         UNREACHABLE;
         return -1;
         }
         */
        
        template<class board_t>
        BitArray64<4, 16> orderRefStoneIndex(const board_t& bd, BitArray64<4, 16> index){
            return index;
        }
        
        template<class board_t>
        void setSampleState(board_t *const);
        
    };
}

#endif // DCURLING_SHOT_SHOT_GENERATOR_HPP_