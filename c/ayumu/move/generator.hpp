//Digital Curling
//generating All Moves

#ifndef DCURLING_AYUMU_MOVEGENERATOR_HPP_
#define DCURLING_AYUMU_MOVEGENERATOR_HPP_

namespace DigitalCurling{
    
    template<class vmove_t, class board_t>
    int genImplicitVMove(vmove_t *const *mv0, const board_t& bd){
        // 探索等せずに瞬時に選ぶ手の定石
        return 0;
    }
    /*
     template<class vmove_t, class board_t>
     int genAllVMove(vmove_t *const mv0, const board_t& bd){
     
     vmove_t *mv = mv0;
     const Color myColor = bd.getTurnColor();
     const Color oppColor = flipColor(myColor);
     //絶対手
     
     //pass
     mv->setPASS(); ++mv;
     
     //draw shot
     for(int p = DrawPos::TEE; p < DrawPos::G3; ++p){
     mv->setDraw(Spin::RIGHT, p); ++mv;
     mv->setDraw(Spin::LEFT, p); ++mv;
     }
     
     //pre guard
     mv->setDraw(Spin::LEFT, DrawPos::G3); ++mv;
     mv->setDraw(Spin::RIGHT, DrawPos::G4); ++mv;
     mv->setDraw(Spin::LEFT, DrawPos::G4); ++mv;
     mv->setDraw(Spin::RIGHT, DrawPos::G5); ++mv;
     
     //1石相対
     iterateStoneWithIndex(bd, [&](uint32_t idx, const auto& st)->void{
     for(int w = 0; w < HitWeight::MAX; ++w){
     mv->setHit(Spin::RIGHT, idx, w); ++mv;
     mv->setHit(Spin::LEFT, idx, w); ++mv;
     }
     for(int l = 0; l < PostGuardLength::MAX; ++l){
     mv->setPostGuard(Spin::RIGHT, idx, l); ++mv;
     mv->setPostGuard(Spin::LEFT, idx, l); ++mv;
     }
     if(isThisSideOfTeeLine(st.y)){
     for(int l = 0; l < ComeAroundLength::MAX;++l){
     mv->setComeAround(Spin::RIGHT, idx, l); ++mv;
     mv->setComeAround(Spin::LEFT, idx, l); ++mv;
     }
     }
     mv->setPeel(Spin::RIGHT, idx, 0); ++mv;
     mv->setPeel(Spin::LEFT, idx, 0); ++mv;
     });
     
     //2石相対
     iterateStoneWithIndex(bd, oppColor, [&, oppColor](uint32_t idx, const auto& st)->void{
     iterate(st.tailStone(), [&, oppColor](uint32_t idx1)->void{
     if (getColor(idx1) == oppColor){
     mv->setDouble(Spin::RIGHT, idx, idx1); ++mv;
     mv->setDouble(Spin::LEFT, idx, idx1); ++mv;
     }
     });
     });
     
     //全体
     if(bd.getTurn() == TURN_LAST){
     //mv->setL1Draw(); ++mv;
     }
     
     //1石全体
     iterateStoneWithIndex(bd, oppColor, [&](uint32_t idx, const auto& st)->void{
     //mv->setTakeOut(idx); ++mv;
     });
     
     return mv - mv0;
     }
     */
    
    template<class move_t, class board_t>
    int genSimpleVMove(move_t *const mv0, const board_t& bd){
        // 実験用の最も単純な着手生成
        move_t *mv = mv0;
        const Color oppColor = flipColor(bd.getTurnColor());
        if(countNInHouse(bd, oppColor) > 0){ // ハウス内に相手の石がある
            const uint32_t no1 = bd.centerInColor[oppColor][0]; // 相手の最も内側の石
            ASSERT(examIndex(no1), cerr << no1 << endl;);
            mv->setHit(Spin::RIGHT, no1, HitWeight::MIDDLE); ++mv;
            mv->setHit(Spin::LEFT, no1, HitWeight::MIDDLE); ++mv;
        }
        mv->setDraw(Spin::RIGHT, DrawPos::TEE); ++mv;
        mv->setDraw(Spin::LEFT, DrawPos::TEE); ++mv;
        return mv - mv0;
    }
    
    template<class move_t>
    int genNullVMove(move_t *const mv0, int e, int t, int rs){
        // ターンtの空場で検討しておきたい定跡手を配列に入れていく
        move_t *mv = mv0;
        const bool clean = isCleanBetterScore(getColor(t), e, rs);
        if (isBlack(getColor(t))){
            if(clean){
                mv->setPASS(); ++mv;
            }else{
                if(isFreeGuardTurn(t)){
                    mv->setPreGuard(Spin::RIGHT, DrawPos::G4); ++mv; // センターガード
                    mv->setPreGuard(Spin::LEFT, DrawPos::G4); ++mv;
                }else if(t == TURN_BEFORE_LAST){
                    mv->setDraw(Spin::RIGHT, DrawPos::TEE); ++mv;
                    mv->setDraw(Spin::LEFT, DrawPos::TEE); ++mv;
                }else{
                    mv->setDraw(Spin::RIGHT, DrawPos::TEE); ++mv;
                    mv->setDraw(Spin::LEFT, DrawPos::TEE); ++mv;
                    mv->setPreGuard(Spin::RIGHT, DrawPos::G4); ++mv; // センターガード
                    mv->setPreGuard(Spin::LEFT, DrawPos::G4); ++mv;
                }
            }
        }else{
            if(rs == 0 && e == END_LAST){
                if(t == TURN_LAST){
                    mv->setDraw(Spin::RIGHT, DrawPos::TEE); ++mv;
                    mv->setDraw(Spin::LEFT, DrawPos::TEE); ++mv;
                }else{
                    mv->setPASS(); ++mv;
                }
            }else if(clean){
                mv->setPASS(); ++mv;
                if(isFreeGuardTurn(t)){
                    mv->setPreGuard(Spin::RIGHT, DrawPos::G3); ++mv; // サイドガード
                    mv->setPreGuard(Spin::LEFT, DrawPos::G3); ++mv;
                    mv->setPreGuard(Spin::RIGHT, DrawPos::G5); ++mv;
                    mv->setPreGuard(Spin::LEFT, DrawPos::G5); ++mv;
                }
                if(t >= 4){
                    mv->setDraw(Spin::RIGHT, DrawPos::L3); ++mv; // サイドショット
                    mv->setDraw(Spin::LEFT, DrawPos::L3); ++mv;
                    mv->setDraw(Spin::RIGHT, DrawPos::L5); ++mv;
                    mv->setDraw(Spin::LEFT, DrawPos::L5); ++mv;
                }
            }else{
                if(isFreeGuardTurn(t)){
                    mv->setPreGuard(Spin::RIGHT, DrawPos::G3); ++mv; // サイドガード
                    mv->setPreGuard(Spin::LEFT, DrawPos::G3); ++mv;
                    mv->setPreGuard(Spin::RIGHT, DrawPos::G5); ++mv;
                    mv->setPreGuard(Spin::LEFT, DrawPos::G5); ++mv;
                }else if(t == TURN_LAST){
                    mv->setPASS(); ++mv;
                    mv->setDraw(Spin::RIGHT, DrawPos::TEE); ++mv;
                    mv->setDraw(Spin::LEFT, DrawPos::TEE); ++mv;
                }else{
                    mv->setDraw(Spin::RIGHT, DrawPos::L3); ++mv; // サイドショット
                    mv->setDraw(Spin::LEFT, DrawPos::L3); ++mv;
                    mv->setDraw(Spin::RIGHT, DrawPos::L5); ++mv;
                    mv->setDraw(Spin::LEFT, DrawPos::L5); ++mv;
                    mv->setPreGuard(Spin::RIGHT, DrawPos::G3); ++mv; // サイドガード
                    mv->setPreGuard(Spin::LEFT, DrawPos::G3); ++mv;
                    mv->setPreGuard(Spin::RIGHT, DrawPos::G5); ++mv;
                    mv->setPreGuard(Spin::LEFT, DrawPos::G5); ++mv;
                }
            }
        }
        return mv - mv0;
    }
    
    template<class vmove_t, class board_t>
    int genChosenVMove(vmove_t *const mv0, const board_t& bd){
        
        //恣意的に選択した手を乗せる
        const Color myColor = bd.getTurnColor();
        const Color oppColor = flipColor(myColor);
        const int rs = bd.getRelScore();
        const int e = bd.getRelScore();
        const int t = bd.getTurn();
        const bool clean = isCleanBetterScore(myColor, e, rs);
        
        if (bd.isAlmostNull()){ // ほぼ空場の時は空場専用ルーチン
            return genNullVMove(mv0, e, t, rs);
        }else{
            vmove_t *mv = mv0;
            
            const int sc = countScore(bd);
            if(sc != 0){ // ハウス内に石がある
                uint32_t no1 = bd.center[0];
                if (getColor(no1) == oppColor){ // 相手の石がNo1
                    if(clean && (countNInHouse(bd) == 1) && (!bd.stone(no1).anyGuard())){
                        Spin s = (bd.stone(no1).getX() < FX_TEE) ? Spin::RIGHT : Spin::LEFT;
                        mv->setHit(s, no1, HitWeight::MIDDLE); return 1;
                    }
                    
                    /*if(bd.stone(no1).frontGuard(Spin::RIGHT)){
                        mv->setTakeOut(Spin::LEFT, no1); ++mv;
                    }else if(bd.stone(no1).frontGuard(Spin::LEFT)){
                        mv->setTakeOut(Spin::RIGHT, no1); ++mv;
                    }*/
                    
                    mv->setHit(Spin::RIGHT, no1, HitWeight::MIDDLE); ++mv;
                    mv->setHit(Spin::LEFT, no1, HitWeight::MIDDLE); ++mv;
                    mv->setHit(Spin::RIGHT, no1, HitWeight::STRONG); ++mv;
                    mv->setHit(Spin::LEFT, no1, HitWeight::STRONG); ++mv;
                    mv->setDraw(Spin::RIGHT, DrawPos::TEE); ++mv;
                    mv->setDraw(Spin::LEFT, DrawPos::TEE); ++mv;
                    
                    if(bd.NInHouse() > 1){
                        uint32_t no2 = bd.center[1];
                        if (getColor(no2) == oppColor){
                            mv->setHit(Spin::RIGHT, no2, HitWeight::MIDDLE); ++mv;
                            mv->setHit(Spin::LEFT, no2, HitWeight::MIDDLE); ++mv;
                            mv->setDouble(Spin::RIGHT, no1, no2); ++mv;
                            mv->setDouble(Spin::LEFT, no1, no2); ++mv;
                        }
                    }
                    
                    if(!clean && bd.stone(no1).y > FY_TEE){
                        mv->setFreeze(Spin::RIGHT, no1, 0); ++mv;
                        mv->setFreeze(Spin::LEFT, no1, 0); ++mv;
                    }
                    
                    //if(bd.getTurn()==TURN_LAST){
                    //    mv->setTakeOut(Spin::RIGHT, no1); ++mv;
                    //    mv->setTakeOut(Spin::LEFT, no1); ++mv;
                    //}else{
                    if(clean){
                        mv->setPeel(Spin::RIGHT, no1, 0); ++mv;
                        mv->setPeel(Spin::LEFT, no1, 0); ++mv;
                    }
                    
                    // ドローレイズ出来そうな自分の石を探す
                    iterateStoneWithIndex(bd, myColor,[&mv](uint32_t i, const auto& st)->void{
                        if(isInDrawRaiseZone(st)){
                            if(st.x < FX_TEE){
                                mv->setDrawRaise(Spin::RIGHT, i); ++mv;
                            }else{
                                mv->setDrawRaise(Spin::LEFT, i); ++mv;
                            }
                        }
                    });
                    
                    // No1にガードが付いている場合にはレイズテイクを検討
                    if(bd.stone(no1).frontGuard()){
                        iterate(bd.stone(no1).frontGuard(Spin::RIGHT), [&mv, &bd, myColor, no1](int i){
                            if(getColor(i) == myColor && !bd.stone(i).frontGuard(Spin::RIGHT)){
                                mv->setRaiseTakeOut(Spin::RIGHT, i, no1); ++mv;
                            }
                        });
                        iterate(bd.stone(no1).frontGuard(Spin::LEFT), [&mv, &bd, myColor, no1](int i){
                            if(getColor(i) == myColor && !bd.stone(i).frontGuard(Spin::LEFT)){
                                mv->setRaiseTakeOut(Spin::LEFT, i, no1); ++mv;
                            }
                        });
                    }
                    
                }else{ // 自分の石がNo1
                    
                    // 相手のNo1を出せば大量点の場合にはそれで決定
                    if(bd.NInHouse(oppColor)){
                        uint32_t oppNo1 = bd.centerInColor[oppColor][0];
                        if(isWhite(myColor) && !bd.stone(oppNo1).anyGuard() && count2ndScore(bd.orderBits, oppColor) <= -1){
                            //cerr << bd; getchar();
                            mv->setHit(Spin::RIGHT, oppNo1, HitWeight::MIDDLE); ++mv;
                            mv->setHit(Spin::LEFT, oppNo1, HitWeight::MIDDLE); ++mv;
                            return 2;
                        }else{
                            mv->setHit(Spin::RIGHT, oppNo1, HitWeight::MIDDLE); ++mv;
                            mv->setHit(Spin::LEFT, oppNo1, HitWeight::MIDDLE); ++mv;
                        }
                    }
                    
                    mv->setDraw(Spin::RIGHT, DrawPos::TEE); ++mv;
                    mv->setDraw(Spin::LEFT, DrawPos::TEE); ++mv;
                    if(t > TURN_LAST){
                        mv->setPostGuard(Spin::RIGHT, no1, PostGuardLength::MIDDLE); ++mv;
                        mv->setPostGuard(Spin::LEFT, no1, PostGuardLength::MIDDLE); ++mv;
                        mv->setPostGuard(Spin::RIGHT, no1, PostGuardLength::LONG); ++mv;
                        mv->setPostGuard(Spin::LEFT, no1, PostGuardLength::LONG); ++mv;
                    }else if(bd.NInHouse(oppColor)){
                        uint32_t oppNo1 = bd.centerInColor[oppColor][0];
                        
                        mv->setHit(Spin::RIGHT, oppNo1, HitWeight::STRONG); ++mv;
                        mv->setHit(Spin::LEFT, oppNo1, HitWeight::STRONG); ++mv;
                        
                        if(clean){
                            //mv->setPeel(Spin::RIGHT, oppNo1, 0); ++mv;
                            //mv->setPeel(Spin::LEFT, oppNo1, 0); ++mv;
                        }else{
                            if(bd.stone(oppNo1).y > FY_TEE){
                                mv->setFreeze(Spin::RIGHT, oppNo1, 0); ++mv;
                                mv->setFreeze(Spin::LEFT, oppNo1, 0); ++mv;
                            }
                        }
                    }
                }
            }else{ // ハウス内に石がない
                if(clean){
                    mv->setPASS(); ++mv;
                    if(!isFreeGuardTurn(t)){
                        // ガードを外す
                        iterateStoneWithIndex(bd, oppColor, [&mv](uint32_t i, const auto& st)->void{
                            if(isInGuardZone(st)){
                                mv->setPeel(Spin::RIGHT, i, 0); ++mv;
                                mv->setPeel(Spin::LEFT, i, 0); ++mv;
                            }
                        });
                    }
                }else{
                    //mv->setDraw(Spin::RIGHT, DrawPos::TEE); ++mv;
                    //mv->setDraw(Spin::LEFT, DrawPos::TEE); ++mv;
                    
                    if(isBlack(myColor)){
                        mv->setPreGuard(Spin::RIGHT, DrawPos::G4); ++mv;
                        mv->setPreGuard(Spin::LEFT, DrawPos::G4); ++mv;
                    }else{
                        // ガードがあったらカムアラする
                        iterateStoneWithIndex(bd, [&mv, myColor](uint32_t i, const auto& st)->void{
                            if(isInGuardZone(st)){
                                mv->setComeAround(Spin::RIGHT, i, ComeAroundLength::TEE); ++mv;
                                mv->setComeAround(Spin::LEFT, i, ComeAroundLength::TEE); ++mv;
                            }
                            if(getColor(i) == myColor){
                                // 押し込む
                                if(st.x < FX_TEE){
                                    mv->setDrawRaise(Spin::RIGHT, i); ++mv;
                                }else{
                                    mv->setDrawRaise(Spin::LEFT, i); ++mv;
                                }
                            }
                        });
                    }
                    mv->setDraw(Spin::RIGHT, DrawPos::TEE); ++mv;
                    mv->setDraw(Spin::LEFT, DrawPos::TEE); ++mv;
                }
            }
            if(bd.getTurn() == TURN_LAST){
                mv->setL1Draw(Spin::RIGHT); ++mv;
                mv->setL1Draw(Spin::LEFT); ++mv;
                //mv->setDraw(Spin::RIGHT, DrawPos::S6); ++mv;
                //mv->setDraw(Spin::LEFT, DrawPos::S2); ++mv;
            }
            return mv - mv0;
        }
        UNREACHABLE;
    }
    
    template<class vmove_t, class board_t, class dice_t>
    int genRootVMove(vmove_t *const mv0, const board_t& bd, dice_t *const pdice){
        // 与えられた局面で検討しておきたい定跡手を配列に入れていく(ルートノード)
        // 生成された手の個数を返す
        // 決定手があった場合にはインデックス0番に生成し、-1を返す
        
        // ルートの手は絶対手のみしか検討しないが、
        // この段階では相対手として生成する
        
        int settled = -1;//決定手のインデックス
        vmove_t *mv = mv0;
        
        const Color myColor = bd.getTurnColor();
        const Color oppColor = flipColor(myColor);
        const int end = bd.getEnd();
        const int turn = bd.getTurn();
        const int rs = bd.getRelScore();
        
        const bool sym = bd.isPerfectSymmetry();
        const bool clean = isCleanBetterScore(myColor, end, rs);
        
        if (turn == TURN_LAST){
            if (bd.countScore() == 0){ // パスでブランクエンドに出来る
                if (gEvaluator.isBlankBetter()){
                    // ブランクエンドが後手1点より良い(後手にとって)場合
                    if (countNInPlayArea(bd, WHITE) == 0){ // 2点以上は取れない
#ifdef USE_NULLMOVE
                        mv0->setPASS();
                        return -1;
#endif
                    }
                }
            }
        }
        
        if (bd.isAlmostNull()){
            // 空場定跡
            if (isBlack(myColor)){ // 先手
                if (clean){ // 守り
                    if (turn == TURN_LAST + 1){
                        mv0->setDraw(static_cast<Spin>(pdice->rand() % 2), DrawPos::S4);
                    }else{
                        mv0->setPASS();
                    }
                    return -1;
                }else{ // 攻め
                    if (turn == TURN_LAST + 1){
                        mv0->setDraw(static_cast<Spin>(pdice->rand() % 2), DrawPos::S4);
                    }else if (turn >= TURN_FREEGUARD){
                        mv0->setDraw(static_cast<Spin>(pdice->rand() % 2), DrawPos::G4);
                    }else{
                        mv0->setDraw(static_cast<Spin>(pdice->rand() % 2), DrawPos::G4);
                    }
                    return -1;
                }
            }
            else{ // 後手
                if (turn == TURN_LAST){
                    if (gEvaluator.isBlankBetter()){
                        mv0->setPASS();
                    }else{
                        mv0->setDraw(static_cast<Spin>(pdice->rand() % 2), DrawPos::TEE);
                    }
                    return -1;
                }else if(turn == TURN_LAST + 2){
                    if(clean){
                        mv0->setPASS();
                    }else{
                        mv0->setDraw(static_cast<Spin>(pdice->rand() % 2), DrawPos::S4);
                    }
                    return -1;
                }
                
                if (clean){ // 守り
                    if (pdice->rand() % 2){
                        mv0->setDraw(Spin::RIGHT, DrawPos::L2);
                    }else{
                        mv0->setDraw(Spin::LEFT, DrawPos::L6);
                    }
                    return -1;
                }else{ // 攻め
                    if (pdice->rand() % 2){
                        mv0->setDraw(Spin::LEFT, DrawPos::G3);
                    }
                    else{
                        mv0->setDraw(Spin::RIGHT, DrawPos::G5);
                    }
                    return -1;
                }
            }
        }
        iterateStoneWithIndex(bd, oppColor, [&, sym](int id, const auto& st)->void{
            if(isInHouse(st)){
                //フリーズを生成
                mv->setFreeze(Spin::RIGHT, id, 0); ++mv;
                if (!sym){
                    mv->setFreeze(Spin::LEFT, id, 0); ++mv;
                }
                
                for (int w = HitWeight::TOUCH; w < HitWeight::MAX; ++w){
                    mv->setHit(Spin::RIGHT, id, w); ++mv;
                    if (!sym){
                        mv->setHit(Spin::LEFT, id, w); ++mv;
                    }
                }
            }else{
                for (int w = HitWeight::WEAK; w < HitWeight::MAX; ++w){
                    mv->setHit(Spin::RIGHT, id, w); ++mv;
                    if (!sym){
                        mv->setHit(Spin::LEFT, id, w); ++mv;
                    }
                }
            }
            if (st.getY() < FY_TEE - FR_HOUSE_RAD / 2){
                // カムアラウンド
                for (int l = ComeAroundLength::MIDDLE; l < ComeAroundLength::MAX; ++l){
                    mv->setComeAround(Spin::RIGHT, id, l); ++mv;
                    if (!sym){
                        mv->setComeAround(Spin::LEFT, id, l); ++mv;
                    }
                }
            }
        });
        iterateStoneWithIndex(bd, myColor, [&, sym, turn](int id, const auto& st)->void{
            // 自分の石があるとき
            if(isInHouse(st)){
                // フリーズを生成
                mv->setFreeze(Spin::RIGHT, id, 0); ++mv;
                if (!sym){
                    mv->setFreeze(Spin::LEFT, id, 0); ++mv;
                }
            }
            
            // プッシュショットを生成
            for (int w = HitWeight::TOUCH; w < HitWeight::MAX; ++w){
                mv->setHit(Spin::RIGHT, id, w); ++mv;
                if (!sym){
                    mv->setHit(Spin::LEFT, id, w); ++mv;
                }
            }
            // カムアラウンドを生成
            if (st.getY() < FY_TEE - FR_HOUSE_RAD / 2){
                for (int l = ComeAroundLength::SHORT; l < ComeAroundLength::MAX; ++l){
                    mv->setComeAround(Spin::RIGHT, id, l); ++mv;
                    if (!sym){
                        mv->setComeAround(Spin::LEFT, id, l); ++mv;
                    }
                }
            }
            if (turn != TURN_LAST && isInHouse(st)){
                // ラストストーンでない時のみ、自分のハウス内の石のガードを生成
                for (int l = 0; l < PostGuardLength::MAX; ++l){
                    mv->setPostGuard(Spin::RIGHT, id, l); ++mv;
                    if (!sym){
                        mv->setPostGuard(Spin::LEFT, id, l); ++mv;
                    }
                }
            }
        });
        // 事前ガードを生成
        mv->setPreGuard(static_cast<Spin>(pdice->rand() % 2), DrawPos::G3); ++mv;
        mv->setPreGuard(static_cast<Spin>(pdice->rand() % 2), DrawPos::G4); ++mv;
        mv->setPreGuard(static_cast<Spin>(pdice->rand() % 2), DrawPos::G5); ++mv;
        
        if (turn == TURN_LAST){
            // ティーショットを生成
            mv->setDraw(Spin::RIGHT, DrawPos::TEE); ++mv;
            if (!sym){
                mv->setDraw(Spin::LEFT, DrawPos::TEE); ++mv;
            }
        }
        return mv - mv0;
    }
    
    /*
     template<class verbalmove_t, class board_t>
     int pushStandard(verbalmove_t *const mv0, const board_t& bd){
     //与えられた局面で検討しておきたい定跡手を配列に入れていく
     verbalmove_t *mv = mv0;
     
     const Color col = bd.getTurnColor();
     const Color oppColor = flipColor(col);
     const bool sym = bd.isSymmetry();
     
     if (countNInHouse(bd, oppColor) != 0){
     int id = oppCol * 8 + 0;
     //No1テイクアウトを生成
     if( bd.order.get(0)==oppCol && bd.order.get(1)==oppCol ){
     //ダブルテイクアウトを狙う事を考える
     //Double::set(mv,id,id+1,Spin::RIGHT);++mv;
     mv->setDouble(Spin::RIGHT,id,id+1);++mv;
     if (!sym){
     //Double::set(mv,id,id+1,Spin::LEFT);++mv;
     mv->setDouble(Spin::LEFT,id,id+1);++mv;
     }
     }else{
     mv->setHit(Spin::RIGHT, id, HitWeight::MIDDLE); ++mv;
     if (!sym){
     mv->setHit(Spin::LEFT, id, HitWeight::MIDDLE); ++mv;
     }
     }
     if (bd.NInHouse[oppCol]>1){
     ++id;
     mv->setHit(Spin::RIGHT, id, HitWeight::MIDDLE); ++mv;
     if (!sym){
     mv->setHit(Spin::RIGHT, id, HitWeight::MIDDLE); ++mv;
     }
     }
     //if( bd.getTurn()==TURN_LAST || col==BLACK ){
     //ティーショットを生成
     //ティーショットを生成
     mv->setDraw(Spin::RIGHT, DrawPos::TEE); ++mv;
     if (!sym){
     mv->setDraw(Spin::LEFT, DrawPos::TEE); ++mv;
     }
     //}else{
     //サイドショットを生成
     //mv->set(Standard::RIGHT_SIDE_SHOT_L); ++mv;
     //if( !sym ){
     //mv->set(Standard::LEFT_SIDE_SHOT_R); ++mv;
     //}
     //}
     }
     else{
     //ティーショットを生成
     mv->setDraw(Spin::RIGHT, DrawPos::TEE); ++mv;
     if (!sym){
     mv->setDraw(Spin::LEFT, DrawPos::TEE); ++mv;
     }
     //サイドショットを生成
     mv->setDraw(Spin::RIGHT, DrawPos::L2); ++mv;
     if (!sym){
     mv->setDraw(Spin::LEFT, DrawPos::L6); ++mv;
     }
     }
     
     return mv - mv0;
     }
     */
    /*
     template<class vrmove_t,class board_t>
     int pushRStandard(vrmove_t *const mv0,const board_t& bd,const int br){
     //与えられた局面で検討しておきたい定跡手を配列に入れていく
     //レンジ手の定跡
     verbalmove_t *mv=mv0;
     
     const int col=bd.getTurnColor();
     const bool sym=bd.isSymmetry();
     
     if( bd.NInHouse[flipColor(col)]!=0 ){
     //No1テイクアウトを生成
     mv->set(bd,br,Standard::HITMIDDLE_opp1_R,16); ++mv;
     if( !sym ){
     mv->set(bd,Standard::HITMIDDLE_opp1_L); ++mv;
     }
     }else{
     //ティーショットを生成
     mv->set(Standard::TEE_SHOT_R); ++mv;
     if( !sym ){
     mv->set(Standard::TEE_SHOT_L); ++mv;
     }
     }
     
     return mv-mv0;
     }
     */
}

#endif //DCURLING_SIMULATION_MOVEGENERATOR_HPP_