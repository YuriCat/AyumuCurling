/*
 l1Draw.hpp
 Katsuki Ohto
 */

// デジタルカーリング
// ラストストーンを空いた所にドロー

#ifndef DCURLING_SHOT_L1DRAW_HPP_
#define DCURLING_SHOT_L1DRAW_HPP_

#include "shotGenerator.hpp"
#include "../ayumu_dc.hpp"

namespace DigitalCurling{
    struct L1Draw : public ShotGenerator{
        
        constexpr static int number()noexcept{ return Standard::L1DRAW; }
        constexpr static int dimension()noexcept{ return 0; }
        constexpr static int flexibility()noexcept{ return 2; }
        constexpr static bool hasContact()noexcept{ return false; }
        constexpr static fpn_t baseSpeed()noexcept{ return FV_TEESHOT; }

        template<class board_t, class vmove_t>
        double estimate(const board_t&, uint32_t stNum, const vmove_t& vmv){
            // ドローで決められそうならその安全確率, ダメそうなら0
            
            return 0;
        }
        
        template<class board0_t, class board1_t, class vmove_t>
        double evaluate(const board0_t& pre, const board1_t& post, uint32_t stNum, const vmove_t& vmv, const ContactTree& ct = 0){
            // 相手がNo1またはブランクのとき...自分がNo1ならMAX
            // 自分がNo1のとき...自分の点が増えたらMAX
            // 他の石に触ったら0点(本来当たってもいいのだが、こうしないとヒットと比較できない)
            
            //cerr << countScore(pre, col) << " -> " << countScore(post, col) << endl;
            
            //if(ct.hasContact()){ return 0; }
            
            double ret;
            const Color col = getColor(stNum);
            
            if (countNInHouse(pre) == 0 || searchNo1Color(pre) == flipColor(col)){
                if (searchNo1Index(post) == (int)stNum){
                    ret = 1.0;
                }else{
                    ret = 0.0;
                }
            }
            else{
                if (countScore(pre, col) + 1 == countScore(post, col)){
                    ret = 1.0;
                }else{
                    ret = 0.0;
                }
            }
            //cerr << ret << endl;
            //if (ret > 0.5){ getchar(); }
            return ret;
        }
        
        template<class move_t, class board_t, class vmove_t>
        void realize(move_t *const pmv, const board_t& bd, const vmove_t& vmv){
            pmv->setSpin(vmv.spin());
            genReal0(pmv, bd);
        }
        
        template<class move_t, class board_t>
        double genReal0(move_t *const pmv, const board_t& bd){
            /*
            // 左から順に見て行って、空いている場所を探す
            fpn_t line = (countNInHouse(bd, BLACK) == 0) ? FR_HOUSE_RAD : bd.stoneCenter(BLACK, 0).getR();
            
            fpn_t best[2];
            int last = 0;
            fpn_t maxZone = -9999;
            int i = 0;
            fpn_t l, r;
            
            // 左端
            while(bd.stoneLeft(i).y > FY_TEE){
                ++i;
                last = i;
                if(i >= bd.NInPlayArea()){
                    best[0] = -line; best[1] = line;
                    maxZone = line + line;
                    goto END;
                }// ガードなし
            }
            l = FX_TEE - line;
            r = min(FX_TEE + line, bd.stone(i).x);
            
            if(r - l > maxZone){
                best[0] = l;
                best[1] = r;
                maxZone = r - l;
            }
            last = i;
            // 石の間
            for(i = last; i < bd.NInPlayArea(); ++i){
                while(bd.stoneLeft(i).y > FY_TEE){
                    ++i;
                    if(i >= bd.NInPlayArea()){ goto RIGHT; } // もうガードなし
                }
                l = max(FX_TEE - line, bd.stone(last).x);
                r = min(FX_TEE + line, bd.stone(i).x);
                
                if(r - l > maxZone){
                    best[0] = l;
                    best[1] = r;
                    maxZone = r - l;
                }
                last = i;
            }
            // 右端
        RIGHT:
            l = max(FX_TEE - line, bd.stone(last).x);
            r = FX_TEE + line;
            
            if(r - l > maxZone){
                best[0] = l;
                best[1] = r;
                maxZone = r - l;
            }
            
        END:
            //cerr << maxZone << endl;
            if(maxZone > 0.3){
                pmv->setSpin((best[0] + best[1] < 0) ? Spin::RIGHT : Spin::LEFT);
                genDraw(pmv, fPosXY<>((best[0] + best[1]) / 2, FY_TEE));
                return 1;
            }else{
                genDraw(pmv, FPOSXY_TEE);
                return 0;
            }
            
            //no drawing position
            pmv->setSpin(Spin::RIGHT);
            genDraw(pmv, FPOSXY_TEE);
            return 0;
             */
            
            // VBBで探す
            fpn_t borderR = bd.NInHouse(BLACK) ? calcDistanceTee(bd.stone(bd.centerInColor[BLACK][0])) : FR_IN_HOUSE;
            borderR = max(0.0, borderR - 0.15);
            
            VortexBitBoard boaderMask(0);
            int vvi = RtoVI(borderR) - 1;
            if(vvi > 0){
                boaderMask.fill(0, vvi - 1);
                
                //cerr << borderR << endl;
                //cerr << RtoVI(borderR) << endl;
                //cerr << boaderMask << endl;
                
                int vi;
                VortexBitBoard vbb[4];
                for(int i = 0; i < 4; ++i){
                    vbb[i].fill();
                }
                if(!pmv->isLeftSpin()){ // right spin
                    iterateStone(bd, [&vbb](const auto& st)->void{
                        int ix = (st.x - FX_PA_MIN) / FR_PA_WIDTH * GVBBTABLE_WIDTH;
                        int iy = (st.y - FY_PA_MIN) / (FR_PA_LENGTH + 2 * FR_STONE_RAD) * GVBBTABLE_LENGTH;
                        
                        ASSERT(0 <= ix && ix < GVBBTABLE_WIDTH,  cerr << st.x << " " << ix << endl;);
                        ASSERT(0 <= iy && iy < GVBBTABLE_LENGTH, cerr << st.y << " " << iy << endl;);
                        
                        for(int i = 0; i < 4; ++i){
                            vbb[i] &= guardVBBTable[ix][iy][i];
                        }
                    });
                    for(int i = 0; i < 4; ++i){
                        VortexBitBoard vbbOk = VortexBitBoard(vbb[i] & boaderMask);
                        if(vbbOk.any()){
                            vi = vbbOk.bsf();
                            fPosXY<> dst;
                            genVortexDrawPos(&dst, vi);
                            pmv->setSpin(Spin::RIGHT);
                            genDraw(pmv, dst);
                            return 1;
                        }
                    }
                    /*for(int i = 0; i < 4; ++i){
                     if(vbb[i].any()){
                     vi = vbb[i].bsf();
                     fPosXY<> dst;
                     genVortexDrawPos(&dst, vi);
                     pmv->setSpin(Spin::RIGHT);
                     genDraw(pmv, dst);
                     return 1;
                     }
                     }*/
                }else{ // left spin
                    iterateStone(bd, [&vbb](const auto& st)->void{
                        int ix = (FX_PA_MAX - st.x) / FR_PA_WIDTH * GVBBTABLE_WIDTH;
                        int iy = (st.y - FY_PA_MIN) / (FR_PA_LENGTH + 2 * FR_STONE_RAD) * GVBBTABLE_LENGTH;
                        
                        ASSERT(0 <= ix && ix < GVBBTABLE_WIDTH,  cerr << st.x << " " << ix << endl;);
                        ASSERT(0 <= iy && iy < GVBBTABLE_LENGTH, cerr << st.y << " " << iy << endl;);
                        
                        for(int i = 0; i < 4; ++i){
                            vbb[i] &= guardVBBTable[ix][iy][i];
                        }
                    });
                    for(int i = 0; i < 4; ++i){
                        VortexBitBoard vbbOk = VortexBitBoard(vbb[i] & boaderMask);
                        if(vbbOk.any()){
                            vi = vbbOk.bsf();
                            fPosXY<> dst;
                            genVortexDrawPos(&dst, vi);
                            dst.flip();
                            pmv->setSpin(Spin::LEFT);
                            genDraw(pmv, dst);
                            return 1;
                        }
                    }
                    /*for(int i = 0; i < 4; ++i){
                     if(vbb[i].any()){
                     vi = vbb[i].bsf();
                     fPosXY<> dst;
                     genVortexDrawPos(&dst, vi);
                     dst.flip();
                     pmv->setSpin(Spin::LEFT);
                     genDraw(pmv, dst);
                     return 1;
                     }
                     }*/
                }
            }
            
            pmv->setVx(FVX_TEESHOT_OFFICIAL[Spin::RIGHT]);
            pmv->setVy(FVY_TEESHOT_OFFICIAL);
            pmv->setSpin(Spin::RIGHT);
            return 0;
        }
        
        template<class board_t, class dice_t>
        void setSampleState(board_t *const pbd, dice_t *const pdice){
            // put stones randomly
            pushStonesInHouse(pbd, BLACK, 1, pdice);
            pushStonesInHouse(pbd, WHITE, 1, pdice);
            pushStonesInPlayArea(pbd, BLACK, 2, pdice);
            pushStonesInPlayArea(pbd, WHITE, 2, pdice);
        }
        
        void init(){};
        
        L1Draw(){}
        ~L1Draw(){}
    };
}

#endif // DCURLING_SHOT_L1DRAW_HPP_