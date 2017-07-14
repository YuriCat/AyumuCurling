//various parameters

#include "../../dc.hpp"
#include "../../simulation/fastSimulator.h"
#include "../../simulation/primaryShot.hpp"

#ifndef DCURLING_SHOT_PARAMS_H_
#define DCURLING_SHOT_PARAMS_H_

namespace DigitalCurling{
    
    // 石の投擲店との距離から、石にギリギリ触る角度を求める
    //V = 32.5
    using hitLimitTable_t = LinearlyInterpolatedTable<std::array<double, 2>, double, 256, 0LL,
    (int64_t)((XYtoR2(FY_PA_MAX - FY_THROW + FR_STONE_RAD, FX_PA_MAX - FX_THROW - FR_STONE_RAD) + FR_STONE_RAD) * 10000000000LL), 10000000000LL>;
    
    // コンタクト限界を両端とした角度から、当たったあとの向こうの石の移動位置をもとめる
    //V = 32.5
    //pos = TEE
    using hitResultTable_t = LinearlyInterpolatedTable<std::array<double, 4>, double, 64, 0, 1>;
    
    // (石のスピード)、石とのコンタクトの後飛ぶ向きから、石のどこに当てればいいか求める
    // V = 29.5
    
    hitLimitTable_t hitLimitTable;
    hitResultTable_t hitResultTable;
    
    
    int initShotParams(){
        constexpr fpn_t V = 32.5;
        
        //double table
        
        
        {//hit limit table
            auto& table = hitLimitTable;
            
            for(int itr = 0; itr < table.arraySize(); ++itr){
                fpn_t tr = table.getKey(itr);
                //cerr<<"key = "<<tr<<endl;
                MiniBoard bd;
                
                fPosXY<> pos(FX_THROW,FY_THROW + table.getKey(itr));
                bd.pushStone(BLACK, pos);
                
                fMoveXY<> hit;
                hit.setSpin(Spin::RIGHT);
                genHit(&hit, pos, V);
                
                double left,right;
                {
                    BiSolver solver(-M_PI / 2, hit.th());
                    
                    //cerr<<"[ "<<solver.alpha()<<", "<<solver.beta()<<" ]"<<endl;
                    
                    for(int i = 0; i < 100; ++i){
                        MiniBoard tbd = bd;
                        
                        fpn_t val = solver.play();
                        
                        fMoveVTh<> mv(V, val, Spin::RIGHT);
                        
                        ContactTree ct = makeMoveNoRand<0>(&tbd, TURN_WHITE_LAST, mv);
                        
                        //cerr<<val<<mv<<ret<<tbd;
                        
                        solver.feed(ct.hasContact() ? true : false);
                    }
                    left = solver.answer();
                }
                {
                    BiSolver solver(hit.th(), +M_PI / 2);
                    
                    //cerr<<"[ "<<solver.alpha()<<", "<<solver.beta()<<" ]"<<endl;
                    
                    for(int i = 0; i < 100; ++i){
                        MiniBoard tbd = bd;
                        
                        fpn_t val = solver.play();
                        
                        fMoveVTh<> mv(V, val, Spin::RIGHT);
                        
                        ContactTree ct = makeMoveNoRand<0>(&tbd, TURN_WHITE_LAST, mv);
                        
                        //cerr<<val<<mv<<ret<<tbd;
                        
                        solver.feed(ct.hasContact() ? false : true);
                    }
                    right = solver.answer();
                }
                table.assign(itr, {left, right});
                
                //cerr<<tth<<","<<endl;
                //cerr<<"tr = "<<tr<<" tth = "<<tth<<" r*th = "<<tr*tth<<endl;
            }
        }
        CERR << "ganerated contact-limit table." << endl;
        
        {//hit result table
            auto& table=hitResultTable;
            
            fPosXY<> dst(FX_TEE, FY_TEE);
            
            // すれすれ点を求める
            fpn_t left, right;
            {
                MiniBoard bd;
                
                fPosXY<> pos = dst;
                bd.pushStone(BLACK, pos);
                
                fMoveXY<> hit;
                hit.setSpin(Spin::RIGHT);
                genHit(&hit, pos, V);
                
                BiSolver solver(-0.1, hit.th());
                
                //cerr<<"[ "<<solver.alpha()<<", "<<solver.beta()<<" ]"<<endl;
                
                for(int i = 0; i < 100; ++i){
                    MiniBoard tbd = bd;
                    
                    fpn_t val = solver.play();
                    
                    fMoveVTh<> mv(V, val, Spin::RIGHT);
                    
                    ContactTree ct = makeMoveNoRand<0>(&tbd, TURN_WHITE_LAST, mv);
                    
                    //cerr<<val<<mv<<ret<<tbd;
                    
                    solver.feed(ct.hasContact() ? true : false);
                }
                
                left = solver.answer();
            }
            {
                MiniBoard bd;
                
                fPosXY<> pos = dst;
                bd.pushStone(BLACK, pos);
                
                fMoveXY<> hit;
                hit.setSpin(Spin::RIGHT);
                genHit(&hit, pos, V);
                
                BiSolver solver(hit.th(), 0.1);
                
                //cerr<<"[ "<<solver.alpha()<<", "<<solver.beta()<<" ]"<<endl;
                
                for(int i = 0; i < 100; ++i){
                    MiniBoard tbd = bd;
                    
                    fpn_t val = solver.play();
                    
                    fMoveVTh<> mv(V, val, Spin::RIGHT);
                    
                    int ret = makeMoveNoRand<0>(&tbd, TURN_WHITE_LAST, mv);
                    
                    //cerr<<val<<mv<<ret<<tbd;
                    
                    solver.feed(!((ret == 2) ? true : false));
                }
                
                right = solver.answer();
            }
            //cerr<<"left = "<<left<<" right = "<<right<<endl;
            
            //cerr<<table.toDebugString()<<endl;
            
            fMoveXY<> mv;
            mv.setSpin(Spin::RIGHT);
            genHit(&mv, dst, V);
            
            for(int ith = 0; ith < table.arraySize(); ++ith){
                fpn_t th = left + table.getKey(ith) * (right - left);
                
                fpn_t dtth = th - mv.th();
                
                fMoveVTh<> tmv(V, mv.th() + dtth, mv.getSpin());
                
                MiniBoard bd;
                bd.pushStone(BLACK, dst);
                
                int ret = makeMoveNoRand<0>(&bd, TURN_WHITE_LAST, tmv);
                
                const fPosXY<> st = bd.onlyStone(BLACK);
                
                table.assign(ith, {st.x - dst.x, st.y - dst.y,
                    calcDistance(dst, st), calcRelativeAngle(dst, st)});
                
                //cerr<<st<<","<<calcDistance(st,dst)<<endl;
                
            }
            

        }
        CERR << "ganerated contact-result table." << endl;
        
        return 0;
    }
}

#endif // DCURLING_SHOT_PARAMS_H_