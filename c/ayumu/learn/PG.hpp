/*
PG.hpp
Katsuki Ohto
*/

#ifndef DCURLING_LEARN_PG_HPP_
#define DCURLING_LEARN_PG_HPP_

#include "../policy/policy.hpp"
#include "../eval/stat.hpp"
#include "../eval/evaluator.hpp"

#include "../structure/thinkBoard.hpp"

namespace DigitalCurling{
    namespace PG{
        //試合ログから方策勾配法で着手方策関数を棋譜の着手が選ばれやすいように近づける
        
        template<class shotLog_t, class policy_t, class learner_t>
        int learnPlayParamsShot(const shotLog_t& shot,
                                const BitSet32 flags,
                                const policy_t& policy,
                                learner_t *const plearner){
            
            const auto& shotClass = shot.vmv;
            
            MovePolicy buf[1024];
            ThinkBoard bd;
            Evaluator evaluator;
            
            bd.setShotLog(shot);
            const int ph = bd.getTurn();
            
            if(shotClass.isILLEGAL()){
                // 分類出来なかった
                if(flags.test(1)){ // feed feature value
                    plearner->feedUnfoundFeatureValue(ph);
                }
                if(flags.test(2)){ // test
                    plearner->feedObjValue(-1, ph);
                }
                return 0;
            }else{
                evaluator.setRewardInBound(bd.getTurnColor(), bd.getTurnColor(), bd.getRelScore(), bd.getEnd(), bd.getTurn(),
                                            [](Color c, int e, int s, int nrs)->double{
                                                return getEndWP(c, e, s, nrs);
                                            });
                evaluator.normalize([](int s)->double{
                    return dScoreProb_EASY[s + N_COLOR_STONES];
                });
                
                const int NMoves = genChosenVMove(buf, bd); // 手を生成
                
                //cerr << bd.toDebugString();
                //cerr << NMoves << endl;
                
                int idx = -1;
                for(int i = 0; i < NMoves; ++i){
                    //cerr << buf[i] << endl;
                    if(buf[i].data() == shotClass.data()){
                        idx = i; break;
                    }
                }
                
                if(idx == -1){
                    // 分類が生成した手のどれにも当てはまらなかった
                    //cerr << shotClass << " " << shot.player << endl; getchar();
                    if(flags.test(1)){ // feed feature value
                        plearner->feedUnfoundFeatureValue(ph);
                    }
                    if(flags.test(2)){ // test
                        plearner->feedObjValue(idx, ph);
                    }
                    return 0;
                }else{
                    if(NMoves > 0){
                        calcPolicyScore<1>(buf, NMoves, bd, policy);
                        
                        if(flags.test(1)){ // feed feature value
                            plearner->feedFeatureValue(ph);
                        }
                        if(flags.test(0)){ // learn
                            plearner->feedSupervisedActionIndex(idx, ph);
#if 0
                            cerr << policy.plearner_ << endl;
                            cerr << " score_sum = " << plearner->score_sum_ << endl;
                            for(int m = 0; m < NMoves; ++m){
                                cerr << buf[m] << " " << plearner->score_.at(m) / plearner->score_sum_;
                                if(m == idx){ cerr << " <-"; }
                                cerr << endl;
                            }
                            cerr << plearner->toFeatureString() << endl;
                            
                            getchar();
#endif
                            plearner->updateParams();
                            
                            // 選ばれたラベル内での最適化問題を解く
                            
                        }
                        if(flags.test(2)){ // test
                            plearner->feedObjValue(idx, ph);
                        }
                    }
                }
            }
            return 0;
        }
    }
}

#endif // DCURLING_LEARN_PG_HPP_
