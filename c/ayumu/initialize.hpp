/*
 initialize.hpp
 Katsuki Ohto
 */

#ifndef DCURLING_AYUMU_INITIALIZE_HPP_
#define DCURLING_AYUMU_INITIALIZE_HPP_

#include "ayumu_dc.hpp"

// 諸々の初期化の順番等がめんどいのでここで一括で管理する
// 初期化成功の場合は 0, 失敗の場合は -1 を返す
// 引数は入力データファイルのあるディレクトリ名

// initMin() 最小限の初期化
// initShot() 戦略的なショットを生成可能なレベル
// initEval() 局面評価が可能なレベル (局面評価関数自体は読み込まない)
// initPolicy() 方策計算が可能なレベル (方策関数自体は読み込まない)
// initAyumu() 思考可能な最高レベル

// 現在物理演算シミュレータの初期化はここではなく勝手にやる

namespace DigitalCurling{
    int initMin(){
        return 0;
    }
    
    int initShot(const std::string& path){
        // 全ての戦略的なショットを生成出来る
#ifdef DCURLING_SHOT_PARAMS_H_
        if(initShotParams() < 0){ return -1; }
#endif
#ifdef DCURLING_SHOT_DOUBLE_HPP_
        if(initDoubleTable(path) < 0){ return -1; }
#endif
#ifdef DCURLING_SHOT_PEEL_HPP_
        if(initPeelTable() < 0){ return -1; }
#endif
#ifdef DCURLING_MOVE_REALIZER_HPP_
        if(initStandardDrawTable() < 0){ return -1; }
#endif
        if(initGuardProb(path) < 0){ return -1; }
        if(initGuardVBB() < 0){ return -1; }
        return 0;
    }
    
    int initEval(const std::string& path){
        // 局面評価を出来る
#ifdef DCURLING_STAT_HPP_
        if(initTransProb() < 0){ return -1; }
        if(initReward() < 0){ return -1; }
#endif
#ifdef DCURLING_AYUMU_ESTIMATOR_HPP_
        if(initEvalStat(path) < 0){ return -1; }
#endif
        return 0;
    }
    
    int initPolicy(const std::string& path){
        // policyで手を選べる
#ifdef DCURLING_SHOT_PARAMS_H_
        if(initShotParams() < 0){ return -1; }
#endif
#ifdef DCURLING_SHOT_DOUBLE_HPP_
        if(initDoubleTable(path) < 0){ return -1; }
#endif
#ifdef DCURLING_SHOT_PEEL_HPP_
        if(initPeelTable() < 0){ return -1; }
#endif
#ifdef DCURLING_MOVE_REALIZER_HPP_
        if(initStandardDrawTable() < 0){ return -1; }
#endif
#ifdef DCURLING_STONEINFO_HPP_
        if(initStoneInfo() < 0){ return -1; }
#endif
        return 0;
    }
    
    int initAyumu(const std::string& path){
        // ayumu本体と同程度のことができる
#ifdef DCURLING_SHOT_PARAMS_H_
        if(initShotParams() < 0){ return -1; }
#endif
#ifdef DCURLING_MOVE_REALIZER_HPP_
        if(initStandardDrawTable() < 0){ return -1; }
#endif
#ifdef DCURLING_SHOT_DOUBLE_HPP_
        if(initDoubleTable(path) < 0){ return -1; }
#endif
#ifdef DCURLING_SHOT_PEEL_HPP_
        if(initPeelTable() < 0){ return -1; }
#endif
        if(initGuardProb(path) < 0){ return -1; }
        if(initGuardVBB() < 0){ return -1; }
        
#ifdef DCURLING_STAT_HPP_
        if(initTransProb() < 0){ return -1; }
        if(initReward() < 0){ return -1; }
#endif
#ifdef DCURLING_AYUMU_ESTIMATOR_HPP_
        if(initEvalStat(path) < 0){ return -1; }
#endif
#ifdef DCURLING_STONEINFO_HPP_
        if(initStoneInfo() < 0){ return -1; }
#endif
        
        initHash();
        //importMoveDB(path + "shotlog.dat");
        
        return 0;
    }
}

#endif // DCURLING_AYUMU_INITIALIZE_HPP_