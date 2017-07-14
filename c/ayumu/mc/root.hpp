/*
 root.hpp
 Katsuki Ohto
 */

// デジタルカーリング
// モンテカルロ関連
// ルートノードでの処理

#ifndef DCURLING_AYUMU_MC_ROOT_HPP_
#define DCURLING_AYUMU_MC_ROOT_HPP_

namespace DigitalCurling{
    namespace Ayumu{
        
        /*
         template<class board_t,class move_t>
         fpn_t do_haetataki(board_t& bd,const move_t& mv,ThreadTools *const tools){
         //与えられた局面と着手からプレイアウトを行う
         //ハエ叩きモードなのでそのまま評価点を出す
         const int owner_col=bd.getTurnColor();
         changed=bd.makeMove(mv,&tools->ddice);//mvをプレーしてみる
         int sc=bd.getTmpScore();
         if( owner_col==BLACK ){
         
         }
         */
        struct RootGrid : public IntegratedGrid{
            ContactTree ct_;
            eval_t oppPassEval_; // 相手がパスしたときの評価
            //fMoveXY<> fmv_;
            //ThinkBoard bd_;
            
            template<class board_t>
            void setInfo(const board_t& pre, const board_t& post, const fMoveXY<>& amv, const ContactTree& act){
                oppPassEval_ = gEvaluator.reward(pre.getTurnColor(), countScore(post));
                //bd_ = post;
                ct_ = act;
            }
        };
        
        template<int W, int L, int ZW, int ZL, int ZDSGMVX, int ZDSGMVY>
        struct RootGridChild : public Integrated2DGridLayer<RootGrid, W, L, ZW, ZL>{
            
            using base_t = Integrated2DGridLayer<RootGrid, W, L, ZW, ZL>;
            using vmove_t = MoveXY;
            
            // 全体のVx, Vyの幅
            constexpr static fpn_t DZVX = ZDSGMVX * FVX_ERROR_SIGMA;
            constexpr static fpn_t DZVY = ZDSGMVY * FVY_ERROR_SIGMA;
            
            constexpr static int dimension()noexcept{ return base_t::dimension(); }
            constexpr static int width()noexcept{ return base_t::width(); }
            constexpr static int length()noexcept{ return base_t::length(); }
            constexpr static int zwidth()noexcept{ return base_t::zwidth(); }
            constexpr static int zlength()noexcept{ return base_t::zlength(); }
            
            constexpr static fpn_t DVX = (DZVX * width()) / zwidth();
            constexpr static fpn_t DVY = (DZVY * length()) / zlength();
            
            constexpr static fpn_t DWtoDVX(int w)noexcept{ return w * (DVX / width()); }
            constexpr static fpn_t DLtoDVY(int l)noexcept{ return l * (DVY / length()); }
            fpn_t WtoVX(int w)const noexcept{ return baseMove.vx() + DWtoDVX(w - (width() + 1) / 2); }
            fpn_t LtoVY(int l)const noexcept{ return baseMove.vy() + DLtoDVY(l - (length() + 1) / 2); }
            
            vmove_t vMove;
            fMoveXY<> baseMove;
            uint64_t hash;
            eval_t size_;
            
            eval_t mean()const noexcept{ return base_t::integratedBestEval; }
            eval_t evalSum()const noexcept{ return mean() * size(); }
            eval_t size()const noexcept{ return size_; }
            
            template<class move_t>
            void set(const vmove_t& avmv, const move_t& amv, uint64_t ahash){
                vMove = avmv;
                baseMove = amv;
                hash = ahash;
                size_ = 0;
                base_t::fill(0, 0);
            }
            
            void feed(int w, int l, size_t sz, eval_t ev){
                base_t::grid_[w][l].feed(sz, ev);
                size_ += sz;
            }
            
            void genGridMove(int w, int l, fMoveXY<> *const pmv)const{
                pmv->x = WtoVX(w);
                pmv->y = LtoVY(l);
                pmv->s = baseMove.spin();
            }
            void genGridMove(const std::array<int, dimension()>& point, fMoveXY<> *const pmv)const{
                genGridMove(point[0], point[1], pmv);
            }
        };
        
        constexpr int ROOT_GRID_W = 20;
        constexpr int ROOT_GRID_L = 20;
        constexpr int ROOT_GRID_ZSGMDVX = 3;
        constexpr int ROOT_GRID_ZSGMDVY = 3;
        constexpr int ROOT_GRID_ZW = 6;
        constexpr int ROOT_GRID_ZL = 6;
        
        constexpr int ROOT_LAST_GRID_W = 256 - 1;
        constexpr int ROOT_LAST_GRID_L = 256 - 1;
        constexpr int ROOT_LAST_GRID_ZSGMDVX = 4;
        constexpr int ROOT_LAST_GRID_ZSGMDVY = 4;
        constexpr int ROOT_LAST_GRID_ZW = ROOT_LAST_GRID_W * ROOT_LAST_GRID_ZSGMDVX * FVX_ERROR_SIGMA / (FVX_MAX - FVX_MIN);
        constexpr int ROOT_LAST_GRID_ZL = ROOT_LAST_GRID_L * ROOT_LAST_GRID_ZSGMDVY * FVY_ERROR_SIGMA / (FVY_MAX - FVY_MIN);
        
        using rootChild_t = RootGridChild<ROOT_GRID_W, ROOT_GRID_L,ROOT_GRID_ZW, ROOT_GRID_ZL, ROOT_GRID_ZSGMDVX, ROOT_GRID_ZSGMDVY>;
        using lastRootChild_t = RootGridChild<ROOT_LAST_GRID_W, ROOT_LAST_GRID_L, ROOT_LAST_GRID_ZW, ROOT_LAST_GRID_ZL, ROOT_LAST_GRID_ZSGMDVX, ROOT_LAST_GRID_ZSGMDVY>;
        
        eval_t pdfTable[ROOT_GRID_ZW + 1][ROOT_GRID_ZL + 1];
        eval_t pdfTableLast[ROOT_LAST_GRID_ZW + 1][ROOT_LAST_GRID_ZL + 1];
        
        void initGridPdf(){
            // normal
            {
                for(int dw = 0; dw <= ROOT_GRID_ZW; ++dw){
                    for(int dl = 0; dl <= ROOT_GRID_ZL; ++dl){
                        pdfTable[dw][dl] = relativePdfDeltaVxVy(rootChild_t::DWtoDVX(dw),
                                                                rootChild_t::DLtoDVY(dl),
                                                                Spin::RIGHT);
                    }
                }
                eval_t sum = 0;
                for(int dw = -ROOT_GRID_ZW; dw <= ROOT_GRID_ZW; ++dw){
                    for(int dl = -ROOT_GRID_ZL; dl <= ROOT_GRID_ZL; ++dl){
                        sum += pdfTable[abs(dw)][abs(dl)];
                    }
                }
                for(int dw = 0; dw <= ROOT_GRID_ZW; ++dw){
                    for(int dl = 0; dl <= ROOT_GRID_ZL; ++dl){
                        pdfTable[dw][dl] /= sum;
                    }
                }
                CERR << "1 step dvx = " << rootChild_t::DWtoDVX(1) << " dvy = " << rootChild_t::DLtoDVY(1) << endl;
                CERR << "fuzzy  dvx  = " << rootChild_t::DWtoDVX(ROOT_GRID_ZW) << " dy = " << rootChild_t::DLtoDVY(ROOT_GRID_ZL) << endl;
                CERR << "inner  dvx  = " << rootChild_t::DWtoDVX(ROOT_GRID_W - 2 * ROOT_GRID_ZW) << " dy = " << rootChild_t::DLtoDVY(ROOT_GRID_L - 2 * ROOT_GRID_ZL) << endl;
                CERR << "whole  dvx  = " << rootChild_t::DWtoDVX(rootChild_t::width()) << " dvy = " << rootChild_t::DLtoDVY(rootChild_t::length()) << endl;
                
                fMoveXY<> fmv[2];
                fPosXY<> pos[2];
                fmv[0].setSpin(Spin::RIGHT);
                FastSimulator::FXYtoFMV(FPOSXY_TEE, &fmv[0]);
                fmv[1] = fmv[0];
                fmv[1].x += rootChild_t::DWtoDVX(1);
                FastSimulator::FMVtoFXY(fmv[1], &pos[0]);
                fmv[1] = fmv[0];
                fmv[1].y += rootChild_t::DLtoDVY(1);
                FastSimulator::FMVtoFXY(fmv[1], &pos[1]);
                
                CERR << "1 step dx  = " << (pos[0].x - FX_TEE) << " dy = " << (pos[1].y - FY_TEE) << endl;
                CERR << "fuzzy  dx  = " << (pos[0].x - FX_TEE) * ROOT_GRID_ZW << " dy = " << (pos[1].y - FY_TEE) * ROOT_GRID_ZL << endl;
                CERR << "inner  dx  = " << (pos[0].x - FX_TEE) * (ROOT_GRID_W - 2 * ROOT_GRID_ZW) << " dy = " << (pos[1].y - FY_TEE) * (ROOT_GRID_L - 2 * ROOT_GRID_ZL) << endl;
                CERR << "whole  dx  = " << (pos[0].x - FX_TEE) * ROOT_GRID_W << " dy = " << (pos[1].y - FY_TEE) * ROOT_GRID_L << endl;
                
                CERR << "pdfTable[][] = {" << endl;
                for(int dw = 0; dw <= ROOT_GRID_ZW; ++dw){
                    for(int dl = 0; dl <= ROOT_GRID_ZL; ++dl){
                        CERR << pdfTable[dw][dl] << ", ";
                    }CERR << endl;
                }
                CERR << "}" << endl;
            }
            // last turn
            {
                for(int dw = 0; dw <= ROOT_LAST_GRID_ZW; ++dw){
                    for(int dl = 0; dl <= ROOT_LAST_GRID_ZL; ++dl){
                        pdfTableLast[dw][dl] = relativePdfDeltaVxVy(lastRootChild_t::DWtoDVX(dw),
                                                                    lastRootChild_t::DLtoDVY(dl),
                                                                    Spin::RIGHT);
                    }
                }
                eval_t sum = 0;
                for(int dw = -ROOT_LAST_GRID_ZW; dw <= ROOT_LAST_GRID_ZW; ++dw){
                    for(int dl = -ROOT_LAST_GRID_ZL; dl <= ROOT_LAST_GRID_ZL; ++dl){
                        sum += pdfTableLast[abs(dw)][abs(dl)];
                    }
                }
                for(int dw = 0; dw <= ROOT_LAST_GRID_ZW; ++dw){
                    for(int dl = 0; dl <= ROOT_LAST_GRID_ZL; ++dl){
                        pdfTableLast[dw][dl] /= sum;
                    }
                }
                CERR << "1 step dvx = " << lastRootChild_t::DWtoDVX(1) << " dvy = " << lastRootChild_t::DLtoDVY(1) << endl;
                CERR << "fuzzy  dvx  = " << lastRootChild_t::DWtoDVX(ROOT_LAST_GRID_ZW) << " dy = " << lastRootChild_t::DLtoDVY(ROOT_LAST_GRID_ZL) << endl;
                CERR << "inner  dvx  = " << lastRootChild_t::DWtoDVX(ROOT_LAST_GRID_W - 2 * ROOT_LAST_GRID_ZW) << " dy = " << lastRootChild_t::DLtoDVY(ROOT_LAST_GRID_L - 2 * ROOT_LAST_GRID_ZL) << endl;
                CERR << "whole  dvx  = " << lastRootChild_t::DWtoDVX(lastRootChild_t::width()) << " dvy = " << lastRootChild_t::DLtoDVY(lastRootChild_t::length()) << endl;
                
                fMoveXY<> fmv[2];
                fPosXY<> pos[2];
                fmv[0].setSpin(Spin::RIGHT);
                FastSimulator::FXYtoFMV(FPOSXY_TEE, &fmv[0]);
                fmv[1] = fmv[0];
                fmv[1].x += lastRootChild_t::DWtoDVX(1);
                FastSimulator::FMVtoFXY(fmv[1], &pos[0]);
                fmv[1] = fmv[0];
                fmv[1].y += lastRootChild_t::DLtoDVY(1);
                FastSimulator::FMVtoFXY(fmv[1], &pos[1]);
                
                CERR << "1 step dx  = " << (pos[0].x - FX_TEE) << " dy = " << (pos[1].y - FY_TEE) << endl;
                CERR << "fuzzy  dx  = " << (pos[0].x - FX_TEE) * ROOT_LAST_GRID_ZW << " dy = " << (pos[1].y - FY_TEE) * ROOT_LAST_GRID_ZL << endl;
                CERR << "inner  dx  = " << (pos[0].x - FX_TEE) * (ROOT_LAST_GRID_W - 2 * ROOT_LAST_GRID_ZW) << " dy = " << (pos[1].y - FY_TEE) * (ROOT_LAST_GRID_L - 2 * ROOT_LAST_GRID_ZL) << endl;
                CERR << "whole  dx  = " << (pos[0].x - FX_TEE) * ROOT_LAST_GRID_W << " dy = " << (pos[1].y - FY_TEE) * ROOT_LAST_GRID_L << endl;
                
                CERR << "pdfTableLast[][] = {" << endl;
                for(int dw = 0; dw <= ROOT_LAST_GRID_ZW; ++dw){
                    for(int dl = 0; dl <= ROOT_LAST_GRID_ZL; ++dl){
                        CERR << pdfTableLast[dw][dl] << ", ";
                    }CERR << endl;
                }
                CERR << "}" << endl;
            }
        }
        
        struct RootNode{
            //モンテカルロ法着手決定するルート
            
            //constexpr static int N_DRAW_CHILDS = 256;
            //constexpr static int N_HIT_CHILDS = 1024;
            //RootGridChild<16, 16> drawChild[N_DRAW_CHILDS];
            //RootGridChild<32, 0> hitChild[N_HIT_CHILDS];
            //int drawChilds, hitChilds;
            
            constexpr static int N_CHILDS = 1024;
            
            int childs;
            
            std::atomic<uint32_t> trials;
            
            uint64_t limitTime; // 時間制限
            uint64_t limitPlayouts; // プレイアウト回数制限
            
            rootChild_t child[N_CHILDS];
            
            RootNode(){ init(); }
            
            //int getNChilds()const noexcept{ return drawChilds + hitChilds; }
            int getNChilds()const noexcept{ return childs; }
            
            void init()noexcept{
                trials = 0;
                childs = 0;
                limitTime = 0;
                limitPlayouts = 0;
            }
            
            template<class board_t, class vmove_t, class move_t>
            void addChild(const board_t& bd, const vmove_t& avmv, const move_t amv){
                uint64_t hash_bd = bd.getHash(0);
                uint64_t hash_move = genHash_AbsoluteMove(avmv);
                /*if(!avmv.hasContact()){
                 assert(0 <= drawChilds && drawChilds < N_DRAW_CHILDS);
                 drawChild[drawChilds].set(avmv, amv, hash_bd ^ hash_move);
                 ++drawChilds;
                 }else{
                 assert(0 <= hitChilds && hitChilds < N_HIT_CHILDS);
                 hitChild[hitChilds].set(avmv, amv, hash_bd ^ hash_move);
                 ++hitChilds;
                 }*/
                assert(0 <= childs && childs < N_CHILDS);
                child[childs].set(avmv, amv, hash_bd ^ hash_move);
                ++childs;
            }
            
            void setLimitTime(uint64_t alt)noexcept{
                limitTime = alt;
            }
            void setLimitPlayouts(uint64_t alp)noexcept{
                limitPlayouts = alp;
            }
            /*
             template<typename callback_t>
             void iterateChild(const callback_t& callback){
             for(int i = 0; i < drawChilds; ++i){
             callback(i, drawChild[i]);
             }
             for(int i = 0; i < hitChilds; ++i){
             callback(N_DRAW_CHILDS + i, hitChild[i]);
             }
             }
             
             template<typename callback_t>
             void iterateChild(const callback_t& callback)const{
             for(int i = 0; i < drawChilds; ++i){
             callback(i, drawChild[i]);
             }
             for(int i = 0; i < hitChilds; ++i){
             callback(N_DRAW_CHILDS + i, hitChild[i]);
             }
             }
             */
            template<typename callback_t>
            void iterateChild(const callback_t& callback){
                for(int i = 0, n = childs; i < n; ++i){
                    callback(i, child[i]);
                }
            }
            template<typename callback_t>
            void iterateChild(const callback_t& callback)const{
                for(int i = 0, n = childs; i < n; ++i){
                    callback(i, child[i]);
                }
            }
            
            template<class dice_t>
            int getBestUCBIndex(dice_t *const dice)const{
                //UCB1-tunedによって検討する着手を選ぶ
                
                int best = -1;
                fpn_t bestScore = -99.0;
                const fpn_t allSize = trials;
                const fpn_t logAS = log(allSize);
                iterateChild([&](int c, const auto& ch)->void{
                    fpn_t tmpScore;
                    if (ch.size() < MIN_TRIALS_ROOT){
                        //最低プレイアウト数をこなしていないものは、大きな値にする
                        //ただし最低回数のもののうちどれかがランダムに選ばれるようにする
                        //tmpScore = (fpn_t)((1U << 16) - ((uint32_t)ch.size()) + (dice->rand() % (1U << 6)));
                        tmpScore = -9;
                    }
                    else{
                        //fpn_t l_s=logAS/size;
                        //fpn_t v=child[c].var(best)*child[c].size(best) + sqrt(2.0*l_s);
                        //fpn_t ucbt=child[c].mean(best) + 2.0*sqrt(l_s * cmin(static_cast<fpn_t>(0.25),v));
                        //fpn_t ucb = child[c].mean(best) + sqrt(logAS) * exp(5.5 - size*0.07);
                        
                        fpn_t ucb=calc_ucb1(ch.mean(), ch.size(), allSize, K_UCB_ROOT);
                        
                        tmpScore = ucb;
                    }
                    if (tmpScore > bestScore){
                        best = c;
                        bestScore = tmpScore;
                    }
                });
                return best;
            }
            
            std::tuple<fMoveXY<>, fpn_t> getBestMoveReward()const{
                fMoveXY<> bestMove = FMVXY_TEESHOT_R;
                fpn_t bestScore = -99999.0;
                iterateChild([&](int c, const auto& ch)->void{
                    fpn_t tmp = (ch.size() > 0) ? ch.mean() : (-99999.0);
                    //cerr << tmp << endl;
                    if (tmp > bestScore){
                        ch.genGridMove(ch.integratedBestPoint, &bestMove);
                        
                        //cerr << ch.integratedBestPoint[0] << ", " << ch.integratedBestPoint[1] << " " << bestMove << endl;
                        
                        bestScore = tmp;
                    }
                });
                return std::make_tuple(bestMove, bestScore);
            }
            
            //void sort()const{
            //現時点での評価が高い順に並べ替える
            //std::sort(child,child+nchilds,RootChild::GreaterMean);
            //}
            
#ifdef MONITOR
            void print()const{
                //モンテカルロ解析結果表示
                iterateChild([&](int c, const auto& ch)->void{
                    
                });
            }
#endif //MONITOR
        };
        
        RootNode gRootNode;
        //RootNode gL1RootNode;
        /*
         template<class root_t, class child_t, class board_t, class evaluator_t, class move_t>
         eval_t startPlayoutContinuousMove(root_t& root, child_t& child, const board_t& bd,
         const evaluator_t& evaluator, const move_t& mv, ThreadTools *const ptools){
         ThinkBoard tbd = bd;
         auto result = doUCT(tbd, mv, evaluator, BOARD_EX_DEPTH_MAX, 1024, ptools);
         //結果を周囲の点に配分
         eval_t aroundGrid[1 << 2];
         eval_t evalDistSum = 0;
         
         {
         auto& grid = chosenLayer.grid_[basePoint[0]][basePoint[1]];
         aroundGrid[0] = -grid.mean();
         grid.feed(fraction[0]*fraction[1] / (W_FVX_STEP * L_FVY_STEP), eval);
         aroundGrid[0] += chosenLayer.grid_[basePoint[0]][basePoint[1]].mean();
         evalDistSum += aroundGrid[0];
         }
         {
         auto& grid = chosenLayer.grid_[basePoint[0]][basePoint[1]+1];
         aroundGrid[1] = -grid.mean();
         grid.feed(fraction[0]*(LtoFDVy(1)-fraction[1]) / (W_FVX_STEP * L_FVY_STEP), eval);
         aroundGrid[1] += grid.mean();
         evalDistSum += aroundGrid[1];
         }
         {
         auto& grid = chosenLayer.grid_[basePoint[0]+1][basePoint[1]];
         aroundGrid[2] = -grid.mean();
         grid.feed((WtoFDVx(1)-fraction[0])*fraction[1] / (W_FVX_STEP * L_FVY_STEP), eval);
         aroundGrid[2] += grid.mean();
         evalDistSum += aroundGrid[2];
         }
         {
         auto& grid = chosenLayer.grid_[basePoint[0]+1][basePoint[1]+1];
         aroundGrid[3] = -grid.mean();
         grid.feed((WtoFDVx(1)-fraction[0])*(LtoFDVy(1)-fraction[1]) / (W_FVX_STEP * L_FVY_STEP), eval);
         aroundGrid[3] += grid.mean();
         evalDistSum += aroundGrid[3];
         }
         
         child.integrateDist();
         }
         */
        template<class root_t, class child_t, class board_t>
        eval_t startPlayoutAllGridsFirst(root_t& root, child_t& child, const board_t& bd, ThreadTools *const ptools){
            ThinkBoard tbd = bd;
            
            iterate2DGrid(child, [&](int w, int l)->void{
                copyBeforePlayout(bd, &tbd);
                //ThinkBoard tbd = bd;
                
                fMoveXY<> fmv;
                child.genGridMove(w, l, &fmv);
                
                //cerr << fmv << endl;
                
                PlayoutResult result;
#ifdef USE_MCTS
                doUCT(&result, tbd, fmv, BOARD_EX_DEPTH_MAX, 1024, ptools);
#else
                doSimulation(&result, tbd, fmv, ptools);
#endif
                
                const fpn_t lambda = 0;//0.1; //0;//0.2;
                eval_t ev = result.eval(bd.getTurnColor(), lambda);
                
                auto& grid = child.grid_[w][l];
                grid.setInfo(bd, tbd, fmv, result.ct);
                
                
                child.feed(w, l, 1, ev);
                ++root.trials;
            });
            
            //Clock cl;
            //cl.start();
            child.integrateAllGridsByTable(pdfTable);
            child.clearIntegratedBest();
            child.searchIntegratedBest();
            //cerr << cl.stop() << endl;
            return child.integratedBestEval;
        }
        
        template<class root_t, class child_t, class board_t>
        eval_t startPlayoutAllGridsAfter(root_t& root, child_t& child, const board_t& bd, ThreadTools *const ptools){
            ThinkBoard tbd = bd;
            iterate2DGrid(child, [&](int w, int l)->void{
                copyBeforePlayout(bd, &tbd);
                //ThinkBoard tbd = bd;
                
                fMoveXY<> fmv;
                child.genGridMove(w, l, &fmv);
                
                //cerr << fmv << endl;
                
                PlayoutResult result;
#ifdef USE_MCTS
                doUCT(&result, tbd, fmv, BOARD_EX_DEPTH_MAX, 1024, ptools);
#else
                doSimulation(&result, tbd, fmv, ptools);
#endif
                
                auto& grid = child.grid_[w][l];
                
                const fpn_t lambda = 0;//0.05 / (1 + log(1 + grid.size));
                eval_t ev = result.eval(bd.getTurnColor(), lambda);
                
                /*if(bd.getTurn() == TURN_BEFORE_LAST){
                    ev = min(ev, grid.oppPassEval_);
                }*/
                child.feed(w, l, 1, ev);
                ++root.trials;
            });
            
            //Clock cl;
            //cl.start();
            child.integrateAllGridsByTable(pdfTable);
            child.clearIntegratedBest();
            child.searchIntegratedBest();
            //cerr << cl.stop() << endl;
            
            return child.integratedBestEval;
        }
        
        eval_t l1ev[ROOT_LAST_GRID_W + 1][ROOT_LAST_GRID_L + 1];
        
        constexpr fpn_t lastWtoVX(int w)noexcept{ return FVX_MIN + lastRootChild_t::DWtoDVX(w); }
        constexpr fpn_t lastLtoVY(int l)noexcept{ return FVY_MIN + lastRootChild_t::DLtoDVY(l); }
        
        void L1Sub(const int tid, Spin s, const ThinkBoard *const pbd){
            eval_t pev = gEvaluator.reward(WHITE, countScore(*pbd));
            for(int w = tid; w <= ROOT_LAST_GRID_W; w += N_THREADS){ // 2016/8/26 田中先生がバグ発見
                for(int l = 0; l <= ROOT_LAST_GRID_L; ++l){
                    fMoveXY<> fmv(lastWtoVX(w), lastLtoVY(l), static_cast<Spin>(s));
                    MiniBoard mbd;
                    iterateStoneWithIndex(*pbd, [&mbd](uint32_t idx, const auto& st){
                        mbd.locateNewStone(idx, st);
                    });
                    ContactTree ct = makeMoveNoRand<1>(&mbd, TURN_LAST, fmv);
                    l1ev[w][l] = gEvaluator.reward(WHITE, countScore(mbd));
                }
            }
        }
        
        std::tuple<fMoveXY<>, eval_t, int> L1(const ThinkBoard& bd){
            
            eval_t best = -9999;
            fMoveXY<> bestMove;
            for(int s = 0; s < 2; ++s){
                
                thread_t thr[N_THREADS];
                for (int th = 0; th < N_THREADS; ++th){
                    thr[th] = thread_t(&L1Sub, th, static_cast<Spin>(s), &bd);
                }
                for (int th = 0; th < N_THREADS; ++th){
                    thr[th].join();
                }
                
                /*for(int w=0;w<ROOT_LAST_GRID_W;++w){
                 for(int l=0;l<ROOT_LAST_GRID_L;++l){
                 fMoveXY<> fmv;
                 fmv.set(FVX_MIN+(FVX_MAX-FVX_MIN)*w/ROOT_LAST_GRID_W, FVY_MIN + (FVY_MAX-FVY_MIN)*l/ROOT_LAST_GRID_L,s);
                 MiniBoard mbd;
                 iterateStoneWithIndex(bd, [&mbd](uint32_t idx, const auto& st){
                 mbd.locateNewStone(idx, st);
                 });
                 ContactTree ct=makeMoveNoRand<1>(&mbd, TURN_LAST, fmv);
                 l1ev[w][l]=bd.evaluator().reward(WHITE, countScore(mbd));
                 }
                 }*/
                
                for(int w = ROOT_LAST_GRID_ZW; w <= ROOT_LAST_GRID_W - ROOT_LAST_GRID_ZW; ++w){
                    for(int l = ROOT_LAST_GRID_ZL; l <= ROOT_LAST_GRID_L - ROOT_LAST_GRID_ZL; ++l){
                        eval_t iev = 0;
                        
                        for(int dw = -ROOT_LAST_GRID_ZW; dw <= ROOT_LAST_GRID_ZW; ++dw){
                            for(int dl = -ROOT_LAST_GRID_ZL; dl <= ROOT_LAST_GRID_ZL; ++dl){
                                fpn_t pdf = pdfTableLast[abs(dw)][abs(dl)];
                                iev += pdf * l1ev[w + dw][l + dl];
                            }
                        }
                        if(iev > best){
                            best = iev;
                            bestMove.set(lastWtoVX(w), lastLtoVY(l), static_cast<Spin>(s));
                        }
                    }
                }
            }
            return std::make_tuple(bestMove, best, 2 * (ROOT_LAST_GRID_W + 1) * (ROOT_LAST_GRID_L + 1));
        }
    }
    
    
}

#endif // DCURLING_AYUMU_MC_ROOT_HPP_