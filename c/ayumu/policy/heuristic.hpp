//デジタルカーリング
//ヒューリスティック方策関数

namespace DigitalCurling{
    
    namespace Heuristic{
        
        template<class board_t, class move_t>
        void generate(move_t *const pmv, const board_t& bd, ThreadTools *const tools){
            
            const Color myColor = bd.getTurnColor();
            const Color opsColor = flipColor(myColor);
            
            DERR << "NInPlayArea - Black : " << countNInPlayArea(bd, BLACK) << " White : " << countNInPlayArea(bd, WHITE) << endl;
            DERR << "NInHouse - Black : " << countNInHouse(bd,BLACK) << " White : " << countNInHouse(bd,WHITE) << endl;
            DERR << "orderBits : " << bd.orderBits << endl;
            DERR << "center - Black : " << bd.centerInColor[BLACK] << " White : " << bd.centerInColor[WHITE] << endl;
            DERR << "front - Black : " << bd.frontInColor[BLACK] << " White : " << bd.frontInColor[WHITE] << endl;
            DERR << "left - Black : " << bd.leftInColor[BLACK] << " White : " << bd.leftInColor[WHITE] << endl;
            
            //分岐条件
            //black white (2)
            //superior - even - inferior (3)
            //turn (3)or(8)
            
            //field
            //null (1)
            //all mine (3) + all ops (3)
            //
            
            if (bd.getTurn() == TURN_LAST){//L1
                if (bd.countScore() == 0){//no stone in House
                    if (gEvaluator.isBlankBetter()){//blank end is better
                        DERR << "Pass" << endl;
                        genPASS(pmv); goto DECIDED;
                    }
                }
                //If we can free-draw into Tee,
                //drawing into Tee is best.
                //else, we should search where to draw or which stone to hit
                //if (genL1Draw(pmv, bd)==0){
                //    goto DECIDED;
                //}
            }
            else if(myColor == WHITE){//white turn
                if (bd.getRelScore() > 0){// taking lead
                    
                }
                else if (bd.getRelScore() == 0){// even
                    
                }
                else{ //disadvantage
                    if (bd.isNull()){
                        if(bd.getTurn() <= TURN_LAST+2
                           && gEvaluator.isBlankBetter()//blank end is better
                           ){
                            genPASS(pmv); goto DECIDED;
                        }else{
                            genStandardDraw(pmv, DrawPos::G4); goto DECIDED;
                        }
                    }else{
                    }
                }
                
            }
            else{// black turn
                if (bd.getRelScore() > 0){// taking lead
                    if (!countNInPlayArea(bd, WHITE)){// no ops stone
                        if (!countNInPlayArea(bd, BLACK)){
                            //null field
                            genPASS(pmv); goto DECIDED;
                        }
                        else{
                            //only my stones
                            //gen
                        }
                    }
                    else{// any ops stones
                        if (!countNInHouse(bd, BLACK)){// only ops stones are in House
                            if (countNInHouse(bd, WHITE) == 1){
                                pmv->setSpin(tools->dice.rand() % 2);
                                if (gPeel.genReal1(pmv, bd.stoneCenter(WHITE, 0)) >= 0){ goto DECIDED; }
                            }
                        }
                        else{// both mine and opss are in Play-Area
                        }
                    }
                }
                else if (bd.getRelScore() == 0){// even
                    if (bd.isNull()){
                        genPASS(pmv); goto DECIDED;
                    }
                    else{
                        genStandardDraw(pmv, DrawPos::S4); goto DECIDED;
                    }
                }
                else{// disadvantage
                    if (countNInPlayArea(bd, WHITE)){// any ops stones int play area
                        if (countNInPlayArea(bd, WHITE) == 1){
                            pmv->setSpin(tools->dice.rand() % 2);
                            if (gPeel.genReal1(pmv, bd.stoneCenter(WHITE, 0)) >= 0){ goto DECIDED; }
                        }
                    }
                }
                int i = 0;
                for(;i<countNInHouse(bd);++i){
                    if(bd.orderBits.get(i)==WHITE){break;}
                }
                for(;i<countNInHouse(bd);++i){
                    if(bd.orderBits.get(i)==BLACK){break;}
                }
            }
            
            // the easiest strategy
            if (countNInHouse(bd, opsColor) >= 2
                && bd.orderBits.get(0) == opsColor && bd.orderBits.get(1) == opsColor){
                //nerau double_take
                
                int ifront = bd.centerInColor[opsColor][0];
                int iback = bd.centerInColor[opsColor][1];
                if (isMoreFrontal(bd.stone(iback), bd.stone(ifront))){
                    swap(ifront, iback);
                }
                gDouble.genReal2(pmv, bd.stone(ifront), bd.stone(iback));
                goto DECIDED;
            }
            
            if(countNInHouse(bd, opsColor)){
                pmv->setSpin(tools->dice.rand() % 2);
                genStandardHit(pmv, bd.stoneCenter(opsColor, 0), HitWeight::MIDDLE, 1);
            }else{
                pmv->setSpin(tools->dice.rand() % 2);
                genStandardDraw(pmv, DrawPos::TEE);
            }
        DECIDED:
            //CERR<<bd.toDebugString();
            //CERR<<*pmv<<endl;
            //getchar();
            return;
        }   
    }
}
