/*
 b2dSimulator.hpp
 */

// Digital Curling
// Simulation With Box2D

#ifndef DCURLING_SIMULATION_B2DSIMULATOR_HPP_
#define DCURLING_SIMULATION_B2DSIMULATOR_HPP_

#include <Box2D/Box2D.h>

#include "../dc.hpp"
#include "b2dSimulator.h"
#include "simuFunc.hpp"

namespace DigitalCurling{
    namespace b2dSimulator{
        
        b2Body* createBody(fpn_t x, fpn_t y, b2World *const world){
            //constexpr bool STONEINFO_DYNAMIC=true;
            //constexpr bool STONEINFO_ISBALL=true;
            constexpr uint32_t STONEINFO_CATEGORYBITS = 3;
            constexpr uint32_t STONEINFO_MASKBITS = 3;
            
            b2BodyDef bodyDef;
            
            bodyDef.type = b2_dynamicBody;
            bodyDef.position.Set(x, y); // 位置
            bodyDef.angle = 0; // 角度(円だから関係無し?)
            
            b2Body* pbody = world->CreateBody(&bodyDef); // ボディの生成
            
            // フィクスチャの作成
            b2FixtureDef fixtureDef;
            
            b2CircleShape dynamicBall;
            dynamicBall.m_radius = FR_STONE_RAD;
            
            fixtureDef.shape = &dynamicBall;
            
            fixtureDef.density = F_DENS_STONE; // 密度
            fixtureDef.restitution = F_REST_STONES; // 反発力
            fixtureDef.friction = F_FRIC_RINK_STONE; // 摩擦力
            fixtureDef.filter.categoryBits = STONEINFO_CATEGORYBITS; // ?
            fixtureDef.filter.maskBits = STONEINFO_MASKBITS; // ?
            
            pbody->CreateFixture(&fixtureDef);
            
            return pbody;
        }
        
        void friction(b2Body *const pbody, fpn_t fric){
            fPosXY<fpn_t> ovec(pbody->GetLinearVelocity().x, pbody->GetLinearVelocity().y),vec;
            fpn_t ow = pbody->GetAngularVelocity();
            fpn_t w;
            
            Simulator::FrictionStep<fpn_t,fPosXY<fpn_t>>(fric, ovec, ow, &vec, &w );
            pbody->SetLinearVelocity(b2Vec2(vec.x, vec.y));
            pbody->SetAngularVelocity(w);
        }
        
        void frictionAll(b2Board *const pbd, fpn_t fric){
            // 全てのストーンについて、摩擦を考慮して速度ベクトルの値を調整する
            // frictionは速度の変化量
            for(int i = 0; i < pbd->NAwake; ++i){
                assert(pbd->stone(i).pbody != nullptr); // 除外物体は含めない
                friction(pbd->stone(i).pbody, fric);
            }
        }
        
        void frictionAll(b2IdentityBoard *const pbd, fpn_t fric){
            // 全てのストーンについて、摩擦を考慮して速度ベクトルの値を調整する
            // frictionは速度の変化量
            for (int i = 0; i < 16; ++i){
                if (pbd->stone(i).pbody != nullptr){
                    friction(pbd->stone(i).pbody, fric);
                }
            }
        }
        
        void step(b2Board *const pb2bd, const fpn_t timeStep){
            frictionAll(pb2bd, F_FRIC_RINK_STONE * timeStep * 0.5);
            pb2bd->world.Step(timeStep, BOX2D_VELOCITY_ITERATIONS, BOX2D_POSITION_ITERATIONS);
            frictionAll(pb2bd, F_FRIC_RINK_STONE * timeStep * 0.5);
        }
        
        template<int RINK_ONLY=1, class board_t> // リンク外も考慮する
        int loop(const board_t& orgBoard, b2Board *const pb2bd, const fpn_t timeStep, const int LoopCount){
            int iRet;
            
            frictionAll(pb2bd, F_FRIC_RINK_STONE * timeStep * 0.5); // 最初に0.5ステップ分摩擦を掛ける
            
            for(iRet = 0; iRet < LoopCount; iRet++){
                ASSERT(pb2bd->exam(),);
                
                // 摩擦力の計算
                pb2bd->world.Step(timeStep, BOX2D_VELOCITY_ITERATIONS, BOX2D_POSITION_ITERATIONS);
                
                // ストーンの除外判定
                // 動いている石にのみ判定
                for(int i = 0; i < pb2bd->NAwake;){
                    assert(pb2bd->stone(i).pbody != nullptr);
                    // ストーンがリンク外に出たら除外する
                    b2Vec2 pos = pb2bd->stone(i).pos();
                    if(RINK_ONLY && !isOnRink(pos.x, pos.y)){//リンク内にない
                        // フリーガードゾーンルール適用ターンの時、フリーガード違反を判定
                        if(isFreeGuardTurn(pb2bd->turn)
                           && !isSameColorIndex(pb2bd->turn, pb2bd->stone(i).index)
                           && isInFreeGuardZone(orgBoard.stone(pb2bd->stone(i).index))){
                            // フリーガード違反なので、シミュレーション結果を破棄
                            CERR << "freeguard foul" << endl;
                            return 0;
                        }
                        
                        pb2bd->world.DestroyBody(pb2bd->stone(i).pbody);
                        --pb2bd->NAwake;
                        // 無くなった分の石を詰める
                        if(i == pb2bd->NAwake){
                            // インデックス最後の石が消えた
                            if(pb2bd->NAsleep != 0){
                                pb2bd->stone(i) = pb2bd->stone(i + pb2bd->NAsleep);
                            }
                        }else{
                            // インデックス最後でない石が消えたので、2手間かけて詰める必要あり
                            pb2bd->stone(i) = pb2bd->stone(pb2bd->NAwake);
                            if(pb2bd->NAsleep != 0){
                                pb2bd->stone(pb2bd->NAwake) = pb2bd->stone(pb2bd->NAwake + pb2bd->NAsleep);
                            }
                        }
                    }else{
                        b2Vec2 v = pb2bd->stone(i).vel();
                        if(!v.x && !v.y){
                            // 動いていた石が静止した
                            --pb2bd->NAwake;
                            ++pb2bd->NAsleep;
                            if(i != pb2bd->NAwake){
                                // インデックス最後でない石が静止した
                                std::swap(pb2bd->stone(i), pb2bd->stone(pb2bd->NAwake));
                            }
                        }else{
                            // 次のインデックスの石へ
                            ++i;
                        }
                    }
                }
                // 止まっている石が動き出したか判定
                if(pb2bd->NAsleep != 0){
                    // インデックス最初のもの
                    b2Vec2 v = pb2bd->stone(pb2bd->NAwake).vel();
                    while(v.x || v.y){
                        DERR << "remove" << endl;
                        //CERR << pb2bd->stone(pb2bd->NAwake).w() << endl;
                        ++pb2bd->NAwake;
                        --pb2bd->NAsleep;
                        if(pb2bd->NAsleep == 0){goto TO_NEXT;}
                    }
                    // インデックス最初でないもの
                    const int size = pb2bd->NAwake + pb2bd->NAsleep;
                    for(int i = pb2bd->NAwake + 1; i < size; ++i){
                        b2Vec2 v = pb2bd->stone(i).vel();
                        if(v.x || v.y){
                            DERR << "remove" << endl;
                            std::swap(pb2bd->stone(i), pb2bd->stone(pb2bd->NAwake));
                            ++pb2bd->NAwake;
                            --pb2bd->NAsleep;
                        }
                    }
                }
                if(pb2bd->NAwake == 0){
                    // 動いている石がなくなった
                    //CERR<<"all stones were stopped."<<endl;
                    //getchar();
                    goto END;
                }
            TO_NEXT:;
                frictionAll(pb2bd, F_FRIC_RINK_STONE * timeStep); // 動摩擦係数×重力加速度×1ステップの時間の摩擦を掛ける
            }
        END:
            return iRet;
        }
        
        int loopInContact(b2IdentityBoard *const pb2bd, const fpn_t timeStep, const fpn_t collisionLine, const int LoopCount)
        {
            // 石同士のコンタクトが無くなるまでのループ
            
            // ここからループ内での関数の定義
            const fpn_t d2 = (FR_STONE_RAD * 2 + collisionLine) * (FR_STONE_RAD * 2 + collisionLine);
            
            //cerr << d2 << endl;
            
            auto hasContact = [d2](b2IdentityBoard *const pbd)->bool{
                for (int i = 0; i < N_STONES; ++i){
                    if (pbd->stone(i).pbody != nullptr){
                        b2Vec2 vi = pbd->stone(i).vel();
                        if (vi.x || vi.y){ // iが動いている
                            fpn_t xi = pbd->stone(i).pos().x; fpn_t yi = pbd->stone(i).pos().y;
                            for (int j = 0; j < i; ++j){
                                if (pbd->stone(j).pbody != nullptr){
                                    b2Vec2 vj = pbd->stone(j).vel();
                                    if (!(vj.x || vj.y)){ // jが静止
                                        fpn_t xj = pbd->stone(j).pos().x; fpn_t yj = pbd->stone(j).pos().y;
                                        
                                        //cerr << "d = " << XYtoR2(xi - xj, yi - yj) << endl;
                                        
                                        if (XYtoR2(xi - xj, yi - yj) < d2){
                                            // 高速シミュレータでコンタクトと判定される範囲
                                            return true;
                                        }
                                    }
                                }
                            }
                            for (int j = i + 1; j < N_STONES; ++j){
                                if (pbd->stone(j).pbody != nullptr){
                                    fpn_t xj = pbd->stone(j).pos().x; fpn_t yj = pbd->stone(j).pos().y;
                                    
                                    //cerr << "d = " << XYtoR2(xi - xj, yi - yj) << endl;
                                    
                                    if (XYtoR2(xi - xj, yi - yj) < d2){
                                        // 高速シミュレータでコンタクトと判定される範囲
                                        return true;
                                    }
                                }
                            }
                        }
                    }
                }
                return false;
            };
            
            // ここから関数の動作
            int iRet;
            //assert(hasContact(pb2bd));
            frictionAll(pb2bd, F_FRIC_RINK_STONE * timeStep * 0.5); // 最初に0.5ステップ分摩擦を掛ける
            for (iRet = 0; iRet < LoopCount; iRet++){
                // 摩擦力の計算
                pb2bd->world.Step(timeStep, BOX2D_VELOCITY_ITERATIONS, BOX2D_POSITION_ITERATIONS);
                
                if(!hasContact(pb2bd)){ break; }
                
                frictionAll(pb2bd, F_FRIC_RINK_STONE * timeStep); // 動摩擦係数×重力加速度×1ステップの時間の摩擦を掛ける
            }
            frictionAll(pb2bd, F_FRIC_RINK_STONE * timeStep * 0.5); // 最後に0.5ステップ分摩擦を掛ける
            
            //cerr << "iRet = " << iRet << endl;
            
            return iRet;
        }
        
        // シミュレーション本体
        template<int RINK_ONLY, class board_t, class mst_t>
        int simulateb2d(board_t *const pbd, uint32_t stNum, const mst_t& org, const fpn_t step){
            
            b2Board b2bd;
            
            // ゲーム情報の初期化
            b2bd.setAwakeStone(stNum, org);
            b2bd.setAsleepStones<RINK_ONLY>(*pbd);
            
            // 物理演算
            int ret = loop<RINK_ONLY>(*pbd, &b2bd, step);
            
            // シミュレーション結果をまとめる
            if(ret != 0){
                // 石の配置が変化する可能性が高い
                pbd->clearStones();
                const int size = b2bd.NAwake + b2bd.NAsleep;
                for(int i = 0; i < size; ++i){
                    assert(b2bd.stone(i).pbody != nullptr);
                    const b2Vec2 pos = b2bd.stone(i).pos();
                    
                    if((!RINK_ONLY) || isInPlayArea(pos.x, pos.y)){
                        pbd->locateStone(b2bd.stone(i).index, pos);
                        DERR << b2bd.stone(i).index << " " << pos << endl;
                    }
                }
            }
            return ret;
        }
        
        template<int RINK_ONLY, class board_t, class move_t>
        int simulate(board_t *const pbd, uint32_t stNum, const move_t& mv){
            // シミュレーション開始座標と速度
            fpn_t w=SpintoThrowFW(mv.getSpin());
            const fMStone<fpn_t> org(FX_THROW, FY_THROW, mv.vx(), mv.vy(), w);
            return simulateb2d<RINK_ONLY>(pbd, stNum, org, BOX2D_TIMESTEP);
        }
    }
}

#endif // DCURLING_SIMULATION_B2DSIMULATOR_HPP_