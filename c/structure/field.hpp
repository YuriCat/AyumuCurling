// デジタルカーリング
// クライアントの主観的情報空間

#ifndef DCURLING_STRUCTURE_CLIENTFIELD_HPP_
#define DCURLING_STRUCTURE_CLIENTFIELD_HPP_

#include "../dc.hpp"

namespace DigitalCurling{
    
    struct GameBoard{
        std::string name[2];
        int limit[2];
        float random;
        int ends;
        int turns;
        int score[16][2];
        int scoreSum[2];
        float stone[16][16 + 1][16][2];
        fMoveXY<float> bestShot[16][16];
        fMoveXY<float> runShot[16][16];
        int reversed[16 + 1];
        
        void setName(const std::string& nm0, const std::string& nm1){
            name[0] = nm0;
            name[1] = nm1;
        }
        void setRule(int e, int t, float ran)noexcept{
            ends = e; turns = t; random = ran;
        }
        void setStones(int e, int t, float st[16][2]){
            for(int i = 0; i < 16; ++i){
                for(int j = 0; j < 2; ++j){
                    stone[e][t][i][j] = st[i][j];
                }
            }
        }
        void setScore(int e, int s){
            if(s >= 0){
                score[e][0] = s;
                score[e][1] = 0;
                reversed[e + 1] = reversed[e];
                
                scoreSum[0] += s;
            }else{
                score[e][0] = 0;
                score[e][1] = -s;
                reversed[e + 1] = reversed[e] ^ 1;
                
                scoreSum[1] += -s;
            }
            /*cerr << "score = " << s << endl;
            cerr << score[e][0] << " " << score[e][1] << endl;
            cerr << scoreSum[0] << " " << scoreSum[1] << endl;
            getchar();*/
        }
        void setLimit(int t0, int t1){
            limit[0] = t0;
            limit[1] = t1;
        }
        void setBestShot(int e, int t, float x, float y, Spin s){
            bestShot[e][t].set(x, y, s);
        }
        void setRunShot(int e, int t, float x, float y, Spin s){
            runShot[e][t].set(x, y, s);
        }
        void initGame()noexcept{
            random = 0;
            name[0] = name[1] == "default";
            scoreSum[0] = scoreSum[1] = 0;
            limit[0] = limit[1] = 0;
            for(int e = 0; e <= 16; ++e){
                reversed[e] = 0;
            }
            for(int e = 0; e < 16; ++e){
                score[e][0] = score[e][1] = 0;
                for(int t = 0; t < 16; ++t){
                    bestShot[e][t].set(0, 0, Spin::RIGHT);
                    runShot[e][t].set(0, 0, Spin::RIGHT);
                }
                for(int t = 0; t <= 16; ++t){
                    for(int i = 0; i < 16; ++i){
                        stone[e][t][i][0] = stone[e][t][i][1] = 0;
                    }
                }
            }
            ends = 0;
            turns = 0;
        }
    };
    
    struct ClientField{
        // 主観的情報、客観的情報を分け隔てなく
        // 基本的にサーバーから送られてくる情報の型、数値のまま保存
        // ゆえに、primitive型表現とは値が異なる事が多いので注意
        
        // ログの役割も果たす(1試合のみ)
        
        // 一応、16エンド戦まで対応
        
        // 全ゲームの情報
        int ngames[2];
        int wons[2];
        int drews[2][2];
        int loses[2];
        
        // 固定情報のインデックスは最初のエンドの先攻後攻
        std::string name[2]; // 名前
        
        int my_first_color; // 最初のエンドでの自分の先攻or後攻
        TimeLimit tl[2];
        int end_score[16][2]; // 各エンドの得点
        int score[2]; // 合計得点
        
        // 現エンドでの情報
        int my_color;
        int color[2];
        
        // エンド
        int end; // 現在エンド
        int to_end; // エンド数
        
        // エンド内での手数
        int turn;
        const static int to_turn = 16;
        
        // エンド中のプレーヤーのインデックスは先攻後攻なので、
        // エンドごとに対応が異なる
        float pos[8][2][2]; // ストーン位置
        
        void initAll(){
            // 起動時の初期化
            for (int c = 0; c < 2; ++c){
                wons[c] = 0;
                loses[c] = 0;
                for (int cc = 0; cc < 2; ++cc){
                    drews[c][cc] = 0;
                }
            }
        }
        void setTimeLimit(int col, uint64_t aleft,uint64_t aby=0ULL){
            tl[col].set(aleft,aby);
        }
        void initGame(){
            // 1ゲーム開始前の初期化
            score[0] = score[1] = 0;
            for (int e = 0; e < 16; ++e){
                end_score[e][0] = end_score[e][1] = 0;
            }
            
            color[0] = 0;
            color[1] = 1;
            
            end = 0;
            my_first_color = -1; // 自分の先攻後攻が不明
        }
        void setMyFirstColor(int col){
            assert(0 <= col && col < 2);
            my_first_color = col;
            my_color = col;
        }
        void setEnd(int aend){
            end = aend;
        }
        void setToEnd(int aend){
            to_end = aend;
        }
        int getNRemainedEnds()const{
            // 残りエンド数(開始していないもの)
            return to_end - end - 1;
        }
        int getNRemainedTurns()const{
            // 残りエンド数(開始していないもの)
            return 16 - turn - 1;
        }
        void setName(int p, std::string aname){
            assert(0 <= p && p < 2);
            name[p] = aname;
        }
        void setTurn(int aturn){
            turn = aturn;
        }
        bool isMyTurn()const{
            return (my_color == (turn % 2));
        }
        int getTurnColor()const{
            return (turn % 2);
        }
        int getFirstColor(int col)const{
            return (my_first_color == my_color) ? col : (1 - col);
        }
        const TimeLimit& getTimeLimit(int col)const{
            return tl[getFirstColor(col)];
        }
        void initEnd(){
            turn = 0;
            for (int i = 0; i < 8; ++i){
                for (int j = 0; j < 2; ++j){
                    for (int k = 0; k < 2; ++k){
                        pos[i][j][k] = 0;
                    }
                }
            }
        }
        void setEndScore(int sc){
            // +ならばid:0のプレーヤー、-ならばid:1のプレーヤーの得点
            // 次エンドが始まってから報告されるようだ
            assert(0 <= end && end < 16);
            if (sc != 0){
                int fcol = (sc > 0) ? 0 : 1;
                sc = abs(sc);
                end_score[end][fcol] = sc;
                end_score[end][1 - fcol] = 0;
                score[fcol] += sc;
            }
        }
        int getRelScore()const{
            return score[0] - score[1];
        }
        void closeEnd(){
            // 得点の入り方によって次のエンドの手番を決める
            if ((end_score[end][0] > 0 && color[0] == 1)
                || (end_score[end][1] > 0 && color[1] == 1)
                ){
                std::swap(color[0], color[1]);
                my_color = 1 - my_color;
            }
        }
        void closeGame(int res){
            
            int my_score = score[my_first_color];
            int ops_score = score[1 - my_first_color];
            
            if (res > 0){ // 勝ち
                ++wons[my_first_color];
            }
            else if (res < 0){ // 負け
                ++loses[my_first_color];
            }
            else{ // 引き分け
                if (my_color == 0){ // 延長戦先手
                    ++drews[my_first_color][0];
                }
                else{ // 延長戦後手
                    ++drews[my_first_color][1];
                }
            }
            ++ngames[my_first_color];
        }
        void closeAll(){
            // 結果まとめ
            int wons_sum = 0;
            int loses_sum = 0;
            int drews_sum[2] = { 0 };
            
            for (int c = 0; c < 2; ++c){
                wons_sum += wons[c];
                loses_sum += loses[c];
                for (int cc = 0; cc < 2; ++cc){
                    drews_sum[cc] += drews[c][cc];
                }
            }
            cerr << "b game "
            << "w : " << wons[0] << " l : " << loses[0] << " dw : " << drews[0][1] << " dl : " << drews[0][0] << endl
            << "w game "
            << "w : " << wons[1] << " l : " << loses[1] << " dw : " << drews[1][1] << " dl : " << drews[1][0] << endl
            << "all    "
            << "w : " << wons_sum << " l : " << loses_sum << " dw : " << drews_sum[1] << " dl : " << drews_sum[0] << endl;
        }
        void setRunShot(int col, float x, float y){
            
        }
        
        void setStonePos(int sn, float x, float y){
            pos[sn / 2][sn % 2][0] = x;
            pos[sn / 2][sn % 2][1] = y;
        }
        std::string toBroadcastString()const{
            std::ostringstream oss;
            
            const std::string player[4] = {
                "Lead", "Second", "Third", "Skip",
            };
            
            int ost = turn / 2;
            
            // 現在状況を実況
            oss << "*** END : " << end + 1 << " to " << to_end << " ";
            oss << " SHOT : " << turn + 1 << " to 16 ( " << player[ost / 2] << " - " << (ost % 2 + 1) << " )";
            oss << " ***" << endl;
            
            // 名前の文字数は長い方に固定(最低でも8)
            int name_len = std::max(name[0].size(), name[1].size());
            if (name_len < 8){ name_len = 8; }
            
            // ストーンの色は最初のエンドから固定する
            if (color[0] == 0){
                oss << " " << std::setw(name_len) << name[0] << " " << std::setw(3) << score[0] << " ";
                for (int i = 0; i < ((17 - turn) / 2) - 1; ++i){ oss << "O"; }
                if (turn % 2 == 0){ oss << "@"; }
                oss << endl;
                oss << " " << std::setw(name_len) << name[1] << " " << std::setw(3) << score[1] << " ";
                for (int i = 0; i < ((18 - turn) / 2) - 1; ++i){ oss << "O"; }
                if (turn % 2 == 1){ oss << "@"; }
                oss << endl;
            }
            else{
                oss << " " << std::setw(name_len) << name[1] << " " << std::setw(3) << score[1] << " ";
                for (int i = 0; i < ((17 - turn) / 2) - 1; ++i){ oss << "O"; }
                if (turn % 2 == 0){ oss << "@"; }
                oss << endl;
                oss << " " << std::setw(name_len) << name[0] << " " << std::setw(3) << score[0] << " ";
                for (int i = 0; i < ((18 - turn) / 2) - 1; ++i){ oss << "O"; }
                if (turn % 2 == 1){ oss << "@"; }
                oss << endl;
            }
            
            return oss.str();
        }
        
        
        ~ClientField(){
            closeAll();
        }
    };
}

#endif // DCURLING_STRUCTURE_CLIENTFIELD_HPP_