/*
 stat.hpp
 Katsuki Ohto
 */

// デジタルカーリング
// 統計的データに基づいたパラメータ(にする予定)

#ifndef DCURLING_STAT_HPP_
#define DCURLING_STAT_HPP_

#include "../../dc.hpp"

namespace DigitalCurling{
    
    // 考慮する得点差の区間(オーバーしたら端に丸める)
    // 昔はもっと幅が狭かったが少し点差が開くとすぐに着手が乱れていたので今は32段階に広げた
    constexpr int SCOREGAP_MIN = -14;
    constexpr int SCOREGAP_MAX = +17;
    
    // 考慮する得点差の幅
    constexpr int SCOREGAP_LENGTH = SCOREGAP_MAX - SCOREGAP_MIN + 1;
    
    // 開区間の得点差を閉区間に落とし込む
    constexpr int RSGtoSG(int rsd)noexcept{
        return (rsd > SCOREGAP_MAX) ? SCOREGAP_MAX : ((rsd < SCOREGAP_MIN) ? SCOREGAP_MIN : rsd);
    }
    
    // 閉区間の得点差からインデックスを出す
    constexpr int SGtoIDX(int sd)noexcept{ return sd - SCOREGAP_MIN; }
    
    // インデックスから閉区間の得点差を出す
    constexpr int IDXtoSG(int idx)noexcept{ return idx + SCOREGAP_MIN; }
    
    /**************************得点差遷移確率**************************/
    
    fpn_t dScoreProb_EASY[N_COLOR_STONES * 2 + 1] = {
        // 先手が+s点を取る確率(適当に決めた値であり和が1でない)
        0.00025,
        0.001,
        0.004,
        0.016,
        0.05,
        0.09,
        0.25,
        0.5,
        0.05, // 0
        0.22,
        0.07,
        0.02,
        0.007,
        0.0007,
        0.00007,
        0.000007,
        0.0000007,
    };
    
    constexpr fpn_t dWinningPercentage_EX = 0.219; // 延長戦の先手予測勝率
    
    fpn_t dScoreProb[N_ENDS][SCOREGAP_LENGTH][SCORE_LENGTH];
    
    int initTransProb(){
        //遷移確率初期化
        
        // dScoreProb_EASYを正規化
        fpn_t sum = 0;
        for (int s = SCORE_MIN; s <= SCORE_MAX; ++s){
            sum += dScoreProb_EASY[StoIDX(s)];
        }
        for (int s = SCORE_MIN; s <= SCORE_MAX; ++s){
            dScoreProb_EASY[StoIDX(s)] /= sum;
        }
        
#ifdef MONITOR
        CERR << "dScoreProb_EASY = {" << endl;
        for (int s = SCORE_MIN; s <= SCORE_MAX; ++s){
            CERR << " " << dScoreProb_EASY[StoIDX(s)] << "," << endl;
        }
        CERR << "};" << endl;
#endif
        
        // エンドを考えない適当な遷移確率
        /*for(int s=SCOREGAP_MIN;s<=SCOREGAP_MAX;++s){
         for(int ss=SCOREGAP_MIN;ss<=SCOREGAP_MAX;++ss){
         transProb_MIDDLE[0][SGtoIDX(s)][SGtoIDX(ss)]=0;
         transProb_MIDDLE[1][SGtoIDX(s)][SGtoIDX(ss)]=0;
         }
         for(int ss=SCOREGAP_MIN;ss<=SCOREGAP_MAX;++ss){
         if( ss>=0 ){//手番そのまま
         transProb_MIDDLE[0][SGtoIDX(s)][SGtoIDX(RSGtoSG(s+ss))]+= transProb_EASY[SGtoIDX(ss)];
         }
         }*/
        
        // 得点確率
        for (int e = 0; e < N_ENDS; ++e){
            for (int s = SCOREGAP_MIN; s <= SCOREGAP_MAX; ++s){
                for (int ss = SCORE_MIN; ss <= SCORE_MAX; ++ss){
                    dScoreProb[e][SGtoIDX(s)][StoIDX(ss)] = dScoreProb_EASY[StoIDX(ss)];
                }
            }
        }
        
        //遷移確率表表示
#ifdef MONITOR
        /*
         CERR<<"transProb_MIDDLE = {"<<endl;
         for(int s=SCOREGAP_MIN;s<=SCOREGAP_MAX;++s){
         for(int ss=SCOREGAP_MIN;ss<=SCOREGAP_MAX;++ss){
         CERR<<" "<<transProb_MIDDLE[SGtoIDX(s)][SGtoIDX(ss)]<<",";
         }
         CERR<<endl;
         }
         CERR<<"};"<<endl;
         */
#endif
        
        return 0;
    }
    
    /**************************エンド後勝率**************************/
    
    // 残りエンド数×次エンドの手番×点差での予測勝率を設定
    
    fpn_t dWinningPercentage[N_ENDS + 1][SCOREGAP_LENGTH];
    
    fpn_t getEndWP(Color c, int e, int s, int next_sg){
        fpn_t ev;
        if (s >= 0){
            int idx = SGtoIDX(RSGtoSG(next_sg));
            if (isBlack(c)){
                ev = dWinningPercentage[e][idx];
            }
            else{
                ev = 1.0 - dWinningPercentage[e][idx];
            }
        }
        else{
            int idx = SGtoIDX(RSGtoSG(-next_sg));
            if (isBlack(c)){
                ev = 1.0 - dWinningPercentage[e][idx];
            }
            else{
                ev = dWinningPercentage[e][idx];
            }
        }
        return ev;
    }
    
    int initReward(){
        // 報酬系初期化
        
        // 最終エンド後の勝率
        for (int s = SCOREGAP_MIN; s <= -1; ++s){ // 先手負け
            dWinningPercentage[0][SGtoIDX(s)] = 0;
        }
        dWinningPercentage[0][SGtoIDX(0)] = dWinningPercentage_EX; // 延長戦突入
        for (int s = 1; s <= SCOREGAP_MAX; ++s){ // 先手勝ち
            dWinningPercentage[0][SGtoIDX(s)] = 1;
        }
        
        // それまでのエンド後の勝率
        for (int e = 1; e < N_ENDS + 1; ++e){
            for (int s = SCOREGAP_MIN; s <= SCOREGAP_MAX; ++s){
                // 次のエンドの予測勝率をエンドの得点分布で畳み込むことで、一つ前のエンドの予測勝率とする
                dWinningPercentage[e][SGtoIDX(s)] = 0;
                for (int ss = SCORE_MIN ; ss <= -1; ++ss){ // 手番交代
                    dWinningPercentage[e][SGtoIDX(s)] += dScoreProb[e - 1][SGtoIDX(s)][StoIDX(ss)] * (1.0 - dWinningPercentage[e - 1][SGtoIDX(RSGtoSG(-(s + ss)))]);
                }
                for (int ss = 0; ss <= SCORE_MAX; ++ss){ // 手番そのまま
                    dWinningPercentage[e][SGtoIDX(s)] += dScoreProb[e - 1][SGtoIDX(s)][StoIDX(ss)] * dWinningPercentage[e - 1][SGtoIDX(RSGtoSG(s + ss))];
                }
            }
        }
        
        // 報酬表表示
#ifdef MONITOR
        CERR << "dWinningPercentage = {" << endl;
        for (int e = 0; e < N_ENDS + 1; ++e){
            for (int s = SCOREGAP_MIN; s <= SCOREGAP_MAX; ++s){
                CERR << " " << dWinningPercentage[e][SGtoIDX(s)] << ",";
            }
            CERR << endl;
        }
        CERR << "};" << endl;
#endif
        return 0;
    }
}

#endif // DCURLING_STAT_HPP_
