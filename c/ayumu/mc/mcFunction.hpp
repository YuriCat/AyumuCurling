/*
 mcFunction.hpp
 Katsuki Ohto
 */

#ifndef DCURLING_AYUMU_MC_MCFUNCTION_HPP_
#define DCURLING_AYUMU_MC_MCFUNCTION_HPP_

// 連続空間モンテカルロのための関数定義

namespace DigitalCurling{
    namespace Ayumu{
    
        double depthWeight(int d){
            // ノードの深さによって掛ける重み
            // 深ければ深いほど大きくする
            return pow(d + 1, 4);
        }
        
        double modifySize(double n, int partNum){
            // 重みつけ加算したトライアル回数を調整する
            // 調整しないと増えすぎる
            return pow(n, 1 / (double)2.5);
        }
        
        template<class node_t>
        bool isExpansionCondition(const node_t& nd, const int range){
            // 状態木展開閾値
            return range > RANGE_MIN_LEAFNODE && nd.size >= pow(1.3, RANGE_MAX_LEAFNODE - range);
        }
    }
}

#endif // DCURLING_AYUMU_MC_MCFUNCTION_HPP_