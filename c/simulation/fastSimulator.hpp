/*
 fastSimulator.hpp
 Katsuki Ohto
 */

// Digital Curling
// Fast Simulation using array ( floating point number calculation )

#include "../dc.hpp"
#include "simuFunc.hpp"
#include "fastSimulator.h"
#include "fastSimulatorFunc.hpp"
#include "collision.hpp"
#include "error.hpp"

#ifndef DCURLING_SIMULATION_FASTSIMULATOR_HPP_
#define DCURLING_SIMULATION_FASTSIMULATOR_HPP_

namespace DigitalCurling{
    namespace FastSimulator{
        
        template<class board_t>
        void FastSimulatorBoard::setAfterFirstColl(const board_t& bd, uint32_t awNum, uint32_t asCollidedNum){
            // 最初の衝突後にデータをセット
            turn = awNum;
            
            awake.reset();
            awake.set(0); awake.set(1);
            NAwake = 2;
            
            // set asleep stones
            asleep = 0U;
            int cnt = NAwake;
            iterateStoneWithIndexWithoutFirstCheckExcept(bd, asCollidedNum, [this, &bd, asCollidedNum, &cnt](uint32_t idx, const auto& st)->void{
                asleep.set(cnt);
                
                stone_[cnt].init(bd.state(idx));
                stone_[cnt].x = st.x; stone_[cnt].y = st.y;
                
                DERR << st << endl;
                ++cnt;
            });
            
            NAsleep = cnt - NAwake;
            
            for(int i = 0; i < 2; ++i){
                stone(i).asleep.reset();
                iterate(asleep, [this, i](int idx)->void{
                    if(hasCollisionChanceAwAs(stone(i), stone(idx))){
                        stone(i).asleep.set(idx);
                    }
                });
                stone(i).awake.reset();
            }
            
            // 石の付加情報
            stone(0).state.setIndex(awNum);
            stone(1).state = bd.state(asCollidedNum);
            
            // 動いている石を衝突可能時刻順に並べる
            /*awakeArray.clear();
            if(stone(0).minCollisionTime < stone(1).minCollisionTime){
                awakeArray.set(0, 0); awakeArray.set(1, 1);
            }else{
                awakeArray.set(0, 1); awakeArray.set(1, 0);
            }*/
        }

        template<int RINK_ONLY = 1, class fsbd_t, class board_t>
        bool simulateF_b2d_FSB_inContact(fsbd_t *const pbd, board_t *const pabd, const fpn_t step){
            // シミュレータ、box2dへの入り口
            // 自作物理演算が解決できない複雑な衝突が起こった場合に呼ぶ
            // コンタクトが無くなるまで続け、それ以降元の物理演算に戻る

            bool foul = false;
            b2dSimulator::b2IdentityBoard b2bd;

            // ゲーム情報の初期化
            // awakeな石をセット
            iterate(pbd->awake, [&b2bd, pbd](int idx)->void{
                b2bd.setAwakeStone(idx, pbd->stone(idx), pbd->stone(idx).state);
            });
            // asleepな石をセット
            iterate(pbd->asleep, [&b2bd, pbd](int idx)->void{
                b2bd.setAsleepStone(idx, pbd->stone(idx), pbd->stone(idx).state);
            });
            
            DERR << "Before b2d : " << pbd->awake.count() << "," << pbd->asleep.count() << endl;

            // 物理演算
            int ret = b2dSimulator::loopInContact(&b2bd, step, FR_COLLISION);
            fpn_t newTime = pbd->stone(pbd->awake.bsf()).t + step * ret;

            // awake, asleepがどうなったかを処理
            BitSet32 newAwake(0), newAsleep(0), removed(0);
            // awakeだった石のデータを更新
            iterate(pbd->awake, [&b2bd, &newAwake, &newAsleep, &removed, pbd, pabd, newTime, &foul](uint32_t idx)->void{
                b2Vec2 v = b2bd.stone(idx).vel();
                b2Vec2 p = b2bd.stone(idx).pos();
                
                //assert(v.x == 0 && v.y == 0 && (RINK_ONLY && !isInPlayArea(p.x, p.y))); ありうる
            
                if (v.x == 0 && v.y == 0){ // awake -> asleep
                    if(RINK_ONLY && !isInPlayArea(p.x, p.y)){ // removed
                        if(isFreeGuardTurn(pbd->turn)
                        && !isSameColorIndex(pbd->turn, idx)
                        && isInFreeGuardZone(pabd->stone(idx))){ // フリーガード違反
                            foul = true;
                        }
                        removed.set(idx);
                        //pbd->stone(idx).set(p.x, p.y, 0.00001, -M_PI, b2bd.stone(idx).w(), newTime);
                    }else{
                        newAsleep.set(idx);
                        pbd->stone(idx).x = p.x; pbd->stone(idx).y = p.y;
                        pbd->stone(idx).t = newTime;
                    }
                }else{ // awake -> awake
                    fpn_t w = b2bd.stone(idx).w();
                    pbd->stone(idx).set(p.x, p.y, XYtoR(v.x, v.y), XYtoT(v.y, v.x), w, newTime);
                }
            });
            // asleepだった石のデータを更新
            iterate(pbd->asleep, [&b2bd, &newAwake, &newAsleep, &removed, pbd, pabd, newTime, &foul](int idx)->void{
                b2Vec2 v = b2bd.stone(idx).vel();
                b2Vec2 p = b2bd.stone(idx).pos();
                
                //assert(v.x == 0 && v.y == 0 && (RINK_ONLY && !isInPlayArea(p.x, p.y))); ありうる
                
                if (v.x == 0 && v.y == 0){ // asleep -> asleep
                    if(fabs(p.x - pbd->stone(idx).x) > 0.000001
                       || fabs(p.y - pbd->stone(idx).y) > 0.000001){ // moved
                        if(RINK_ONLY && !isInPlayArea(p.x, p.y)){ // removed
                            if(isFreeGuardTurn(pbd->turn)
                               && !isSameColorIndex(pbd->turn, idx)
                               && isInFreeGuardZone(pabd->stone(idx))){ // フリーガード違反
                                foul = true;
                            }
                            removed.set(idx);
                            //newAwake.set(idx);
                            //pbd->stone(idx).set(p.x, p.y, 0.00001, -M_PI, b2bd.stone(idx).w(), newTime);
                        }else{
                            newAsleep.set(idx);
                            pbd->stone(idx).x = p.x; pbd->stone(idx).y = p.y;
                        }
                    }
                    pbd->stone(idx).t = newTime;
                }else{ // asleep -> awake
                    newAwake.set(idx);
                    fpn_t w = b2bd.stone(idx).w();
                    pbd->stone(idx).set(p.x, p.y, XYtoR(v.x, v.y), XYtoT(v.y, v.x), w, newTime);
                }
            });
            pbd->awake = (pbd->awake | newAwake) & (~newAsleep) & (~removed);
            pbd->asleep = (pbd->asleep | newAsleep) & (~newAwake) & (~removed);
            
            pbd->NAwake = pbd->awake.count();
            pbd->NAsleep = pbd->asleep.count();
            
            DERR << "After b2d : " << pbd->awake.count() << "," << pbd->asleep.count() << " ( " <<ret << " count)" << endl;
            
            // newAwake, newAsleepによって衝突可能性石を差分更新
            iterate(pbd->awake, [pbd](int idx)->void{
                /*iterateExcept(newAwake,id,[pbd](int id)->void{
                    
                });
                iterate(newAsleep,[pbd](int id)->void{
                    
                });*/
                pbd->stone(idx).awake = pbd->awake ^ BitSet32::mask(idx);
                pbd->stone(idx).asleep = pbd->asleep;
            });
            
            return foul;
        }

        template<int RINK_ONLY = 1, class board_t, class move_t>
        ContactTree simulateSoloF(board_t *const pbd, uint32_t stNum, const move_t& mv){
            // ただ停止位置を計算し、石を置くだけ
            // 元々石がある場合にはおかしなことになりうるので、注意
            fpn_t x, y, r, th;
            //FMVtoFXY(mv,&dst);
            FXYVThtoFXYRTh(&x, &y, &r, &th, FX_THROW, FY_THROW, mv.v(), mv.th(), mv.getSpin());
            if ((!RINK_ONLY) || isInPlayArea(x, y)){
                pbd->locateNewStone(stNum, fPosXY<>(x, y));
                return ContactTree::PUT;
            }
            else{
                return ContactTree::PASS;
            }
        }
        
        template<int RINK_ONLY = 1, class board_t, class mst_t>
        ContactTree loop1_1(board_t *const pbd, uint32_t awNum, mst_t& aw){
            // awake x 1、asleep x 1の場合のみのシミュレーション
            // 静止石分のループがいらないという性質, 衝突したら即座に停止位置計算して終了できるという性質を利用
            ASSERT(countNInPlayArea(*pbd) == 1,
                   cerr << countNInPlayArea(*pbd, BLACK) << "," << countNInPlayArea(*pbd, WHITE) << endl;);
            
            FMoment as;
            ASSERT(pbd->sb().any(),);
            uint32_t asNum = pbd->sb().bsf();
            as.x = pbd->stone(asNum).getX(); as.y = pbd->stone(asNum).getY();
            
            ASSERT(aw.exam(), cerr << awNum << " " << aw.toString() << endl;);
            ASSERT(as.exam(), cerr << asNum << " " << as.toString() << endl;);
            fpn_t r2_hantei, r2_asleep;
            
            r2_asleep = XYtoR2(aw.x - as.x, aw.y - as.y);
            
            for (;;){
                r2_hantei = (aw.gr + 2 * FR_STONE_RAD) * (aw.gr + 2 * FR_STONE_RAD);
                // まず非衝突判定
                if(r2_hantei < r2_asleep){ // 到達しない
                    break;
                }
                if(r2_asleep < pow(FR_COLLISION + 2 * FR_STONE_RAD, 2)){ // 衝突
                    // 衝突処理
                    //CERR<<"collision!"<<endl;
                    
                    collisionAw_As(&aw, &as);
                    ContactTree ct;
                    ct.clear();
                    ct.setContact(0, asNum);
                    
                    //CERR<<" AW "<<aw.x<<","<<aw.y<<","<<aw.v*sin(aw.th)<<","<<aw.v*cos(aw.th)<<","<<aw.w<<endl;
                    //CERR<<" AS "<<as.x<<","<<as.y<<","<<as.v*sin(as.th)<<","<<as.v*cos(as.th)<<","<<as.w<<endl;
                    
                    //1_1では1度衝突したら2度と衝突することはない
                    stepToStop(&aw, aw.v());
                    stepToStop(&as, as.v());
                    
                    if((!RINK_ONLY) || isInPlayArea(as.x, as.y)){
                        pbd->relocateStone(asNum, as);
                    }else{
                        // フリーガード違反判定
                        if (isFreeGuardTurn(awNum)
                            && !isSameColorIndex(awNum, asNum)
                            && isInFreeGuardZone(pbd->stone(asNum))){
                            DERR << "freeguard foul" << endl;
                            ASSERT(ct.isFoul(), cerr << ct << endl;);
                            return ct;
                        }else{
                            pbd->clearStones();
                        }
                    }
                    if ((!RINK_ONLY) || isInPlayArea(aw.x, aw.y)){
                        pbd->locateNewStone(awNum, aw);
                    }
                    ct.setChanged();
                    return ct;
                }
                else{ // 1ステップ進める
                    ASSERT(r2_asleep >= 0, cerr << r2_asleep << endl;);
                    fpn_t r_asleep = sqrt(r2_asleep) - 2 * FR_STONE_RAD;
                    
                    if (aw.gr < r_asleep){
                        break;
                    }
                    fpn_t nr2 = aw.gr * aw.gr + r_asleep * r_asleep - 2.0 * aw.gr * r_asleep * cos(FTHETA_CURL_MAX);
                    ASSERT(nr2 >= 0, cerr << nr2 << endl;);
                    fpn_t nr = sqrt(nr2);
                    //CERR<<aw.x<<","<<aw.y<<","<<aw.v*sin(aw.th)<<","<<aw.v*cos(aw.th)<<","<<aw.t;
                    stepByNextR(&aw, aw.v(), nr);
                    //CERR<<" -> "<<aw.x<<","<<aw.y<<","<<aw.v*sin(aw.th)<<","<<aw.v*cos(aw.th)<<","<<aw.t<<endl;
                    
                    ASSERT(aw.exam(), cerr << awNum << " " << aw.toString() << endl;);
                }
                
                fpn_t r2_asleep_last = r2_asleep;
                r2_asleep = XYtoR2(aw.x - as.x, aw.y - as.y);
                if (r2_asleep > r2_asleep_last + 0.00001){ // 到達しない
                    break;
                }
            }
            stepToStop(&aw, aw.v());
            if ((!RINK_ONLY) || isInPlayArea(aw.x, aw.y)){ // put
                pbd->locateNewStone(awNum, aw);
                return ContactTree::PUT;
            }
            else{
                return ContactTree::PASS;
            }
            UNREACHABLE;
        }
        
        template<int RINK_ONLY = 1, class board_t, class fsbd_t>
        ContactTree loopAfterFirstCollision(board_t *const pabd, fsbd_t *const pbd){
            
            // 石が複数個ある場合のシミュレーション(FastSimulatorBoardによる)
            // 最初の衝突が起こった後に呼ばれる
            
            ContactTree ct;
            ct.clear();
            pbd->growContactTreeFirst(&ct, 0, 1);

            int& NAwake = pbd->NAwake;
            int& NAsleep = pbd->NAsleep;
            BitSet32& awake = pbd->awake;
            BitSet32& asleep = pbd->asleep;
            //auto stone = [pbd](int i)->(typename board_t::stone_t)&{ return pbd->stone(i); };

            bool contactPhase = false;
            
            //for (int loop = 0;; ++loop){
            for(;;){
                ASSERT(pbd->awake.count() == NAwake,);
                ASSERT(pbd->asleep.count() == NAsleep,);
                ASSERT(pbd->exam(),);
                //if(loop%100==0 && loop){cerr<<loop<<endl;}
                
                // ここから衝突処理
                // TODO: 衝突が起こるとわかった瞬間に、それ以降の停止時刻計算は意味がなくなる
                
                bool coll = false; // 衝突が起きうるかどうか
                BitArray64<4, 16> NContacts(0); // 衝突数
                BitArray64<16, 4> contactArray(0); // コンタクト組のスタック
                
                fpn_t minCollisionTime = 9999; // 衝突しうる最速の時刻

                // awakeなそれぞれの石に対して、asleepな石に対しての移動可能距離を求める
                //ASSERT(awake.any(),);
                ASSERT(NAwake > 0,);
                iterateWithoutFirstCheck(awake, [pbd, &NContacts, &contactArray, &minCollisionTime, &coll](int iaw)->void{
                    auto& aw = pbd->stone(iaw);
                    
                    ASSERT(0 <= aw.v() && aw.v() <= FV_TABLE_MAX, cerr << aw.v() / FV_TABLE_MAX << endl;);
                    // asleep_stones
                    const fpn_t gr = aw.gr;
                    const fpn_t r2_hantei = (gr + 2 * FR_STONE_RAD) * (gr + 2 * FR_STONE_RAD);
                    fpn_t r2_asleep = 9999;
                    
                    iterate(aw.asleep, [pbd, &NContacts, &contactArray, &aw, iaw, &r2_asleep, r2_hantei](int ias)->void{
                        auto& as = pbd->stone(ias);
                        fpn_t r2 = XYtoR2(as.x - aw.x, as.y - aw.y);
                        if(r2 < r2_hantei){
                            if(r2 < pow(FR_COLLISION + 2 * FR_STONE_RAD, 2)){
                                // 動-静衝突
                                DERR << "collision AW_AS " << endl;
                                NContacts.plus(iaw, 1);
                                NContacts.plus(ias, 1);
                                contactArray.insert(0, (ias << 4) | iaw);
                            }
                            if(r2 < r2_asleep){
                                r2_asleep = r2;
                            }
                        }else{
                            // 非衝突が証明された
                            aw.asleep.reset(ias);
                        }
                    });

                    if(r2_asleep < r2_hantei){ // 静止まで動かせない
                        fpn_t r_asleep = sqrt(r2_asleep) - 2 * FR_STONE_RAD;
                        fpn_t nr = sqrt(max(fpn_t(0.0), aw.gr * aw.gr + r_asleep * r_asleep - 2.0 * aw.gr * r_asleep * cos(FTHETA_CURL_MAX)));
                        minCollisionTime = min(minCollisionTime, aw.gt - FRtoFT(nr));

                        coll = true;
                    }

                    // awake_stones
                    if(aw.awake){
                        // 自分以下の番号の石のみ抽出
                        BitSet32 taw = aw.awake & ((1U << iaw) - 1U);
                        
                        for(; taw.any(); taw.pop_lsb()){
                            int last = taw.bsf();
                            auto& aw1 = pbd->stone(last);
                            fpn_t r2 = XYtoR2(aw.x - aw1.x, aw.y - aw1.y);

                            if(r2 < pow(FR_COLLISION + 2 * FR_STONE_RAD, 2)){
                                // 動-動衝突
                                DERR << "collision 2AW" << endl;
                                NContacts.plus(iaw, 1);
                                NContacts.plus(last, 1);
                                contactArray.insert(0, (1 << 8) | (last << 4) | (iaw));
                            }

                            fpn_t r_awake = sqrt(r2) - 2 * FR_STONE_RAD;

                            if(r_awake < aw.gr + aw1.gr){ // 静止まで動かせない

                                // 減速せずに互いに一直線に近づくときの衝突までの時間を計算
                                fpn_t dt = r_awake / (aw.v() + aw1.v());
                                /*
                                fpn_t vSum = aw.v + aw1.v;
                                fpn_t D = vSum*vSum - 4 * F_FRIC_RINK_STONE * r_awake;
                                fpn_t dt1 = 2 * r_awake / (vSum + sqrt(vSum*vSum - 4 * F_FRIC_RINK_STONE * r_awake));
                                
                                ASSERT(D >= 0,
                                    cerr << "aw0 gr = " << aw.gr << " VtoR = " << FVtoFR(aw.v) << " linearR = " << pow(aw.v,2) / F_FRIC_RINK_STONE/2 << endl;
                                cerr << "aw1 gr = " << aw1.gr << " VtoR = " << FVtoFR(aw1.v) << " linearR = " << pow(aw1.v,2) / F_FRIC_RINK_STONE/2 << endl;
                                cerr << vSum << ", " << r_awake << endl;

                                );
                                */
                                //cerr << "dt(old) = " << dt0 << " dt(v2) = " << dt << endl;

                                minCollisionTime = min(minCollisionTime, aw.t + dt);
                                coll = true;
                            }else{ // 非衝突が証明された
                                aw.awake.reset(last);
                                aw1.awake.reset(iaw);
                            }
                        }
                    }
                    
                    /*
                     if(NContacts.any()){
                     iterate(awake,[stone,&NContacts,&contactVector](int iaw)->void{
                     auto& aw = stone(iaw);
                     //asleep_stones
                     iterate(aw.asleep,[stone,&NContacts,&contactVector,&aw,iaw](int ias)->void{
                     auto& as = stone(ias);
                     fpn_t r2 = XYtoR2(as.x - aw.x, as.y - aw.y);
                     if (r2 < pow(FR_COLLISION + 2 * FR_STONE_RAD, 2)){
                     DERR << "collision AW_AS " << endl;
                     NContacts.plus(iaw,1);
                     NContacts.plus(ias,1);
                     contactVector.emplace_back((ias<<4)|(iaw));
                     }
                     //if (r2 < r2_hantei){
                     //}else{
                     //非衝突が証明された
                     //    aw.asleep.reset(ias);
                     // }
                     });
                     //awake_stones
                     if (aw.awake){
                     //自分以下の番号の石のみ抽出
                     BitSet32 taw = aw.awake & ((1U << iaw) - 1U);
                     for (; taw.any(); taw.pop_lsb()){
                     int last = taw.bsf();
                     auto& aw1 = stone(last);
                     fpn_t r2 = XYtoR2(aw.x - aw1.x, aw.y - aw1.y);
                     
                     if (r2 < pow(FR_COLLISION + 2 * FR_STONE_RAD, 2)){
                     //動-動衝突
                     DERR << "collision 2AW" << endl;
                     NContacts.plus(iaw,1);
                     NContacts.plus(last,1);
                     contactVector.emplace_back((1<<8)|(last<<4)|(iaw));
                     }
                     
                     //fpn_t r_awake = sqrt(r2) - 2 * FR_STONE_RAD;
                     //if (r_awake < aw.gr + aw1.gr){//静止まで動かせない
                     //}
                     //else{//非衝突が証明された
                     //    aw.awake.reset(last);
                     //    aw1.awake.reset(iaw);
                     //}
                     }
                     }
                     });
                     }
                     */
                });
                
                // １つコンタクトが発見された際のそれ以降の処理
                if(!coll){
                    // 全ての石が衝突せずに静止する事が証明されたので、全ての石を置いて終わり
                    DERR << "no collision" << endl;
                    if(0 <= search(awake, [pbd, pabd, &asleep](int iaw)->bool{
                        auto& aw = pbd->stone(iaw);
                        stepToStop(&aw, aw.v());
                        if ((!RINK_ONLY) || isInPlayArea(aw.x, aw.y)){ // エリア内
                            asleep.set(iaw);
                        }else{ // エリア外
                            if(isFreeGuardTurn(pbd->turn)
                                && !isSameColorIndex(pbd->turn, aw.state.getIndex())
                                && isInFreeGuardZone(pabd->stone(aw.state.getIndex()))){ // フリーガード違反
                                DERR << "freeguard foul" << endl;
                                return true;
                            }
                        }
                        return false;
                    })){ // フリーガード違反
                        ASSERT(ct.isFoul(), cerr << ct << endl;);
                        return ct;
                    }
                    goto END;
                }
            SOLVE_CONTACT:
                if(NContacts.any()){ // コンタクトあり
                    if(contactPhase || (NContacts.data() & (~0x1111111111111111ULL))){
                        // 連続コンタクト(無限回コンタクトしうる)または重コンタクトのときはBox2Dに投げる
                        //cerr << NContacts << endl;
                        ct.setComplex();
                        bool foul = simulateF_b2d_FSB_inContact<RINK_ONLY>(pbd, pabd, BOX2D_TIMESTEP);
                        if(foul){
                            DERR << "freeguard foul" << endl;
                            ASSERT(ct.isFoul(), cerr << ct << endl;);
                            return ct;
                        }
                        if(!NAwake){ break; }
                        continue;
                    }else{ // 単コンタクトのみ
                        // コンタクトの処理
                        ASSERT(contactArray.any(), cerr << contactArray << endl;);
                        iterateAnyWithoutFirstCheck(contactArray, [&](uint32_t value)->void{
                            if(!(value & (1 << 8))){ // aw_as
                                uint32_t iaw = value & 15;
                                uint32_t ias = (value >> 4) & 15;
                                auto& aw = pbd->stone(iaw);
                                auto& as = pbd->stone(ias);
                                
                                collisionAw_As(&aw, &as); // 衝突
                                pbd->growContactTree(&ct, iaw, ias);
                                
                                assert(aw.exam()); assert(as.exam());
                                
                                asleep.reset(ias);
                                
                                //aw.asleep = asleep;
                                //as.asleep = asleep;
                                
                                aw.asleep.reset();
                                iterate(asleep, [pbd, &aw](int idx)->void{
                                    if(hasCollisionChanceAwAs(aw, pbd->stone(idx))){
                                        aw.asleep.set(idx);
                                    }
                                });
                                as.asleep.reset();
                                iterate(asleep, [pbd, &as](int idx)->void{
                                    if(hasCollisionChanceAwAs(as, pbd->stone(idx))){
                                        as.asleep.set(idx);
                                    }
                                });
                                
                                ++NAwake; --NAsleep;
                                
                                // この2つ以外の石の衝突可能石を判定
                                BitSet32 taw = awake;
                                taw.reset(iaw);
                                
                                aw.awake = taw; as.awake = taw;
                                
                                awake.set(ias);
                                
                                ASSERT(pbd->awake.count() == NAwake,);
                                ASSERT(pbd->asleep.count() == NAsleep,);
                                
                                iterate(taw, [pbd, iaw, ias](int idx)->void{
                                    //stone(idx).awake.set(iaw);
                                    pbd->stone(idx).awake.set(ias);
                                    pbd->stone(idx).asleep.reset(ias);
                                });
                            }else{ // aw_aw
                                uint32_t iaw = value & 15;
                                uint32_t iaw1 = (value >> 4) & 15;
                                auto& aw = pbd->stone(iaw);
                                auto& aw1 = pbd->stone(iaw1);
                                
                                collision2Aw(&aw, &aw1); // 衝突
                                pbd->growContactTree(&ct, iaw, iaw1);
                                
                                assert(aw.exam()); assert(aw1.exam());
                                
                                aw.asleep.reset();
                                iterate(asleep, [pbd, &aw](int idx)->void{
                                    if(hasCollisionChanceAwAs(aw, pbd->stone(idx))){
                                        aw.asleep.set(idx);
                                    }
                                });
                                aw1.asleep.reset();
                                iterate(asleep, [pbd, &aw1](int idx)->void{
                                    if(hasCollisionChanceAwAs(aw1, pbd->stone(idx))){
                                        aw1.asleep.set(idx);
                                    }
                                });
                                
                                // 動-動関係を整理
                                if (NAwake > 2){
                                    BitSet32 taw = awake;
                                    taw.reset(iaw); taw.reset(iaw1);
                                    
                                    aw.awake = taw;
                                    aw1.awake = taw;
                                    
                                    /*
                                    iterate(taw,[stone,iaw,iaw1](int idx)->void{
                                        stone(idx).awake.set(iaw);
                                        stone(idx).awake.set(iaw1);
                                    });
                                     */
                                }
                                else{
                                    aw.awake.reset();
                                    aw1.awake.reset();
                                }
                                
                                ASSERT(pbd->awake.count() == NAwake,);
                                ASSERT(pbd->asleep.count() == NAsleep,);
                            }
                        });
                        assert(pbd->exam());
                        contactPhase = true; // 衝突直後であることを記録
                        continue; // 演算ループの最初に戻る
                    }
                }else{ // no contacts
                    contactPhase = false;
                }

                DERR << "Min Time : " << minCollisionTime << endl;

                // それぞれの石を移動可能な分動かす
                {
                    for (BitSet32 taw = awake; taw.any(); taw.pop_lsb()){
                        int iaw = taw.bsf();
                        auto& aw = pbd->stone(iaw);

                        DERR << "(" << iaw << ") " << aw.toString();

                        if (aw.gt < minCollisionTime){

                            // 最短衝突時刻より停止時刻が早い石は静止させる
                            stepToStop(&aw, aw.v());

                            DERR << " -> " << aw.toString() << " stop" << endl;

                            awake.reset(iaw);
                            --NAwake;
                            
                            ASSERT(pbd->awake.count() == NAwake,);
                            ASSERT(pbd->asleep.count() == NAsleep,);

                            if ((!RINK_ONLY) || isInPlayArea(aw.x, aw.y)){
                                asleep.set(iaw);
                                ++NAsleep;
                                
                                ASSERT(pbd->awake.count() == NAwake,);
                                ASSERT(pbd->asleep.count() == NAsleep,);
                                
                                // asleep stones for each awake stones
                                iterate(awake, [pbd, iaw, &aw](int idx)->void{
                                    pbd->stone(idx).awake.reset(iaw);
                                    //if(hasCollisionChanceAwAs(stone(idx),aw)){
                                    //    stone(idx).asleep.set(iaw);
                                    //}
                                    pbd->stone(idx).asleep.set(iaw);
                                });
                            }
                            else{ // エリア外
                                if (isFreeGuardTurn(pbd->turn)
                                    && !isSameColorIndex(pbd->turn, aw.state.getIndex())
                                    && isInFreeGuardZone(pabd->stone(aw.state.getIndex()))){ // フリーガード違反
                                    DERR << "freeguard foul" << endl;
                                    ASSERT(ct.isFoul(), cerr << ct << endl;);
                                    return ct;
                                }
                                iterate(awake, [pbd, iaw, &aw](int idx)->void{
                                    pbd->stone(idx).awake.reset(iaw);
                                });
                            }
                        }
                        else{ // 静止しない
                            stepByNextT(&aw, aw.v(), minCollisionTime);
                            assert(aw.exam());

                            DERR << " -> " << aw.toString() << endl;
                            
                            /*if (RINK_ONLY && !isInPlayArea(aw.x, aw.y)){ // エリア外
                             if (isFreeGuardTurn(pbd->turn)
                             && !isSameColorIndex(pbd->turn, aw.state.getIndex())
                             && isInFreeGuardZone(pabd->stone(aw.state.getIndex()))){ // フリーガード違反
                             DERR << "freeguard foul" << endl;
                             return ct;
                             }
                             awake.reset(iaw);
                             --NAwake;
                             
                             ASSERT(pbd->awake.count() == NAwake,);
                             ASSERT(pbd->asleep.count() == NAsleep,);
                             
                             iterate(awake, [pbd, iaw, &aw](int idx)->void{
                             pbd->stone(idx).awake.reset(iaw);
                             });
                             }*/
                        }
                    }
                }
                if (NAwake == 0){ // 動石がなくなったので終了
                    ASSERT(!awake.any(),);
                    break;
                }else{
                    ASSERT(awake.any(),);
                }
            }
        END:
            // プレイアウト用の局面構造体に石を置く
            pabd->clearStones();
            for (BitSet32 tmp = asleep; tmp.any(); tmp.pop_lsb()){
                uint32_t id = tmp.bsf();
                DERR << id << " ";
                pabd->locateNewStone(pbd->stone(id).state.getIndex(), pbd->stone(id));
            }
            DERR << endl;

            ct.setChanged();
            return ct;
        }

//        template<int RINK_ONLY = 1, class board_t, class movingStone_t, class simulationInfo_t>
//        ContactTree loop1_N(board_t *const pbd, uint32_t awNum, movingStone_t& aw,
//                            simulationInfo_t *const presult){
        template<int RINK_ONLY = 1, class board_t, class movingStone_t>
        ContactTree loop1_N(board_t *const pbd, uint32_t awNum, movingStone_t& aw){
        
            // 静止石が複数ある場合に、最初の衝突が起こるまでのループ

            if (aw.gr > 10000){ DERR << "bad rgoal" << endl; }

            // ヒットゾーンでのシミュレーション
            struct AsInfo{
                int i;
                fpn_t r2_last;
            };

            std::array<AsInfo, N_STONES> asleepList;

            fpn_t r2_hantei = (aw.gr + 2 * FR_STONE_RAD) * (aw.gr + 2 * FR_STONE_RAD);
            fpn_t r2_asleep = 9999;
            int as_near;

            // asleepList作成
            //fpn_t dvtheta = (w > 0) ? (+FTHETA_CURL_MAX / 2) : (-FTHETA_CURL_MAX / 2);
            //fpn_t vtheta_ex = v_theta + dvtheta;
            int NAsleep = 0;

            fpn_t thl, thr;
            if (aw.w > 0){
                thl = aw.th(); thr = aw.gth;
            }
            else{
                thl = aw.gth; thr = aw.th();
            }
            fpn_t cosL = cos(thl); fpn_t sinL = sin(thl);
            fpn_t cosR = cos(thr); fpn_t sinR = sin(thr);

            iterateStoneWithIndexWithoutFirstCheck(*pbd,
                [r2_hantei, &asleepList, &NAsleep, &as_near, &aw, &r2_asleep
                , cosL, cosR, sinL, sinR
                ](uint32_t idx, const auto& st)
                ->void{
                // 衝突可能性判定
                fpn_t dx = st.getX() - aw.x;
                fpn_t dy = st.getY() - aw.y;
                fpn_t r2 = XYtoR2(dx, dy);
                if (r2 < r2_hantei){// R Judge
                    //cerr<<(dx*cos(thl) - dy*sin(thl))<<endl;
                    //cerr<<(dx*cos(thr) - dy*sin(thr))<<endl;
                    if (//1
                        ((dx * cosL - dy * sinL >= -2.0 * FR_STONE_RAD)
                        && (dx * cosR - dy * sinR <= +2.0 * FR_STONE_RAD))
                        ){//Side Judge
                        asleepList[NAsleep].i = idx;
                        asleepList[NAsleep].r2_last = r2;
                        ++NAsleep;
                        if (r2 < r2_asleep){
                            r2_asleep = r2;
                            as_near = idx;
                        }
                    }
                }
            });
            //cerr<<NAsleep<<endl;
            //uint32_t id = FVtoID(aw.v);

            for (;;){
                if (r2_asleep > r2_hantei){ // 到達しない
                    stepToStop(&aw, aw.v());
                    if ((!RINK_ONLY) || isInPlayArea(aw.x, aw.y)){ // put
                        pbd->locateNewStone(awNum, aw);
                        return ContactTree::PUT;
                    }
                    else{ // pass
                        return ContactTree::PASS;
                    }
                }
                else if (r2_asleep < pow(FR_COLLISION + 2 * FR_STONE_RAD, 2)){ // 衝突
                    
                    // 物理演算用の盤面情報クラスに情報をセットする
                    // as_near はこの時点では、衝突された静止石を意味している
                    FastSimulatorBoard fsb;
                    fsb.stone(0) = aw;
                    fsb.stone(1).x = pbd->stone(as_near).x;
                    fsb.stone(1).y = pbd->stone(as_near).y;
                    collisionAw_As(&fsb.stone(0), &fsb.stone(1));
                    assert(fsb.stone(0).exam()); assert(fsb.stone(1).exam());
                    
                    fsb.setAfterFirstColl(*pbd, awNum, as_near); // ここで残りの石の情報をセット
                    return loopAfterFirstCollision<RINK_ONLY>(pbd, &fsb);
                }
                else{ // 1ステップ進める
                    fpn_t r_asleep = sqrt(r2_asleep) - 2 * FR_STONE_RAD;
                    fpn_t nr = sqrt(max(fpn_t(0.0), aw.gr * aw.gr + r_asleep * r_asleep - 2.0 * aw.gr * r_asleep * cos(FTHETA_CURL_MAX)));
                    stepByNextR(&aw, aw.v(), nr);
                    assert(aw.exam());
                }
                // 最も近距離の石を探す
                r2_hantei = (aw.gr + 2 * FR_STONE_RAD) * (aw.gr + 2 * FR_STONE_RAD);
                r2_asleep = 9999;
                //vtheta_ex = aw.th + dvtheta;
                for (int i = NAsleep - 1; i >= 0; --i){
                    fpn_t dx = pbd->stone(asleepList[i].i).x - aw.x;
                    fpn_t dy = pbd->stone(asleepList[i].i).y - aw.y;
                    fpn_t r2 = XYtoR2(dx, dy);
                    if (r2<r2_hantei && r2 < asleepList[i].r2_last + 0.00001){

                        //fpn_t theta = XYtoT(dy, dx);
                        //if( fabs( theta-vtheta_ex ) <= FTHETA_CURL_MAX/2 ){
                        // 衝突可能性あり
                        //ASSERT( !((sqrt(r2)-2*FR_STONE_RAD < FR_COLLISION) && (r2>=asleepList[i].r2_last)),
                        //cerr<<r2<<" , "<<asleepList[i].r2_last<<endl;);

                        asleepList[i].r2_last = r2;
                        if (r2 < r2_asleep){
                            r2_asleep = r2;
                            as_near = asleepList[i].i;
                        }
                        continue;
                        //}
                    }
                    // 非衝突が証明された
                    asleepList[i] = asleepList[--NAsleep]; // リストを詰める
                }
            }
            UNREACHABLE;
        }

//        template<int RINK_ONLY = 1, class board_t, class move_t, class simulationInfo_t>
//        void simulateF(board_t *const pbd, int stoneNum, const move_t& mv,
//                       simulationInfo_t *const presult){
        template<int RINK_ONLY = 1, class board_t, class move_t>
        ContactTree simulateF(board_t *const pbd, uint32_t stoneNum, const move_t& mv){
        
            // 高速シミュレータ入口
            ASSERT(isValidMove(mv), cerr << "invalid Move! " << mv << endl;); // validity
            
            ContactTree ret;

            // 石の個数によって関数を替える
            if (countNInPlayArea(*pbd) == 0){
                ret = simulateSoloF<RINK_ONLY>(pbd, stoneNum, mv);
            }else{
                FSMovingStone st;
                st.set(FX_THROW, FY_THROW, min(FV_SIMU_MAX, (fpn_t)mv.v()), mv.th(), SpintoThrowFW(mv.getSpin()), 0);
                
                if(countNInPlayArea(*pbd) == 1){
                    ret = loop1_1<RINK_ONLY>(pbd, stoneNum, st);
                }else{
                    ret = loop1_N<RINK_ONLY>(pbd, stoneNum, st);
                }
            }
            
            // RINK_ONLYの場合に全てPA内にあるか確認
            if(RINK_ONLY){
                ASSERT(searchStone(*pbd, [](const auto& st)->bool{
                    return !isInPlayArea(st);
                }) < 0, int outIndex = searchStone(*pbd, [](const auto& st)->bool{ return !isInPlayArea(st); });
                       cerr << ret << endl << outIndex << " " << pbd->stone(outIndex) << endl << *pbd << endl;);
            }
            return ret;
        }
    }
    
    template<int RINK_ONLY, class board_t, class move_t>
    ContactTree makeMoveNoRand(board_t *const pbd, uint32_t idx, const move_t& mv){
        // 石を投げて、盤面を更新する
       return FastSimulator::simulateF<RINK_ONLY>(pbd, idx, mv);
    }
    
    template<int RINK_ONLY, class board_t, class move_t, class dice_t>
    ContactTree makeMove(board_t *const pbd, uint32_t stNum, const move_t& mv, dice_t *const pdice){
        // 石を投げて、盤面を更新する 外乱あり
        ASSERT(mv.vy() <= FVY_LEGAL_MAX, cerr << "Invalid Move : " << mv.vy()
               << " in " << FVY_LEGAL_MAX << " " << mv << endl;);
        fMoveXY<> tmp;
        addRandToMove(&tmp, mv, pdice); // 乱数をかける
        return makeMoveNoRand<RINK_ONLY>(pbd, stNum, tmp);
    }
    /*
    template<int RINK_ONLY, class board_t, class move_t>
    ContactTree makeMoveNoRand(board_t *const pbd, uint32_t idx, const move_t& mv,
                               movedStones_t *const pmovedStones){
        // 石を投げて、盤面を更新する
        return FastSimulator::simulateF<RINK_ONLY>(pbd, idx, mv, pmovedStones);
    }
    
    template<int RINK_ONLY, class board_t, class move_t, class movedStones_t& class dice_t>
    ContactTree makeMove(board_t *const pbd, uint32_t stNum, const move_t& mv,
                         movedStones_t *const pmovedStones, dice_t *const pdice){
        // 石を投げて、盤面を更新する(連続直交ベクトル)
        // 外乱あり
        ASSERT(mv.vy() <= FVY_LEGAL_MAX, cerr << "Invalid Move : " << mv.vy()
               << " in " << FVY_LEGAL_MAX << " " << mv << endl;);
        fMoveXY<> tmp;
        addRandToMove(&tmp, mv, pdice); // 乱数をかける
        return makeMoveNoRand<RINK_ONLY>(pbd, stNum, tmp);
    }*/
}

#endif // DCURLING_SIMULATION_FASTSIMULATOR_HPP_