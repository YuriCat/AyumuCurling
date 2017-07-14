/*
gridsolver.hpp
Katsuki Ohto
*/

#ifndef DCURLING_GRID_SOLVER_HPP_
#define DCURLING_GRID_SOLVER_HPP_

namespace DigitalCurling{
    
    struct WeightGrid{
        float weight;
        float ev;
    };
    
    struct WeightGrid{
        float weight;
        float ev;
    };
    
    struct InfoGrid{
        eval_t ev;
        uint64_t info;
    };
    
    struct SimpleGrid{
        eval_t ev;
    };

    template<class grid_t,int _WIDTH,int _LENGTH>
    struct SimpleGridLayer{
        constexpr int DIMENSION_ = 2;
        constexpr static int WIDTH_ = _WIDTH;
        constexpr static int LENGTH_ = _LENGTH;
        grid_t grid[WIDTH_+1][LENGTH_+1];
        
    };
    
    template<class _layer_t,int64_t _REAL_XMIN, int64_t _REAL_XMAX>
    class SimpleGridSolver{
    public:
        typedef _layer_t layer_t;

        layer_t layer_;
        eval_t alpha_, beta_;
        eval_t passScore_;

    public:
        constexpr static int WIDTH = layer_t::WIDTH;
        constexpr static int LENGTH = layer_t::LENGTH;

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
            assert(dw <= cw <= WIDTH-dw && dl <= cl <= LENGTH-dl);
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

    struct WeightGridVectorLayer{
        std::vector<WeightGrid> grid;
    };

    class WeightGridVectorLayerSolver{

    };
    
    template<int _WIDTH,int _LENGTH,class move_t,class board_t,class evaluationCallback_t>
    eval_t solveBySimpleGrid(move_t *const dst, const board_t& bd, Color col, const evaluationCallback_t& evaluate){

    template<int _WIDTH,int _LENGTH,class move_t,class board_t,class evaluationCallback_t>
    eval_t solveBySimpleGrid(move_t *const dst, const board_t& bd, Color col, const evaluationCallback_t& evaluate){

        //単純な一様格子点による求解
        //座標系：VxVy座標

        typedef SimpleGridSolver<SimpleGridLayer<_WIDTH, _LENGTH>> solver_t;

        solver_t solver;
        //std::pair<int,int> tmpBestWL;
        eval_t bestScore = -65536;

        eval_t betaTolerance = 0.0001;

        auto WtoFDVx = [](int dw)->fpn_t{ return dw * ((FVX_MAX - FVX_MIN) / solver_t::WIDTH); };
        auto LtoFDVy = [](int dl)->fpn_t{ return dl * ((FVY_MAX - FVY_MIN) / solver_t::LENGTH); };

        auto WtoFVx = [WtoFDVx](int w)->fpn_t{ return FVX_MIN + WtoFDVx(w); };
        auto LtoFVy = [LtoFDVy](int l)->fpn_t{ return FVY_MIN + LtoFDVy(l); };

        constexpr int WZ = (F_RAND_SIGMA * 3) / (FVX_MAX - FVX_MIN) * solver_t::WIDTH;
        constexpr int LZ = (F_RAND_SIGMA * 3) / (FVY_MAX - FVY_MIN) * solver_t::LENGTH;

        fpn_t probSum = solver.tatamiSize(WZ, LZ, [WtoFDVx, LtoFDVy](int dw, int dl)->fpn_t{
            return relativePdfDeltaVxVy(WtoFDVx(dw), LtoFDVy(dl));
        });

        //cerr << WZ << LZ << endl; getchar();

        //solver.setAlpha(calcStrictAlpha(bd, evaluator));
        //solver.setBeta(calcStrictBeta(bd, evaluator));

        solver.setWindows(-9999, +9999);

        for (int s = 0; s < 2; ++s){
            for (int w = 0; w <= solver_t::WIDTH; ++w){
                for (int l = 0; l <= solver_t::LENGTH; ++l){
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
                    if (ow >= WZ && ol >= LZ && ow <= solver_t::WIDTH - WZ && ol <= solver_t::LENGTH - LZ){
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