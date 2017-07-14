// デジタルカーリング
// 思考用局面表現に付加情報を付け、ClientFieldと同等に扱えるようにする

#ifndef DCURLING_TOOLS_THINKFIELD_H_
#define DCURLING_TOOLS_THINKFIELD_H_

#include "thinkBoard.hpp"

namespace DigitalCurling{
    struct ThinkField : public ThinkBoard{

        int reversed;
        //time limit
        TimeLimit tl[2];
        
        void init(){
            ThinkBoard::init();
            reversed = 0;
            setTimeLimit(BLACK, 0, 0);
            setTimeLimit(WHITE, 0, 0);
        }

        const TimeLimit& getTimeLimit(Color col)const{
            return tl[col];
        }

        void setTimeLimit(Color col, uint64_t aleft, uint64_t aby = 0ULL){
            tl[col].set(aleft, aby);
        }
        
        Color getTurnFirstColor(){
            // 手番を持つプレーヤーの試合開始時点での色を返す
            return static_cast<Color>(getTurnColor() ^ reversed);
        }
        Color getCurrentColor(const Color firstColor){
            // 試合開始時点での色から現在の色を返す
            return static_cast<Color>(firstColor ^ reversed);
        }
        
        int getRelScore(Color firstColor)const noexcept{
            // 試合開始時点での色を指定して相対得点を返す
            int rs = ThinkBoard::getRelScore();
            if(reversed ^ (isBlack(firstColor) ? 0 : 1)){
                rs = -rs;
            }
            return rs;
        }
        int getRelScore()const noexcept{
            return ThinkBoard::getRelScore();
        }
        
        template<class clientField_t>
        void setClientField(const clientField_t& cf){
            ThinkBoard::setClientField<clientField_t>(cf);
            // 時間制限
            for(int c = 0; c < 2; ++c){
                tl[c] = cf.getTimeLimit(1 - c);
            }
        }
        
        void procEnd(int asc){
            ThinkBoard::rscore += asc;
            if (asc < 0){ // flip color
                ThinkBoard::rscore = -ThinkBoard::rscore;
                reversed = 1 - reversed;
                swap(tl[0], tl[1]);
            }
            --ThinkBoard::end;
            initBoard();
            ThinkBoard::turn = TURN_FIRST;
        }

    };
}

#endif //DCURLING_TOOLS_THINKFIELD_HPP_