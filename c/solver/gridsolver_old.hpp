/*
gridsolver.hpp
Katsuki Ohto
*/

#ifndef DCURLING_GRID_SOLVER_HPP_
#define DCURLING_GRID_SOLVER_HPP_

namespace DigitalCurling{
    
    struct Grid{
        float weight;
        float ev;
    };

    struct SimpleGridLayer{
        constexpr static int WIDTH = 32;
        constexpr static int HEIGHT = 16;
        eval_t ev[WIDTH][HEIGHT];
    };
    
    class SimpleGridSolver{
    private:
        SimpleGridLayer layer_;
        eval_t alpha_, beta_;
        eval_t passScore_;

    public:
        void setAlpha(eval_t a){ alpha = a; }
        void setBeta(eval_t b){ alpha = b; }
        void setWindows(eval_t a,eval_t b){
            alpha = a; beta = b;
        }

        void feed(int w,int l,eval_t aev){
            layer_.ev[w][l] = aev;
        }

        template<typename callback_t>
        eval_t tatami(int cw, int cl, int dw, int dl,const callback_t& weightFunc){
            assert(dw <= cw < SimpleGridLayer::WIDTH-dw && dl <= cl < SimpleGridLayer::HEIGHT-dl);
            eval_t sumScore(0), sumWeight(0);
            for (int tw = -dw; tw < dw; ++tw){
                for (int tl = -dl; tl < dl; ++tl){
                    int w = cw + tw; int l = cl + tl;
                    eval_t wei = weightFunc(tw, tl);
                    sumWeight += wei;
                    sumScore += wei*layer.eval[w][l];
                }
            }
            if (sumWeight > 0){
                return sumScore / sumWeight;
            }
            else{
                return 0;
            }
            UNREACHABLE;
        }
    };

    struct FreeGrid{
        float ev;
        float weight;

    };

    struct FreeGridVectorLayer{
        std::vector<FreeGrid> grid;
    };

    class FreeGridVectorLayerSolver{

    };

    template<class move_t,class board_t,class evaluator_t>
    int solveBySimpleGrid(move_t *const dst, const board_t& bd, Color col, evaluator_t evaluator){

        //単純な一様格子点による求解
        //座標系：VxVy座標

        SimpleGridSolver solver;
        //std::pair<int,int> tmpBestWL;
        eval_t bestScore = -65536;

        eval_t betaTolerance = 0.0001;

        constexpr int FX_MOVE_MIN = FX_PA_MIN;
        constexpr int FX_MOVE_MAX = FX_PA_MAX;
        constexpr int FY_MOVE_MIN = FY_PA_MIN;
        constexpr int FY_MOVE_MAX = 46.5425 - FY_THROW;

        fpn_t WtoFDX(int dw){ return dw * ((FX_MOVE_MAX - FX_MOVE_MIN) / SimpleGridSolver::WIDTH); }
        fpn_t LtoFDY(int dl){ return dl * ((FY_MOVE_MAX - FY_MOVE_MIN) / SimpleGridSolver::LENGTH); }

        fpn_t WtoFX(int w){ return FX_MOVE_MIN + WtoFDX(w); }
        fpn_t LtoFY(int l){ return FY_MOVE_MIN + LtoFDY(l); }

        fpn_t WLtoFP(int dw, int dl){ exp(-XYtoR2(WtoFDX(dw), LtoFDY(dl)) / pow(F_RAND_SIGMA, 2)); }//点の確率密度(相対値)を計算

        constexpr int WZ = (F_RAND_SIGMA * 2) / ((FX_MOVE_MAX - FX_MOVE_MIN) / SimpleGridSolver::WIDTH);
        constexpr int HZ = (F_RAND_SIGMA * 2) / ((FY_MOVE_MAX - FY_MOVE_MIN) / SimpleGridSolver::LENGTH);
        //solver.setAlpha(calcStrictAlpha(bd, evaluator));
        //solver.setBeta(calcStrictBeta(bd, evaluator));

        solver.setWindows(-9999, +9999);

        for (int s = 0; s < 2; ++s){
            for (int w = 0; w < SimpleGridSolver::WIDTH; ++w){
                for (int l = 0; l < SimpleGridSolver::LENGTH; ++l){
                    //手を決定
                    fPosXY<> dstPos(WtoFX(w), LtoFY(l));
                    fMoveXY<> mv;
                    mv.setSpin(s);
                    genDraw(&mv, dstPos);

                    //この手を投げてみる
                    board_t tbd = bd;
                    makeMove(&tbd, col, mv);

                    //ここで局面を評価
                    eval_t ev;
                    if (bd.getTurn() == TURN_LAST){
                        ev = evaluator.evaluateLast(col, tbd);
                    }
                    else{
                        ev = evaluator.evaluateMiddle(col, tbd);
                    }
                    //格子点評価を記録
                    solver.feed(w, l, ev);

                    //これまでの点で評価を畳み込み可能な点の評価を計算
                    int ow(w - WZ), ol(l - LZ);
                    if (ow >= WZ && ol >= LZ){
                        fpn_t osc = solver.tatami(ow, ol, WZ, WL, [](int dw, int dl)->double{
                            return WLtoFP(dw, dl);
                        });
                        if (osc >= solver.beta - betaTolerance){
                            //ベータ超えなので終了
                            *dst = mv; return 0;
                        }
                        if (osc > bestScore){
                            //これまでで最高手
                            *dst = mv;
                            bestScore = osc;
                        }
                    }
                }
            }
        }
        return 0;
    }
}

#endif // DCURLING_GRID_SOLVER_HPP_