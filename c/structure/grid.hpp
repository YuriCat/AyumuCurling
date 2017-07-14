/*
 grid.hpp
 Katsuki Ohto
 */

#ifndef DCURLING_GRID_HPP_
#define DCURLING_GRID_HPP_

#include "../settings.h"

namespace DigitalCurling{
    
    struct InfoGrid{
        eval_t ev;
        uint64_t info;
    };
    
    struct MinGrid{
        eval_t ev;
    };
    
    template<class grid_t,int _WIDTH, int _LENGTH>
    struct SimpleGridLayer{
        constexpr static int DIMENSION_ = 2;
        constexpr static int WIDTH_ = _WIDTH;
        constexpr static int LENGTH_ = _LENGTH;
        grid_t grid[WIDTH_ + 1][LENGTH_ + 1];
        
    };
    
    template<class _layer_t>
    class SimpleGridSolver{
    public:
        using layer_t = _layer_t;
        
        layer_t layer_;
        eval_t alpha_, beta_;
        eval_t passScore_;
        
    public:
        constexpr static int WIDTH_ = layer_t::WIDTH_;
        constexpr static int LENGTH_ = layer_t::LENGTH_;
        
        
        void setAlpha(eval_t a){ alpha_ = a; }
        void setBeta(eval_t b){ beta_ = b; }
        void setWindows(eval_t a,eval_t b){
            alpha_ = a; beta_ = b;
        }
        
        void feed(int w, int l, eval_t aev){
            layer_.grid[w][l].ev = aev;
        }
        
        template<typename callback_t>
        eval_t integrate(int cw, int cl, int dw, int dl, const callback_t& weightFunc){
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
    
    struct IntegratedGrid{
        
        using size_t = eval_t;
        
        float size, evalSum, integratedEval;
        //eval_t integratedBestEvalFar;
        //uint64_t info;uint64_t integratedInfo;
        
        void set(size_t sz, eval_t ev)noexcept{
            evalSum = sz * ev;
            size = sz;
        }
        void feed(size_t sz, eval_t ev)noexcept{
            evalSum += sz * ev;
            size += sz;
        }
        eval_t mean()const{
            return evalSum / size;
        }
    };

    template<class integratedGrid_t, int _WIDTH, int _LENGTH, int _ZW, int _ZL>
    struct Integrated2DGridLayer{
        
        using size_t = typename integratedGrid_t::size_t;
        constexpr static int DIMENSION_ = 2;
        using point_t = std::array<int, DIMENSION_>;
        
        constexpr static int WIDTH_ = _WIDTH;
        constexpr static int LENGTH_ = _LENGTH;
        constexpr static int ZWIDTH_ = _ZW;
        constexpr static int ZLENGTH_ = _ZL;
        
        integratedGrid_t grid_[WIDTH_ + 1][LENGTH_ + 1];
        
        point_t integratedBestPoint;
        eval_t integratedBestEval;
        
        constexpr static int dimension()noexcept{ return DIMENSION_; }
        constexpr static int width()noexcept{ return WIDTH_; }
        constexpr static int length()noexcept{ return LENGTH_; }
        constexpr static int zwidth()noexcept{ return ZWIDTH_; }
        constexpr static int zlength()noexcept{ return ZLENGTH_; }
        
        static constexpr int grids()noexcept{ return (WIDTH_ + 1) * (LENGTH_ + 1); }
        static constexpr int innerGrids()noexcept{ return (WIDTH_ + 1 - 2 * ZWIDTH_) * (LENGTH_ + 1 - 2 * ZLENGTH_); }
        
        static void assert_point(const point_t& point){
            ASSERT(0 <= point[0] && point[0] <= WIDTH_,
                   std::cerr << point[0] << " in " << 0 << " ~ " << WIDTH_ << std::endl);
            ASSERT(0 <= point[1] && point[1] <= LENGTH_,
                   std::cerr << point[1] << " in " << 0 << " ~ " << LENGTH_ << std::endl);
        }
        static void assert_w(const int w){
            ASSERT(0 <= w && w <= WIDTH_,
                   std::cerr << w << " in " << 0 << " ~ " << WIDTH_ << std::endl);
        }
        static void assert_l(const int l){
            ASSERT(0 <= l && l <= LENGTH_,
                   std::cerr << l << " in " << 0 << " ~ " << LENGTH_ << std::endl);
        }
        
        void feed(int w, int l, size_t sz, eval_t ev){
            assert_w(w);
            assert_l(l);
            grid_[w][l].feed(sz, ev);
        }
        
        void fill(fpn_t sz, eval_t ev)noexcept{
            for (int w = 0; w <= WIDTH_; ++w){
                for (int l = 0; l <= LENGTH_; ++l){
                    grid_[w][l].set(sz, ev);
                    grid_[w][l].integratedEval = ev;
                }
            }
            integratedBestPoint = {(WIDTH_ + 1) / 2, (LENGTH_ + 1) / 2};
            integratedBestEval = ev;
        }
        void clear()noexcept{
            fill(0, 0);
            clearIntegratedBest();
        }
        
        template<typename callback_t>
        void iterateGridIndex(const callback_t& callback)noexcept{
            for(int w = 0; w <= width(); ++w){
                for(int l = 0; l <= length(); ++l){
                    callback(w, l);
                }
            }
        }
        template<typename callback_t>
        void iterateInnerGridIndex(const callback_t& callback)noexcept{
            for(int w = _ZW; w <= width() - _ZW; ++w){
                for(int l = _ZL; l <= length() - _ZL; ++l){
                    callback(w, l);
                }
            }
        }
        
        template<typename callback_t>
        void iterateGrid(const callback_t& callback)noexcept{
            iterateGridIndex([&](int w, int l)->void{
                callback(grid_[w][l]);
            });
        }
        template<typename callback_t>
        void iterateInnerGrid(const callback_t& callback)noexcept{
            iterateGridIndex([&](int w, int l)->void{
                callback(grid_[w][l]);
            });
        }
        
        void integrateAllGridsByTable(const eval_t pdfTable[_ZW + 1][_ZL + 1])noexcept{
            iterateInnerGridIndex([&](int w, int l)->void{
                eval_t evalSum = 0, pdfSum = 0;
                for(int w1 = w - _ZW; w1 < w; ++w1){
                    for(int l1 = l - _ZL; l1 < l; ++l1){
                        eval_t pdf = pdfTable[w - w1][l - l1];
                        pdfSum += pdf;
                        evalSum += grid_[w1][l1].mean() * pdf;
                    }
                    for(int l1 = l; l1 <= l + _ZL; ++l1){
                        eval_t pdf = pdfTable[w - w1][l1 - l];
                        pdfSum += pdf;
                        evalSum += grid_[w1][l1].mean() * pdf;
                    }
                }
                for(int w1 = w; w1 <= w + _ZW; ++w1){
                    for(int l1 = l - _ZL; l1 < l; ++l1){
                        eval_t pdf = pdfTable[w1 - w][l - l1];
                        pdfSum += pdf;
                        evalSum += grid_[w1][l1].mean() * pdf;
                    }
                    for(int l1 = l; l1 <= l + _ZL; ++l1){
                        eval_t pdf = pdfTable[w1 - w][l1 - l];
                        pdfSum += pdf;
                        evalSum += grid_[w1][l1].mean() * pdf;
                    }
                }
                grid_[w][l].integratedEval = evalSum / pdfSum;
            });
        }
        
        template<class pdfCallback_t>
        void integrateAllGrids(const pdfCallback_t& pdfCallback)noexcept{
            iterateInnerGridIndex([&](int w, int l)->void{
                eval_t evalSum = 0, pdfSum = 0;
                for(int w1 = w - _ZW; w1 < w; ++w1){
                    for(int l1 = l - _ZL; l1 < l; ++l1){
                        eval_t pdf = pdfCallback(w - w1, l - l1);
                        pdfSum += pdf;
                        evalSum += grid_[w1][l1].mean() * pdf;
                    }
                    for(int l1 = l; l1 <= l + _ZL; ++l1){
                        eval_t pdf = pdfCallback(w - w1, l1 - l);
                        pdfSum += pdf;
                        evalSum += grid_[w1][l1].mean() * pdf;
                    }
                }
                for(int w1 = w; w1 <= w + _ZW; ++w1){
                    for(int l1 = l - _ZL; l1 < l; ++l1){
                        eval_t pdf = pdfCallback(w1 - w, l - l1);
                        pdfSum += pdf;
                        evalSum += grid_[w1][l1].mean() * pdf;
                    }
                    for(int l1 = l; l1 <= l + _ZL; ++l1){
                        eval_t pdf = pdfCallback(w1 - w, l1 - l);
                        pdfSum += pdf;
                        evalSum += grid_[w1][l1].mean() * pdf;
                    }
                }
                grid_[w][l].integratedEval = evalSum / pdfSum;
            });
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
            
            iterateInnerGridIndex([&](int w, int l)->void{
                if(grid_[w][l].integratedEval > newEval){
                    newPoint = {w,l};
                    newEval = grid_[w][l].integratedEval;
                }
            });
            
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
    };
    
    template<class layer_t, typename callback_t>
    void iterate2DGrid(layer_t& layer, const callback_t& callback){
        for(int w = 0; w <= layer_t::width(); ++w){
            for(int l = 0; l <= layer_t::length(); ++l){
                callback(w, l);
            }
        }
    }
    
    template<class layer_t, typename callback_t>
    void iterate1DGridByCenter(layer_t& layer, const callback_t& callback){
        constexpr int W = layer_t::width();
        constexpr int HW = W / 2;
        int w = HW, s = 1;
        for(;;){
            callback(w);
            w -= s;
            if(w < 0)break;
            ++s;
            callback(w);
            w += s;
            if(w >= W)break;
            ++s;
        }
    }
    template<class layer_t, typename callback_t>
    void iterateCircleGridByCenter(layer_t& layer, const callback_t& callback){
        callback();
    }
}

#endif // DCURLING_GRID_HPP_