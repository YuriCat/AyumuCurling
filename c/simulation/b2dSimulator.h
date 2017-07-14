/*
 b2dSimulation.h
 */

// Digital Curling
// Simulation With Box2D

#ifndef DCURLING_SIMULATION_B2DSIMULATOR_H_
#define DCURLING_SIMULATION_B2DSIMULATOR_H_

#include <Box2D/Box2D.h>

#include "../dc.hpp"

namespace DigitalCurling{
    namespace b2dSimulator{
        
        b2Body* createBody(fpn_t x, fpn_t y, b2World *const world);
        
        std::ostream& operator<<(std::ostream& os, const b2Vec2& vec){
            os << "( " << vec.x << ", " << vec.y << " )";
            return os;
        }
        
        /**************************Box2D物理演算用の石クラス*************************/
        
        struct b2IdentityBoardStone{
            b2Body* pbody;
            
            float vx()const noexcept{ return pbody->GetLinearVelocity().x; }
            float vy()const noexcept{ return pbody->GetLinearVelocity().y; }
            float w()const  noexcept{ return pbody->GetAngularVelocity(); }
            
            float x()const noexcept{ return pbody->GetPosition().x; }
            float y()const noexcept{ return pbody->GetPosition().y; }
            
            const b2Vec2& vel()const noexcept{ return pbody->GetLinearVelocity(); }
            const b2Vec2& pos()const noexcept{ return pbody->GetPosition(); }
        };
        
        struct b2BoardStone : public b2IdentityBoardStone{
            uint32_t index; // 詰めるために石番のデータも必要
            
            void set(uint32_t ai, b2Body* ap){
                index = ai; pbody = std::move(ap);
            }
        };
        
        /**************************Box2D物理演算用の局面クラス(石を詰めてループ)*************************/
        
        struct b2Board{
            
            // Box2dシミュレーションのための局面表現
            b2World world;
            b2BoardStone stone_[N_STONES];
            
            //int fg;
            int NAwake,NAsleep;
            int NMoved; // 結果的に動いた石の数
            
            int turn;
            
            b2Board():
            world( b2Vec2(0,0) ),
            NAwake(0),NAsleep(0)
            {}
            
            ~b2Board(){
                const int size = NAwake + NAsleep;
                for(int i=0; i<size; i++){
                    assert(stone(i).pbody != nullptr);
                    world.DestroyBody(stone(i).pbody);
                }
            }
            
            const b2BoardStone& stone(int idx)const{ return stone_[idx]; }
            b2BoardStone& stone(int idx){ return stone_[idx]; }
            
            template<class mst_t>
            void setAwakeStone(uint32_t num, const mst_t& org){
                // 動く石を1つセット
                // 偶然速さが0になる場合を否定出来ないので、速さチェックは入れない
                b2Body* newBody = createBody(org.getX(), org.getY(), &world);
                
                newBody->SetLinearVelocity(b2Vec2(org.vx(), org.vy())); // Vx, Vyを設定
                newBody->SetAngularVelocity(org.getW()); // 回転を設定
                
                stone(NAwake).set(num, newBody);
                ++NAwake;
                
                turn = num;
            }
            
            template<int RINK_ONLY = 1, class board_t>
            void setAsleepStones(const board_t& bd){
                // 静止している石をセット
                // 動く石のセットの後の必要あり
                
                int cnt = NAwake;
                iterateStoneWithIndex(bd, [&cnt, this](uint32_t index, const auto& st)->void{
                    if (RINK_ONLY){
                        ASSERT(isInPlayArea(st), cerr << st << endl;); // 有効エリアに入っているものしか考慮しない
                    }
                    b2Body* newBody = createBody(st.getX(), st.getY(), &world);
                    
                    newBody->SetLinearVelocity(b2Vec2(0.0, 0.0)); // Vx, Vyを設定
                    newBody->SetAngularVelocity(0.0);
                    
                    stone_[cnt].set(index, newBody);
                    ++cnt;
                });
                NAsleep = cnt - NAwake;
            }
            
            template<int RINK_ONLY = 1, class st_t>
            void setAsleepStone(uint32_t stNum, const st_t& org){
                // 静止している石を1つセット
                // 動く石のセットの後の必要あり
                
                const int idx = NAwake + NAsleep;
                
                if(RINK_ONLY){
                    assert(isInPlayArea(org.x, org.y)); // 有効エリアに入っているものしか考慮しない
                }
                b2Body* newBody = createBody(org.x, org.y, &world);
                
                newBody->SetLinearVelocity(b2Vec2(0.0, 0.0)); // Vx, Vyを設定
                newBody->SetAngularVelocity(0.0);
                
                stone(idx).set(stNum, newBody);
                ++NAsleep;
            }
            
            bool exam()const{
                const int size = NAwake + NAsleep;
                // pointer to body
                for(int i = 0; i < size; ++i){
                    if(stone(i).pbody == nullptr){
                        cerr << "b2Board::exam() : null body" << endl;
                        return false;
                    }
                }
                // uinqueness
                for(int i = 0; i < size; ++i){
                    for(int j = 0; j < i; ++j){
                        if(stone(i).index == stone(j).index){
                            cerr << "b2Board::exam() : uniqueness fault" << endl;
                            return false;
                        }
                    }
                }
                return true;
            }
        };
        
        /**************************Box2D物理演算用の局面クラス(石を詰めずにループ)*************************/
        
        struct b2IdentityBoard{
            // Box2dシミュレーションのための局面表現
            // 石番の位置に石を置くタイプ
            b2World world;
            b2IdentityBoardStone stone_[N_STONES];
            BitSet32 modified;
            
            const b2IdentityBoardStone& stone(int idx)const{ return stone_[idx]; }
            b2IdentityBoardStone& stone(int idx){ return stone_[idx]; }
            
            // 以下 idx は FSBoardのものなので注意
            template<class st_t>
            void setAsleepStone(int idx, const st_t& org, const StoneState& astate){
                // set 1 asleep stone
                b2Body* newBody = createBody(org.x, org.y, &world);
                
                newBody->SetLinearVelocity(b2Vec2(0.0, 0.0)); // Vx, Vyを設定
                newBody->SetAngularVelocity(0.0);
                
                stone(idx).pbody = newBody;
            }
            template<class st_t>
            void setAwakeStone(int idx, const st_t& org, const StoneState& astate){
                // set 1 awake stone
                b2Body* newBody = createBody(org.x, org.y, &world);
                
                newBody->SetLinearVelocity(b2Vec2(org.vx(), org.vy())); // Vx,Vyを設定
                newBody->SetAngularVelocity(org.getW());
                
                stone(idx).pbody = newBody;
            }
            
            b2IdentityBoard():
            world(b2Vec2(0,0))
            {
                for(int i = 0; i < N_STONES; ++i){
                    stone(i).pbody = nullptr;
                }
            }
            
            ~b2IdentityBoard(){
                for(int i = 0; i < N_STONES; ++i){
                    if(stone(i).pbody != nullptr){
                        world.DestroyBody(stone(i).pbody);
                    }
                }
            }
        };
        
        void friction(b2Body *const pbody, fpn_t fric);
        
        void frictionAll(b2Board *const pbd, fpn_t fric);
        
        void frictionAll(b2IdentityBoard *const pbd, fpn_t fric);
        
        void step(b2Board *const pb2bd, const fpn_t timeStep);
        
        template<int RINK_ONLY = 1, class board_t> // リンク外も考慮する
        int loop(const board_t&, b2Board *const, const fpn_t timeStep, const int LoopCount = INT_MAX);
        
        int loopInContact(b2IdentityBoard *const, const fpn_t timeStep, const fpn_t collisionLine, const int LoopCount = INT_MAX);
        
        // シミュレーション本体
        template<int RINK_ONLY = 1, class board_t, class mst_t>
        int simulateb2d(board_t *const, uint32_t, const mst_t&, const fpn_t);
        
        template<int RINK_ONLY = 1, class board_t, class move_t>
        int simulate(board_t *const, uint32_t, const move_t& mv);
    }
}

#endif // DCURLING_SIMULATION_B2DSIMULATOR_H_