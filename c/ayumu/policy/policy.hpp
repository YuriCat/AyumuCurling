/*
 policy.hpp
 Katsuki Ohto
 */


// Digital Curling
// simulation policy for AYUMU

#include "../../dc.hpp"

#ifndef DCURLING_AYUMU_POLICY_HPP_
#define DCURLING_AYUMU_POLICY_HPP_
/*
 template<int N_PARAM_SETS_>
 class NormalPolicy{
 private:
 struct paramSet_t{
 double c;
 int n;
 double k[n][2];
 };
 
 public:
 
 const paramSet_t& ps(int s)const{ return ps_[s]; }
 
 double calc(int s, const double *const value)const{
 double sum = 0, sum2 = 0;
 for(int i = 0; i < ps(s).n; ++i){
 double dist = value[i] - pa(s).k[1];
 sum += ps(s).k[i][0] * dist;
 sum2 += ps(s).k[i][0] * dist * dist;
 }
 return (sum + c) * exp(sum2 + c);
 }
 
 protected:
 struct paramSet_t{
 double c;
 int n;
 double k[n][2];
 };
 
 paramSet_t ps_[N_PARAM_SETS_];
 };
 */

namespace DigitalCurling{
    
#define Foo(i) {s += pol.param(i); pol.template feedFeatureScore<M>((i), 1.0);}
#define FooX(i, x) s += pol.param(i) * (x); pol.template feedFeatureScore<M>((i), (x));}
   
#define FooC(p, i) {ASSERT(POL_IDX(p) <= i && i < POL_IDX((p) + 1),\
    cerr << "FooC() : illegal index " << (i) << " in " << POL_IDX(p) << "~" << (POL_IDX((p) + 1) - 1);\
    cerr << " (pi = " << (p) << ")" << endl; );\
    s += pol.param(i); pol.template feedFeatureScore<M>((i), 1.0);}
    
#define FooXC(p, i, x) {ASSERT(POL_IDX(p) <= i && i < POL_IDX((p) + 1),\
    cerr << "FooXC() : illegal index " << (i) << " in " << POL_IDX(p) << "~" << (POL_IDX((p) + 1) - 1);\
    cerr << " (pi = " << (p) << ")" << endl; );\
    s += pol.param(i) * (x); pol.template feedFeatureScore<M>((i), (x));}
    
    /*
     constexpr int N_PASS_MOVES = 1;
     constexpr int N_DRAW_MOVES = 1 + 1;
     constexpr int N_1S_MOVES = 6 + 1;
     constexpr int N_2S_MOVES = 8;
     
     constexpr int N_PARAMS_CENTRAL = 8;
     constexpr int N_PARAMS_PASS_MOVE = 3;
     constexpr int N_PARAMS_DRAW_MOVE = 16;
     constexpr int N_PARAMS_1S_MOVE = 16;
     constexpr int N_PARAMS_2S_MOVE = 16 * 2 + 16;
     
     constexpr int polTypeNumTableInTurn[] = {
     N_PARAMS_CENTRAL,
     N_PASS_MOVES * N_PARAMS_PASS_MOVE,
     N_DRAW_MOVES * N_PARAMS_DRAW_MOVE,
     N_1S_MOVES * N_PARAMS_1S_MOVE,
     N_2S_MOVES * N_PARAMS_2S_MOVE,
     };
    
    
    constexpr int polMoveNumTableInTurn[] = {
        N_PARAMS_CENTRAL,
        N_PARAMS_PASS_MOVE,
        N_PARAMS_DRAW_MOVE,
    };
    */
    constexpr int N_TURN_INDICES = 6;
    
    constexpr int TtoTI(unsigned int t)noexcept{
        return (t == TURN_LAST ? 0 :
                (t == (TURN_LAST + 1) ? 1 :
                 (!isBlack(getColor(t)) ?
                  (isFreeGuardTurn(t) ? 4 : 2) :
                  (isFreeGuardTurn(t) ? 5 : 3))));
    }
    
    constexpr int N_PARAMS_PER_MOVE_SPIN = 48;
    
    namespace PolicySpace{
        
        enum{
            POL_1STONE,
            POL_2STONES,
            POL_2SMOVE,
            POL_END_SCORE_TO_TYPE, // 現在の盤上の得点と着手タイプ
            POL_2ND_SCORE_TO_TYPE, // 現在の盤上の相手の第2得点と着手タイプ
            POL_ALL,
        };
        
        constexpr int polNumTable[]={
            //N_PARAMS_PER_MOVE_SPIN * 2 * N_STANDARD_MOVES * 6,
            6 * N_STANDARD_MOVES * 16,
            6 * N_STANDARD_MOVES * 2 * 16 * 16,
            6 * N_TYPE_MOVES(MoveType::TWO) * 2 * 17,
            6 * 2 * 5 * 4, // turn index * clean or not * 得点パターン * タイプ
            6 * 2 * 5 * 4, // turn index * clean or not * 得点パターン * タイプ
        };
        
        constexpr int POL_NUM(unsigned int p){
            return polNumTable[p];
        }
        
        constexpr int POL_IDX(unsigned int p){
            return (p == 0) ? 0 : (POL_IDX(p - 1) + POL_NUM(p - 1));
        }
        
        constexpr int POL_NUM_ALL = POL_IDX(POL_ALL);
        
        constexpr int POL_MOVE_IDX(unsigned int m){
            return 2 * N_PARAMS_PER_MOVE_SPIN * m;
        }
        constexpr int POL_NUM_ALL_IN_TURN = POL_MOVE_IDX(N_STANDARD_MOVES);
        
        int commentToPolicyParam(std::ofstream& ofs, const double *const param){
            return 0;
        }
    }
    
    /*
     constexpr int POL_NUM_IN_TURN(unsigned int fea){
     return polNumTableInTurn[fea];
     }
     constexpr int POL_TYPE_IDX(unsigned int ty){
     return (fea == 0) ? 0 : (POL_TYPE_IDX_IN_TURN(fea - 1) + POL_NUM_IN_TURN(fea - 1));
     }
     */
    
    using PlayPolicy = SoftmaxPolicy<PolicySpace::POL_NUM_ALL, 16>;
    using PlayPolicyLearner = SoftmaxPolicyLearner<PlayPolicy>;
    
    int foutComment(const PlayPolicy& pol, const std::string& fName){
        std::ofstream ofs(fName, std::ios::out);
        return PolicySpace::commentToPolicyParam(ofs, pol.param_);
    }
    
    template<int M = 0, class board_t, class move_t, class policy_t>
    void calcPolicyScore(move_t *const buf, const int NMoves, const board_t& bd, const policy_t& pol){
        
        using namespace PolicySpace;
        
        fpn_t s = 0, sum = 0;
        
        pol.template initCalculatingScore<M>(NMoves);
        
        const int turn = bd.getTurn();
        const int turnIndex = TtoTI(turn);
        
        const Color myColor = bd.getTurnColor();
        const Color oppColor = flipColor(myColor);
        const int end = bd.getEnd();
        const int rs = bd.getRelScore();
        const int score = countScore(bd);
        const int opp2ndScore = count2ndScore(bd.orderBits, oppColor);
        const auto sb = bd.sb();
        
        // このターンのパラメータインデックスのベース
        const int baseIndexTurn = turnIndex * POL_NUM_ALL_IN_TURN;
        //cerr << "NMoves = " << NMoves << endl;
        
        const bool clean = isCleanBetterScore(myColor, end, rs);
        
        for(int m = 0; m < NMoves; ++m){
            
            pol.template initCalculatingCandidateScore<M>();
            
            double s = 0;
            move_t& mv = buf[m];
            int type = mv.getType();
            assert(0 <= type && type < N_STANDARD_MOVES);
            
            // 1石
            {
                constexpr int base = POL_IDX(POL_1STONE);
                
                iterate(sb, [&](uint32_t idx)->void{
                    const auto& st = bd.stone(idx);
                    int si = getPositionIndex(st);
                    int i = base + turnIndex * N_STANDARD_MOVES * 16 + si * N_STANDARD_MOVES + type;
                    FooC(POL_1STONE, i);
                });
            }
            
            // 2石
            {
                constexpr int base = POL_IDX(POL_2STONES);
                
                if(type < IDX_TYPE_MOVE(WHOLE)){ // pass
                }else if(type < IDX_TYPE_MOVE(ONE)){ // ドロー手
                    const StoneInfo& dp = drawPositionInfo[type]; // ドロー目標点の情報
                    // ドロー点と全ての石の関係
                    iterate(sb, [&](uint32_t idx){
                        RelativeStoneInfo rel;
                        rel.set(dp, bd.stone(idx));
                        if(rel.rp < 0){ return; }
                        int i = base
                        + turnIndex * N_STANDARD_MOVES * 2 * 16 * 16
                        + type * 2 * 16 * 16
                        + mv.getSpin() * 16 * 16
                        + getPositionIndex(dp) * 16
                        + rel.rp;
                        FooC(POL_2STONES, i);
                    });
                }else if(type < IDX_TYPE_MOVE(TWO)){ // 1石手
                    int idx = mv.getNum0();
                    auto tsb = sb;
                    tsb.reset(idx);
                    // 基準石と他の石の関係
                    iterate(tsb, [&](uint32_t objIndex){
                        const auto& rel = bd.relStone(idx, objIndex);
                        /*if(bd.stone(idx).r < bd.stone(objIndex).r){
                            Foo(baseIndex + 16);
                        }else{
                            Foo(baseIndex + 17);
                        }*/
                        if(rel.rp < 0){ return; }
                        int i = base
                        + turnIndex * N_STANDARD_MOVES * 2 * 16 * 16
                        + type * 2 * 16 * 16
                        + mv.getSpin() * 16 * 16
                        + getPositionIndex(bd.stone(idx)) * 16
                        + rel.rp;
                        FooC(POL_2STONES, i);
                    });
                }else{ // 2石手
                    int idx0 = mv.getNum0(), idx1 = mv.getNum1();
                    auto tsb = sb;
                    tsb.reset(idx0); tsb.reset(idx1);
                    // 第1石と他の石の関係
                    iterate(tsb, [&](uint32_t objIndex){
                        const auto& rel = bd.relStone(idx0, objIndex);
                        if(rel.rp < 0){ return; }
                        int i = base
                        + turnIndex * N_STANDARD_MOVES * 2 * 16 * 16
                        + type * 2 * 16 * 16
                        + mv.getSpin() * 16 * 16
                        + getPositionIndex(bd.stone(idx0)) * 16
                        + rel.rp;
                        FooC(POL_2STONES, i);
                    });
                    
                    // 2個目の石と他の石の関係
                    /*iterate(tsb, [&](uint32_t objIndex){
                     const auto& rel = bd.relStone(idx1, objIndex);
                     if(rel.rp < 0){ return; }
                     Foo(baseIndex + 16 + rel.rp);
                     });*/
                }
            }
            
            // 2石手の石同士の関係
            {
                constexpr int base = POL_IDX(POL_2SMOVE);
                if(type >= IDX_TYPE_MOVE(TWO)){
                    int idx0 = mv.getNum0(), idx1 = mv.getNum1();
                    const auto& rel = bd.relStone(idx0, idx1);
                    int i = base
                    + turnIndex * N_TYPE_MOVES(MoveType::TWO) * 2 * 17
                    + (type - IDX_TYPE_MOVE(TWO)) * 2 * 17
                    + mv.getSpin() * 17
                    + ((rel.rp < 0) ? 16 : rel.rp);
                    FooC(POL_2SMOVE, i);
                }
            }

            // 現在の盤上得点の効果
            {
                constexpr int base = POL_IDX(POL_END_SCORE_TO_TYPE);
                int i = base
                + turnIndex * (2 * 5 * 4)
                + clean * (5 * 4)
                + (2 + max(-2, min(+2, score))) * 4
                + ((type < IDX_TYPE_MOVE(1)) ? 0 : ((type < IDX_TYPE_MOVE(2)) ? 1 : ((type < IDX_TYPE_MOVE(3)) ? 2 : 3)));
                /*
                cerr << (2 + max(-2, min(+2, score))) << endl;
                cerr << ((type < IDX_TYPE_MOVE(1)) ? 0 : ((type < IDX_TYPE_MOVE(2)) ? 1 : ((type < IDX_TYPE_MOVE(3)) ? 2 : 3))) << endl;
                cerr << i << endl;
                cerr << POL_IDX(POL_END_SCORE_TO_TYPE) << endl;
                cerr << POL_IDX(POL_END_SCORE_TO_TYPE + 1) << endl;
                */
                FooC(POL_END_SCORE_TO_TYPE, i);
            }
            
            // 現在の相手の第2得点の効果
            {
                constexpr int base = POL_IDX(POL_2ND_SCORE_TO_TYPE);
                int i = base
                + turnIndex * (2 * 5 * 4)
                + clean * (5 * 4)
                + (2 + max(-2, min(+2, opp2ndScore))) * 4
                + ((type < IDX_TYPE_MOVE(1)) ? 0 : ((type < IDX_TYPE_MOVE(2)) ? 1 : ((type < IDX_TYPE_MOVE(3)) ? 2 : 3)));
                FooC(POL_2ND_SCORE_TO_TYPE, i);
            }
            
            mv.weight = s;
            
            FASSERT(s,);
            double exps = exp(s / pol.temperature());
            FASSERT(exps, cerr << "s = " << s << " T = " << pol.temperature() << endl;);
            
            pol.template feedCandidateScore<M>(exps);
        }
    }
#undef Foo
#undef FooX
#undef FooC
#undef FooXC
    
    template<class move_t, class board_t, class policy_t, class tools_t>
    void playWithPolicy(move_t *const pmv, const board_t& bd, const policy_t& pol, tools_t *const ptools){
        
        MovePolicy buf[64];
        
#ifdef USE_HANDMADE_MOVES
        const int NMoves = genChosenVMove(buf, bd);
#else
        const int NMoves = genSimpleVMove(buf, bd);
#endif
        
        int chosen = 0;
#ifdef USE_POLICY_SCORE
        if(NMoves != 1){
            //DERR << NMoves << " moves." << endl;
            calcPolicyScore<0>(buf, NMoves, bd, pol);
            fpn_t sum = 0;
            for(int i = 0; i < NMoves; ++i){
                buf[i].weight = exp(buf[i].weight / pol.temperature());
                sum += buf[i].weight;
                //DERR << i << ",";
            }
            sum *= ptools->ddice.drand();
            
            for(int i = 0; i < NMoves; ++i){
                sum -= buf[i].weight;
                if(sum <= 0){ chosen = i; break; }
            }
            
            //for(int i = 0; i < NMoves; ++i){
            //DERR << buf[i] << " " << buf[i].weight;
            //if(chosen == i){ DERR << " <-"; }
            //DERR << endl;
            //}
            //getchar();
        }
#else
        chosen = ptools->ddice.rand() % NMoves;
#endif
        //cerr << buf[chosen] << endl;
        realizeMove(pmv, bd, buf[chosen]);
    }
    
    template<class move_t, class board_t, class policy_t, class tools_t>
    void playWithBestPolicy(move_t *const pmv, const board_t& bd, const policy_t& pol, tools_t *const ptools){
        
        std::array<MovePolicy, 64> buf;
        
#ifdef USE_HANDMADE_MOVES
        const int NMoves = genChosenVMove(&buf[0], bd);
#else
        const int NMoves = genSimpleVMove(&buf[0], bd);
#endif
        
        int chosen = 0;
#ifdef USE_POLICY_SCORE
        if(NMoves != 1){
            int bestIndex[NMoves];
            bestIndex[0] = -1;
            int NBestMoves = 0;
            fpn_t bestScore = -99999;
            //DERR << NMoves << " moves." << endl;
            calcPolicyScore<0>(&buf[0], NMoves, bd, pol);
            
            for(int i = 0; i < NMoves; ++i){
                if(buf[i].weight > bestScore){
                    bestScore = buf[i].weight;
                    bestIndex[0] = i;
                    NBestMoves = 1;
                }else if(buf[i].weight == bestScore){
                    bestIndex[NBestMoves] = i;
                    ++NBestMoves;
                }
            }
            
            cerr << NMoves << " moves" << endl;;
            fpn_t probSum = 0;
            for(int i = 0; i < NMoves; ++i){
                probSum += exp(buf[i].weight / pol.temperature());
            }
            std::array<MovePolicy, 64> tmpBuf = buf;
            std::stable_sort(tmpBuf.begin(), tmpBuf.begin() + NMoves, [](const auto& m0, const auto& m1)->bool{
                return m0.weight >= m1.weight;
            });
            for(int i = 0; i < NMoves; ++i){
                fMoveXY<> fmv;
                realizeMove(&fmv, bd, tmpBuf[i]);
                cerr << i << " " << tmpBuf[i] << " " << exp(tmpBuf[i].weight / pol.temperature()) / probSum << " " << fmv << endl;
            }
            
            chosen = (NBestMoves <= 1) ? bestIndex[0] : bestIndex[ptools->dice.rand() % NBestMoves];
        }else{
            chosen = 0;
        }
#else
        chosen = ptools->ddice.rand() % NMoves;
#endif
        realizeMove(pmv, bd, buf[chosen]);
    }
}

#endif // DCURLING_AYUMU_POLICY_HPP_