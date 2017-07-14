//Digital Curling
//classifying shots

#include "../../dc.hpp"
#include "../../simulation/b2dSimulator.hpp"
#include "../../simulation/fastSimulator.hpp"

#ifndef DCURLING_CLASSIFIER_HPP_
#define DCURLING_CLASSIFIER_HPP_

namespace DigitalCurling{
    
    // 局面と狙った手から、やろうとしたショットの言語手に分類する
    // いかなる手にも分類できなかった場合には、MOVE_ILLEGALを返す
    template<class board_t, class move_t>
    MoveXY classifyShot(const board_t& bd, const move_t& nrmv, auto *const pdist = nullptr){
        
        // 分類基準...activeゾーンに入らなければパス、コンタクトがなければ置く系
        // コンタクトがある場合、first contact stone の同一性を確認
        // spinは必ず同一である必要がある
        
        int best = -1;
        double minDistance2 = 9999999;
        MoveXY buf[1024];
        ThinkBoard tbd = bd;
        
        //cerr << nrmv << endl;
        //cerr << tbd.toDebugString() << endl;
        ContactTree ct = makeMoveNoRand<0>(&tbd, bd.getTurn(), nrmv);
        //cerr << tbd << endl;
        //cerr << ct << endl;
        
        const int NMoves = genChosenVMove(buf, bd);
        for(int i = 0; i < NMoves; ++i){
            ++pdist[buf[i].getType()][0];
        }
        
        if(!ct.hasContact()){
            // パスまたは置くだけ
            if(ct.isPass() || !isInActiveZone(tbd.stone(bd.getTurn()))){
                if(pdist != nullptr){
                    ++pdist[Standard::PASS][1];
                }
                return MOVE_PASS;
            }else{
                
                // 手の中で最も近いものを探す
                for(int i = 0; i < NMoves; ++i){
                    MoveXY mv = buf[i];
                    fMoveXY<> fmv;
                    realizeMove(&fmv, bd, mv);
                    if(!mv.hasContact() && mv != MOVE_PASS && (nrmv.spin() == fmv.spin())){
                        double d = XYtoR2(2 * (fmv.vx() - nrmv.vx()), fmv.vy() - nrmv.vy());
                        if(d < minDistance2){
                            best = i;
                            minDistance2 = d;
                        }
                    }
                }
            }
        }else{
            
            // 手の中で最も近いものを探す
            for(int i = 0; i < NMoves; ++i){
                MoveXY mv = buf[i];
                //cerr << mv << endl;
                fMoveXY<> fmv;
                realizeMove(&fmv, bd, mv);
                if(ct[0] == mv.getNum0() && nrmv.spin() == fmv.spin()){
                    double d = XYtoR2(2 * (fmv.vx() - nrmv.vx()), fmv.vy() - nrmv.vy());
                    if(d < minDistance2){
                        best = i;
                        minDistance2 = d;
                    }
                }
            }
        }
        
        if(best == -1){
            return MOVE_ILLEGAL;
        }else{
            if(pdist != nullptr){
                ++pdist[buf[best].getType()][1];
            }
            return buf[best];
        }
    }

    template<class board_t, class move_t>
    MoveXY classifyShotVxVy(const board_t& bd, const move_t& nrmv, int *const pdist = nullptr){
        
        // 分類基準...activeゾーンに入らなければパス、それ以外は(Vx, Vy 距離)
        // spinは必ず同一である必要がある
        
        int best = -1;
        double minDistance2 = 9999999;
        MoveXY buf[1024];
        ThinkBoard tbd = bd;
        
        //cerr << nrmv << endl;
        //cerr << tbd.toDebugString() << endl;
        ContactTree ct = makeMoveNoRand<0>(&tbd, bd.getTurn(), nrmv);
        //cerr << tbd << endl;
        //cerr << ct << endl;
        
        if(!ct.hasContact()){
            // パスまたは置くだけ
            if(ct.isPass() || !isInActiveZone(tbd.stone(bd.getTurn()))){
                if(pdist != nullptr){
                    ++pdist[Standard::PASS];
                }
                return MOVE_PASS;
            }
        }
        const int NMoves = genChosenVMove(buf, bd);
        
        // 手の中で最も近いものを探す
        for(int i = 0; i < NMoves; ++i){
            MoveXY mv = buf[i];
            //cerr << mv << endl;
            fMoveXY<> fmv;
            realizeMove(&fmv, bd, mv);
            if(nrmv.spin() == fmv.spin()){
                double d = XYtoR2(fmv.vx() - nrmv.vx(), fmv.vy() - nrmv.vy());
                if(d < minDistance2){
                    best = i;
                    minDistance2 = d;
                }
            }
        }
        
        if(best == -1){
            return MOVE_ILLEGAL;
        }else{
            if(pdist != nullptr){
                ++pdist[buf[best].getType()];
            }
            return buf[best];
        }
    }
}

#endif //DCURLING_AYUMU_CLASSIFIER_HPP_