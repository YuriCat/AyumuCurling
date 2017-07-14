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
        std::array<FSimuData, (1 << FV_TABLE_RESOLUTION) + 4> FV_FAllTable;
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
        
        fpn_t ID_FRACtoFX(uint32_t id, fpn_t frac){
            return (FV_FAllTable[id].x * (FV_TABLE_STEP - frac) + FV_FAllTable[id + 1].x * frac) * FV_TABLE_INVSTEP;
        }
        fpn_t ID_FRACtoFY(uint32_t id, fpn_t frac){
            return (FV_FAllTable[id].y * (FV_TABLE_STEP - frac) + FV_FAllTable[id + 1].y * frac) * FV_TABLE_INVSTEP;
        }
        fpn_t ID_FRACtoFTh(uint32_t id, fpn_t frac){
            return (FV_FAllTable[id].th * (FV_TABLE_STEP - frac) + FV_FAllTable[id + 1].th * frac) * FV_TABLE_INVSTEP;
        }

        // メインテーブルの利用
        fpn_t FVtoFX(fpn_t fv){
            uint32_t id = FVtoID(fv); fpn_t frac = FVtoFRAC(fv);
            return (FV_FAllTable[id].x * (FV_TABLE_STEP - frac) + FV_FAllTable[id + 1].x * frac) * FV_TABLE_INVSTEP;
        }
        fpn_t FVtoFY(fpn_t fv){
            uint32_t id = FVtoID(fv); fpn_t frac = FVtoFRAC(fv);
            return (FV_FAllTable[id].y * (FV_TABLE_STEP - frac) + FV_FAllTable[id + 1].y * frac) * FV_TABLE_INVSTEP;
        }
        fpn_t FVtoFTh(fpn_t fv){
            uint32_t id = FVtoID(fv); fpn_t frac = FVtoFRAC(fv);
            return (FV_FAllTable[id].th * (FV_TABLE_STEP - frac) + FV_FAllTable[id + 1].th * frac) * FV_TABLE_INVSTEP;
        }
        fpn_t FVtoFRbyAllTable(fpn_t fv){
            uint32_t id = FVtoID(fv); fpn_t frac = FVtoFRAC(fv);
            return (FV_FAllTable[id].r * (FV_TABLE_STEP - frac) + FV_FAllTable[id + 1].r * frac) * FV_TABLE_INVSTEP;
        }
        
        /**************************初期化*************************/

        // initialiing thread

        // V_ALL変換初期化
        void initALL(){
            constexpr pfpn_t step = static_cast<pfpn_t>(1) / static_cast<pfpn_t>(1500000);

            fMStone<pfpn_t> mst(0, 0, 0, 0, 0), lmst, tmst;
            FV_FAllTable[0].set(0, 0, 0, 0);
            fpn_t tm = 0; // 逆算時刻
            for (auto i = 1; i < FV_FAllTable.size(); ++i){
                pfpn_t line = IDtoFV<pfpn_t>(i);
                pfpn_t fv;
                while (1){
                    fv = XYtoR(mst.vx(), mst.vy());
                    if (fv >= line){ break; }
                    Simulator::invStepSolo<pfpn_t>(&mst, step);
                    tm += step;
                }
                if (i % 64 == 0){
                    //cerr<<XYtoT(mst.y,mst.x)<<" , "<<XYtoT(mst.vy(),mst.vx())<<endl;
                }
                tmst = mst;
                lmst = mst;
                Simulator::stepSolo<pfpn_t>(&lmst, step);
                pfpn_t lfv = XYtoR(lmst.vx(), lmst.vy());
                pfpn_t frac;
                if (fv != lfv){
                    frac = (line - lfv) / (fv - lfv);
                }
                else{
                    frac = static_cast<pfpn_t>(0.5);
                }
                ASSERT(0 <= frac && frac <= 1.0, cerr << double(lfv) << "->" << double(fv) << "(" << double(line) << ")" << endl;);
                // 進行方向基準に回転
                //CERR<<double(tmst.x)<<","<<double(tmst.y)<<endl;
                //rotate( tmst.y, tmst.x, -XYtoT(tmst.vy,tmst.vx), &tmst.y, &tmst.x );
                //CERR<<" -> "<<tmst.x<<","<<tmst.y<<endl;

                //rotate( lmst.y, lmst.x, -XYtoT(lmst.vy,lmst.vx), &lmst.y, &lmst.x );

                FV_FAllTable[i].set(
                    -lmst.x * ((pfpn_t)1 - frac) + -tmst.x * frac,
                    -lmst.y * ((pfpn_t)1 - frac) + -tmst.y * frac,
                    XYtoT(lmst.vy(), lmst.vx()) * ((pfpn_t)1 - frac) + XYtoT(tmst.vy(), tmst.vx()) * frac,
                    //(tm - step)*((pfpn_t)1 - frac) + tm*frac
                    XYtoR(lmst.y, lmst.x) * ((pfpn_t)1 - frac) + XYtoR(tmst.y, tmst.x) * frac
                    );
            }
        }
        
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
            
            thread_t th2(initALL);
            thread_t th4(initR_CZ);
            
            th2.join();
            th4.join();
        }
        
        /**************************物理系性質演算*************************/
        
        /**************************石のデータのステップ関数*************************/

        // 衝突した瞬間に、今後衝突しなかった場合に停止する位置と時刻を計算しておくか否かで
        // 2種類の関数を用意
        
        template<class mst_t>
        void stepByNextT(mst_t *const dst, fpn_t fv, fpn_t fnt){
            // 前計算データから石の位置をステップさせる
            fpn_t
                fdt(fnt - dst->t),
                fnv(fv - FDTtoFDV(fdt)),
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
                dst->th + dtheta,
                &dst->y, &dst->x); // 位置を進める
            dst->setTh(dst->th() + dvtheta); // 向き設定
            dst->setV(fnv);
            dst->t = fnt; // 時刻設定
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

        template<>
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
        }

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

        template<>
        void stepToStop<FSMovingStone>(FSMovingStone *const dst, const fpn_t fv){
            // 前計算データから石の位置をステップさせる
            // 衝突せずに静止するまで動かす(停止時情報あり)
            dst->x = dst->gx; dst->y = dst->gy;
            dst->t = dst->gt;
        }
        
        /**************************位置<->座標変換*************************/
        
        template<class pos_t, class mv_t>
        void FXYtoFMV(const pos_t& pos, mv_t *const mv){
            // posで静止するVX, VYを計算
            // ドロー系のショット生成の基本関数
            fpn_t dx(pos.getX() - FX_THROW);
            fpn_t dy(pos.getY() - FY_THROW);
            fpn_t r(XYtoR(dx, dy));
            fpn_t dtheta(XYtoT(dy, dx));
            fpn_t v(FRtoFV(r));
            fpn_t dtx, vtheta; // テーブルでの変化量
            if (!mv->isLeftSpin()){
                dtx = +FVtoFX(v);
                vtheta = +FVtoFTh(v);
            }
            else{
                dtx = -FVtoFX(v);
                vtheta = -FVtoFTh(v);
            }
            
            fpn_t dttheta(XYtoT(FVtoFY(v), dtx));
            rotate(v, 0,
                   (vtheta + dtheta - dttheta),
                   &mv->y, &mv->x);
        }
        
        template<class pos_t, class move_t>
        void FMVtoFXY(const move_t& mv, pos_t *const dst){
            // 停止位置を計算
            fpn_t v(mv.v()); // 速さ
            fpn_t vtheta(mv.th()); // 投げる点での向き
            fpn_t x(FX_THROW), y(FY_THROW); // 投げる点の座標
            fpn_t dx, dtheta; // テーブル上でのx方向の移動量と回転量
            
            if (!mv.isLeftSpin()){
                dx = +FVtoFX(v);
                dtheta = +FVtoFTh(v);
            }
            else{
                dx = -FVtoFX(v);
                dtheta = -FVtoFTh(v);
            }
            // 移動分を回転させてて現在の位置座標に足す
            rotateToAdd(FVtoFY(v), dx,
                        vtheta - dtheta,
                        &y, &x);
            dst->set(x, y);
        }
        
        template<class pos_t>
        void FV_FRtoFXY(fpn_t fv, fpn_t r, pos_t *const pos, fpn_t *const theta, Spin spin){
            // 初速vのショットが投擲点から距離rの点での相対位置を求める
            // もしrまで到達しないならば、適当な値が帰るので注意
            // ヒット系のショットの生成の基本関数
            assert(r >= 0);
            assert(0 <= fv && fv < FV_SIMU_MAX);
            //const fpn_t r2 = r*r;
            int id = FVtoID(fv);
            fpn_t gx, gy, x, y;
            if (isRight(spin)){
                *theta = FV_FAllTable[id].th;
            }
            else{
                *theta = -FV_FAllTable[id].th;
            }
            gx = FV_FAllTable[id].x;
            gy = FV_FAllTable[id].y;

            // 到達点から距離 fr - r の点を探す
            fpn_t fr2 = XYtoR2(gx, gy);
            const fpn_t r2Line = (r + FR_COLLISION) * (r + FR_COLLISION);
            if (fr2 > r2Line){
                fpn_t fr = sqrt(fr2);

                fpn_t tr = fr - r;

                //cerr<<"fr = "<<fr<<" tr = "<<tr<<endl;
                fpn_t fv = FRtoFV(tr);
                id = FVtoID(fv);

                while (1){
                    // この点のところまで位置を移動
                    x = gx - FV_FAllTable[id].x;
                    y = gy - FV_FAllTable[id].y;
                    fr2 = XYtoR2(x, y);

                    //cerr<<tr<<endl;

                    if (fr2 <= r2Line){
                        //ASSERT(fr>=r,cerr<<"r = "<<r<<" fr = "<<fr<<endl;);
                        fpn_t frac = FVtoFRAC(fv);
                        fpn_t tx = (gx - ID_FRACtoFX(id, frac));
                        if (isLeft(spin)){ tx = -tx; }
                        pos->set(tx, gy - ID_FRACtoFY(id, frac));
                        break;
                    }
                    fr = sqrt(fr2);
                    int nid;
                    //do{
                    tr += (fr - r);
                    //CERR<<tr<<endl;
                    fv = FRtoFV(tr);
                    nid = FVtoID(fv);
                    //} while (nid == id);
                    if (nid == id){
                        fpn_t frac = FVtoFRAC(fv);
                        fpn_t tx = (gx - ID_FRACtoFX(id, frac));
                        if (isLeft(spin)){ tx = -tx; }
                        pos->set(tx, gy - ID_FRACtoFY(id, frac));
                        break;
                    }
                    id = nid;
                }
                // TODO: 補間してちょうどの位置を返す方が正確

            }
            else{
                if (isLeft(spin)){ gx = -gx; }
                pos->set(gx, gy);
            }


            //CERR<<" ( "<<x<<" , "<<y<<" )"<<endl;
            //cerr<<"r = "<<r<<" last FR = "<<sqrt(fr2)<<endl;
        }
        
        template<class pos_t, class move_t>
        void rotateToPassPointF(move_t *const pmv, const pos_t& pos, fpn_t v){
            // 指定初速で、指定された位置を通る速度ベクトルを生成
            // ヒット系の着手の生成で呼ばれる
            
            const fPosXY<> tar(pos.x - FX_THROW, pos.y - FY_THROW);
            const fpn_t r = XYtoR(tar.getX(), tar.getY());
            
            fPosXY<> tp;
            fpn_t theta;
            
            FV_FRtoFXY(v, r, &tp, &theta, pmv->getSpin());
            
            fpn_t dtheta = XYtoT(tar.y, tar.x) - XYtoT(tp.y, tp.x) + theta;
            rotate(v, 0, dtheta, &pmv->y, &pmv->x);
            
            //CERR << tar << *mv << endl;
        }
        
        template<class pos_t, class move_t>
        void rotateToPassPointGoF(move_t *const pmv, const pos_t& pos, fpn_t r){
            // 指定された位置を通り、さらに指定距離分(投擲点から見て)進む速度ベクトルを生成
            // ヒット系の着手の生成で呼ばれる
            
            const fPosXY<> tar(pos.x - FX_THROW, pos.y - FY_THROW);
            const fpn_t pr = XYtoR(tar.getX(), tar.getY());
            const fpn_t v = FRtoFV(pr + r);
            
            fPosXY<> tp;
            fpn_t theta;
            
            FV_FRtoFXY(v, pr, &tp, &theta, pmv->getSpin());
            
            fpn_t dtheta = XYtoT(tar.y, tar.x) - XYtoT(tp.y, tp.x) + theta;
            
            rotate(v, 0, dtheta, &pmv->y, &pmv->x);
        }
        
        template<class pos_t, class move_t>
        void rotateToPassPointCircumferenceF(move_t *const pmv, const pos_t& pos, fpn_t av, fpn_t atheta, fpn_t ar = 2 * FR_STONE_RAD){
            // 指定初速で、指定された位置の石からの相対位置 ar, atheta の位置を通る速度ベクトルを生成
            // 基本は ar = 2 × 石半径、つまり石のどの点をたたくかという使い方
            rotateToPassPointF(pmv, fPosXY<>(pos.x + ar*sin(atheta), pos.y + ar*cos(atheta)), av);
        }
    }
}

#endif // DCURLING_SIMULATION_FASTSIMULATORFUNC_HPP_