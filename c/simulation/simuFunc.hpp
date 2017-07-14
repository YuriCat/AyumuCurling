/*
 simuFunc.hpp
 */

// Digital Curling
// Simulation Function besed on official products

#include "../dc.hpp"

#ifndef DCURLING_SIMULATION_SIMULATOR_HPP_
#define DCURLING_SIMULATION_SIMULATOR_HPP_

namespace DigitalCurling{
    
    namespace Simulator{
        
        // ショットの強さを摩擦力を考慮した値に直す
        template<class vxy_t, class pos_t>
        void DIRtoVEC(const pos_t& arg, vxy_t *const dst)noexcept{
            
            fpn_t x = arg.x - FX_THROW;
            fpn_t y = arg.y - FY_THROW;
            
            fpn_t l = sqrt(2 * F_FRIC_RINK_STONE) / sqrt(XYtoR(x, y));
            dst->setVX(x * l);
            dst->setVY(y * l);
        }
        
        // ^の逆関数
        template<class vxy_t, class pos_t>
        void VECtoDIR(const vxy_t& arg, pos_t *const dst)noexcept{
            const fpn_t l = XYtoR(arg.x, arg.y) / (2 * F_FRIC_RINK_STONE);
            dst->x = arg.x * l + FX_THROW;
            dst->y = arg.y * l + FY_THROW;
        }
        
        // 停止位置座標からノーレンジ着手を生成
        // スピンはmvにすでに指定されているものとする
        template<class pos_t, class nrmove_t>
        void calcDeparture(const pos_t& pos, nrmove_t *const pmv)noexcept
        {
            constexpr fpn_t tt = static_cast<fpn_t>(0.0335f);
            constexpr fpn_t k = static_cast<fpn_t>(1.22f);
            
            fPosXY<> tpos;
            
            if (pmv->getSpin() != Spin::RIGHT){ // 左回転
                tpos.set(k + pos.x - tt * (pos.y - FY_TEE), tt * (pos.x - FX_TEE) + pos.y);
            }
            else{ // 右回転
                tpos.set(-k + pos.x + tt * (pos.y - FY_TEE), -tt * (pos.x - FX_TEE) + pos.y);
            }
            
            DIRtoVEC(tpos, pmv);
        }
        
        // ^の逆関数
        // 着手から乱数無し時の予測停止位置を計算する
        template<class move_t, class pos_t>
        void calcDestination(const move_t& mv, pos_t *const pos)noexcept
        {
            
            constexpr fpn_t tt = static_cast<fpn_t>(0.0335f);
            constexpr fpn_t tt2p1 = tt * tt + static_cast<fpn_t>(1.0f);
            constexpr fpn_t k = static_cast<fpn_t>(1.22f);
            fPosXY<> dvec;
            
            VECtoDIR(mv, &dvec);
            if (mv.getSpin() != Spin::RIGHT){ // 左回転
                pos->set((dvec.x - tt * FY_TEE - k + tt * dvec.y + tt * tt * FX_TEE) / tt2p1,
                         (-tt * dvec.x + tt * tt * FY_TEE + k * tt + dvec.y + tt * FX_TEE) / tt2p1);
            }
            else{ // 右回転
                pos->set((dvec.x + tt * FY_TEE + k - tt * dvec.y + tt * tt * FX_TEE) / tt2p1,
                         (+tt * dvec.x + tt * tt * FY_TEE + k * tt + dvec.y - tt * FX_TEE) / tt2p1);
            }
        }
        
        template<class float_t, class vec_t>
        void FrictionStep(float_t fric, const vec_t& vec, float_t angle,vec_t *const dst, float_t *const dstangle)noexcept
        {
            // 速度を返す
            float_t v=XYtoR(vec.x, vec.y);
            
            if(v > fric){
                float_t vx_v = vec.x / v;
                float_t vy_v = vec.y / v;
                float_t dx = -fric*vx_v;
                float_t dy = -fric*vy_v;
                if(angle != 0){
                    float_t d = FANGV_ORG * fric;
                    
                    if(angle > 0){
                        dx += d * vy_v;
                        dy += -d * vx_v;
                    }else{
                        dx += -d * vy_v;
                        dy += d * vx_v;
                    }
                }
                dst->set(vec.x + dx, vec.y + dy);
                *dstangle = angle;
            }else{
                dst->set(0, 0);
                *dstangle = 0;
            }
        }
        
        template<class float_t, class vec_t>
        void FrictionInvStep(float_t fric, const vec_t& vec, float_t angle, vec_t *const dst, float_t *const dstangle)noexcept
        {
            // FrictionStepの逆変換
            // 石が静止しているときだけは、前の状態を生成出来ないが、そのときは速度をY軸の正方向にする
            if(vec.x == 0 && vec.y == 0){
                dst->set(0, fric / 20000);
                *dstangle = FANGV_ORG;
            }else{
                float_t v = XYtoR(vec.x, vec.y);
                float_t vx_v = vec.x / v;
                float_t vy_v = vec.y / v;
                float_t dx = fric * vx_v;
                float_t dy = fric * vy_v;
                if(angle != 0){
                    float_t d = FANGV_ORG * fric;
                    
                    if(angle > 0){
                        dx += -d * vy_v;
                        dy += d * vx_v;
                    }else{
                        dx += d * vy_v;
                        dy += -d * vx_v;
                    }
                }
                dst->set(vec.x + dx, vec.y + dy);
                *dstangle = angle;
            }
        }
        
        
        template<class float_t, class mst_t>
        void stepSolo(mst_t *const mst, const float_t step)noexcept{
            // 一つの石の状態を進める
            fPosXY<float_t> ov(mst->vx(), mst->vy()), v; // ストーンの速度
            float_t ow(mst->getW()) ,w; // ストーンの角速度
            fPosXY<float_t> pos(mst->getX(), mst->getY());
            
            FrictionStep<float_t>(static_cast<float_t>(F_FRIC_RINK_STONE) * step * static_cast<float_t>(0.5), ov, ow, &v, &w); // 速度を半ステップ進める
            const float_t dx = v.x * step;
            const float_t dy = v.y * step;
            mst->x += dx; mst->y += dy; // 位置更新
            ov = v; ow = w;
            FrictionStep<float_t>(static_cast<float_t>(F_FRIC_RINK_STONE) * step * static_cast<float_t>(0.5), ov, ow, &v, &w); // 速度を半ステップ進める
            mst->vx_ = v.x; mst->vy_ = v.y;
            mst->w = w;
        }
        
        template<class float_t, class mst_t>
        void invStepSolo(mst_t *const mst, const float_t step)noexcept{
            // 一つの石の状態を元に戻す
            fPosXY<float_t> ov(mst->vx(), mst->vy()), v; // ストーンの速度
            float_t ow(mst->getW()), w; // ストーンの角速度
            fPosXY<float_t> pos(mst->getX(), mst->getY());
            FrictionInvStep<float_t>(static_cast<float_t>(F_FRIC_RINK_STONE) * step * static_cast<float_t>(0.5), ov, ow, &v, &w); // 速度を半ステップ戻す
            const float_t dx = v.x * step; const float_t dy = v.y * step;
            mst->x -= dx; mst->y -= dy; // 位置更新
            ov = v; ow = w;
            FrictionInvStep<float_t>(static_cast<float_t>(F_FRIC_RINK_STONE) * step * static_cast<float_t>(0.5), ov, ow, &v, &w); // 速度を半ステップ戻す
            mst->vx_ = v.x; mst->vy_ = v.y;
            mst->w = w;
        }
        
        template<class float_t = fpn_t, class mst0_t, class mst1_t>
        int simulateSolo(const mst0_t& org, mst1_t *const dst, const float_t step){
            
            // 衝突を考えず、1つの石の軌道をシミュレートする
            // 精度最優先
            // 最終到達位置を記録する
            // ステップは1フレームの時間
            
            fPosXY<float_t> ov(org.vx(), org.vy()), v; // ストーンの速度
            float_t ow(org.getW()) ,w; // ストーンの角速度
            fPosXY<float_t> pos(org.getX(), org.getY());
            
            FrictionStep((float_t)F_FRIC_RINK_STONE * step * (float_t)0.5, ov, ow, &v, &w);
            for(int i = 1;;){
                ++i;
                if(v.y == (float_t)0 && v.x == (float_t)0){
                    dst->setX(pos.x);
                    dst->setY(pos.y);
                    return 2;
                }
                
                const fpn_t dx = v.x * step;
                const fpn_t dy = v.y * step;
                pos.x += dx;
                pos.y += dy;
                
                ov = v;
                ow = w;
                FrictionStep(static_cast<float_t>(F_FRIC_RINK_STONE) * step, ov, ow, &v, &w); // 速度更新
            }
            UNREACHABLE;
        }
        
        template<class mst0_t, class mst1_t>
        int simulateSoloEx(const mst0_t& org, mst1_t *const dst, const fpn_t step){
            
            // 衝突を考えず、1つの石の軌道をシミュレートする
            // 精度最優先
            // それぞれの時間での位置を記録する
            // フレーム数を返す
            // ステップは1フレームの時間
            
            fPosXY<> ov(org.vx(), org.vy()), v; // ストーンの速度
            fpn_t ow(org.getW()), w; // ストーンの角速度
            dst[0] = org;
            
            FrictionStep(F_FRIC_RINK_STONE * step * 0.5, ov, ow, &v, &w);
            for(int i = 1;;){
                
                // 摩擦力の計算
                const fpn_t dx = v.x * step;
                const fpn_t dy = v.y * step;
                
                dst[i].setX(dst[i - 1].getX() + dx);
                dst[i].setY(dst[i - 1].getY() + dy);
                dst[i].setVX(v.x);
                dst[i].setVY(v.y);
                dst[i].setW(w);
                
                ++i;
                if(v.y == 0 && v.x == 0){ return 2; }
                
                ov = v;
                ow = w;
                FrictionStep(F_FRIC_RINK_STONE * step, ov, ow, &v, &w); // 速度更新
            }
            UNREACHABLE;
        }
        
        template<class pos_t, class move_t>
        void rotateToPassPoint(const pos_t& pos, move_t *const mv, const fpn_t step){
            // 指定された位置を通るように速度ベクトルを回転させる
            
            // ストーンの初期位置を(0, 0)とする座標系を使う
            
            //const b2Vec2 tar(pos.x - FX_THROW, FY_THROW - pos.y);
            const fpn_t ltar = XYtoR(pos.x - FX_THROW, pos.y - FY_THROW);
            const fpn_t ttar = atan2(pos.x - FX_THROW, pos.y - FY_THROW); // x,yの順は別にどっちでもいいが
            
            //CERR<<"l "<<ltar<<" theta "<<ttar<<endl;
            
            fPosXY<> otp(0, 0), tp; // ストーンの位置
            fPosXY<> ov(mv->vx(), mv->vy()), v; // ストーンの速度
            fpn_t ow, w; // ストーンの角速度
            
            // 回転を設定
            if(mv->isLeftSpin()){
                ow = FANGV_ORG * -1;
            }else{
                ow = FANGV_ORG;
            }
            
            fpn_t ol = 0;
            FrictionStep(F_FRIC_RINK_STONE * step * 0.5, ov, ow, &v, &w);
            
            for(int i = 0;;++i){
                
                // 摩擦力の計算
                const fpn_t dx = v.x * step;
                const fpn_t dy = v.y * step;
                tp.x = otp.x + dx;
                tp.y = otp.y + dy;
                
                //CERR<<tp<<endl;
                
                const fpn_t l = XYtoR(tp.x, tp.y); // 原点との距離
                
                if(l > ltar){ // 目標より遠くに行ってしまったので終了
                    // 距離の割合から、位置を微調整
                    fpn_t a = l - ol;
                    fpn_t b = ltar - ol;
                    
                    assert(a > 0);
                    
                    const fpn_t s = b / a;
                    otp.x += s * dx;
                    otp.y += s * dy;
                    
                    const fpn_t n = atan2(otp.x, otp.y) - ttar; // 角度差
                    
                    const fPosXY<> u = fPosXY<>(mv->x, mv->y); // 停止時の向きのみ正しい着手ベクトル
                    mv->setVX(+cos(n) * u.x + sin(n) * u.y);
                    mv->setVY(-sin(n) * u.x + cos(n) * u.y);
                    return;
                }
                if(v.y == 0){return;}
                
                ol = l;
                ov = v;
                ow = w;
                otp = tp;
                FrictionStep(F_FRIC_RINK_STONE * step, ov, ow, &v, &w);//速度更新
            }
            UNREACHABLE;
        }
    }
}

#endif // DCURLING_SIMULATION_SIMULATOR_HPP_