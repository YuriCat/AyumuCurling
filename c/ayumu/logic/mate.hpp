/*
 mate.hpp
 Katsuki Ohto
 */

#ifndef DCURLING_AYUMU_LOGIC_MATE_HPP_
#define DCURLING_AYUMU_LOGIC_MATE_HPP_

// カーリング必勝判定

// 返り値
// 先手の必勝 BLACK
// 後手の必勝 WHITE
// いずれの必勝とも判定しない COLOR_NONE

namespace DigitalCurling{
    
    template<class board_t>
    Color judgePerfectMate(const board_t& bd){
        // 物理的な系に依存しない厳格な意味での詰みを判定する
        
        // ラストストーン後手勝ちの局面のみ、確実にパスする必要がある場合も考慮する
        
        if(bd.getTurn() == TURN_LAST){ // エンド最終投
            if(bd.getRelScore() + bd.countScore() < -getGameRemStones(BLACK, bd.getEnd(), TURN_LAST)){
                return WHITE;
            }else if(bd.getRelScore() > getGameRemStones(WHITE, bd.getEnd(), TURN_LAST) + countNInPlayArea(bd, WHITE)){
                return BLACK;
            }
        }else{
            if(bd.getRelScore() < -getGameRemStones(BLACK, bd.getEnd(), bd.getTurn()) - countNInPlayArea(bd, BLACK)){
                return WHITE;
            }else if(bd.getRelScore() > getGameRemStones(WHITE, bd.getEnd(), bd.getTurn()) + countNInPlayArea(bd, WHITE)){
                return BLACK;
            }
        }
        return COLOR_NONE;
    }
    
    template<class board_t>
    Color judgeFuzzyMate(const board_t& bd){
        // だいたいの詰みを判定する
        // ラストストーン後手勝ちの局面のみ、確実にパスする必要がある場合も考慮する
        
        if(bd.getTurn() == TURN_LAST){ // エンド最終投
            if(bd.getRelScore() + bd.countScore() < -getGameRemStones(BLACK, bd.getEnd(), TURN_LAST)){
                return WHITE;
            }else if(bd.getRelScore() > getGameRemStones(WHITE, bd.getEnd(), TURN_LAST) + countNInActiveZone(bd, WHITE)){
                return BLACK;
            }
        }else{
            if(bd.getRelScore() < -getGameRemStones(BLACK, bd.getEnd(), bd.getTurn()) - countNInActiveZone(bd, BLACK)){
                return WHITE;
            }else if(bd.getRelScore() > getGameRemStones(WHITE, bd.getEnd(), bd.getTurn()) + countNInActiveZone(bd, WHITE)){
                return BLACK;
            }
        }
        return COLOR_NONE;
    }
}

#endif // DCURLING_AYUMU_LOGIC_MATE_HPP_
