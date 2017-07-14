/*
 evaluator.hpp
 Katsuki Ohto
 */

// デジタルカーリング
// エンド末端報酬
// エンドと得点差により勝率の分散が異なるので正規化して報酬として使う
// 正規化の必要がある。画面表示のために勝率にも戻せるようにする

#include "../../dc.hpp"

#ifndef DCURLING_EVAL_EVALUATOR_HPP_
#define DCURLING_EVAL_EVALUATOR_HPP_

namespace DigitalCurling{

    class Evaluator{
        // return estimated winning percentage
        
    public:
        template<class board_t>
        eval_t evalSub(Color c, const board_t& bd){
            
            // ルート直下にのみ使用する補助評価関数
            // モンテカルロの不得意部分を隠すためのアドホックな関数
            
            // col
            
            bool clean = isCleanBetterScore(c, bd.getEnd(), bd.getRelScore());
            eval_t ans = 0;
            if (isBlack(c) && (clean || bd.getEnd() >= 6))
            {
                if(bd.getTurn() >= 3){
                    ans -= 0.04 * bd.NInPlayArea(WHITE);
                    ans -= 0.06 * bd.NInHouse(WHITE);
                }else{
                    ans -= 0.01 * bd.NInPlayArea(WHITE);
                    ans -= 0.02 * bd.NInHouse(WHITE);
                }
                if (bd.getTurn() >= 5){
                    ans -= 0.01 * bd.NInPlayArea(BLACK);
                }
                else{
                    ans += 0.01 * bd.NInHouse(BLACK);
                }
                ans -= bd.count2ndScore(BLACK) * 0.04;
            }else if(isWhite(c) && !clean){
                if(bd.getTurn() >= 4){
                    //ans -= 0.01 * bd.NInHouse(BLACK);
                }
                if(bd.getTurn() >= 2){
                    //前方、後方のガードに加点
                    /*if(bd.NInHouse(WHITE) && isWhite(getColor(bd.center[0]))){
                        int idx=bd.center[0];
                        iterateExcept(bd.sb(), idx, [&bd, &ans, idx](int idx1){
                            fpn_t dx = fabs(bd.stone(idx1).x - bd.stone(idx).x);
                            if(dx < FR_STONE_RAD){
                                if(bd.stone(idx).y < bd.stone(idx1).y){
                                    ans -= 0.01;
                                }else{
                                    if(bd.stone(idx).y < FY_TEE){
                                        ans -= 0.04;
                                    }else{
                                        ans -= 0.02;
                                    }
                                }
                                if(getColor(idx1) == WHITE){
                                    ans -= 0.01;
                                }
                            }else if(dx < 2 * FR_STONE_RAD){
                                if(bd.stone(idx).y < bd.stone(idx1).y){
                                    ans -= 0.005;
                                }else{
                                    if(bd.stone(idx).y < FY_TEE){
                                        ans -= 0.02;
                                    }else{
                                        ans -= 0.01;
                                    }
                                }
                                if(getColor(idx1) == WHITE){
                                    ans -= 0.01;
                                }
                            }
                            if(getColor(idx1) == WHITE){
                                if(bd.relStone(idx, idx1).r < FR_STONE_RAD * 5){
                                    ans += 0.02;
                                }
                            }
                        });
                    }*/
                    /*ans -= 0.03 * bd.NInHouse(WHITE);
                    ans -= 0.01 * bd.NInHouse(WHITE);
                    if(countScore(bd) < -1){
                        ans -= 0.02 * (-1 - countScore(bd));
                    }*/
                }
            }
            return ans;
        }
        
        // 勝率 <-> 報酬変換
        fpn_t rewardToWP(Color c, eval_t r)const noexcept{ // conversion Reward -> wp
            return isBlack(c) ? (r * sigma_ + mu_) : (1.0 - ((-r * sigma_) + mu_));
        }
        eval_t wpToReward(Color c, fpn_t p)const noexcept{ // conversion wp -> reward
            fpn_t bp = isBlack(c) ? p : (1.0 - p);
            eval_t br = (bp - mu_) / sigma_;
            return isBlack(c) ? br : (-br);
        }
        
        // 色と得点を指定して報酬 or 勝率を取得
        eval_t reward(Color col, int sc)const{
            return isBlack(col) ? rewardTable_[sc + N_COLOR_STONES] : (-rewardTable_[sc + N_COLOR_STONES]);
        }
        fpn_t wp(Color col, int sc)const{ return rewardToWP(col, reward(col, sc)); }
        
        // 後手ラストストーンで0点より1点を選ぶべきかどうか
        bool isBlankBetter()const noexcept{ return blankBetter_; }
        
        // evaluate CHECKMATE situation
        eval_t rewardMate(Color col, Color mateCol)const{
            return reward(col, isBlack(mateCol) ? (+N_COLOR_STONES) : (-N_COLOR_STONES));
        }
        eval_t rewardMateWithTiming(Color col, Color mateCol, int turn, eval_t step)const{
            // MATEが早く出たことに加点
            eval_t ev = rewardMate(col, mateCol);
            eval_t timingScore = step * turn;
            if (isBlack(mateCol)){
                ev += timingScore;
            }else{
                ev -= timingScore;
            }
            return isBlack(col) ? ev : (-ev);
        }
        
        // evaluate blank or white 1 situation
        eval_t reward01(Color col = BLACK)const{ return isBlack(col) ? reward01_ : (-reward01_); }
        fpn_t wp01(Color col = BLACK)const{ return rewardToWP(col, reward01(col)); }
        
        // evaluate after 1 end
        eval_t evaluateLast(Color col, int sc)const{ return reward(col, sc); }
        
        // eval vector との内積計算
        eval_t evaluateProb(Color c, const eval_t probVector[SCORE_LENGTH], const eval_t sum){
            eval_t ev = 0;
            for(int i = 0; i < SCORE_LENGTH; ++i){
                ev += probVector[i] * rewardTable_[i];
            }
            ev /= sum;
            return isBlack(c) ? ev : (-ev);
        }
        
        template<typename wpFunc_t>
        void setRewardInBound(Color myColor, // 丸め込みのために自分の色も必要
                              Color turnColor,
                              int rs, int e, int t,
                              const wpFunc_t& wpFunc){
            
            int conv[N_TURNS + 1]; // 得点換算テーブル(256が1点に相当, なぜそうしたのか不明)
            
            for (int s = -8; s <= +8; ++s){
                conv[s + 8] = s * 256;
            }
            
#if defined(MODIFY_END_SCORE)
            int myLead = isBlack(myColor) ? rs : (-rs);
            if (myLead > 0){
                // 自分がリードしている
                // 自分の得点の丸め込み(限度設定)を行う
                // これにより守り重視の戦略が選ばれることを期待している
                int marume_line = 8 * 256;
                if (isBlack(myColor)){
                    switch (myLead){
                        case 0: UNREACHABLE; break;
                        case 1: marume_line = 256 + 170; break;
                        case 2: marume_line = 205; break;
                        default: marume_line = 3; break;
                    }
                    for (int s = (marume_line + 255) / 256; s <= 8; ++s){
                        conv[s + 8] = marume_line;
                    }
                }
                else{
                    if (t != TURN_LAST){ // ラストストーンならやらない
                        switch (myLead){
                            case 0: UNREACHABLE; break;
                            case 1: marume_line = 5 * 256; break;
                            case 2: marume_line = 4 * 256; break;
                            case 3: marume_line = 3 * 256; break;
                            default: marume_line = 2 * 256; break;
                        }
                        for (int s = -8; s <= (-marume_line - 255) / 256; ++s){
                            conv[s + 8] = -marume_line;
                        }
                    }
                }
            }
            else if (myLead == 0){ // 同点
                
            }
#endif // MODIFY_END_SCORE
            
            for (int s = -8; s <= -1; ++s){
                int sc = conv[s + 8] / 256;
                int hasu = conv[s + 8] % 256;
                rewardTable_[s + 8] = wpFunc(BLACK, e, sc, rs + sc) * (256 - hasu) + wpFunc(BLACK, e, max(-8, sc - 1), rs + max(-8, sc - 1)) * hasu;
            }
            for (int s = 0; s <= +8; ++s){
                int sc = conv[s + 8] / 256;
                int hasu = conv[s + 8] % 256;
                rewardTable_[s + 8] = wpFunc(BLACK, e, sc, rs + sc) * (256 - hasu) + wpFunc(BLACK, e, min(+8, sc + 1), rs + min(+8, sc + 1)) * hasu;
            }
            for(auto& r : rewardTable_){
                r /= 256.0;
            }
            setBlankOrder();
        }
        
        std::string toRewardString()const{ // 先手視点報酬
            std::ostringstream oss;
            oss << "Evaluator::Reward(by Black) = {" << endl;
            for (int s = -N_COLOR_STONES; s <= -1; ++s){
                oss << " " << reward(BLACK, s) << ",";
            }
            oss << endl;
            oss << " " << reward(BLACK, 0) << ",";
            oss << endl;
            for (int s = 1; s <= +N_COLOR_STONES; ++s){
                oss << " " << reward(BLACK, s) << ",";
            }
            oss << endl;
            oss << "};" << endl;
            return oss.str();
        }
        
        std::string toWPString()const{ // 先手視点勝率
            std::ostringstream oss;
            oss << "Evaluator::WP(by Black) = {" << endl;
            for (int s = -N_COLOR_STONES; s <= -1; ++s){
                oss << " " << wp(BLACK, s) << ",";
            }
            oss << endl;
            oss << " " << wp(BLACK, 0) << ",";
            oss << endl;
            for (int s = 1; s <= +N_COLOR_STONES; ++s){
                oss << " " << wp(BLACK, s) << ",";
            }
            oss << endl;
            oss << "};" << endl;
            return oss.str();
        }
        
        void setBlankOrder()noexcept{
            if (reward(WHITE, 0) > reward(WHITE, -1)){
                // blank end is better for WHITE
                blankBetter_ = true;
                reward01_ = reward(BLACK, 0);
            }
            else{
                blankBetter_ = false;
                reward01_ = reward(BLACK, -1);
            }
        }
        
        template<typename scorePosFunc_t>
        void normalize(const scorePosFunc_t& posFunc){
            // rewardTable_ を正規化
            fpn_t sum = 0, sum2 = 0;
            for(int s = -N_COLOR_STONES; s <= N_COLOR_STONES; ++s){
                fpn_t r = rewardTable_[s + N_COLOR_STONES];
                sum += r * posFunc(s);
                sum2 += r * r * posFunc(s);
            }
            mu_ = sum;
            sigma_ = sqrt(max(0.00000001, sum2 - mu_ * mu_));
            
            //CERR << "mu = " << mu_ << " sigma = " << sigma_ << endl;
            
            for(auto& r : rewardTable_){
                r = (r - mu_) / sigma_;
            }
            setBlankOrder();
        }

    private:
        eval_t rewardTable_[SCORE_LENGTH]; // winning percentage of Black when score
        bool blankBetter_;
        eval_t reward01_; // 後手が 0 or 1 点を自由に選んだ場合の報酬
        fpn_t mu_, sigma_; // 勝率の平均と標準偏差(エンド得点が)
    };
}

#endif // DCURLING_EVAL_EVALUATOR_HPP_