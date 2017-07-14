// Digital Curling
// realizing verbal shots

#ifndef DCURLING_MOVE_ESTIMATOR_HPP_
#define DCURLING_MOVE_ESTIMATOR_HPP_

namespace DigitalCurling{
    
    /**************************着手生成器の評価**************************/
    
    template<class board_t, class move_t, class nrmove_t>
    void estimateMove(const board_t& bd, const move_t& amv){
        if (!amv.isRelative()){
            // 絶対着手
            return 0;
        }
        else{
            // 相対着手
            int s = amv.getSpin();
            int turn = bd.getTurn();
            switch (amv.getType()){
                case Standard::PASS:
                case Standard::DRAW:
                case Standard::PREGUARD:
                case Standard::FREEZE:
                case Standard::HIT:
                case Standard::COMEAROUND:
                case Standard::POSTGUARD:{
                    return 0;
                }break;
                case Standard::DOUBLE:{
                    return gDouble.estimate(bd, turn, amv);
                }break;
                case Standard::RAISETAKEOUT:{
                    return gRaiseTakeOut.estimate(bd, turn, amv);
                }break;
                case Standard::PEEL:{
                    return gPeel.estimate(bd, turn, amv);
                }break;
                case Standard::DRAWRAISE:{
                    return gDrawRaise.estimate(bd, turn, amv);
                }break;
                case Standard::TAKEOUT:{
                    return gTakeOut.estimate(bd, turn, amv);
                }break;
                case Standard::L1DRAW:{
                    return gL1Draw.estimate(bd, turn, amv);
                }break;
                default: ASSERT(0, cerr << amv.getType() << endl;); break;
            }
        }
    }
}

#endif //DCURLING_MOVE_ESTIMATOR_HPP_