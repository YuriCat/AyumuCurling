// デジタルカーリング
// 思考用局面表現

#ifndef DCURLING_AYUMU_STRUCTURE_THINKBOARD_H_
#define DCURLING_AYUMU_STRUCTURE_THINKBOARD_H_

#include "../../dc.hpp"
#include "../ayumu_dc.hpp"

#include "stoneInfo.hpp"

namespace DigitalCurling{
   
    class ThinkBoard{
        //　思考用の局面表現
    public:
        using stone_t = StoneInfo;
        using relStone_t = RelativeStoneInfo;
        using sb_t = BitSet32;
        
        // 石へのアクセス
        const stone_t& stone(uint32_t idx)const{ assert(examIndex(idx)); return stone_[idx]; }
        stone_t& stone(uint32_t idx){ assert(examIndex(idx)); return stone_[idx]; }
        
        // 石一つのとき
        stone_t& onlyStone()noexcept{ return stone(sb().bsf()); }
        stone_t& onlyStone(Color c){ return stone(sbColor(c).bsf()); }
        
        const stone_t& onlyStone()const noexcept{ return stone(sb().bsf()); }
        const stone_t& onlyStone(Color c)const{ return stone(sbColor(c).bsf()); }
        
        // ハウス中心から
        const stone_t& stoneCenter(int n)const{ return stone(center.at(n)); }
        stone_t& stoneCenter(int n){ return stone(center.at(n)); }
        const stone_t& stoneCenter(Color c, int n)const{ return stone(centerInColor[c].at(n)); }
        stone_t& stoneCenter(Color c, int n){ return stone(centerInColor[c].at(n)); }
        
        // 手前から
        const stone_t& stoneFront(int n)const{ return stone(front.at(n)); }
        stone_t& stoneFront(int n){ return stone(front.at(n)); }
        const stone_t& stoneFront(Color c, int n)const{ return stone(frontInColor[c].at(n)); }
        stone_t& stoneFront(Color c, int n){ return stone(frontInColor[c].at(n)); }

        // 左から
        const stone_t& stoneLeft(int n)const{ return stone(left.at(n)); }
        stone_t& stoneLeft(int n){ return stone(left.at(n)); }
        const stone_t& stoneLeft(Color c, int n)const{ return stone(leftInColor[c].at(n)); }
        stone_t& stoneLeft(Color c, int n){ return stone(leftInColor[c].at(n)); }
        
        // 石の存在ビット
        sb_t sb()const noexcept{ return sb_; }
        sb_t sbColor(Color c)const{ return sb_t(sb_ & (0x5555 << c)); }
        sb_t sbSpace()const noexcept{ return sb_t(0xffff - sb_); }
        sb_t sbColorSpace(Color c)const{ return sb_t(sbSpace() & (0x5555 << c)); }
        
        // 石数
        int NInHouse(Color c)const{ assert(examColor(c)); return NInHouse_[c]; }
        int NInPlayArea(Color c)const{ assert(examColor(c)); return NInPlayArea_[c]; }
        int NInActiveZone(Color c)const{ assert(examColor(c)); return NInActiveZone_[c]; }
        
        int NInHouse()const noexcept{ return NInHouse(BLACK) + NInHouse(WHITE); }
        int NInPlayArea()const noexcept{ return NInPlayArea(BLACK) + NInPlayArea(WHITE); }
        int NInActiveZone()const noexcept{ return NInActiveZone(BLACK) + NInActiveZone(WHITE); }

        // 相対石情報
        const relStone_t& relStone(uint32_t i0, uint32_t i1)const{ return relStone_[i0][i1]; }
        relStone_t& relStone(uint32_t i0, uint32_t i1){ return relStone_[i0][i1]; }
        
        StoneState state(uint32_t idx)const{
            StoneState state;
            state.init();
            state.setIndex(idx);
            //if(isInFreeGuardZone(stone(idx).x, stone(idx).y)){
            //    state.setInFreeGuardZone();
            //}
            return state;
        }

        void init()noexcept{
            sb_.reset();
            for (int c = 0; c < 2; ++c){
                NInPlayArea_[c] = 0;
                NInHouse_[c] = 0;
                NInActiveZone_[c] = 0;
                
                centerInColor[c].clear();
                leftInColor[c].clear();
                frontInColor[c].clear();
            }
            memset(stone_, 0, sizeof(stone_));
            
            end = 0; turn = 0; tscore = 0; rscore = 0;
            orderBits.reset();
            
            center.clear();
            left.clear();
            front.clear();
            
            memset(hash_stones, 0, sizeof(hash_stones));
            hash_orderBits = 0ULL;
        }
        void initBoard()noexcept{
            sb_.reset();
            for (int c = 0; c < 2; ++c){
                NInPlayArea_[c] = 0;
                NInHouse_[c] = 0;
                NInActiveZone_[c] = 0;
                
                centerInColor[c].clear();
                leftInColor[c].clear();
                frontInColor[c].clear();
            }
            memset(stone_, 0, sizeof(stone_));
            
            tscore = 0;
            orderBits.reset();
            
            center.clear();
            left.clear();
            front.clear();
            
            memset(hash_stones, 0, sizeof(hash_stones));
            hash_orderBits = 0ULL;
        }
        void setOrderBits(){
            const int nw = NInHouse(WHITE);
            const int nb = NInHouse(BLACK);
            orderBits.reset();
            if (nw == 0){
                if (nb != 0){
                    orderBits.fill(0, nb - 1);
                }
            }
            else{
                if (nb == 0){
                    orderBits.fill_through_back(nw);
                }
                else{
                    int i(0), n(nw + nb - 1);
                    do{
                        if (isBlack(getColor(center.at(i)))){
                            orderBits.set(i);
                        }
                    } while (i++ < n);
                    if (isWhite(getColor(center.at(n)))){
                        orderBits.fill_through_back(n + 1);
                    }
                }
            }
            assert(exam_orderBits());
        }
        void updateInfo(int mode = 3);
        void updateInfoLast();
        void updateDiffInfo_removed(uint32_t idx);
        void updateDiffInfo_relocated(uint32_t idx);
        void updateDiffInfo_new(uint32_t idx);
        
        void updateBaseRange();
        
        template<class clientField_t>
        void setClientField(const clientField_t& cf){
            // ClientFieldの情報(公式のデータ型に近い)を
            // 自分の思考用のデータに変換する
            init();
            
            end = cf.to_end - 1 - cf.end; // エンド数は最終を0とする
            turn = cf.to_turn - 1 - cf.turn; // ターン数は最終を0とする
            rscore = (cf.color[0] == 0) ? cf.getRelScore() : (-cf.getRelScore());

            // 先手の石
            for (int i = 0; i < N_COLOR_STONES; ++i){
                fPosXY<> mypos = convPosition_Official_Ayumu(fPosXY<>(cf.pos[i][0][0], cf.pos[i][0][1]));
                if (isInPlayArea(mypos)){
                    locateNewStone(TURN_BLACK_LAST + N_STONES - (i + 1) * 2, mypos);
                }
            }
            // 後手の石
            for (int i = 0; i < N_COLOR_STONES; ++i){
                fPosXY<> mypos = convPosition_Official_Ayumu(fPosXY<>(cf.pos[i][1][0], cf.pos[i][1][1]));
                if (isInPlayArea(mypos)){
                    locateNewStone(TURN_WHITE_LAST + N_STONES - (i + 1) * 2, mypos);
                }
            }
            updateInfo(3);
        }
        template<class shotLog_t>
        void setShotLog(const shotLog_t& log){
            init();
            end = log.end; turn = log.turn; rscore = log.rscore;
            for(int t = 0; t < N_TURNS; ++t){
                if(isInPlayArea(log.previousStone[t])){
                    locateNewStone(N_TURNS - 1 - t, log.previousStone[t]);
                }
            }
            updateInfo(3);
        }
        
        void set(const ThinkBoard& atb){ *this = atb; }
        void set(const MiniBoard& amb);
        void setEnd(int e)noexcept{ end = e; }
        void setTurn(int t)noexcept{ turn  = t; }
        
        // game procision
        void procTurn(int changed = 2);
        void procTurnPass();
        void procTurnPut();
        void procTurnAll();
        void procTurnLast();
        
        // accessor
        int getTurn()const noexcept{ return turn; }
        int getEnd()const noexcept{ return end; }
        int getNRemainedTurns()const noexcept{ return turn; }
        int getNRemainedEnds()const noexcept{ return end; }
        int getRelScore()const noexcept{ return rscore; }
        Color getTurnColor()const noexcept{ return getColor(getTurn()); }
        int countScore()const noexcept{ return tscore; }
        uint64_t getHash(int r)const{
            return hash_stones[r] ^ hash_orderBits ^ getTurn();
        }
        
        // game phase
        bool isNull()const noexcept{ return !NInPlayArea(); }
        bool isAlmostNull()const noexcept{ return !NInActiveZone(); }
        bool hasAdvantage(Color c)const noexcept{
            return isBlack(c) ? (getRelScore() > 0) : (getRelScore() < 0);
        }
        bool isTie()const noexcept{ // 同点
            return (getRelScore() == 0);
        }
        bool isFinalTurn()const noexcept{ // ゲームの最終投
            return ((getTurn() == TURN_LAST) && (getEnd() == END_LAST));
        }
        
        template<class pos_t>
        FORCE_INLINE void pushStone(Color c, const pos_t& pos){
            sb_t tsb = sbColorSpace(c);
            ASSERT(tsb.any(),); // 石を置くことが可能
            uint32_t idx = tsb.bsr();
            
            stone(idx).set(pos);
            stone(idx).state.setIndex(idx);
            
            sb_.set(idx);
            if(isInHouse(stone(idx))){
                ++NInHouse_[c];
            }
            ++NInPlayArea_[c];
        }
        template<class pos_t>
        FORCE_INLINE void locateStone(uint32_t idx, const pos_t& pos){
            Color c = getColor(idx);
            if(sb().test(idx)){
                if(isInHouse(stone(idx))){
                    --NInHouse_[c];
                }
            }else{
                stone(idx).state.setIndex(idx);
                sb_.set(idx);
                ++NInPlayArea_[c];
            }
            stone(idx).set(pos);
            if(isInHouse(stone(idx))){
                ++NInHouse_[c];
            }
        }
        template<class pos_t>
        FORCE_INLINE void locateNewStone(uint32_t idx, const pos_t& pos){
            Color c = getColor(idx);
            ASSERT(!sb().test(idx), cerr << "index = " << idx << endl;);
            sb_.set(idx);
            stone(idx).set(pos);
            stone(idx).state.setIndex(idx);
            if(isInHouse(stone(idx))){
                ++NInHouse_[c];
            }
            ++NInPlayArea_[c];
        }
        template<class pos_t>
        FORCE_INLINE void relocateStone(uint32_t idx, const pos_t& pos){
            ASSERT(sb().test(idx), cerr << "index = " << idx << endl;);
            Color c = getColor(idx);
            if(isInHouse(stone(idx))){
                --NInHouse_[c];
            }
            stone(idx).set(pos);
            if(isInHouse(stone(idx))){
                ++NInHouse_[c];
            }
        }
        FORCE_INLINE void removeStone(uint32_t idx){
            ASSERT(sb().test(idx), cerr << "index = " << idx << endl;);
            Color c = getColor(idx);
            if(isInHouse(stone(idx))){
                --NInHouse_[c];
            }
            --NInPlayArea_[c];
            sb_.reset(idx);
        }
        FORCE_INLINE void clearStones()noexcept{
            sb_.reset();
            NInHouse_[BLACK] = NInHouse_[WHITE] = 0;
            NInPlayArea_[BLACK] = NInPlayArea_[WHITE] = 0;
        }
        
        void setBiRelStone(int idx0, int idx1){
            // 石同士の相対的な情報を両側まとめて設定
            const auto& st0 = stone(idx0);
            const auto& st1 = stone(idx1);
            
            fpn_t r = calcDistance(st0, st1);
            fpn_t th = calcRelativeAngle(st0, st1);
            relStone_[idx0][idx1].r = relStone_[idx1][idx0].r = r;
            relStone_[idx0][idx1].th = th;
            relStone_[idx1][idx0].th = fmod(th + M_PI, 2 * M_PI) - M_PI;
            
            const int orp01 = relStone_[idx0][idx1].rp;
            const int orp10 = relStone_[idx1][idx0].rp;
            
            /*if(orp01 >= 0){
                stone(idx0).resetRelPosSB(idx1, relStone_[idx0][idx1].rp);
            }
            if(orp10 >= 0){
                stone(idx1).resetRelPosSB(idx0, relStone_[idx1][idx0].rp);
            }
            stone(idx0).tailStone_.reset(idx1);
            stone(idx1).tailStone_.reset(idx0);*/
            
            const int nrp01 = labelRelPos(st0, st1, relStone_[idx0][idx1]);
            const int nrp10 = labelRelPos(st1, st0, relStone_[idx1][idx0]);
            
            relStone_[idx0][idx1].rp = nrp01;
            relStone_[idx1][idx0].rp = nrp10;
            
            /*if(nrp01 >= 0){
                stone(idx0).setRelPosSB(idx1, nrp01);
            }
            if(nrp10 >= 0){
                stone(idx1).setRelPosSB(idx0, nrp10);
            }
            if(isTailStone(st0, st1)){
                stone(idx0).tailStone_.set(idx1);
            }
            if(isTailStone(st1, st0)){
                stone(idx1).tailStone_.set(idx0);
            }*/
        }
        void resetBiRelStone(int idx0, int idx1){
            // 片方の石がremoveされた時に
        }
        
        template<class simuBoard_t>
        FORCE_INLINE void locateDiffStones(const simuBoard_t& sbd)noexcept{
            // 取り除く石
            itrateRemovedStones(sbd, [this](int idx, const auto& st)->void{
                removeStone(idx);
                // 石の相対位置ビットを外す
                // 片方だけが取り除かれる石の関係を整理する
                
            });
            // 移動した石
            itrateRelocatedStones(sbd, [this](int idx0, const auto& st)->void{
                this->relocateStone(idx0, st);
                // 石の相対位置を再計算
                iterateExcept(this->sb(), idx0, [this, idx0](int idx1)->void{
                    this->setBiRelStone(idx0, idx1);
                });
            });
            // 追加された石
            int added = sbd.addedStoneIndex();
            if(added >= 0){
                auto osb = this->sb();
                this->locateNewStone(added, sbd.addedStone());
                // 石の相対位置を再計算
                iterate(osb, [this, added](int idx1)->void{
                    this->setBiRelStone(added, idx1);
                });
            }
        }
        
        int count2ndScore(Color col)const{
            assert(examColor(col));
            // colの第2ストーン以内にあるops_colの石数+1を返す
            const Color opsCol = flipColor(col);
            if (NInHouse(col) <= 1){ // 第2ストーンまでハウス内にない
                return NInHouse(opsCol) + 1; // 相手のハウス内の全て+1
            }
            else{ // 第2ストーンがハウス内にある
                const fpn_t da = stoneCenter(col, 1).getR(); // 第2ストーンの距離
                int i = 0;
                for (; i < NInHouse(opsCol); ++i){
                    if (stoneCenter(opsCol, i).getR() > da){ break; }
                }
                return i + 1;
            }
        }
        bool isSymmetry()const noexcept{ // Symmetry or Not
            if (NInPlayArea() == 0){
                return true; // 石が無いので対称
            }
            return false; // TODO: それ以外は未実装
        }
        bool isPerfectSymmetry()const noexcept{
            // 完全に左右対称かどうか
            // 石がないときだけ
            return (NInPlayArea() == 0);
        }
        
        bool exam_end()const;
        bool exam_turn()const;
        bool exam_fg()const;
        bool exam_nstones(int t = TURN_LAST)const;
        bool exam_stone_turn(int t = TURN_LAST)const;
        bool exam_stone()const;
        bool exam_stoneSequence()const;
        bool exam_orderBits()const;
        bool exam_hash()const;
        bool exam(int timing = 3)const;
        
        std::string toString()const;
        std::string toDebugString()const;
        
    private:

        // iteration
        template<typename callback_t>
        void iterateStone(const callback_t& callback)noexcept{
            iterate(sb(), [this, callback](uint32_t idx)->void{
                callback(stone_[idx]);
            });
        }
        template<typename callback_t>
        void iterateStone(Color c, const callback_t& callback){
            iterate(sbColor(c), [this, callback](uint32_t idx)->void{
                callback(stone_[idx]);
            });
        }
        template<typename callback_t>
        void iterateStoneWithIndex(const callback_t& callback)noexcept{
            iterate(sb(), [this, callback](uint32_t idx)->void{
                callback(idx, stone_[idx]);
            });
        }
        template<typename callback_t>
        void iterateStoneWithIndex(Color c, const callback_t& callback){
            iterate(sbColor(c), [this, callback](uint32_t idx)->void{
                callback(idx, stone_[idx]);
            });
        }
        
        // board state calculation
        int countScoreByOrderBits()const noexcept{
            assert(exam_orderBits());
            return DigitalCurling::countScore(orderBits);
        }
        
        template<class seq_t, typename callback_t>
        void insertStoneSequence(uint32_t idx, seq_t& seq, int i, callback_t callback);
        
        template<class seq_t, class dstseq_t, typename callback_t>
        void mergeStoneSequence(dstseq_t *const dst, seq_t seqb, int nb, seq_t seqw, int nw, const callback_t& callback);
        
        void clearStoneSequence()noexcept;
        
    public:
        // variables
        stone_t stone_[N_STONES]; // 石の情報
        relStone_t relStone_[N_STONES][N_STONES]; // 石の相対的な値
        
        // 石の存在
        BitSet32 sb_;
        
        //BitField occupied; // 石がある場所
        //BitField guarded[2]; // ガードが効いている場所
        
        int NInHouse_[2], NInPlayArea_[2]; // Number of stones in House or Play-Area
        int NInActiveZone_[2];
        
        int end, turn;
        int rscore; // このエンドの先手から見た累積得点の差
        int tscore; // 現在の石の配置で終了した場合の、このエンドのみの得点
        
        // 石順序
        BitArray64<4, 16> center;
        BitArray32<4, 8> centerInColor[2];
        
        BitArray64<4, 16> front;
        BitArray32<4, 8> frontInColor[2];
        
        BitArray64<4, 16> left;
        BitArray64<4, 8> leftInColor[2];
        // 石集合
        BitSet32 doubleRollChance, guard;
        
        OrderBits orderBits; // ティー中心からの石順序
        
        // Hash value
        static constexpr int HASH_RANGE = 16;
        uint64_t hash_stones[HASH_RANGE]; // 局面認識ハッシュ値の石成分(レンジ幅分)
        uint64_t hash_orderBits; // 局面認識ハッシュ値のオーダー成分
        
        uint32_t ownerTurn_;
    };
    
    std::ostream& operator<<(std::ostream& os, const ThinkBoard& bd){
        os << bd.toString();
        return os;
    }
    
    template<>int countNInHouse<ThinkBoard>(const ThinkBoard& bd, Color c){ return bd.NInHouse(c); }
    template<>int countNInHouse<ThinkBoard>(const ThinkBoard& bd)noexcept{ return bd.NInHouse(); }
    template<>int countNInPlayArea<ThinkBoard>(const ThinkBoard& bd, Color c){ return bd.NInPlayArea(c); }
    template<>int countNInPlayArea<ThinkBoard>(const ThinkBoard& bd)noexcept{ return bd.NInPlayArea(); }
    template<>int countNInActiveZone<ThinkBoard>(const ThinkBoard& bd, Color c){ return bd.NInActiveZone(c); }
    template<>int countNInActiveZone<ThinkBoard>(const ThinkBoard& bd)noexcept{ return bd.NInActiveZone(); }

    template<int RINK_ONLY = 1, class move_t>
    ContactTree makeMoveNoRand(ThinkBoard *const, uint32_t, const move_t&);
    template<int RINK_ONLY = 1, class move_t>
    ContactTree makeMoveNoRand(ThinkBoard *const, const move_t&);
    
    template<int RINK_ONLY = 1, class dice_t>
    ContactTree makeMove(ThinkBoard *const, uint32_t, const fMoveXY<>& mv, dice_t *const);
    template<int RINK_ONLY = 1, class dice_t>
    ContactTree makeMove(ThinkBoard *const, const fMoveXY<>&, dice_t *const);
    template<int RINK_ONLY = 1, class dice_t>
    ContactTree makeMove(ThinkBoard *const , const MoveXY&, dice_t *const);

    // iteration by stone array (when no ZERO stone)
    template<typename callback_t>
    void iterateStoneCenter(const ThinkBoard& bd, const callback_t& callback)noexcept{
        iterateAny(bd.center, [&](uint32_t idx)->void{
            callback(bd.stone(idx));
        });
    }
    template<typename callback_t>
    void iterateStoneCenter(const ThinkBoard& bd, Color c, const callback_t& callback){
        iterateAny(bd.centerInColor[c], [&](uint32_t idx)->void{
            callback(bd.stone(idx));
        });
    }
    template<typename callback_t>
    void iterateStoneFront(const ThinkBoard& bd, const callback_t& callback)noexcept{
        iterateAny(bd.front, [&](uint32_t idx)->void{
            callback(bd.stone(idx));
        });
    }
    template<typename callback_t>
    void iterateStoneFront(const ThinkBoard& bd, Color c, const callback_t& callback){
        iterateAny(bd.frontInColor[c], [&](uint32_t idx)->void{
            callback(bd.stone(idx));
        });
    }
    template<typename callback_t>
    void iterateStoneLeft(const ThinkBoard& bd, const callback_t& callback)noexcept{
        iterateAny(bd.left, [&](uint32_t idx)->void{
            callback(bd.stone(idx));
        });
    }
    template<typename callback_t>
    void iterateStoneLeft(const ThinkBoard& bd, Color c, const callback_t& callback){
        iterateAny(bd.leftInColor[c], [&](uint32_t idx)->void{
            callback(bd.stone(idx));
        });
    }

    Color searchNo1Color(const ThinkBoard& bd)noexcept{
        return Color(bd.orderBits.get(0));
    }
    int searchNo1Index(const ThinkBoard& bd)noexcept{
        return countNInHouse(bd) ? bd.center[0] : (-1);
    }
    int searchNo1Index(const ThinkBoard& bd, Color c){
        return countNInHouse(bd, c) ? bd.centerInColor[c][0] : (-1);
    }
    
    int countScore(const ThinkBoard& bd, Color c = BLACK){
        return isBlack(c) ? bd.tscore : (-bd.tscore);
    }

    void copyBeforePlayout(const ThinkBoard& arg, ThinkBoard *const dst)noexcept{
        // 毎回のプレイアウトで初期化する必要のある情報のみをコピーする
        dst->sb_ = arg.sb_;
        
        iterateStoneWithIndex(arg, [&arg, dst](uint32_t idx, const auto& st)->void{
            dst->stone_[idx] = st;
            iterateExcept(arg.sb(), idx, [&arg, dst, idx](uint32_t idx1)->void{
                dst->relStone_[idx][idx1] = arg.relStone_[idx][idx1];
            });
        });

        // dst->end = arg.end; // 変化しない
        dst->turn = arg.turn;
        // dst->rscore = arg.rscore; // 変化しない
        dst->tscore = arg.tscore;
        
        for (int c = 0; c < 2; ++c){
            dst->NInHouse_[c] = arg.NInHouse_[c];
            dst->NInPlayArea_[c] = arg.NInPlayArea_[c];
            dst->NInActiveZone_[c] = arg.NInActiveZone_[c];
            
            dst->centerInColor[c] = arg.centerInColor[c];
            dst->leftInColor[c] = arg.leftInColor[c];
            dst->frontInColor[c] = arg.frontInColor[c];
        }
        
        dst->center = arg.center;
        dst->left = arg.left;
        dst->front = arg.front;
        
        // 石集合
        dst->doubleRollChance = arg.doubleRollChance;
        dst->guard = arg.guard;
        
        dst->orderBits = arg.orderBits;
        
        for (int r = 0; r < ThinkBoard::HASH_RANGE; ++r){
            dst->hash_stones[r] = arg.hash_stones[r];
        }
        dst->hash_orderBits = arg.hash_orderBits;
    }
}

#endif // DCURLING_AYUMU_STRUCTURE_THINKBOARD_H_