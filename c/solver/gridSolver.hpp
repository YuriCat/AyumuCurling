/*
 gridSolver.hpp
 Katsuki Ohto
*/

#ifndef DCURLING_SOLVER_GRIDSOLVER_HPP_
#define DCURLING_SOLVER_GRIDSOLVER_HPP_

namespace DigitalCurling{
    
    struct InfoGrid{
        eval_t ev;
        uint64_t info;
    };
    
    struct MinGrid{
        eval_t ev;
    };

    template<class grid_t, int _WIDTH, int _LENGTH>
    struct SimpleGridLayer{
        constexpr static int DIMENSION_ = 2;
        constexpr static int WIDTH_ = _WIDTH;
        constexpr static int LENGTH_ = _LENGTH;
        grid_t grid[WIDTH_ + 1][LENGTH_ + 1];
        
    };
    
    template<class _layer_t>
    class SimpleGridSolver{
    public:
        typedef _layer_t layer_t;

        layer_t layer_;
        eval_t alpha_, beta_;
        eval_t passScore_;

    public:
        static constexpr int WIDTH_ = layer_t::WIDTH_;
        static constexpr int LENGTH_ = layer_t::LENGTH_;

        void setAlpha(eval_t a){ alpha_ = a; }
        void setBeta(eval_t b){ beta_ = b; }
        void setWindows(eval_t a,eval_t b){
            alpha_ = a; beta_ = b;
        }

        void feed(int w, int l, eval_t aev){
            layer_.grid[w][l].ev = aev;
        }

        template<typename callback_t>
        eval_t integrate(int cw, int cl, int dw, int dl,const callback_t& weightFunc){
            assert(dw <= cw <= WIDTH_-dw && dl <= cl <= LENGTH_-dl);
            eval_t sumScore(0);
            for (int tw = -dw; tw <= dw; ++tw){
                for (int tl = -dl; tl <= dl; ++tl){
                    int w = cw + tw; int l = cl + tl;
                    fpn_t wei = weightFunc(tw, tl);

                    //cerr << "weight = " << wei << " , ev = " << layer_.grid[w][l].ev << endl;
                    sumScore += wei * layer_.grid[w][l].ev;
                }
            }
            return sumScore;
            
            UNREACHABLE;
        }

        template<typename callback_t>
        fpn_t integrateWeight(int dw, int dl, const callback_t& weightFunc){
            fpn_t sum = 0;
            for (int tw = -dw; tw <= dw; ++tw){
                for (int tl = -dl; tl <= dl; ++tl){
                    sum += weightFunc(tw, tl);
                }
            }
            return sum;
        }
    };

    template<int _WIDTH, int _LENGTH, class move_t, class board_t, class evaluationCallback_t>
    eval_t solveBySimpleGrid(move_t *const dst, const board_t& bd,
                             int turn, const evaluationCallback_t& evaluate){

        using solver_t = SimpleGridSolver<SimpleGridLayer<MinGrid, _WIDTH, _LENGTH>>;

        solver_t solver;
        //std::pair<int,int> tmpBestWL;
        eval_t bestScore = -65536;

        eval_t betaTolerance = 0.0001;

        constexpr fpn_t W_FVX_STEP = (FVX_MAX - FVX_MIN) / solver_t::WIDTH_;
        constexpr fpn_t L_FVY_STEP = (FVY_MAX - FVY_MIN) / solver_t::LENGTH_;
        
        auto WtoFDVx = [](int dw)->fpn_t{ return dw * W_FVX_STEP; };
        auto LtoFDVy = [](int dl)->fpn_t{ return dl * L_FVY_STEP; };

        auto WtoFVx = [WtoFDVx](int w)->fpn_t{ return FVX_MIN + WtoFDVx(w); };
        auto LtoFVy = [LtoFDVy](int l)->fpn_t{ return FVY_MIN + LtoFDVy(l); };

        constexpr int WZ = (F_ERROR_SIGMA_VX * 3) / W_FVX_STEP;
        constexpr int LZ = (F_ERROR_SIGMA_VX * 3) / L_FVY_STEP;

        fpn_t weightSum = solver.integrateWeight(WZ, LZ, [WtoFDVx, LtoFDVy](int dw, int dl)->fpn_t{
            return relativePdfDeltaVxVy(WtoFDVx(dw), LtoFDVy(dl));
        });

        //cerr << WZ << LZ << endl; getchar();

        //solver.setAlpha(calcStrictAlpha(bd, evaluator));
        //solver.setBeta(calcStrictBeta(bd, evaluator));

        solver.setWindows(-9999, +9999);

        for (int s = 0; s < 2; ++s){
            for (int w = 0; w <= solver_t::WIDTH_; ++w){
                for (int l = 0; l <= solver_t::LENGTH_; ++l){
                    // try move
                    fMoveXY<> mv(WtoFVx(w), LtoFVy(l), s);

                    // making move
                    board_t tbd = bd;
                    makeMoveNoRand(&tbd, turn, mv);

                    // evaluate after board
                    eval_t ev = evaluate(tbd, col);

                    // feed to solver
                    solver.feed(w, l, ev);

                    // integrate
                    int ow(w - WZ), ol(l - LZ);
                    if (ow >= WZ
                        && ol >= LZ
                        && ow <= solver_t::WIDTH_ - WZ
                        && ol <= solver_t::LENGTH_ - LZ){
                        
                        fpn_t osc = solver.integrate(ow, ol, WZ, LZ, [WtoFDVx, LtoFDVy](int dw, int dl)->fpn_t{
                            return relativePdfDeltaVxVy(WtoFDVx(dw), LtoFDVy(dl));
                        }) / weightSum;

                        //cerr << osc << endl;

                        if (osc >= solver.beta_ - betaTolerance){
                            // over beta. finish
                            fMoveXY<> omv(WtoFVx(ow), LtoFVy(ol), s);
                            *dst = omv; return osc;
                        }
                        if (osc > bestScore){
                            // currrent best score
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

#endif // DCURLING_SOLVER_GRIDSOLVER_HPP_