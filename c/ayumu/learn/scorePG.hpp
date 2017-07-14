/*
 scorePG.hpp
 Katsuki Ohto
 */

#include "../eval/estimator.hpp"

#include "../structure/thinkBoard.hpp"

namespace DigitalCurling{
    namespace PG{
        // 試合ログから方策勾配を用いた教師有り学習でエンドの最終得点予測パラメータ
        template<class shotLog_t, class estimator_t, class learner_t>
        int learnEvalParamsShot(const shotLog_t& shot,
                            const BitSet32 flags,
                            const estimator_t& estimator,
                            learner_t *const plearner){
            
            double score[SCORE_LENGTH];
            double scoreSum;
            
            ThinkBoard bd;
            
            bd.setShotLog(shot);
            
            calcEvalScore<1>(score, &scoreSum, bd, estimator);
            
            int idx = StoIDX(shot.escore);
            
            const int ph = bd.getTurn();
            
            if(flags.test(1)){ // feed feature value
                plearner->feedFeatureValue(ph);
            }
            if(flags.test(0)){ // learn
                plearner->feedSupervisedActionIndex(idx, ph);
#if 0
                cerr << estimator.plearner_ << endl;
                cerr << " score_sum = " << plearner->score_sum_ << endl;
                for(int m = 0; m < SCORE_LENGTH; ++m){
                    cerr << IDXtoS(m) << " " << plearner->score_.at(m) / plearner->score_sum_;
                    if(m == idx){ cerr << " <-"; }
                    cerr << endl;
                }
                cerr << plearner->toFeatureString() << endl;
                
                //getchar();
#endif
                plearner->updateParams();
            }
            if(flags.test(2)){ // test
                plearner->feedObjValue(idx, ph);
            }
            return 0;
        }
    }
}
