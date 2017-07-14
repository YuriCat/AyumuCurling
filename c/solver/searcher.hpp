/*
search.hpp
Katsuki Ohto
*/

#ifndef DCURLING_SEARCH_HPP_
#define DCURLING_SEARCH_HPP_

namespace DigitalCurling{
    
    struct Grid{
        float weight;
        float ev;
    };

    //Layer
    struct GuardLayer{
        constexpr static int WIDTH = 1 + (1 << 5);
        constexpr static int LENGTH = 1 + (1 << 2);
        eval_t ev[WIDTH][LENGTH];
    };

    struct HouseLayer{
        constexpr static int WIDTH = 1 + (1 << 8);
        constexpr static int LENGTH = 1 + (1 << 6);
        eval_t ev[WIDTH][LENGTH];
    };

    struct ContactLayer{
        constexpr static int WIDTH = 1 + (1 << 3);
        constexpr static int LENGTH = 1 + (1 << 1);
        eval_t ev[WIDTH][LENGTH];
    };
    
    class MoveGridSolver{
    public:
        GuardLayer guardLayer_;
        houseLayer houseLayer_;
        ContactLayer contactLayer_[16][4];

        eval_t alpha_, beta_;
        eval_t passScore_;

    public:

        void setAlpha(eval_t a){ alpha_ = a; }
        void setBeta(eval_t b){ beta_ = b; }
        void setWindows(eval_t a,eval_t b){
            alpha_ = a; beta_ = b;
        }

        void feed(int w,int l,eval_t aev){
            layer_.ev[w][l] = aev;
        }

        template<typename callback_t>
        eval_t tatami(int cw, int cl, int dw, int dl,const callback_t& weightFunc){
            assert(dw <= cw < WIDTH-dw && dl <= cl < LENGTH-dl);
            eval_t sumScore(0), sumWeight(0);
            for (int tw = -dw; tw <= dw; ++tw){
                for (int tl = -dl; tl <= dl; ++tl){
                    int w = cw + tw; int l = cl + tl;
                    fpn_t wei = weightFunc(tw, tl);

                    //cerr << "weight = " << wei << " , ev = " << layer_.ev[w][l] << endl;

                    //sumWeight += wei;
                    sumScore += wei*layer_.ev[w][l];
                }
            }
            //if (sumWeight > 0){
                return sumScore;// / sumWeight;
            //}
            //else{
            //    return 0;
            //}
            UNREACHABLE;
        }

        template<typename callback_t>
        fpn_t tatamiSize(int dw, int dl, const callback_t& weightFunc){
            fpn_t sum = 0;
            for (int tw = -dw; tw <= dw; ++tw){
                for (int tl = -dl; tl <= dl; ++tl){
                    sum += weightFunc(tw, tl);
                }
            }
            return sum;
        }
    };

    template<class move_t,class board_t,class evaluationCallback_t>
    eval_t search(move_t *const dst, const board_t& bd, eval_t alpha, eval_t beta, const evaluationCallback_t& evaluate){

        //単純な一様格子点による求解
        //座標系：VxVy座標

        MoveGridSolver solver;

        eval_t bestScore = -65536;

        eval_t betaTolerance = 0.0001;

        auto WtoFDVx = [](int dw)->fpn_t{ return dw * ((FVX_MAX - FVX_MIN) / SimpleGridSolver::WIDTH); };
        auto LtoFDVy = [](int dl)->fpn_t{ return dl * ((FVY_MAX - FVY_MIN) / SimpleGridSolver::LENGTH); };

        auto WtoFVx = [WtoFDVx](int w)->fpn_t{ return FVX_MIN + WtoFDVx(w); };
        auto LtoFVy = [LtoFDVy](int l)->fpn_t{ return FVY_MIN + LtoFDVy(l); };

        constexpr int WZ = (F_RAND_SIGMA * 3) / (FVX_MAX - FVX_MIN) * SimpleGridSolver::WIDTH;
        constexpr int LZ = (F_RAND_SIGMA * 3) / (FVY_MAX - FVY_MIN) * SimpleGridSolver::LENGTH;

        fpn_t probSum = solver.tatamiSize(WZ, LZ, [WtoFDVx, LtoFDVy](int dw, int dl)->fpn_t{
            return relativePdfDeltaVxVy(WtoFDVx(dw), LtoFDVy(dl));
        });

        solver.setWindows(-9999, +9999);

        //コンタクトムーブを読む
        iterateAllStones(bd, [&solver](const auto& st)->void{
            for (int w = 0; w < HitWeight::MAX; ++w){
                if ()
            }
        });


        for (int s = 0; s < 2; ++s){
            for (int w = 0; w < SimpleGridSolver::WIDTH; ++w){
                for (int l = 0; l < SimpleGridSolver::LENGTH; ++l){
                    //手を決定
                    fMoveXY<> mv(WtoFVx(w), LtoFVy(l), s);

                    //この手を投げてみる
                    board_t tbd = bd;
                    makeMoveNoRand(&tbd, col, mv);

                    //ここで局面を評価
                    eval_t ev;
                    ev = evaluate(tbd, col);

                    //格子点評価を記録
                    solver.feed(w, l, ev);

                    //これまでの点で評価を畳み込み可能な点の評価を計算
                    int ow(w - WZ), ol(l - LZ);
                    if (ow >= WZ && ol >= LZ && ow < SimpleGridSolver::WIDTH - WZ && ol < SimpleGridSolver::LENGTH - LZ){
                        fpn_t osc = solver.tatami(ow, ol, WZ, LZ, [WtoFDVx, LtoFDVy](int dw, int dl)->fpn_t{
                            return relativePdfDeltaVxVy(WtoFDVx(dw), LtoFDVy(dl));
                        }) / probSum;

                        //cerr << osc << endl;

                        if (osc >= solver.beta_ - betaTolerance){
                            //ベータ超えなので終了
                            fMoveXY<> omv(WtoFVx(ow), LtoFVy(ol), s);
                            *dst = omv; return osc;
                        }
                        if (osc > bestScore){
                            //これまでで最高手
                            fMoveXY<> omv(WtoFVx(ow), LtoFVy(ol), s);
                            *dst = omv;
                            bestScore = osc;
                        }
                    }
                }
            }
        }
        return bestScore;
    }
}

#endif // DCURLING_GRID_SOLVER_HPP_