/*
 rbf.hpp
 Katsuki Ohto
 */

#include "../ayumu_dc.hpp"

#ifndef DCURLING_AYUMU_MODEL_RBF_HPP_
#define DCURLING_AYUMU_MODEL_RBF_HPP_

namespace DigitalCurling{
    
    template<int D>
    fpn_t rbf(const fpn_t i[D], const )
    
    template<int D>
    fpn_t rbfFilter(const fpn_t i[D], const fpn_t W[D][D], const fpn_t ){
        // 線形変換
        fpn_t a[D] = {0};
        for(int i = 0; i < D; ++i){
            for(int j = 0; j < D; ++j){
                a[D] += W[D][D]
            }
        }
    }
}

#endif //DCURLING_AYUMU_MODEL_RBF_HPP_