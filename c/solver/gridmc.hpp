/*
gridmc.hpp
Katsuki Ohto
*/

#ifndef DCURLING_GRID_MC_HPP_
#define DCURLING_GRID_MC_HPP_

namespace DigitalCurling{
    
    struct IntegratedInfoGrid{
        
        using size_t = eval_t;
        
        float size;
        
        float evalSum;
        //float eval;
        
        float integratedEval;
        uint32_t info;
        //eval_t integratedBestEvalFar;
        //uint64_t info;uint64_t integratedInfo;
        
        void set(size_t sz, eval_t ev)noexcept{
            evalSum = sz * ev;
            //eval = ev;
            size = sz;
        }
        void feed(size_t sz, eval_t ev)noexcept{
            
            evalSum += sz * ev;
            //eval = (eval * size + ev * sz) / (size + sz);
            
            size += sz;
        }
        eval_t mean()const{
            return evalSum / size;
            //return eval;
        }
    };

    template<int _D_, class integratedGrid_t, int _WIDTH, int _LENGTH>
    struct IntegratedGridLayer{
        
        typedef std::array<int,_D_> point_t;
        typedef typename integratedGrid_t::size_t size_t;
        
        constexpr static int DIMENSION_ = _D_;
        constexpr static int WIDTH_ = _WIDTH;
        constexpr static int LENGTH_ = _LENGTH;
        
        //integratedGrid_t grid_[WIDTH_+1][LENGTH_+1];
        integratedGrid_t (*grid_)[LENGTH_+1];
        
        point_t integratedBestPoint;
        eval_t integratedBestEval;
        
        constexpr static int dimension()noexcept{return DIMENSION_;}
        constexpr static int width()noexcept{return WIDTH_;}
        constexpr static int length()noexcept{return LENGTH_;}
        
        static void assert_point(const point_t& point){
            ASSERT(0<=point[0] && point[0]<=WIDTH_,
                   std::cerr << point[0] << " in " << 0 << " ~ " << WIDTH_ << std::endl);
            ASSERT(0<=point[1] && point[1]<=LENGTH_,
                   std::cerr << point[1] << " in " << 0 << " ~ " << LENGTH_ << std::endl);
        }
        
        void fill(fpn_t sz, eval_t ev){
            for (int w = 0; w <= WIDTH_; ++w){
                for (int l = 0; l <= LENGTH_; ++l){
                    
                    grid_[w][l].set(sz, ev);
                    grid_[w][l].integratedEval=ev;
                    //grid_[w][l].integratedBestEvalFar=ev;
                }
            }
            integratedBestPoint = {WIDTH_/2,LENGTH_/2};
            integratedBestEval = ev;
        }
        
        /*
        template<typename callback_t>
        integrate(const std::array<int,DIMENSION_>& point,
                  const std::array<int,DIMENSION_>& zone
                  const callback_t& weightFunc){
            
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
            return sumScore;
        }
         */
        
        void integrateDist(const point_t& point, const eval_t evalDist, const size_t weight){
            assert_point(point);
            
            //cerr << "weight = " << weight << " dist = " << evalDist <<endl;
            grid_[point[0]][point[1]].integratedEval += weight * evalDist;
        }
        
        void clearIntegratedBest()noexcept{
            integratedBestEval = -10000;
        }
        
        void searchIntegratedBest()noexcept{
            eval_t newEval = integratedBestEval;
            point_t newPoint;
            
            for (int w = 0; w <= WIDTH_; ++w){
                for (int l = 0; l <= LENGTH_; ++l){
                    if(grid_[w][l].integratedEval > newEval){
                        newPoint = {w,l};
                        newEval = grid_[w][l].integratedEval;
                    }
                }
            }
            
            integratedBestPoint = newPoint;
            integratedBestEval = newEval;
        }
        
        void updateIntegratedBest(const point_t& point)noexcept{
            assert_point(point);
            eval_t newEval = grid_[point[0]][point[1]].integratedEval;
            if(newEval >= integratedBestEval){
                integratedBestPoint = point;
                integratedBestEval = newEval;
            }
        }
        
        IntegratedGridLayer():
        grid_(nullptr)
        {
            grid_ = new integratedGrid_t[WIDTH_+1][LENGTH_+1];
        }
        
        ~IntegratedGridLayer(){
            delete[] grid_;
            grid_ = nullptr;
        }
    };
    
    template<int _WIDTH, int _LENGTH, class move_t, class board_t, class dice_t, class playoutCallback_t>
    eval_t solveWithIntegratedGridWithMC(
                                         move_t *const dst, const board_t& bd, Color col, int trials,
                                         dice_t *const pdice, const playoutCallback_t& playout){

        typedef IntegratedGridLayer<2, IntegratedInfoGrid, _WIDTH, _LENGTH> layer_t;

        Clock cl;
        
        //layer_t *const layer = new layer_t[2];
        layer_t layer[2];
        
        int layerTrials[2];
        eval_t sumEval = 0;
        eval_t betaTolerance = 0.0001;
        
        uint64_t time[4] = {0};
        
        constexpr fpn_t W_FVX_STEP = (FVX_MAX - FVX_MIN) / layer_t::width();
        constexpr fpn_t L_FVY_STEP = (FVY_MAX - FVY_MIN) / layer_t::length();
        
        auto FVxtoBaseW = [](fpn_t fvx)->int{
            return int((fvx - FVX_MIN) / W_FVX_STEP);
        };
        auto FVytoBaseL = [](fpn_t fvy)->int{
            return int((fvy - FVY_MIN) / L_FVY_STEP);
        };
        
        auto WtoFDVx = [](int dw)->fpn_t{ return dw * W_FVX_STEP; };
        auto LtoFDVy = [](int dl)->fpn_t{ return dl * L_FVY_STEP; };

        auto WtoFVx = [WtoFDVx](int w)->fpn_t{ return FVX_MIN + WtoFDVx(w); };
        auto LtoFVy = [LtoFDVy](int l)->fpn_t{ return FVY_MIN + LtoFDVy(l); };
        
        constexpr fpn_t FVxZ = F_RAND_SIGMA * 3;
        constexpr fpn_t FVyZ = F_RAND_SIGMA * 3;
        
        constexpr int WZ = FVxZ / W_FVX_STEP;
        constexpr int LZ = FVyZ / L_FVY_STEP;


        
        fpn_t pdfTable[WZ + 1][LZ + 1];
        for(int dw = 0; dw <= +WZ; ++dw){
            for(int dl = 0; dl <= +LZ; ++dl){
                pdfTable[dw][dl] = relativePdfDeltaVxVy(WtoFDVx(dw), LtoFDVy(dl), 0);
            }
        }
        
        fpn_t weightSum = 0;
        for(int dw = -WZ; dw <= +WZ; ++dw){
            for(int dl = -LZ; dl <= +LZ; ++dl){
                weightSum += pdfTable[abs(dw)][abs(dl)];
            }
        }
        for(int dw = 0; dw <= +WZ; ++dw){
            for(int dl = 0; dl <= +LZ; ++dl){
                pdfTable[dw][dl] /= weightSum;
            }
        }
        
        //VCERR(weightSum);getchar();
        
        //cerr << WZ << LZ << endl; getchar();

        //solver.setAlpha(calcStrictAlpha(bd, evaluator));
        //solver.setBeta(calcStrictBeta(bd, evaluator));

        //solver.setWindows(-9999, +9999);
        
        //initializing layer
        for(int s=0;s<2;++s){
            layer[s].fill(0.1,-4);
            layerTrials[s] = 0;
        }
        int bestSpin=Spin::RIGHT;
        
        constexpr fpn_t EPSILON_SPIN = 1;
        constexpr fpn_t EPSILON_LAYER = 1;
        
        auto modFunc = [](int t)->fpn_t{ return sqrt(t+1); };
        //auto modFunc = [](int t)->fpn_t{ return 1+log(1+log(1+log(t+1))); };
        //auto modFunc = [](int t)->fpn_t{ return t+1; };
        
        for(int t=0; t< trials; ++t){
            
            //cerr<<t<<" ";
            
            //トライする手を決める
            fMoveXY<> runMove;
            
            std::array<int,layer_t::dimension()> basePoint;
            
            cl.start();
            do{
                if(pdice->drand() > EPSILON_SPIN / modFunc(t)){
                    runMove.setSpin(bestSpin);
                }else{
                    runMove.setSpin(pdice->rand()%2);
                }
                
                auto& chosenLayer = layer[runMove.spin()];
                
                if(pdice->drand() > EPSILON_LAYER / modFunc(layerTrials[runMove.spin()])){
                    //積分後最高点の周囲に分布
                    runMove.setVx(WtoFVx(chosenLayer.integratedBestPoint[0]));
                    runMove.setVy(LtoFVy(chosenLayer.integratedBestPoint[1]));
                    
                    addRandToMove(&runMove,pdice);
                    
                }else{
                    //ランダムに選ぶ
                    runMove.setVx(FVX_MIN + pdice->drand() * (FVX_MAX - FVX_MIN));
                    runMove.setVy(FVY_MIN + pdice->drand() * (FVY_MAX - FVY_MIN));
                }
        
                basePoint[0] = FVxtoBaseW(runMove.vx());
                basePoint[1] = FVytoBaseL(runMove.vy());
            
            }while(!(0<=basePoint[0] && basePoint[0]<=_WIDTH-1 && 0<=basePoint[1] && basePoint[1]<=_LENGTH-1));
            
            time[3] += cl.stop();
            
            //cerr << runMove << endl;
            
            auto& chosenLayer = layer[runMove.spin()];
            
            cl.start();
            eval_t eval = playout(runMove);
            time[2] += cl.stop();
            
            sumEval += eval;
            
            //cerr << " eval = " << eval << endl;
            
            std::array<fpn_t,layer_t::dimension()> fraction = {
                (runMove.vx() - WtoFVx(basePoint[0])),
                (runMove.vy() - LtoFVy(basePoint[1])),
            };
            
            eval_t aroundGrid[1<<2];
            eval_t evalDistSum = 0;
            
            {
                auto& grid = chosenLayer.grid_[basePoint[0]][basePoint[1]];
                aroundGrid[0] = -grid.mean();
                //cerr << grid.mean() << " -> ";
                
                grid.feed(fraction[0]*fraction[1] / (W_FVX_STEP * L_FVY_STEP), eval);
                
                //cerr << grid.mean() << endl; getchar();
                aroundGrid[0] += chosenLayer.grid_[basePoint[0]][basePoint[1]].mean();
                evalDistSum += aroundGrid[0];
                //cerr<<fraction[0]*fraction[1] / (W_FVX_STEP * L_FVY_STEP) << endl;
            }
            {
                auto& grid = chosenLayer.grid_[basePoint[0]][basePoint[1]+1];
                aroundGrid[1] = -grid.mean();
                grid.feed(fraction[0]*(LtoFDVy(1)-fraction[1]) / (W_FVX_STEP * L_FVY_STEP), eval);
                aroundGrid[1] += grid.mean();
                evalDistSum += aroundGrid[1];
                //cerr << fraction[0]*(LtoFDVy(1)-fraction[1]) / (W_FVX_STEP * L_FVY_STEP) << endl;
            }
            {
                auto& grid = chosenLayer.grid_[basePoint[0]+1][basePoint[1]];
                aroundGrid[2] = -grid.mean();
                grid.feed((WtoFDVx(1)-fraction[0])*fraction[1] / (W_FVX_STEP * L_FVY_STEP), eval);
                aroundGrid[2] += grid.mean();
                evalDistSum += aroundGrid[2];
                //cerr << (WtoFDVx(1)-fraction[0])*fraction[1] / (W_FVX_STEP * L_FVY_STEP) << endl;
            }
            {
                auto& grid = chosenLayer.grid_[basePoint[0]+1][basePoint[1]+1];
                aroundGrid[3] = -grid.mean();
                grid.feed((WtoFDVx(1)-fraction[0])*(LtoFDVy(1)-fraction[1]) / (W_FVX_STEP * L_FVY_STEP), eval);
                aroundGrid[3] += grid.mean();
                evalDistSum += aroundGrid[3];
                //cerr << (WtoFDVx(1)-fraction[0])*(LtoFDVy(1)-fraction[1]) / (W_FVX_STEP * L_FVY_STEP) << endl;
            }
            //getchar();
            
            int wmin = max(0,basePoint[0]-WZ+1), wmax = min(_WIDTH, basePoint[0]+1+WZ-1);
            int lmin = max(0,basePoint[1]-LZ+1), lmax = min(_LENGTH, basePoint[1]+1+LZ-1);
            
            if((!(wmin <= chosenLayer.integratedBestPoint[0] && chosenLayer.integratedBestPoint[0] <= wmax
               && lmin <= chosenLayer.integratedBestPoint[1] && chosenLayer.integratedBestPoint[1] <= lmax))
               ||
               (evalDistSum > 0)
               ){
                
                 cl.start();
                
                for(int w=wmin; w<=wmax; ++w){
                    for(int l=lmin; l<=lmax; ++l){
                        
                        const std::array<int,2> point={w,l};
                        
                        /*
                        const std::array<fpn_t,2> baseDist = {WtoFDVx(w-basePoint[0]), LtoFDVy(l-basePoint[1])};
                        
                        cl.start();
                        
                        chosenLayer.integrateDist
                        (point, aroundGrid[0],
                         relativePdfDeltaVxVy(baseDist[0], baseDist[1]) / weightSum);
                        
                        chosenLayer.integrateDist
                        (point, aroundGrid[1],
                         relativePdfDeltaVxVy(baseDist[0], baseDist[1] - LtoFDVy(1)) / weightSum);
                        
                        chosenLayer.integrateDist
                        (point, aroundGrid[2],
                         relativePdfDeltaVxVy(baseDist[0] - WtoFDVx(1), baseDist[1]) / weightSum);
                        
                        chosenLayer.integrateDist
                        (point, aroundGrid[3],
                         relativePdfDeltaVxVy(baseDist[0] - WtoFDVx(1), baseDist[1] - LtoFDVy(1)) / weightSum);
                        
                        chosenLayer.updateIntegratedBest(point);
                        
                        time[0] += cl.stop();
                         */
                        
                       
                        
                        chosenLayer.integrateDist
                        (point, aroundGrid[0],
                         pdfTable[abs(w-basePoint[0])][abs(l-basePoint[1])]);
                        
                        chosenLayer.integrateDist
                        (point, aroundGrid[1],
                         pdfTable[abs(w-basePoint[0])][abs(l-basePoint[1]-1)]);
                        
                        chosenLayer.integrateDist
                        (point, aroundGrid[2],
                         pdfTable[abs(w-basePoint[0]-1)][abs(l-basePoint[1])]);
                        
                        chosenLayer.integrateDist
                        (point, aroundGrid[3],
                         pdfTable[abs(w-basePoint[0]-1)][abs(l-basePoint[1]-1)]);
                        
                        chosenLayer.updateIntegratedBest(point);
                        
                    }
                }
                
                time[0] += cl.stop();
                
            }else{
                cl.start();
                
                for(int w=wmin; w<=wmax; ++w){
                    for(int l=lmin; l<=lmax; ++l){
                        
                        const std::array<int,2> point={w,l};
                        const std::array<fpn_t,2> baseDist = {WtoFDVx(w-basePoint[0]), LtoFDVy(l-basePoint[1])};
                        
                        //cerr << baseDist[0] << ", " << baseDist[1] << endl;
                        //cerr << relativePdfDeltaVxVy(baseDist[0], baseDist[1]) << endl;
/*
                        chosenLayer.integrateDist
                        (point, aroundGrid[0],
                         relativePdfDeltaVxVy(baseDist[0], baseDist[1]) / weightSum);
                        
                        chosenLayer.integrateDist
                        (point, aroundGrid[1],
                         relativePdfDeltaVxVy(baseDist[0], baseDist[1] - LtoFDVy(1)) / weightSum);
                        
                        chosenLayer.integrateDist
                        (point, aroundGrid[2],
                         relativePdfDeltaVxVy(baseDist[0] - WtoFDVx(1), baseDist[1]) / weightSum);
 
 chosenLayer.integrateDist
 (point, aroundGrid[3],
 relativePdfDeltaVxVy(baseDist[0] - WtoFDVx(1), baseDist[1] - LtoFDVy(1)) / weightSum);
 */
                        
                        chosenLayer.integrateDist
                        (point, aroundGrid[0],
                         pdfTable[abs(w-basePoint[0])][abs(l-basePoint[1])]);
                        
                        chosenLayer.integrateDist
                        (point, aroundGrid[1],
                         pdfTable[abs(w-basePoint[0])][abs(l-basePoint[1]-1)]);
                        
                        chosenLayer.integrateDist
                        (point, aroundGrid[2],
                         pdfTable[abs(w-basePoint[0]-1)][abs(l-basePoint[1])]);
                        
                        chosenLayer.integrateDist
                        (point, aroundGrid[3],
                         pdfTable[abs(w-basePoint[0]-1)][abs(l-basePoint[1]-1)]);
                        
                        //cerr << aroundGrid[0] << endl;
                        //chosenLayer.updateIntegratedBest(point);
                    }
                }
                
                chosenLayer.clearIntegratedBest();
                chosenLayer.searchIntegratedBest();
                time[1] += cl.stop();
            }
            
            if(layer[Spin::RIGHT].integratedBestEval >= layer[Spin::LEFT].integratedBestEval){
                bestSpin = Spin::RIGHT;
            }else{
                bestSpin = Spin::LEFT;
            }
            layerTrials[runMove.spin()]++;
            
            //cerr << layer[bestSpin].integratedBestPoint[0] << ", ";
            //cerr << layer[bestSpin].integratedBestPoint[1] << " ";
            //cerr << layer[bestSpin].integratedBestEval << endl;
        }
        
        dst->setSpin(bestSpin);
        dst->setVx(WtoFVx(layer[bestSpin].integratedBestPoint[0]));
        dst->setVy(LtoFVy(layer[bestSpin].integratedBestPoint[1]));
        
        cerr << time[0] << endl;
        cerr << time[1] << endl;
        cerr << time[2] << endl;
        cerr << time[3] << endl;
        cerr << "mean Eval = " << sumEval / trials << endl;
        
        return layer[bestSpin].integratedBestEval;
    }
}

#endif // DCURLING_GRID_SOLVER_HPP_