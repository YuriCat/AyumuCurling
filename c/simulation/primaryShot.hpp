/*
 primaryShot.hpp
 Katsuki Ohto
 */

// Digital Curling
// generating Primary Shots

#ifndef DCURLING_SIMULATION_PRIMARYSHOT_HPP_
#define DCURLING_SIMULATION_PRIMARYSHOT_HPP_

#include "../dc.hpp"
#include "fastSimulator.h"

namespace DigitalCurling{
    
    /**************************ノーレンジ着手の生成**************************/
    
    template<class move_t>
    void genPASS(move_t *const pmv){
        // パス
        // TODO: パスにならない場合あり
        pmv->setVX(FVX_MIN); pmv->setVY(FVY_MIN);
    }
    
    template<class pos_t, class move_t>
    void genDraw(move_t *const pmv, const pos_t& pos){
        // 位置　pos へのドローショット
        FastSimulator::FXYtoFMV(pos, pmv);
    }
    
    template<class pos_t, class move_t>
    void genHit(move_t *const pmv, const pos_t& pos, fpn_t v){
        // 位置 pos の石を 初速 v でヒット
        FastSimulator::rotateToPassPointF(pmv, fPosXY<>(pos.getX(), pos.getY() - 0.015), v); // TODO: 石の中心より微妙に手前側を狙う効果はある?
    }
    
    template<class pos0_t, class pos1_t>
    void genComeAroundPosition(pos0_t *const pdst, const pos1_t& pos, fpn_t l){
        // 位置 pos にある石の奥 距離 l にカムアラウンドする位置
        fpn_t tr = calcDistanceThrow(pos);
        fpn_t rate = (tr + l) / tr;
        pdst->setRCS(FX_THROW + (pos.getX() - FX_THROW) * rate,
                     FY_THROW + (pos.getY() - FY_THROW) * rate);
    }
    template<class pos_t, class move_t>
    void genComeAround(move_t *const pmv, const pos_t& pos, fpn_t l){
        // カムアラウンド着手
        fpn_t tr = calcDistanceThrow(pos);
        fpn_t rate = (tr + l) / tr;
        fPosXY<> dpos(FX_THROW + (pos.getX() - FX_THROW) * rate,
                      FY_THROW + (pos.getY() - FY_THROW) * rate);
        FastSimulator::FXYtoFMV(dpos, pmv);
    }
    template<class pos_t, class move_t>
    void genComeAroundByY(move_t *const pmv, const pos_t& pos, fpn_t y){
        // 位置 pos にある石にカムアラウンド ただし静止するy座標を指定
        fpn_t rate = (y - FY_THROW) / (pos.getY() - FY_THROW);
        fPosXY<> dpos(FX_THROW + (pos.getX() - FX_THROW) * rate, y);
        FastSimulator::FXYtoFMV(dpos, pmv);
    }
    
    template<class pos0_t, class pos1_t>
    void genPostGuardPosition(pos0_t *const pdst, const pos1_t& pos, fpn_t l){
        // 既にある石に対してのガード位置
        fpn_t tr = calcDistanceThrow(pos);
        fpn_t rate = (tr - l) / tr;
        pdst->setRCS(FX_THROW + (pos.getX() - FX_THROW) * rate,
                     FY_THROW + (pos.getY() - FY_THROW) * rate);
    }
    
    template<class pos_t, class move_t>
    void genPostGuard(move_t *const pmv, const pos_t& pos, fpn_t l){
        // 既にある石に対してのガード着手
        fpn_t tr = calcDistanceThrow(pos);
        fpn_t rate = (tr - l) / tr;
        fPosXY<> dpos(FX_THROW + (pos.getX() - FX_THROW) * rate,
                      FY_THROW + (pos.getY() - FY_THROW) * rate);
        FastSimulator::FXYtoFMV(dpos, pmv);
    }
    
    template<class pos_t, class move_t>
    void genPostGuardByY(move_t *const pmv, const pos_t& pos, fpn_t y){
        // 既にある石に対してのガード(y座標指定された時)
        // カムアラウンドと同じ
        fpn_t rate = (y - FY_THROW) / (pos.getY() - FY_THROW);
        fPosXY<> dpos(FX_THROW + (pos.getX() - FX_THROW) * rate, y);
        FastSimulator::FXYtoFMV(dpos, pmv);
    }
}

#endif // DCURLING_SIMULATION_PRIMARYSHOT_HPP_