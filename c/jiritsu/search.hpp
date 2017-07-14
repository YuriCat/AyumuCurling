/*
 search.hpp
 Shu Kato (ideas)
 Katsuki Ohto
 */

// デジタルカーリング
// Shu Kato 氏の「じりつくん」に倣った着手候補点の離散化による探索

#ifndef DCURLING_JIRITSU_SEARCH_HPP_
#define DCURLING_JIRITSU_SEARCH_HPP_

#include "../dc.hpp"
#include "../structure/grid.hpp"

#include "../simulation/fastSimulator.hpp"

namespace DigitalCurling{
    
    namespace Jiritsu{
        
        struct Grid : public IntegratedGrid{
            ContactTree ct_;
            //eval_t oppPassEval_; // 相手がパスしたときの評価
            fpn_t aroundEvalSum_; // 周囲も含めた報酬和
            fpn_t aroundSize_;
            
            template<class board_t>
            void setInfo(const board_t& pre, const board_t& post, const fMoveXY<>& amv, const ContactTree& act){
                //oppPassEval_ = gEvaluator.reward(pre.getTurnColor(), countScore(post));
                //bd_ = post;
                ct_ = act;
            }
        };
        
        enum LayerType{
            kDrawLayer,
            kTakeOutLayer,
            kAllLayers,
        };
        
        constexpr fpn_t SMALL_RADIUS = 0.61f;
        
        constexpr fpn_t GxTable[] = {FR_STONE_RAD / 2, FR_STONE_RAD / 4};
        constexpr fpn_t GyTable[] = {FR_STONE_RAD * 2, DBL_MAX};
        
        constexpr size_t ZwTable[] = {6, 12};
        constexpr size_t ZlTable[] = {3, 0};
        
        constexpr fpn_t xMinTable[] = {FX_TEE - FR_HOUSE_RAD - GxTable[0] * ZwTable[0],
            FX_PA_MIN - SMALL_RADIUS - GxTable[1] * ZwTable[1]};
        constexpr fpn_t xMaxTable[] = {FX_TEE + FR_HOUSE_RAD + GxTable[0] * ZwTable[0],
            FX_PA_MAX + SMALL_RADIUS + GxTable[1] * ZwTable[1]};
        constexpr fpn_t yMinTable[] = {FY_PA_MIN + 2 * FR_STONE_RAD,
            FY_PA_MAX + 3.05 + 3.3 * FR_HOUSE_RAD - 1}; // 根拠不明
        constexpr fpn_t yMaxTable[] = {FY_PA_MAX + FY_ERROR_SIGMA * ZlTable[0],
            FY_PA_MAX + 3.05 + 3.3 * FR_HOUSE_RAD - 1}; // 根拠不明
        
        template<LayerType kLayerType>
        struct GridBoard{
            
        public:
            
            // 盤面定数
            static constexpr fpn_t X_MIN = xMinTable[kLayerType];
            static constexpr fpn_t X_MAX = xMaxTable[kLayerType];
            
            static constexpr fpn_t Y_MIN = yMinTable[kLayerType];
            static constexpr fpn_t Y_MAX = yMaxTable[kLayerType];
            
            // 全体の幅
            static constexpr fpn_t DX = X_MAX - X_MIN;
            static constexpr fpn_t DY = Y_MAX - Y_MIN;
            
            // 格子点の間隔
            static constexpr fpn_t GX = GxTable[kLayerType];
            static constexpr fpn_t GY = GyTable[kLayerType];
            
            // 積分に用いる格子点の数
            static constexpr size_t ZW = ZwTable[kLayerType];
            static constexpr size_t ZL = ZlTable[kLayerType];
            
            // 全体格子点の数
            static constexpr size_t W = static_cast<size_t>(fabs(DX / GX) + 0.999999); // c言語のabsが呼ばれると整数にされる
            static constexpr size_t L = static_cast<size_t>(fabs(DY / GY) + 0.999999);
            
            using base_t = Integrated2DGridLayer<Grid, W, L, ZW, ZL>;
            
        public:
            
            static constexpr size_t grids()noexcept{ return base_t::grids(); }
            static constexpr size_t xgrids()noexcept{ return W + 1; }
            static constexpr size_t ygrids()noexcept{ return L + 1; }
            
            static constexpr fpn_t DWtoDX(size_t w)noexcept{ return w * GX; }
            static constexpr fpn_t DLtoDY(size_t l)noexcept{ return l * GY; }
            
            static constexpr fpn_t WtoX(size_t w)noexcept{ return X_MIN + DWtoDX(w); }
            static constexpr fpn_t LtoY(size_t l)noexcept{ return Y_MIN + DLtoDY(l); }
            
            static constexpr size_t ItoW(size_t i)noexcept{ return i / ygrids(); }
            static constexpr size_t ItoL(size_t i)noexcept{ return i % ygrids(); }
            
            eval_t integratedBestEval()const noexcept{ return layer_.integratedBestEval; }
            
            fMoveXY<> bestMove(int spin)const noexcept{
                fMoveXY<> dst;
                dst.setSpin(static_cast<Spin>(spin));
                auto bestGrids = layer_.integratedBestPoint;
                FastSimulator::FXYtoFMV(fPosXY<>(WtoX(bestGrids[0]), LtoY(bestGrids[1])), &dst);
                return dst;
            }
            
            void feed(size_t w, size_t l, eval_t sz, eval_t eval){
                layer_.feed(w, l, sz, eval);
            }
            void feed(size_t index, eval_t sz, eval_t eval){
                feed(ItoW(index), ItoL(index), sz, eval);
            }
            static fMoveXY<> genGridMove(int w, int l, int spin)noexcept{
                fMoveXY<> dst;
                dst.setSpin(static_cast<Spin>(spin));
                fPosXY<> pos(WtoX(w), LtoY(l));
                DERR << w << " " << l << " " << pos << endl;
                FastSimulator::FXYtoFMV(pos, &dst);
                ASSERT(isValidMove(dst), cerr << dst << " " << dst.v() << " <-> " << FastSimulator::FRtoFV(calcDistance(FPOSXY_THROW, pos)) << endl;);
                return dst;
            }
            static fMoveXY<> genGridMove(int index, int spin)noexcept{
                return genGridMove(ItoW(index), ItoL(index), spin);
            }
            
            std::string toInfoString()const{
                std::ostringstream oss;
                oss << "x = [" << X_MIN << ", " << X_MAX << "] (" << DX << ") ";
                oss << "y = [" << Y_MIN << ", " << Y_MAX << "] (" << DY << ")" << endl;
                oss << "grid-dx = " << GX << " grid-dy = " << GY << endl;
                oss << "width = " << W << " length = " << L << " grids = " << grids() << endl;
                return oss.str();
            }
            
            void clear()noexcept{ layer_.clear(); }
            void integrate()noexcept{
                layer_.integrateAllGrids([](int w, int l)->fpn_t{
                    NormalDistribution<fpn_t> distX(0, FX_ERROR_SIGMA);
                    NormalDistribution<fpn_t> distY(0, FY_ERROR_SIGMA);
                    const fpn_t dx = DWtoDX(w), dy = DLtoDY(l);
                    const fpn_t density = distX.relative_dens(dx) * distY.relative_dens(dy);
                    //cerr << OutXY<fpn_t>(dx, dy) << " " << density << endl;
                    return density;
                });
                layer_.clearIntegratedBest();
                layer_.searchIntegratedBest();
            }
            
        private:
        
            base_t layer_;
        };
        
        struct LayerNode{
            
            // 格子レイヤー
            using drawLayer_t = GridBoard<kDrawLayer>;
            using takeOutLayer_t = GridBoard<kTakeOutLayer>;
            
            drawLayer_t drawLayer_[2];
            takeOutLayer_t takeOutLayer_[2];
            unsigned int trials;
            
            static constexpr size_t grids()noexcept{
                // グリッド数全体
                return (drawLayer_t::grids() + takeOutLayer_t::grids()) * 2;
            }
            
            static fMoveXY<> genGridMove(size_t index){
                ASSERT(index < grids(), cerr << index << " in " << grids() << endl;);
                size_t spin = index % 2;
                index /= 2;
                size_t layerType = index / drawLayer_t::grids();
                size_t gridIndex = index % drawLayer_t::grids();
                switch(layerType){
                    case kDrawLayer:{
                        return drawLayer_t::genGridMove(gridIndex, spin);
                    }break;
                    case kTakeOutLayer:{
                        return takeOutLayer_t::genGridMove(gridIndex, spin);
                    }break;
                    default: UNREACHABLE; break;
                }
                UNREACHABLE;
                return FMVXY_TEESHOT_R;
            }
            
            void feed(size_t index, const eval_t ev){
                ASSERT(index < grids(), cerr << index << " in " << grids() << endl;);
                size_t spin = index % 2;
                index /= 2;
                size_t layerType = index / drawLayer_t::grids();
                size_t gridIndex = index % drawLayer_t::grids();
                switch(layerType){
                    case kDrawLayer:{
                        drawLayer_[spin].feed(gridIndex, 1, ev);
                    }break;
                    case kTakeOutLayer:{
                        takeOutLayer_[spin].feed(gridIndex, 1, ev);
                    }break;
                    default: UNREACHABLE; break;
                }
            }
            void init()noexcept{
                for(int s = 0; s < 2; ++s){
                    drawLayer_[s].clear();
                    takeOutLayer_[s].clear();
                }
                trials = 0;
            }
            std::tuple<fMoveXY<>, eval_t> bestMoveEval()const noexcept{
                eval_t bestEval = -99999;
                fMoveXY<> bestMove = FMVXY_TEESHOT_R;
                for(int s = 0; s < 2; ++s){
                    if(takeOutLayer_[s].integratedBestEval() > bestEval){
                        bestEval = takeOutLayer_[s].integratedBestEval();
                        bestMove = takeOutLayer_[s].bestMove(s);
                    }
                    if(drawLayer_[s].integratedBestEval() > bestEval){
                        bestEval = drawLayer_[s].integratedBestEval();
                        bestMove = drawLayer_[s].bestMove(s);
                    }
                }
                return std::make_tuple(bestMove, bestEval);
            }
            void integrate()noexcept{
                for(int s = 0; s < 2; ++s){
                    drawLayer_[s].integrate();
                    takeOutLayer_[s].integrate();
                }
            }
        };
      
        template<class node_t, class board_t, class playout_t, class dice_t>
        void doPlayouts(node_t& node,
                        const board_t& bd,
                        const playout_t& playout,
                        dice_t *const pdice){
            
            // 点をランダムに入れ替えてシミュレーションを行う
            std::array<size_t, node_t::grids()> order;
            for(size_t i = 0; i < order.size(); ++i){
                order[i] = i;
            }
            std::mt19937 dice(pdice->rand());
            std::shuffle(order.begin(), order.end(), dice);
            
            for(int index : order){
                
                // 着手の生成
                fMoveXY<> fmv = node.genGridMove(index);
                eval_t ev = playout(node, bd, fmv);
                DERR << node.trials << " : " << index << " " << fmv << " " << ev << endl;
                node.feed(index, ev);
                node.trials += 1;
            }
            node.integrate();
        }
        
        template<class node_t, class board_t, class playout_t, class dice_t>
        void doPlayoutsWithUCB(node_t& node,
                        const board_t& bd,
                        const playout_t& playout,
                        dice_t *const pdice){
            
            // 点をランダムに入れ替えてシミュレーションを行う
            std::array<size_t, node_t::grids()> order;
            for(size_t i = 0; i < order.size(); ++i){
                order[i] = i;
            }
            std::mt19937 dice(pdice->rand());
            std::shuffle(order.begin(), order.end(), dice);
            
            for(int index : order){
                // 着手の生成
                fMoveXY<> fmv = node.genGridMove(index);
                eval_t ev = playout(bd, fmv);
                DERR << node.trials << " : " << index << " " << fmv << " " << ev << endl;
                node.feed(index, ev);
                node.trials += 1;
            }
            cerr << order.size() << endl;
            node.integrate();
        }
        
        template<class board_t, class playout_t, class dice_t>
        std::tuple<fMoveXY<>, eval_t> doMonteCarloSearch(const board_t& bd,
                                                         int playouts,
                                                         const playout_t& playout,
                                                         dice_t *const pdice){
            LayerNode node;
            node.init();
            do{
                doPlayouts(node, bd, playout, pdice);
            }while(node.trials < playouts && bd.getTurn() != TURN_LAST);
            cerr << node.trials << " playouts were done." << endl;
            return node.bestMoveEval();
        }
        template<class board_t, class playout_t, class dice_t>
        std::tuple<fMoveXY<>, eval_t> doMonteCarloSearchByTime(const board_t& bd,
                                                               uint64_t limitTime,
                                                               const playout_t& playout,
                                                               dice_t *const pdice){
            ClockMS clms;
            clms.start();
            LayerNode node;
            node.init();
            do{
                doPlayouts(node, bd, playout, pdice);
            }while(clms.stop() < limitTime && bd.getTurn() != TURN_LAST);
            cerr << node.trials << " playouts were done." << endl;
            return node.bestMoveEval();
        }
    }
}

#endif // DCURLING_JIRITSU_SEARCH_HPP_