/*
 fastSimulatorFunc.hpp
 Katsuki Ohto
 */

// Digital Curling
// Fast Simulation using array ( floating point number calculation )
// Fast Simulatorが利用する値と小関数、初期化関数

#include "../dc.hpp"
#include "simuFunc.hpp"
#include "fastSimulator.h"

#ifndef DCURLING_SIMULATION_FASTSIMULATORFUNC_HPP_
#define DCURLING_SIMULATION_FASTSIMULATORFUNC_HPP_

namespace DigitalCurling{
    namespace FastSimulator{

        /**************************前計算データ*************************/
        
        // arrays
        std::array<std::array<fpn_t, 2>, (1 << FR_CZ_TABLE_RESOLUTION) + 4> FR_CZTable;

        /**************************前計算データへのアクセス*************************/

        void FVThtoFRTh(fpn_t *const pfr, fpn_t *const pfth, fpn_t fv, fpn_t fth, Spin s){
            *pfr = FVtoFR(fv);
            *pfth = isRight(s) ? (fth + FTHETA_CURL) : (fth - FTHETA_CURL);
        }

        void FVThtoFXYRTh(fpn_t *const pfx, fpn_t *const pfy, fpn_t *const pfr, fpn_t *const pfth,
            fpn_t fv, fpn_t fth, Spin s){
            FVThtoFRTh(pfr, pfth, fv, fth, s);
            *pfx = FX_THROW + *pfr * sin(*pfth);
            *pfy = FY_THROW + *pfr * cos(*pfth);
        }

        void FXYVThtoFXYRTh(fpn_t *const pfx, fpn_t *const pfy, fpn_t *const pfr, fpn_t *const pfth,
            fpn_t fx, fpn_t fy, fpn_t fv, fpn_t fth, Spin s){
            FVThtoFRTh(pfr, pfth, fv, fth, s);
            *pfx = fx + *pfr * sin(*pfth);
            *pfy = fy + *pfr * cos(*pfth);
        }
        
        /**************************初期化*************************/

        // initialiing thread
        
        void initR_CZ(){
            // 到達点から静止石までの距離 -> 衝突が起こる角度の範囲の変換
            constexpr pfpn_t step = static_cast<pfpn_t>(1) / static_cast<pfpn_t>(1500000);
            
            constexpr fpn_t MIN = -DBL_MAX, MAX = +DBL_MAX;
            
            // テーブル初期化
            for(auto i = 0; i < FR_CZTable.size(); ++i){
                FR_CZTable[i][0] = MAX;
                FR_CZTable[i][1] = MIN;
            }
            
            fMStone<pfpn_t> mst(0, 0, 0, 0, 0);
            while(XYtoR2(mst.x, mst.y) <= pow(FR_CZ_TABLE_STEP * FR_CZTable.size(), 2)){
                const fpn_t rm = XYtoR(mst.x, mst.y); // 動いている石の到達点との距離
                const fpn_t thm = XYtoT(mst.y, mst.x); // 動いている石の到達点基準の角度
                
                // 衝突可能性がある距離の全てに対して、範囲の上端下端を広げていく
                const int fid = max(0, int((rm - 2 * FR_STONE_RAD) / FR_CZ_TABLE_STEP));
                const int lid = min((int)FR_CZTable.size() - 1,
                              int((rm + 2 * FR_STONE_RAD) / FR_CZ_TABLE_STEP) + 1);
                
                for(int i = fid; i <= lid; ++i){
                    const fpn_t rs = i * FR_CZ_TABLE_STEP; // 静止石の、動石の到達点からの距離
                    // 三角形の成立条件による判定
                    if(rm + rs < 2 * FR_STONE_RAD){ // 石が必ずかぶる
                        FR_CZTable[i][0] = MIN;
                        FR_CZTable[i][1] = MAX;
                    }else if(rm + 2 * FR_STONE_RAD < rs){ //コンタクトし得ない
                    }else if(rs + 2 * FR_STONE_RAD < rm){ //コンタクトし得ない
                    }else{ // コンタクトする範囲がある
                        //  ぎりぎりコンタクトする時の到達点-静止石ベクトルと到達点-動石ベクトルのなす角度を余弦定理で求める
                        fpn_t cosrth = (pow(rm, 2) + pow(rs, 2) -  pow(2 * FR_STONE_RAD, 2)) / (2 * rm * rs);
                        fpn_t rth = acos(cosrth);
                        // 相対的角度
                        FR_CZTable[i][0] = min(thm - rth, FR_CZTable[i][0]);
                        FR_CZTable[i][1] = max(thm + rth, FR_CZTable[i][1]);
                    }
                }
                
                Simulator::invStepSolo<pfpn_t>(&mst, step);
            }
            for(auto i = 0; i < FR_CZTable.size(); ++i){
                //cerr << FR_CZTable[i][0] << " " << FR_CZTable[i][1] << endl;
            }
        }
        
        void init(){
            thread_t th4(initR_CZ);
            th4.join();
        }
        
        /**************************石のデータのステップ関数*************************/

        // 衝突した瞬間に、今後衝突しなかった場合に停止する位置と時刻を計算しておくか否かで
        // 2種類の関数を用意
        
        template<class mst_t>
        void stepByNextT(mst_t *const dst, const fpn_t v, const fpn_t nt){
            // 前計算データから石の位置をステップさせる
            fpn_t r = FVtoFR(v);
            fpn_t dt(nt - dst->t),
            nv(v - FDTtoFDV(dt))
            dtheta(FVtoFTh(fv)),
            dvtheta(dtheta - FVtoFTh(fnv));
            
            fpn_t theta = calcThetabyR(r);
            fpn_t nr = FVtoFR(nv);
            
            fpn_t ntheta = nr;
            
            fpn_t l2 = r * r + nr * nr - 2 * r * nr * cos(theta - ntheta);
            
            fpn_t 

            if (dst->w > 0){
                rotateToAdd(
                            dy, dx,
                            dst->th + dtheta,
                            &dst->y, &dst->x); // 位置を進める
                dx = -dx;
                dvtheta = -dvtheta;
                dtheta = -dtheta;
            }

            dst->setTh(dst->th() + dvtheta); // 向き設定
            dst->setV(nv);
            dst->t = nt; // 時刻設定
            dst->stepGoalInfo();
        }

        template<class mst_t>
        void stepByNextR(mst_t *const dst, fpn_t fv, fpn_t fnr){
            // 前計算データから石の位置をステップさせる
            fpn_t
                fnv(FRtoFV(fnr)),
                dx(FVtoFX(fv) - FVtoFX(fnv)),
                dy(FVtoFY(fv) - FVtoFY(fnv)),
                dtheta(FVtoFTh(fv)),
                dvtheta(dtheta - FVtoFTh(fnv));

            if (!(dst->w > 0)){
                dx = -dx;
                dvtheta = -dvtheta;
                dtheta = -dtheta;
            }
            rotateToAdd(
                dy, dx,
                dst->th() + dtheta,
                &dst->y, &dst->x); // 位置を進める
            dst->setTh(dst->th() + dvtheta); // 向き設定
            dst->setV(fnv);
            dst->t += FDVtoFDT(fv - fnv); // 時刻設定
            dst->stepGoalInfoByNextR(fnr);
        }

        /*template<>
        void stepByNextT(FSMovingStone *const dst, fpn_t fv, fpn_t fnt){
            // 前計算データから石の位置をステップさせる(停止時情報あり)
            //cerr<<"+"<<endl;
            fpn_t
                fdt(fnt - dst->t),
                fnv(fv - FDTtoFDV(fdt)),
                nx(dst->gx), ny(dst->gy),
                tx(-FVtoFX(fnv)), ty(-FVtoFY(fnv)),
                tth(FVtoFTh(fnv));

            if (!(dst->w > 0)){
                tx = -tx;
                tth = -tth;
            }
            rotateToAdd(
                ty, tx,
                dst->dth,
                &ny, &nx); // 位置を進める
            dst->x = nx; dst->y = ny;
            dst->setV(fnv);
            dst->setTh(tth + dst->dth);
            dst->t = fnt;
            dst->stepGoalInfo();
        }

        template<>
        void stepByNextR(FSMovingStone *const dst, fpn_t fv, fpn_t fnr){
            // 前計算データから石の位置をステップさせる(停止時情報あり)
            //cerr << "!" << endl;
            fpn_t
                fnv(FRtoFV(fnr)),
                nx(dst->gx), ny(dst->gy),
                tx(-FVtoFX(fnv)), ty(-FVtoFY(fnv)),
                tth(FVtoFTh(fnv));

            if (!(dst->w > 0)){
                tx = -tx;
                tth = -tth;
            }
            rotateToAdd(
                ty, tx,
                dst->dth,
                &ny, &nx); // 位置を進める
            dst->x = nx; dst->y = ny;
            dst->setV(fnv);
            dst->setTh(tth + dst->dth);
            dst->t += FDVtoFDT(fv - fnv);
            dst->stepGoalInfoByNextR(fnr);
        }*/

        template<class mst_t>
        void stepToStop(mst_t *const dst, fpn_t fv){
            // 前計算データから石の位置をステップさせる
            // 衝突せずに静止するまで動かす
            fpn_t dx, dtheta;
            if (dst->w > 0){
                dx = +FVtoFX(fv);
                dtheta = +FVtoFTh(fv);
            }
            else{
                dx = -FVtoFX(fv);
                dtheta = -FVtoFTh(fv);
            }
            rotateToAdd(
                        FVtoFY(fv), dx,
                        dst->th() - dtheta,
                        &dst->y, &dst->x); // 座標設定
            dst->t += FDVtoFDT(fv); // 時刻設定
        }

        /*template<>
        void stepToStop<FSMovingStone>(FSMovingStone *const dst, const fpn_t fv){
            // 前計算データから石の位置をステップさせる
            // 衝突せずに静止するまで動かす(停止時情報あり)
            dst->x = dst->gx; dst->y = dst->gy;
            dst->t = dst->gt;
        }*/
        
        /**************************位置<->座標変換*************************/
        
        template<class pos_t, class shot_t>
        void FXYtoFMV(const pos_t& pos, shot_t *const pshot){
            // pos で静止する (Vx, Vy) を計算
            // ドロー系のショット生成の基本関数
            // pshot 内ですでに回転が指定されている必要がある
            fpn_t dx(pos.getX() - FX_THROW);
            fpn_t dy(pos.getY() - FY_THROW);
            fpn_t r(XYtoR(dx, dy));
            fpn_t theta(XYtoT(dy, dx));
            fpn_t v(FRtoFV(r));
            fpn_t vtheta = !pshot->isLeftSpin() ? (vtheta + FTHETA_CURL) : (vtheta - FTHETA_CURL);
            pshot->setPCS(v, vtheta, pshot->spin());
        }
        
        template<class pos_t, class shot_t>
        void FMVtoFXY(const shot_t& shot, shot_t *const dst){
            // 停止位置を計算（ショットが極座標で与えられる場合が望ましい）
            fpn_t v(shot.v()); // 速さ
            fpn_t vtheta(shot.th()); // 投げる点での向き
            fpn_t x(FX_THROW), y(FY_THROW); // 投げる点の座標)
            fpn_t theta = !shot.isLeftSpin() ? (vtheta + FTHETA_CURL) : (vtheta - FTHETA_CURL);
            rotate(FVtoFR(v), 0, theta, &y, &x);
            dst->set(x, y);
        }
        
        /*template<class pos_t, class shot_t>
        static pos_t calcXYbyVxVy(const shot_t& shot){
            // 停止位置を計算（ショットが直交座標で与えられる場合）
            fpn_t x(FX_THROW), y(FY_THROW); // 投げる点の座標)
            return matmul(FANGMAT_CURL[mv.spin()],
        }*/
        
        fpn_t FRLtoFR(const fpn_t rOriginal, const fpn_t l){
            // 停止点まで距離 rOriginal のショットの 投擲点から距離 l の点での停止点との距離を求める
            fpn_t lLine = l + FR_COLLISION;
            fpn_t r = 0;
            fpn_t thetaOriginal = calcThetabyR(rOriginal);
            
            fpn_t tl = rOriginal;
            int steps = 0;
            do{
                r += tl - l;
                fpn_t theta = calcThetabyR(r);
                tl = sqrt((r * r) + (rOriginal * rOriginal) - 2 * r * rOriginal * cos(thetaOriginal - theta));
                
                steps += 1;
            }while(tl > lLine);
            return r;
        }
        
        fpn_t FVLtoFR(const fpn_t v, const fpn_t l){
            return FRLtoFR(FVtoFR(v), l);
        }
        
        fpn_t FRLtoFT(const fpn_t R, const fpn_t l){
            // 停止点まで距離 R のショット が 投擲点から距離 l の場合の初速度ベクトルの相対的な向きを返す
            fpn_t r = FRLtoFR(R, l);
            fpn_t cosAlpha = ((l * l) + (R * R) - (r * r)) / (2 * l * R);
            fpn_t alpha = acos(cosAlpha);
            return alpha + FTHETA_CURL;
        }
        
        template<class pos_t>
        fpn_t FXYVtoFT(const pos_t& pos, const fpn_t v, Spin spin){
            // 初速 v のショットが pos を通る場合の初速度の向きを求める
            fpn_t l = calcDistance(FPOSXY_THROW, pos);
            fpn_t R = FVtoFR(v);
            fpn_t theta = calcRelativeAngle(FPOSXY_THROW, pos);
            fpn_t dtheta = FRLtoFT(R, l);
            if(isRight(spin)){
                return theta + dtheta;
            }else{
                return theta - dtheta;
            }
            UNREACHABLE;
        }
        
        template<class pos_t>
        std::pair<fpn_t, fpn_t> FXYRtoFVT(const pos_t& pos, fpn_t r, Spin spin){
            // pos を通り r 進むショットの初速度の速さと向きを求める
            fpn_t l = calcDistance(FPOSXY_THROW, pos);
            fpn_t theta = calcRelativeAngle(FPOSXY_THROW, pos);
            fpn_t v = FRtoFV(r + l);
            fpn_t dtheta = FRLtoFT(r + l, l);
            if(isRight(spin)){
                return std::make_pair(v, theta + dtheta);
            }else{
                return std::make_pair(v, theta - dtheta);
            }
            UNREACHABLE;
        }
        
        /*template<class pos_t>
        void FV_FRtoFXY(fpn_t v, fpn_t r, pos_t *const pos, fpn_t *const theta, Spin spin){
            // 初速 v のショットが投擲点から距離rの点での相対位置を求める
            // もしrまで到達しないならば、適当な値が帰るので注意
            // ヒット系のショットの生成の基本関数
            assert(r >= 0);
        }*/
        
        template<class pos_t, class shot_t>
        void rotateToPassPointF(shot_t *const pshot, const pos_t& pos, fpn_t v){
            // 指定初速で、指定された位置を通る速度ベクトルを生成
            // ヒット系の着手の生成で呼ばれる
            fpn_t vtheta = FXYVtoFT(pos, v, pshot->spin());
            rotate(v, 0, vtheta, &pshot->y, &pshot->x);
        }
        
        template<class pos_t, class shot_t>
        void rotateToPassPointGoF(shot_t *const pshot, const pos_t& pos, fpn_t r){
            // 指定された位置を通り、さらに指定距離分(投擲点から見て)進む速度ベクトルを生成
            // ヒット系の着手の生成で呼ばれる
            auto v_th = FXYRtoFVT(pos, r, pshot->spin());
            rotate(v_th.first, 0, v_th.second, &pshot->y, &pshot->x);
        }
        
        template<class pos_t, class shot_t>
        void rotateToPassPointCircumferenceF(shot_t *const pshot, const pos_t& pos, fpn_t av, fpn_t atheta, fpn_t ar = 2 * FR_STONE_RAD){
            // 指定初速で、指定された位置の石からの相対位置 ar, atheta の位置を通る速度ベクトルを生成
            // 基本は ar = 2 × 石半径、つまり石のどの点をたたくかという使い方
            rotateToPassPointF(pshot, fPosXY<>(pos.x + ar * sin(atheta), pos.y + ar * cos(atheta)), av);
        }
    }
}

#endif // DCURLING_SIMULATION_FASTSIMULATORFUNC_HPP_