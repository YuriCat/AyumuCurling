/*
 collision.hpp
 Katsuki Ohto
 */

// Digital Curling
// 衝突処理と衝突可能性判定

// TODO: 衝突の逆処理を書きたい

#ifndef DCURLING_SIMULATION_COLLISION_HPP_
#define DCURLING_SIMULATION_COLLISION_HPP_

#include "../dc.hpp"
#include "fastSimulator.h"

namespace DigitalCurling{
    namespace FastSimulator{

        template<class mst_t, class st_t>
        void collisionAw_As(mst_t *const aw, st_t *const as){
            // 衝突処理(動-静)

            // 衝突面がx軸と平行になるように回転
            fpn_t coltheta(XYtoT(as->y - aw->y, as->x - aw->x));

            DERR << coltheta << endl;

            // relative velocity(+)
            fpn_t vt(aw->v() * sin(aw->th() - coltheta)); // tangent velocity
            fpn_t vn(max(0.0, aw->v() * cos(aw->th() - coltheta))); // normal velocity

            //cerr << vn << ", " << vt << endl;
            //ASSERT(vn > -0.000001, cerr << "minus Normal-Velocity! " << vn << endl;);

            constexpr fpn_t velocityThreshold = 1;

            fpn_t ni_m = vn;
            if (vn <= velocityThreshold){
                ni_m /= 2; // 当たりが高速な時はバイアスをかける(Box2Dのよくわからない処理)
            }
            fpn_t maxFriction = F_FRIC_STONES * ni_m;

            fpn_t oldTangentLambda = 1 / 6.0 * vt;
            fpn_t ti_m = max(-maxFriction, min(oldTangentLambda, +maxFriction)); // 範囲内に収める

            fpn_t dw(ti_m * (2.0 / FR_STONE_RAD));

            // update params
            as->setV(XYtoR(ni_m, ti_m));
            as->setTh(XYtoT(ni_m, ti_m) + coltheta);
            as->w = dw;

            as->t = aw->t;

            aw->setV(XYtoR(vn - ni_m, vt - ti_m));
            aw->setTh(XYtoT(vn - ni_m, vt - ti_m) + coltheta);
            aw->w += dw;
            
            //cerr << as->w << endl;

            aw->setGoalInfo();
            as->setGoalInfo();
        }

        template<class mst0_t, class mst1_t>
        void collision2Aw(mst0_t *const aw0, mst1_t *const aw1){
            // 衝突処理(動-動)

            // 衝突面がx軸と平行になるように回転
            fpn_t coltheta(XYtoT(aw1->y - aw0->y, aw1->x - aw0->x));
            //CERR << coltheta << endl;

            fpn_t v0n(aw0->v() * cos(aw0->th() - coltheta));
            fpn_t v0t(aw0->v() * sin(aw0->th() - coltheta));

            fpn_t v1n(aw1->v() * cos(aw1->th() - coltheta));
            fpn_t v1t(aw1->v() * sin(aw1->th() - coltheta));

            fpn_t vn(v0n - v1n);
            fpn_t vt(v0t - v1t);

            //ASSERT(vn > -0.000001, cerr << "minus Normal-Velocity! " << vn << endl;);
            
            constexpr fpn_t velocityThreshold = 1;

            fpn_t ni_m = vn;
            if (fabs(vn) <= velocityThreshold){
                ni_m /= 2; // 当たりが高速な時はバイアスをかける(Box2Dのよくわからない処理)
            }
            fpn_t maxFriction = F_FRIC_STONES * ni_m;

            fpn_t oldTangentLambda = 1 / 6.0 * vt;
            fpn_t ti_m = max(-maxFriction, min(oldTangentLambda, +maxFriction)); // 範囲内に収める

            fpn_t dw(ti_m * (2.0 / FR_STONE_RAD));

            // update params
            aw1->setV(min(FV_SIMU_MAX, XYtoR(v1n + ni_m, v1t + ti_m)));
            aw1->setTh(XYtoT(v1n + ni_m, v1t + ti_m) + coltheta);
            aw1->w += dw;

            aw0->setV(min(FV_SIMU_MAX, XYtoR(v0n - ni_m, v0t - ti_m)));
            aw0->setTh(XYtoT(v0n - ni_m, v0t - ti_m) + coltheta);
            aw0->w += dw;

            aw0->setGoalInfo();
            aw1->setGoalInfo();
        }
/*
        template<class mst_t, class st_t>
        int invCollisionAw_As(mst_t *const aw, st_t *const as){
            //衝突逆処理(動-静)
            //前状態が生成不可なら-1を返す
            
            //衝突面がx軸と平行になるように回転
            fpn_t coltheta(XYtoT(as->y - aw->y, as->x - aw->x));
            
            DERR << coltheta << endl;
            
            //relative velocity(+)
            fpn_t vt(aw->v * sin(aw->th - coltheta));//tangent velocity
            fpn_t vn(max(0.0, aw->v * cos(aw->th - coltheta)));//normal velocity

            constexpr fpn_t velocityThreshold = 1.0;
            
            fpn_t ni_m = vn;
            if (vn <= velocityThreshold){
                ni_m /= 2;//当たりが高速な時はバイアスをかける
            }
            fpn_t maxFriction = F_FRIC_STONES * ni_m;
            
            fpn_t oldTangentLambda = 1 / 6.0 * vt;
            fpn_t ti_m = max(-maxFriction, min(oldTangentLambda, +maxFriction));//範囲内に収める
            
            fpn_t dw(ti_m * (2.0 / FR_STONE_RAD));
            
            //update params
            as->v = XYtoR(ni_m, ti_m);
            as->th = XYtoT(ni_m, ti_m) + coltheta;
            as->w = dw;
            
            as->t = aw->t;
            
            aw->v = XYtoR(vn - ni_m, vt - ti_m);
            aw->th = XYtoT(vn - ni_m, vt - ti_m) + coltheta;
            aw->w += dw;

            aw->setGoalInfo();
            as->setGoalInfo();
            return 0;
        }
        
        template<class mst0_t, class mst1_t>
        void invCollision2Aw(mst0_t *const aw0, mst1_t *const aw1){
            //衝突処理(動-動)
            
            //衝突面がx軸と平行になるように回転
            fpn_t coltheta(XYtoT(aw1->y - aw0->y, aw1->x - aw0->x));
            //CERR<<coltheta<<endl;
            
            fpn_t v0n(aw0->v * cos(aw0->th - coltheta));
            fpn_t v0t(aw0->v * sin(aw0->th - coltheta));
            
            fpn_t v1n(aw1->v * cos(aw1->th - coltheta));
            fpn_t v1t(aw1->v * sin(aw1->th - coltheta));
            
            fpn_t vn(v0n - v1n);
            fpn_t vt(v0t - v1t);
            
            //ASSERT(vn>-0.000001, cerr<<"minus Normal-Velocity! "<<vn<<endl; );
            
            constexpr fpn_t velocityThreshold = 1;
            
            fpn_t ni_m = vn;
            if (fabs(vn) <= velocityThreshold){
                ni_m /= 2;//当たりが高速な時はバイアスをかける
            }
            fpn_t maxFriction = F_FRIC_STONES * ni_m;
            
            fpn_t oldTangentLambda = 1 / 6.0 * vt;
            fpn_t ti_m = max(-maxFriction, min(oldTangentLambda, +maxFriction));//範囲内に収める
            
            fpn_t dw(ti_m * (2.0 / FR_STONE_RAD));
            
            //update params
            aw1->v = min(FV_SIMU_MAX, XYtoR(v1n + ni_m, v1t + ti_m));
            aw1->th = XYtoT(v1n + ni_m, v1t + ti_m) + coltheta;
            aw1->w += dw;
            
            aw0->v = min(FV_SIMU_MAX, XYtoR(v0n - ni_m, v0t - ti_m));
            aw0->th = XYtoT(v0n - ni_m, v0t - ti_m) + coltheta;
            aw0->w += dw;

            aw0->setGoalInfo();
            aw1->setGoalInfo();
        }
*/
        
        template<class pos0_t, class pos1_t>
        inline bool hasCollisionFan(const pos0_t& aw, const pos1_t& as){
            // 動石中心の扇形による(動-静)衝突可能性判定
            fpn_t cosL, sinL, cosR, sinR;
         
#if 0
            fpn_t thl, thr;
             if (aw.w > 0){
             thl = aw.th(); thr = aw.gth;
             }
             else{
             thl = aw.gth; thr = aw.th();
             }
             fpn_t cosL = cos(thl); fpn_t sinL = sin(thl);
             fpn_t cosR = cos(thr); fpn_t sinR = sin(thr);
#endif
            if (aw.w >= 0){
                cosL = cos(aw.th()); sinL = sin(aw.th());
                cosR = cosAaddB(sinL, cosL, sin(FTHETA_CURL_MAX), cos(FTHETA_CURL_MAX));
                sinR = sinAaddB(sinL, cosL, sin(FTHETA_CURL_MAX), cos(FTHETA_CURL_MAX));
            }
            else{
                cosR = cos(aw.th()); sinR = sin(aw.th());
                cosL = cosAsubB(sinR, cosR, sin(FTHETA_CURL_MAX), cos(FTHETA_CURL_MAX));
                sinL = sinAsubB(sinR, cosR, sin(FTHETA_CURL_MAX), cos(FTHETA_CURL_MAX));
            }
            
            fpn_t r_2R2 = (aw.gr + 2 * FR_STONE_RAD) * (aw.gr + 2 * FR_STONE_RAD);
            
            // 衝突可能性判定
            fpn_t dx = as.getX() - aw.getX();
            fpn_t dy = as.getY() - aw.getY();
            fpn_t r2 = XYtoR2(dx, dy);
            if (r2 < r_2R2){ // R Judge
                if (((dx * cosL - dy * sinL >= -2 * FR_STONE_RAD)
                     && (dx * cosR - dy * sinR <= +2 * FR_STONE_RAD))
                    ){ // Side Judge
                    return true;
                }
            }
            return false;
        }
        
        template<class pos0_t, class pos1_t, class pos2_t, class move_t>
        inline bool hasCollisionDraw(const pos0_t& aw, const move_t& mv, const pos1_t& dp, const pos2_t& as){
            // 停止位置を利用した計算テーブルによる(動-静)衝突可能性判定
            //if(XYtoR2(as.x - aw.x, as.y - aw.y) > pow(aw.gr + 2 * FR_STONE_RAD, 2)){ // 距離が足りない
            //    return false;
            //}
            
            // (gx, gy)と静止石の位置関係
            const fpn_t fr2 = XYtoR2(as.x - dp.x, as.y - dp.y);
            const fpn_t fr = sqrt(fr2);
            const fpn_t asth = XYtoT(as.y - dp.y, as.x - dp.x);
            
            // (gx, gy)から距離frの点において被りがあるか調べる
            const int rid = int(fr / FR_CZ_TABLE_STEP); // 事前計算テーブル上のインデックス
            fpn_t rth;
            
            // 停止点に対しての、静止石と動石の相対的な角度を計算し、区間内に収める
            
            if(!mv.isLeftSpin()){
                rth = fmod(asth - mv.th() + FVtoFTh(mv.v()) + 2 * M_PI, 2 * M_PI);
            }else{
                rth = fmod(-asth + mv.th() + FVtoFTh(mv.v()) + 2 * M_PI, 2 * M_PI);
            }
#if 0
            cerr << mv.th() << endl;
            cerr << FVtoFTh(mv.v()) << endl;
            cerr << "dp" << endl;
            cerr << dp.toString() << endl;
            cerr << "awake" << endl;
            cerr << aw.toString() << endl;
            cerr << "asleep" << endl;
            cerr << "xy = (" << as.x << ", " << as.y << ")" << endl;
            cerr << "r = " << fr << " " << "th = " << asth << endl;
            cerr << "rth = " << rth << endl;
            cerr << "in " << FR_CZTable[rid][0] << " ~ " << FR_CZTable[rid][1] << endl;
            getchar();
#endif
            return (FR_CZTable[rid][0] <= rth && rth <= FR_CZTable[rid][1]);
        }
        
        template<class pos0_t, class pos1_t, class pos2_t>
        inline bool hasCollisionGoal(const pos0_t& aw, const pos1_t& dp, const pos2_t& as){
            return hasCollisionDraw(aw, aw, dp, as);
        }
        
        template<class pos0_t, class pos1_t>
        inline bool hasCollisionChanceAwAs(const pos0_t& aw, const pos1_t& as){
            fPosXY<fpn_t> dp;
            dp.set(aw.gx, aw.gy);
            return hasCollisionGoal(aw, dp, as);
            //return hasCollisionFan(aw, as);
        }
    }
}

#endif // DCURLING_SIMULATION_COLLISION_HPP_