/*
 image.hpp
 Katsuki Ohto
 */

// 盤面を2次元画像として処理

#ifndef DCURLING_AYUMU_MODEL_IMAGE_HPP_
#define DCURLING_AYUMU_MODEL_IMAGE_HPP_

#include "../ayumu_dc.hpp"
#include "../../simulation/fastSimulator.hpp"
#include "../../simulation/primaryShot.hpp"

namespace DigitalCurling{
    
    /**************************局面イメージ**************************/
    
    constexpr fpn_t BOARD_IMAGE_STONE_SIGMA = FR_STONE_RAD / 2;
    constexpr fpn_t BOARD_IMAGE_STONE_SIGMA2 = BOARD_IMAGE_STONE_SIGMA * BOARD_IMAGE_STONE_SIGMA;
    
    template<int W, int L, int P>
    struct ImageBase : public std::array<std::array<std::array<float, P>, L>, W>{
        constexpr static int width()noexcept{ return W; }
        constexpr static int length()noexcept{ return L; }
        constexpr static int plains()noexcept{ return P; }
        
        void clear()noexcept{
            for(int w = 0; w < width(); ++w){
                for(int l = 0; l < length(); ++l){
                    for(int p = 0; p < plains(); ++p){
                        *this[w][l][p] = 0;
                    }
                }
            }
        }
    };
    
    template<int W, int L, int P>
    struct ImageVer0 : ImageBase<W, L, P>{
    };
    
    fpn_t getGaussianDistance(fpn_t mx, fpn_t my, fpn_t x, fpn_t y)noexcept{
        fpn_t dx = mx - x;
        fpn_t dy = my - y;
        return 100 * exp(-(dx * dx + dy * dy) / (2 * BOARD_IMAGE_STONE_SIGMA2)) / (sqrt(2 * M_PI) * BOARD_IMAGE_STONE_SIGMA);
    }
    
    using ImageVer0_28x28x1 = ImageVer0<28, 28, 1>;
    
    template<int W>
    constexpr fpn_t WtoFX(int w)noexcept{
        return FX_PA_MIN + w * FR_PA_WIDTH / (W - 1);
    }
    template<int L>
    constexpr fpn_t LtoFY(int l)noexcept{
        return FY_PA_MIN + l * FR_PA_LENGTH / (L - 1);
    }
    
    template<class board_t, class image_t>
    void genBoardImageVer0ByPixel(const board_t& bd, image_t *const pimg){
        // 画素ベースで画像生成
        for(int w = 0; w < image_t::width(); ++w){
            for(int l = 0; l < image_t::length(); ++l){
                const fpn_t mx = WtoFX<image_t::width()>(w), my = LtoFY<image_t::length()>(l);
                for(int p = 0; p < image_t::plains(); ++p){
                    fpn_t v = 0;
                    iterateStone(bd, BLACK, [&v, mx, my](const auto& st)->void{
                        v += getGaussianDistance(mx, my, st.x, st.y);
                    });
                    iterateStone(bd, WHITE, [&v, mx, my](const auto& st)->void{
                        v -= getGaussianDistance(mx, my, st.x, st.y);
                    });
                    (*pimg)[w][l][p] = v;
                }
            }
        }
    }
    /*
     template<class board_t, typename image_t>
     void genBoardImageByStone(const board_t& bd, image_t *const pimg){
     // 石ベースで画像生成
     iterateStone(bd, BLACK, [&v, mx, my](const auto& st)->void{
     for(int w = 0; w < image_t::width(); ++w){
     for(int l = 0; l < image_t::length(); ++l){
     for(int p = 0; p < image_t::planes(); ++p){
     *pimg[w][l][p] = 0;
     v += image_t::getImageValue(mx, my, st.x, st.y);
     });
     iterateStone(bd, WHITE, [&v, mx, my](const auto& st)->void{
     v -= image_t::getImageValue(mx, my, st.x, st.y);
     });
     
     for(int w = 0; w < image_t::width(); ++w){
     for(int l = 0; l < image_t::length(); ++l){
     const fpn_t mx = WtoFX<image_t::width()>(w), my = LtoFY<image_t::length()>(l);
     for(int p = 0; p < image_t::planes(); ++p){
     fpn_t v = 0;
     
     *pimg[w][l][p] = v;
     }
     }
     }
     }
     */
    namespace Plain{
        enum{
            TEE = 0,
            HOUSE,
            BLACK_OCCUPY,
            WHITE_OCCUPY,
            GUARD_ZONE,
        };
        constexpr int ALL = 5;
    }
    
    using ImageVer1 = ImageBase<27, 51, Plain::ALL>;
    
    /*template<int W, int L>
     fpn_t occupyValue(fpn_t cx[W + 1], fpn_t cy[L + 1],
     int w, int l,
     fpn_t x, fpn_t y){ // ピクセル中の石の占有率の計算
     
     int nw, nl; // 最近コーナーのインデックス
     int fw, fl; // 最遠コーナーの座標
     
     if(x > mx){
     nx = mx + dx; fx = mx - dx;
     if(y > my){
     cy = my + dy;
     
     
     }else{
     
     }
     }else{
     const fpn_t cx = mx + dx;
     if(y > my){
     const fpn_t cy = my + dy;
     
     }else{
     
     }
     }
     if(XYtoR2(x - cx[nw], y - cy[nl]) < FR2_STONE_RAD){ // 最近コーナーと被っていない
     if(){ // 最近辺と被っている可能性がある
     }
     return 0;
     }else if(XYtoR2(x - cx[fw], y - cy[fl]) > FR2_STONE_RAD){ // 最遠コーナーを含んでいる
     return 1;
     }else{
     
     }
     }*/
    
    template<class board_t, typename image_t>
    void genBoardImageVer1(const board_t& bd, image_t *const pimg){
        constexpr int W = image_t::width();
        constexpr int L = image_t::length();
        // 画像生成
        pimg->clear();
        // plain 0 : ティー
        (*pimg)[W / 2][L / 2][Plain::TEE] = 1;
        // plain 1 : ハウス
        for(int w = 0; w < W; ++w){
            for(int l = 0; l < L; ++l){
                fpn_t x = WtoFX<W>(w), y = LtoFY<L>(l);
                if(isInHouse(x, y)){
                    (*pimg)[w][l][Plain::HOUSE] = 1;
                }
            }
        }
        // plain 2 : 黒
        /*iterateStone(bd, BLACK, [pimg](int i)->void{
         for(int w = 0; w < W; ++w){
         for(int l = 0; l < L; ++l){
         getGaussianDistance
         }
         }
         });
         // plain 3 : 白
         iterateStone(bd, WHITE, [pimg](int i)->void{
         
         });*/
        
        for(int w = 0; w < image_t::width(); ++w){
            for(int l = 0; l < image_t::length(); ++l){
                const fpn_t mx = WtoFX<image_t::width()>(w), my = LtoFY<image_t::length()>(l);
                fpn_t v[2] = {0};
                iterateStone(bd, BLACK, [&v, mx, my](const auto& st)->void{
                    v[BLACK] += getGaussianDistance(mx, my, st.x, st.y);
                });
                iterateStone(bd, WHITE, [&v, mx, my](const auto& st)->void{
                    v[WHITE] += getGaussianDistance(mx, my, st.x, st.y);
                });
                (*pimg)[w][l][Plain::BLACK_OCCUPY] = v[BLACK];
                (*pimg)[w][l][Plain::WHITE_OCCUPY] = v[WHITE];
            }
        }
        
        // ガード
        for(int w = 0; w < image_t::width(); ++w){
            for(int l = 0; l < image_t::length(); ++l){
                const fpn_t mx = WtoFX<image_t::width()>(w), my = LtoFY<image_t::length()>(l);
                // この点がガードされるか調べる
                fPosXY<> pos(mx, my);
                fpn_t v = 0;
                iterateStone(bd, BLACK, [&v, pos](const auto& st)->void{
                    fMoveXY<> mv;
                    genDraw(&mv, pos);
                    if(FastSimulator::hasCollisionDraw(FPOSXY_THROW, mv, pos, st)){
                        v += 1;
                    }
                });
                iterateStone(bd, WHITE, [&v, pos](const auto& st)->void{
                    fMoveXY<> mv;
                    genDraw(&mv, pos);
                    if(FastSimulator::hasCollisionDraw(FPOSXY_THROW, mv, pos, st)){
                        v += 1;
                    }
                });
                (*pimg)[w][l][Plain::GUARD_ZONE] = sqrt(v);
            }
        }
    }
    
    /**************************局面基本情報+エンド得点+局面イメージログ**************************/
    
    template<class image_t>
    struct ImageLog : public image_t{
        int end, turn, rscore, escore;
        
        void clear()noexcept{
            end = END_LAST; turn = TURN_LAST; rscore = 0; escore = 0;
            for(int w = 0; w < image_t::width(); ++w){
                for(int l = 0; l < image_t::length(); ++l){
                    for(int p = 0; p < image_t::plains(); ++p){
                        (*this)[w][l][p] = 0;
                    }
                }
            }
        }
        
        std::string toString()const{
            std::ostringstream oss;
            oss << end << " " << turn << " " << rscore << " " << escore;
            for(int w = 0; w < image_t::width(); ++w){
                for(int l = 0; l < image_t::length(); ++l){
                    for(int p = 0; p < image_t::plains(); ++p){
                        oss << " " << (*this)[w][l][p];
                    }
                }
            }
            return oss.str();
        }
    };
    
    template<class imageLog_t>
    int readImageLog(std::ifstream& ifs, imageLog_t *const pil){
        while(ifs){
            ifs >> pil->end >> pil->turn >> pil->rscore >> pil->escore;
            for(int w = 0; w < imageLog_t::width(); ++w){
                for(int l = 0; l < imageLog_t::length(); ++l){
                    for(int p = 0; p < imageLog_t::planes(); ++p){
                        ifs >> *pil[w][l][p];
                    }
                }
            }
        }
        return 0;
    }
}

#endif // DCURLING_AYUMU_MODEL_IMAGE_HPP_