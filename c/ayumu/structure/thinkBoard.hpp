// デジタルカーリング
// 思考用局面表現

#ifndef DCURLING_AYUMU_STRUCTURE_THINKBOARD_HPP_
#define DCURLING_AYUMU_STRUCTURE_THINKBOARD_HPP_

#include "thinkBoard.h"
#include "../logic/logic.hpp"

namespace DigitalCurling{
    
    //int baseRangeCounter[8192] = {0};
    void printBRC(){
        /*for(int i = 0; i < 8192; ++i){
            cerr << baseRangeCounter[i] << ' ';
            if(countBits((uint32_t)i) == 1){
                cerr << "<" << i << "> ";
            }
        }*/
    }
    void ThinkBoard::clearStoneSequence()noexcept{
        centerInColor[0].clear(); centerInColor[1].clear();
        leftInColor[0].clear(); leftInColor[1].clear();
        frontInColor[0].clear(); frontInColor[1].clear();
        center.clear();
        front.clear();
        left.clear();
    }
    
    // private function
    template<class seq_t, class callback_t>
    void ThinkBoard::insertStoneSequence(uint32_t idx, seq_t& seq, int i, callback_t callback){
        for (int j = 0; j < i; ++j){
            if (callback(stone(idx)) < callback(stone(seq.at(j)))){
                seq.insert(j, idx); return;
            }
        }
        seq.set(i, idx);
    }
    template<class seq_t, class dstseq_t, class callback_t>
    void ThinkBoard::mergeStoneSequence(dstseq_t *const dst,
                                        seq_t seqb, int nb, seq_t seqw, int nw,
                                        const callback_t& callback){
        if (nb == 0){
            *dst = static_cast<dstseq_t>(seqw); return;
        }
        else if (nw == 0){
            *dst = static_cast<dstseq_t>(seqb); return;
        }
        else{
            int cnt = 0;
            while (1){
                if (callback(stone(seqb.at(0))) < callback(stone(seqw.at(0)))){
                    dst->set(cnt, seqb.at(0));
                    --nb;
                    if (nb == 0){
                        dst->set(cnt + 1, seqw); return;
                    }
                    seqb.remove(0);
                }
                else{
                    dst->set(cnt, seqw.at(0));
                    --nw;
                    if (nw == 0){
                        dst->set(cnt + 1, seqb); return;
                    }
                    seqw.remove(0);
                }
                ++cnt;
            }
        }
        UNREACHABLE;
    }
    
    void ThinkBoard::updateBaseRange(){
        // ベースレンジを決定
        iterateStoneWithIndex([this](int i0, auto& st0)->void{
            // 中央にある石、直線上に他の石がある石、他の石と近い石は減点
            if(!isInActiveZone(st0)){ st0.baseRange = 9999; return; } // 意味のなさそうな石
            fpn_t mindtth = 9999, mind2 = 9999;
            iterateExcept(this->sb(), i0, [&, this](int i1)->void{
                const auto& st1 = this->stone(i1);
                fpn_t dtth = fabs(st1.tth - st0.tth);
                mindtth = min(dtth, mindtth);
                fpn_t d2 = calcDistance2(st0, st1);
                mind2 = min(d2, mind2);
            });
            fpn_t score = 0;
            score += 1 * (mindtth + 0.02);
            score += 2 * (sqrt(mind2) + 0.1);
            score += 4 * (st0.r + 0.05);
            
            st0.baseRange = min(8U, bsr((uint32_t)max(1, ((int)score))));
            //baseRangeCounter[int(score)]++;

        });
        
    }
    /*
    void ThinkBoard::firstSetInfo(int mode){
        // 局面セット後最初の情報設定
        // 後から変更することがないパラメータがある場合はここに
        
    }
    */
    // public function
    void ThinkBoard::updateInfo(int mode){
        // 石の状態を一括設定
        
        NInActiveZone_[BLACK] = NInActiveZone_[WHITE] = 0;
        
        if (NInPlayArea() != 0){
            
            // 初期化
            for (int r = 0; r < HASH_RANGE; ++r){ hash_stones[r] = 0ULL; }
            
#if defined(USE_BASE_RANGE)
            updateBaseRange(); // ベースレンジの計算はhash値計算の前の必要あり
#endif
            
            // それぞれの石の状態を演算 & 全体での代表値を同時に計算
            iterateStoneWithIndex([this](uint32_t idx, auto& si)->void{
                
                Color c = getColor(idx);
                if(isInActiveZone(si)){ ++NInActiveZone_[c]; }
                
                //addHash_Pos(hash_stones, idx, FXtoI16X(si.getX()), FYtoI16Y(si.getY()), 0, HASH_RANGE - 1, si.baseRange);
                
                for (int r = 0; r < HASH_RANGE; ++r){
#if defined(USE_BASE_RANGE)
                    hash_stones[r] ^= genHash_Pos(idx, 0, FXtoI16X(si.getX()), r + si.baseRange);
                    hash_stones[r] ^= genHash_Pos(idx, 1, FYtoI16Y(si.getY()), r + si.baseRange);
#else
                    hash_stones[r] ^= genHash_Pos(idx, 0, FXtoI16X(si.getX()), r);
                    hash_stones[r] ^= genHash_Pos(idx, 1, FYtoI16Y(si.getY()), r);
#endif
                }
                
                si.clearRelPosSB();
                si.tailStone_.reset();
                
                iterateExcept(this->sb(), idx, [this, idx, &si](uint32_t idx1)->void{
                    
                    // 相対情報を入れる
                    auto& tsi = this->stone(idx1);
                    
                    this->relStone(idx, idx1).set(si, tsi);
                    
                    if(this->relStone(idx, idx1).rp >= 0){
                        si.setRelPosSB(idx1, this->relStone(idx, idx1).rp);
                    }
                    if(isTailStone(si, tsi)){
                        si.tailStone_.set(idx1);
                    }
                });
            });
            
            // 石順序の作成
            {
                // initialize
                clearStoneSequence();
                
                for (int c = 0; c < 2; ++c){
                    int cnt = 0;
                    iterate(sbColor(static_cast<Color>(c)), [this, c, &cnt](uint32_t idx)->void{
                        insertStoneSequence(idx, centerInColor[c], cnt, [](const stone_t& st)->fpn_t{ return st.getR(); });
                        insertStoneSequence(idx, leftInColor[c], cnt, [](const stone_t& st)->fpn_t{ return st.getX(); });
                        insertStoneSequence(idx, frontInColor[c], cnt, [](const stone_t& st)->fpn_t{ return st.getY(); });
                        ++cnt;
                    });
                }
                
                DERR << "inserted" << endl;
                
                // knit ~InColor arrays by meege-sort
                mergeStoneSequence(&center,
                                   centerInColor[BLACK], NInPlayArea(BLACK),
                                   centerInColor[WHITE], NInPlayArea(WHITE),
                                   [](const stone_t& st)->fpn_t{ return st.getR(); });
                mergeStoneSequence(&left,
                                   leftInColor[BLACK], NInPlayArea(BLACK),
                                   leftInColor[WHITE], NInPlayArea(WHITE),
                                   [](const stone_t& st)->fpn_t{ return st.getX(); });
                mergeStoneSequence(&front,
                                   frontInColor[BLACK], NInPlayArea(BLACK),
                                   frontInColor[WHITE], NInPlayArea(WHITE),
                                   [](const stone_t& st)->fpn_t{ return st.getY(); });
                
                DERR << "merged" << endl;
            }
            if (isFreeGuardTurn(turn)){
                // フリーガードの石を設定
                Color c = getTurnColor();
                iterateStone(c, [](auto& si)->void{
                    if (isInFreeGuardZone(si)){
                        si.state.setInFreeGuardZone();
                    }
                });
                iterateStone(flipColor(c), [](auto& si)->void{
                    if (isInFreeGuardZone(si)){
                        si.state.setInFreeGuardZone();
                        si.state.setFreeGuard();
                    }
                });
            }
            setOrderBits(); // オーダー整理
            hash_orderBits = genHash_OrderBits(orderBits);
            tscore = countScoreByOrderBits(); // 仮得点
        }
        else{
            hash_stones[0] = 0ULL;
            orderBits.reset();
            hash_orderBits = 0ULL;
            tscore = 0; // 仮得点
        }
#if 0
        CERR << NInHouse() << endl;
        CERR << NInPlayArea() << endl;
        CERR << toString() <<endl;
        CERR << orderBits << endl;
        CERR << "tscore = " << tscore << endl;
#endif
        
        assert(exam(mode));
    }
    
    void ThinkBoard::updateDiffInfo_removed(uint32_t idx){
        // 石列処理
        //Color c = getColor(idx);
        //center.remove(numCenter(idx)); centerInColor[c].remove(numCenterInColor(idx));
        //center.remove(numFront(idx)); centerInColor[c].remove(numFrontInColor(idx));
        //center.remove(numLeft(idx)); centerInColor[c].remove(numLeftInColor(idx));
        iterateStone([idx](auto& st)->void{
            st.remove(idx);
        });
        
    }
    
    void ThinkBoard::updateInfoLast(){
        // ラストターンなので、得点さえわかれば良い。tscoreに得点だけ入れて、終了
        fpn_t bmin(FR_IN_HOUSE), wmin(FR_IN_HOUSE);
        iterateStone(BLACK, [&bmin](const auto& st)->void{
            bmin = min(bmin, st.r);
        });
        iterateStone(WHITE, [&wmin](const auto& st)->void{
            wmin = min(wmin, st.r);
        });
        if (bmin == wmin){
            tscore = 0;
        }
        else if (bmin > wmin){
            int cnt = 0;
            iterateStone(WHITE, [bmin, &cnt](const auto& st)->void{
                if (st.r < bmin){ ++cnt; }
            });
            tscore = -cnt;
        }
        else{
            int cnt = 0;
            iterateStone(BLACK, [wmin, &cnt](const auto& st)->void{
                if (st.r < wmin){ ++cnt; }
            });
            tscore = +cnt;
        }
    }
    void ThinkBoard::procTurnPass(){
        // 局面変化なし
        assert(turn > TURN_LAST);
        --turn;
        DERR << "uc." << endl;
        if (turn >= TURN_FREEGUARD - 1){
            Color c = getTurnColor();
            if (turn == TURN_FREEGUARD - 1){
                // フリーガード終了
                iterateStone(c, [](auto& si)->void{ si.state.resetFreeGuard(); });
            }
            else{
                // フリーガード反転
                Color oc = flipColor(c);
                iterateStone(c, [](auto& st)->void{ st.state.resetFreeGuard(); });
                iterateStone(oc, [](auto& st)->void{
                    st.state |= ((uint32_t)st.state & (1U << 2)) << 1;
                });
            }
        }
        // それ以外の操作は必要なし
        assert(exam(0));
    }
    /*void ThinkBoard::procTurnPut(){
        // 石を置いただけ
        assert(turn>TURN_LAST);
        --turn;
        DERR << "put only." << endl;
        
        int c = getTurnColor();
        int oc = flipColor(c);
        int nn = turn + 1;
        
        auto& si = stone(nn);
        
        si.state.init();
        si.state.setIndex(nn);
        
        if (turn >= TURN_FREEGUARD - 1){
            if (turn == TURN_FREEGUARD - 1){
                //フリーガード終了
                iterateStone(c, [](auto& si)->void{
                    si.state.resetFreeGuard();
                });
            }
            else{
                //フリーガード反転
                if (isInFreeGuardZone(si)){
                    //fg_id[nc][NFG[nc]]=nn;
                    //++NFG[nc];
                    si.state.setInFreeGuardZone();
                }
                
                iterateStone(c, [](auto& tsi)->void{
                    tsi.state.resetFreeGuard();
                });
                iterateStone(oc, [](auto& tsi)->void{
                    tsi.state |= ((uint32_t)tsi.state & (1U << 2)) << 1;
                });
                
            }
        }
        if (NInPlayArea() == 1){
            for (int i = 0; i<HASH_RANGE; ++i){ hash_stones[i] = 0ULL; }
        }
        addHash_Pos(hash_stones, nn, FXtoI16X(si.getX()), FYtoI16Y(si.getY()), 0, HASH_RANGE - 1);
        
        orderBits.init();
        setOrderBits();
        
        tscore=countScoreByOrderBits();//仮得点
        
        hash_orderBits = genHash_OrderBits(orderBits);
        assert(exam(1));
    }*/
    void ThinkBoard::procTurnAll(){
        assert(turn > TURN_LAST);
        --turn;
        // 全ての情報を1からまとめる
        updateInfo(2);
    }
    void ThinkBoard::procTurn(int mode){
        procTurnAll();
    }
    
    bool ThinkBoard::exam_end()const{
        if (!(0 <= end && end < N_ENDS)){
            cerr << "ThinkBoard : illegal end." << endl;
            return false;
        }
        return true;
    }
    bool ThinkBoard::exam_turn()const{
        if (!(0 <= turn && turn < N_TURNS)){
            cerr << "ThinkBoard : illegal turn." << endl;
            return false;
        }
        return true;
    }
    bool ThinkBoard::exam_fg()const{
        if (isFreeGuardTurn(turn)){
            /*if( !(0<=NFG[0] && NFG[0]<=2 && 0<=NFG[1] && NFG[1]<=2) ){
             cerr<<"ThinkBoard : illegal the number of free-guard stones."<<endl;
             return false;
             }*/
            if (DigitalCurling::searchStone(*this, [](const auto& si)->bool{
                if (isInFreeGuardZone(si)){
                    /*for(int i=0;i<NFG[c];++i){
                     if( fg_id[c][i]==n ){
                     if( c==getTurnColor() ){
                     if( state[c][n].isFreeGuard() ){
                     cerr<<"ThinkBoard : turn player's free-guard stone("<<c<<n<<") has free-guard flag."<<endl;
                     return false;
                     }
                     }else{
                     if( !state[c][n].isFreeGuard() ){
                     cerr<<"ThinkBoard : ops player's free-guard stone("<<c<<n<<") doesn't have free-guard flag."<<endl;
                     return false;
                     }
                     }
                     break;
                     }
                     if( i==NFG[c]-1 ){
                     cerr<<"ThinkBoard : free-guard stone("<<c<<n<<") is not in free-guard list."<<endl;
                     return false;
                     }
                     }*/
                }
                else{
                    /*for(int i=0;i<NFG[c];++i){
                     if( fg_id[c][i]==n ){
                     cerr<<"ThinkBoard : non free-guard stone("<<c<<n<<") is in free-guard list."<<endl;
                     return false;
                     }
                     }*/
                }
                return false;
            }) >= 0){
                return false;
            }
        }
        return true; // とりあえず適応外のターンの場合は不問としておく
    }
    bool ThinkBoard::exam_nstones(int t)const{ // 石数
        for(int ci = 0; ci < 2; ++ci){
            const Color c = static_cast<Color>(ci);
            // そのターン開始時点での最大個数以内かどうか
            if (!(0 <= NInPlayArea(c) && NInPlayArea(c) <= N_COLOR_STONES - getEndRemStones(c, t))){
                cerr << "ThinkBoard : illegal number of stones (" << toColorString(c) << " : " << NInPlayArea(c);
                cerr << ") in turn " << t << "." << endl;
                return false;
            }
        }
        return true;
    }
    bool ThinkBoard::exam_stone()const{ // 石の情報
        if (searchStoneWithIndex(*this, [](uint32_t idx, const auto& si)->bool{
            /*if (idx != si.state.getIndex()){
                cerr << "ThinkBoard : illegal stone state( "
                << idx << " )." << endl;
                return true;
            }
            else{
                return false;
            }*/
            
            if(!isInPlayArea(si)){
                cerr << "ThinkBoard : out of play-area " << idx << " " << si.toString() << endl;
                return true;
            }
            
            return false;
            
        }) >= 0){
            return false;
        }
        return true;
    }
    bool ThinkBoard::exam_stoneSequence()const{ // 石の順序列
        DERR << "st -seq" << endl;
        // 色ごと
        for (int ci = 0; ci < 2; ++ci){
            const Color c = static_cast<Color>(ci);
            for (int i = 0; i < NInPlayArea(c) - 1; ++i){
                if (isMoreCentral(stoneCenter(c, i + 1), stoneCenter(c, i))){
                    cerr << centerInColor[c][i + 1] << ", " << centerInColor[c][i] << endl;
                    cerr << stoneCenter(c, i + 1).r << ", " << stoneCenter(c, i).r << endl;
                    cerr << "ThinkBoard : illegal stone sequence <" << toColorString(c) << ", center>" << endl;
                    cerr << centerInColor[c] << endl;
                    return false;
                }
            }
            for (int i = 0; i < NInPlayArea(c) - 1; ++i){
                if (isMoreFrontal(stoneFront(c, i + 1), stoneFront(c, i))){
                    cerr << stoneFront(c, i + 1).r << ", " << stoneFront(c, i).r << endl;
                    cerr << "ThinkBoard : illegal stone sequence <" << toColorString(c) << ", front>" << endl;
                    cerr << frontInColor[c] << endl;
                    return false;
                }
            }
            for (int i = 0; i < NInPlayArea(c) - 1; ++i){
                if (isMoreLeft(stoneLeft(c, i + 1), stoneLeft(c, i))){
                    cerr << stoneLeft(c, i + 1).r << ", " << stoneLeft(c, i).r << endl;
                    cerr << "ThinkBoard : illegal stone sequence <" << toColorString(c) << ", left>" << endl;
                    cerr << leftInColor[c] << endl;
                    return false;
                }
            }
        }
        // 全体
        for (int i = 0; i < NInPlayArea() - 1; ++i){
            if (isMoreCentral(stoneCenter(i + 1), stoneCenter(i))){
                cerr << "ThinkBoard : illegal stone sequence <center>" << endl;
                cerr << center << endl;
                return false;
            }
        }
        for (int i = 0; i < NInPlayArea() - 1; ++i){
            if (isMoreFrontal(stoneFront(i + 1), stoneFront(i))){
                cerr << "ThinkBoard : illegal stone sequence <front>" << endl;
                cerr << front << endl;
                return false;
            }
        }
        for (int i = 0; i < NInPlayArea() - 1; ++i){
            if (isMoreLeft(stoneLeft(i + 1), stoneLeft(i))){
                cerr << "ThinkBoard : illegal stone sequence <left>" << endl;
                cerr << left << endl;
                return false;
            }
        }
        DERR << "fi -seq" << endl;
        return true;
    }
    bool ThinkBoard::exam_orderBits()const{ // 順序
        if (NInHouse() == 0){
            if ((uint32_t)orderBits != 0){
                cerr << "ThinkBoard : illegal orderBits (0,0)" << endl;
                for (int i = 0; i < N_STONES; ++i){
                    cerr << (((uint32_t)orderBits >> i) & 1U);
                }
                cerr << endl;
                return false;
            }
        }
        else{
            for (int i = 0; i < NInHouse(); ++i){
                if (getColor(center[i]) != orderBits.get(i)){
                    cerr << "ThinkBoard : illegal orderBits ("
                    << NInHouse(BLACK) << ", " << NInHouse(WHITE)
                    << ")" << endl;
                    for (int i = 0; i < N_STONES; ++i){
                        cerr << (((uint32_t)orderBits >> i) & 1U);
                    }
                    return false;
                }
            }
        }
        return true;
    }
    bool ThinkBoard::exam_hash()const{
        // 石が無い場合はハッシュ値は正しくなくていいので注意
        if (NInPlayArea() > 0){
            for (int r = 0; r < HASH_RANGE; ++r){
                uint64_t hash_tmp = 0ULL;
                DigitalCurling::iterateStoneWithIndex(*this, [&hash_tmp, r](uint32_t idx, const auto& st)->void{
#if defined(USE_BASE_RANGE)
                    hash_tmp ^= genHash_Pos(idx, 0, FXtoI16X(st.getX()), r + st.baseRange);
                    hash_tmp ^= genHash_Pos(idx, 1, FYtoI16Y(st.getY()), r + st.baseRange);
#else
                    hash_tmp ^= genHash_Pos(idx, 0, FXtoI16X(st.getX()), r);
                    hash_tmp ^= genHash_Pos(idx, 1, FYtoI16Y(st.getY()), r);
#endif
                });
                if (hash_stones[r] != hash_tmp){
                    cerr << "ThinkBoard : illegal hash(stones) value( range " << r << " ";
                    cerr << std::hex << hash_stones[r] << " - " << hash_tmp << std::dec << " ) ." << endl;
                    return false;
                }
            }
            if (hash_orderBits != genHash_OrderBits(orderBits)){
                cerr << "ThinkBoard : illegal hash(orderBits) value( ";
                cerr << std::hex << hash_orderBits << " - " << genHash_OrderBits(orderBits) << std::dec << " ) ." << endl;
                return false;
            }
        }
        return true;
    }
    bool ThinkBoard::exam_stone_turn(int t)const{
        // そのターンより後に出てくる石がないことを確認
        if (searchStoneWithIndex(*this, [t](uint32_t idx, const auto& si)->bool{
            if (idx <= t){
                cerr << "ThinkBoard : illegal stone index " << idx << " in turn " << t << "." << endl;
                return true;
            }
            else{
                return false;
            }
        }) >= 0){
            return false;
        }
        return true;
    }
    bool ThinkBoard::exam(int timing)const{
        DERR << "start exam" << endl;
        // 基本データ
        bool bRet = true;
        if (!exam_end()){ bRet = false; }
        if (!exam_turn()){ bRet = false; }
        if (!exam_fg()){ bRet = false; }
        // 石数
        if (!exam_nstones(turn)){ bRet = false; }
        // ターンと存在しうる石
        if (!exam_stone_turn(turn)){ bRet = false; }
        // 石の状態
        if (!exam_stone()){ bRet = false; }
        // 石順序
        if (!exam_stoneSequence()){ bRet = false; }
        // オーダー
        if (!exam_orderBits()){ bRet = false; }
        // ハッシュ値
        if (!exam_hash()){ bRet = false; }
        
        if (!bRet){
            switch (timing){
                case 0: cerr << "( after throw )" << endl; break;
                case 1: cerr << "( after put )" << endl; break;
                case 2: cerr << "( after set )" << endl; break;
                case 3: cerr << "( after first set )" << endl; break;
                default: break;
            }
            cerr << toDebugString();
        }
        DERR << "finish exam" << endl;
        return bRet;
    }
    
    std::string ThinkBoard::toString()const{
        std::ostringstream oss;
        oss << "B";
        DigitalCurling::iterateStoneWithIndex(*this, BLACK, [&oss](uint32_t idx, const auto& si)->void{
            oss << ' ' << idx << si.toString();
        });
        oss << "W";
        DigitalCurling::iterateStoneWithIndex(*this, WHITE, [&oss](uint32_t idx, const auto& si)->void{
            oss << ' ' << idx << si.toString();
        });
        return oss.str();
    }
    
    std::string ThinkBoard::toDebugString()const{
        std::ostringstream oss;
        oss << "end = " << getEnd() << "  turn = " << getTurn() << " ( ";
        oss << toColorString(getTurnColor()) << "'s turn ) rscore : " << rscore << endl;
        oss << "NInPlayArea = " << "B : " << NInPlayArea(BLACK) << " W : " << NInPlayArea(WHITE);
        oss << "  NInHouse = " << "B : " << NInHouse(BLACK) << " W : " << NInHouse(WHITE) << endl;
        oss << "temporal score = " << tscore << endl;
        oss << toString() << endl;
        return oss.str();
    }
    
    template<int RINK_ONLY, class move_t>
    ContactTree makeMoveNoRand(ThinkBoard *const pbd, uint32_t idx, const move_t& mv){
        // 非合法な手を合法な手に変える
        /*fMoveXY<> tmp;
        tmp.s = mv.getSpin();
        fpn_t v2 = mv.v2();
        if (v2 >= FV2_MAX){
            fpn_t v = sqrt(v2);
            tmp.x = mv.vx() * ((FV_MAX - 0.001) / v);
            tmp.y = mv.vy() * ((FV_MAX - 0.001) / v);
        }else{
            tmp.x = mv.vx(); tmp.y = mv.vy();
        }
        ASSERT(tmp.v2() <= FV2_MAX, cerr << "Invalid Move : " << v2 << " -> " << tmp.v2()
               << " in " << FV2_MAX << " " << tmp << endl;);
        return FastSimulator::simulateF<RINK_ONLY>(pbd, idx, tmp);*/
        return FastSimulator::simulateF<RINK_ONLY>(pbd, idx, mv);
    }
    template<int RINK_ONLY, class move_t>
    ContactTree makeMoveNoRand(ThinkBoard *const pbd, const move_t& mv){
        return makeMoveNoRand<RINK_ONLY>(pbd, pbd->getTurn(), mv);
    }
    
    template<int RINK_ONLY, class dice_t>
    ContactTree makeMove(ThinkBoard *const pbd, uint32_t stNum, const fMoveXY<>& mv, dice_t *const pdice){
        // 石を投げて、盤面を更新する(連続直交ベクトル)
        ASSERT(mv.vy() <= FVY_LEGAL_MAX, cerr << "Invalid Move : " << mv.vy()
               << " in " << FVY_LEGAL_MAX << " " << mv << endl;);
        fMoveXY<> tmp;
        addRandToMove(&tmp, mv, pdice); // 乱数をかける
        return makeMoveNoRand<RINK_ONLY>(pbd, stNum, tmp);
    }
    template<int RINK_ONLY, class dice_t>
    ContactTree makeMove(ThinkBoard *const pbd, const fMoveXY<>& mv, dice_t *const pdice){
        // 石を投げて、盤面を更新する(連続直交ベクトル)
        return makeMove<RINK_ONLY>(pbd, pbd->getTurn(), mv, pdice);
    }

    // alpha, beta score
    int calcStrictAlphaScore(Color col, const ThinkBoard& bd){
        if (isBlack(col)){
            return -getEndRemStones(WHITE, bd.getTurn()) - countNInPlayArea(bd, WHITE);
        }
        else{
            return +getEndRemStones(BLACK, bd.getTurn()) + countNInPlayArea(bd, BLACK);
        }
    }
    int calcStrictBetaScore(Color col, const ThinkBoard& bd){
        if (isBlack(col)){
            return +getEndRemStones(BLACK, bd.getTurn()) + countNInPlayArea(bd, BLACK);
        }
        else{
            return -getEndRemStones(WHITE, bd.getTurn()) - countNInPlayArea(bd, WHITE);
        }
    }
    
    template<class board_t,class evaluator_t>
    eval_t calcStrictAlpha(Color col, const board_t& bd, evaluator_t evaluator){
        return min(evaluator.evalLast(col, 0), evaluator.evalLast(col, calcStrictAlphaScore(col, bd)));
    }
    template<class board_t, class evaluator_t>
    eval_t calcStrictBeta(Color col, const board_t& bd, evaluator_t evaluator){
        return max(evaluator.evalLast(col, 0), evaluator.evalLast(col, calcStrictBetaScore(col, bd)));
    }
    

}

#endif // DCURLING_AYUMU_STRUCTURE_THINKBOARD_HPP_