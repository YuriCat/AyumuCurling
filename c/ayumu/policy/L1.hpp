//デジタルカーリング
//エンド最終投

namespace DigitalCurling{
    
    namespace L1{
        
        //eval 局面評価値を出す
        //play 決定着手を返す
        constexpr int opsCol=BLACK;
        
        template<class board_t,class evaluator_t>
        eval_t eval(board_t& bd,const int ownerColor){
            if (bd.getEndScore() == 0){//ハウス内に石がない
                //白が0か1か選ぶ事が出来る
                return evaluator.evaluate01(ownerColor);
            }
            if (bd.NInHouse[BLACK] == 0){//邪魔な黒石がない
                //白は必ずドロー
                DERR << "easydraw" << bd.NInHouse[WHITE] << endl;
                const int sc = -(bd.NInHouse[WHITE] + 1);
                return evaluator.evaluateLast(ownerColor, sc);
                //}
            }
            
            return -99999;
        }
        
        template<class board_t,class move_t,class evaluator_t>
        int play(board_t& bd,move_t const pmv,ThreadTools *const ptools){
            //L1で手を返す
            if (bd.getEndScore() == 0){//ハウス内に石がない
                if( 1 /*getTmpEndReward(WHITE,0) > getTmpEndReward(WHITE,1)*/ ){
                    //0-0を選ぶ
                    genPASS(pmv);return 256;
                }else{
                    //1点取ろうとする
                    pmv->setSpin(ptools->dice.rand() % 2);
                    genStandardDraw(pmv,DrawPos::TEE);return 1;
                }
            }else{
                if( bd.NInHouse[opsCol] != 0 ){
                    //相手の石がハウス内にある
                    uint32_t ord = bd.orderBits;
                    uint32_t bw = (WHITE << 1) | BLACK;
                    bool h = true;// false;
                    for (int i = 0; i < (countNInPlayArea(bd) - 1); ++i){
                        if ((ord >> i) & 3U == bw){
                            h = true; break;
                        }
                    }
                    if (h){
                        const auto& opsNo1 = accessStone(bd,bd.nearTeeInColor[BLACK][0]);
                        
                        //相手のNo1ストーンを叩く
                        pmv->setSpin(ptools->dice.rand() % 2);
                        genHit(pmv, opsNo1, HitWeight::MIDDLE, 1);
                    }else{
                        pmv->setSpin(ptools->dice.rand() % 2);
                        genStandardDraw(pmv, DrawPos::TEE);
                    }
                }else{
                    pmv->setSpin(ptools->dice.rand() % 2);
                    genStandardDraw(pmv, DrawPos::TEE);
                    
                }
                return 1;
            }
        }
        
        template<class board_t,class move_t,class tools_t>
        int playSlow(board_t& bd,move_t const pmv,tools_t *const ptools){
            //L1で手を返す(遅くてよい)
            if (bd.getEndScore() == 0){//ハウス内に石がない
                if( 1 /*getTmpEndReward(WHITE,0) > getTmpEndReward(WHITE,1)*/ ){
                    //0-0を選ぶ
                    genPASS(pmv);return 256;
                }else{
                    //1点取ろうとする
                    pmv->setSpin(ptools->dice.rand() % 2);
                    genStandardDraw(pmv, DrawPos::TEE);return 1;
                }
            }else{
                if( bd.NInHouse[opsCol] != 0 ){
                    //相手の石がハウス内にある
                    uint32_t ord = bd.orderBits;
                    uint32_t bw = (WHITE << 1) | BLACK;
                    bool h = true;// false;
                    for (int i = 0; i < (countNInPlayArea(bd) - 1); ++i){
                        if ((ord >> i) & 3U == bw){
                            h = true; break;
                        }
                    }
                    if (h){
                        const auto& opsNo1 = accessStone(bd, opsCol, 0);
                        
                        
                        
                        /*if( bd.NInPlayArea[opsCol]>1 ){
                         
                         fMoveXY<> tmv[2];
                         ThinkBoard tbd[2];
                         double tev[2]={0};
                         
                         for(int i=0;i<4;++i){
                         tbd[0]=bd;
                         
                         tmv[0].setSpin(tools->dice.rand() % 2);
                         genHit(opsNo1, &tmv[0], HitWeight::MIDDLE, 1);
                         
                         tbd[0].makeMove(tmv[0],&tools->ddice);
                         tev[0]+=getOpeningEndReward(bd.getTurnColor(),tbd[0].countEndScore());
                         }
                         
                         for(int i=0;i<4;++i){
                         tbd[1]=bd;
                         
                         tmv[1].setSpin(tools->dice.rand() % 2);
                         genHit(opsNo1, &tmv[1], HitWeight::MIDDLE, 1);
                         
                         tbd[1].makeMove(tmv[1],&tools->ddice);
                         tev[1]+=getOpeningEndReward(bd.getTurnColor(),tbd[1].countEndScore());
                         }
                         
                         if( tev[0]>tev[1] ){
                         *mv=tmv[0];
                         }else{
                         *mv=tmv[1];
                         }
                         
                         }else{*/
                        //相手のNo1ストーンを叩く
                        pmv->setSpin(ptools->dice.rand() % 2);
                        genHit(pmv, opsNo1, HitWeight::MIDDLE, 1);
                        //}
                    }else{
                        pmv->setSpin(ptools->dice.rand() % 2);
                        genStandardDraw(pmv, DrawPos::TEE);
                    }
                }else{
                    pmv->setSpin(ptools->dice.rand() % 2);
                    genStandardDraw(pmv, DrawPos::TEE);
                    
                }
                return 1;
            }
        }
    }
    
}
