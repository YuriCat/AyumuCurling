/*
 estimator.hpp
 Katsuki Ohto
 */

// Digital Curling
// board evaluation for AYUMU

#include "../../dc.hpp"

#ifndef DCURLING_AYUMU_ESTIMATOR_HPP_
#define DCURLING_AYUMU_ESTIMATOR_HPP_

namespace DigitalCurling{
    
    double scoreBias[N_TURNS][SCORE_LENGTH][SCORE_LENGTH];
    double o2ScoreBias[N_TURNS][SCORE_LENGTH][SCORE_LENGTH];
    
    int initEvalStat(const std::string& dir){
        {
            //cerr <<(dir + "tscore_to_escore.dat") << endl; getchar();
            std::ifstream ifs(dir + "tscore_to_escore.dat");
            if(!ifs){ cerr << "failed to import tscore_to_escore.dat!" << endl; return -1; }
            for(int t = 0; t < N_TURNS; ++t){
                for(int i0 = 0; i0 < SCORE_LENGTH; ++i0){
                    for(int i1 = 0; i1 < SCORE_LENGTH; ++i1){
                        ifs >> scoreBias[t][i0][i1];
                        //cout << scoreBias[t][i0][i1] << endl;
                    }
                }
            }
        }
        {
            std::ifstream ifs(dir + "to2score_to_escore.dat");
            if(!ifs){ cerr << "failed to import to2score_to_escore.dat!" << endl; return -1; }
            for(int t = 0; t < N_TURNS; ++t){
                for(int i0 = 0; i0 < SCORE_LENGTH; ++i0){
                    for(int i1 = 0; i1 < SCORE_LENGTH; ++i1){
                        ifs >> o2ScoreBias[t][i0][i1];
                        //cout << o2ScoreBias[t][i0][i1] << endl;
                    }
                }
            }
        }
        return 0;
    }
    
#define Foo(i) {s += ev.param(i); ev.template feedFeatureScore<M>((i), 1.0);}
#define FooX(i, x) s += ev.param(i) * (x); ev.template feedFeatureScore<M>((i), (x));}
   
#define FooC(p, i) {ASSERT(EV_IDX(p) <= i && i < EV_IDX((p) + 1),\
    cerr << "FooC() : illegal index " << (i) << " in " << EV_IDX(p) << "~" << (EV_IDX((p) + 1) - 1);\
    cerr << " (pi = " << (p) << ")" << endl; );\
    s += ev.param(i); ev.template feedFeatureScore<M>((i), 1.0);}
    
#define FooXC(p, i, x) {ASSERT(EV_IDX(p) <= i && i < EV_IDX((p) + 1),\
    cerr << "FooXC() : illegal index " << (i) << " in " << EV_IDX(p) << "~" << (EV_IDX((p) + 1) - 1);\
    cerr << " (pi = " << (p) << ")" << endl; );\
    s += ev.param(i) * (x); ev.template feedFeatureScore<M>((i), (x));}
    
    namespace EvalSpace{
        
        enum{
            EV_SCORE, // 現在の盤上の得点
            EV_2ND_SCORE, // 現在の盤上の第2得点
            EV_NO1_GUARD, // No1の位置とガードがあるか
            EV_STONE_POS, // 石の位置
            EV_N_MY_STONES,
            EV_N_OPP_STONES,
            EV_ALL,
        };
        
        constexpr int evNumTable[]={
            0, //(N_TURNS - 1) * SCORE_LENGTH * SCORE_LENGTH,
            0, //(N_TURNS - 1) * SCORE_LENGTH * SCORE_LENGTH,
            N_TURNS * 16 * 8 * SCORE_LENGTH,
            0, //N_TURNS * 16 * 2 * SCORE_LENGTH,
            0, //N_TURNS * 2 * SCORE_LENGTH,
            0, //N_TURNS * 2 * SCORE_LENGTH,
        };
        
        constexpr int EV_NUM(unsigned int p){
            return evNumTable[p];
        }
        
        constexpr int EV_IDX(unsigned int p){
            return (p == 0) ? 0 : (EV_IDX(p - 1) + EV_NUM(p - 1));
        }
        
        constexpr int EV_NUM_ALL = EV_IDX(EV_ALL);
        
        int commentToEvalParam(std::ofstream& ofs, const double *const param){
            return 0;
        }
    }
    
    using ScoreEstimator = SoftmaxPolicy<EvalSpace::EV_NUM_ALL, N_TURNS>;
    using ScoreEstimatorLearner = SoftmaxPolicyLearner<ScoreEstimator>;
    
    int foutComment(const ScoreEstimator& pol, const std::string& fName){
        std::ofstream ofs(fName, std::ios::out);
        return EvalSpace::commentToEvalParam(ofs, pol.param_);
    }
    
    template<int M = 0, class board_t, class estimator_t>
    void calcEvalScore(eval_t *const pev, eval_t *const psum, const board_t& bd, const estimator_t& ev){
        
        using namespace EvalSpace;
        
        *psum = 0;
        ev.template initCalculatingScore<M>(SCORE_LENGTH);
        
        const int turn = bd.getTurn();
        
        const Color myColor = bd.getTurnColor();
        const Color oppColor = flipColor(myColor);
        const int end = bd.getEnd();
        const int rs = bd.getRelScore();
        
        const auto sb = bd.sb();
        const auto order = bd.orderBits;
        
        const int score = countScore(bd);
        const int oppScore2 = count2ndScore(order, oppColor);
        
        
        const bool clean = isCleanBetterScore(myColor, end, rs);
        
        const int no1PositionIndex = bd.NInHouse() ? getPositionIndex(bd.stoneCenter(0)) : -1;
        
        for(int m = 0; m < SCORE_LENGTH; ++m){
            
            ev.template initCalculatingCandidateScore<M>();
            
            double s = 0;
            int sc = IDXtoS(m);

            s += scoreBias[turn][StoIDX(score)][m]; // 現在の盤上得点の効果
            s += o2ScoreBias[turn][StoIDX(oppScore2)][m]; // 現在の盤上第2得点の効果
            /*
            {
                constexpr int base = EV_IDX(EV_SCORE);
                int i = base + turn * SCORE_LENGTH * SCORE_LENGTH + StoIDX(score) * SCORE_LENGTH + m;
                FooC(EV_SCORE, i);
            }
            
            // 現在の盤上第2得点の効果
            {
                constexpr int base = EV_IDX(EV_2ND_SCORE);
                int i = base + turn * SCORE_LENGTH * SCORE_LENGTH + StoIDX(oppScore2) * SCORE_LENGTH + m;
                FooC(EV_2ND_SCORE, i);
            }*/
            
            { // No1 position & guard
                constexpr int base = EV_IDX(EV_NO1_GUARD);
                if(bd.NInHouse() != 0){
                    int type = (order.get(0) == myColor) * 4;
                    uint32_t no1 = bd.center[0];
                    if(bd.stone(no1).frontGuard(Spin::RIGHT) && bd.stone(no1).frontGuard(Spin::LEFT)){
                        type += 3;
                    }else if(bd.stone(no1).frontGuard(Spin::RIGHT)){
                        type += 1;
                    }else if(bd.stone(no1).frontGuard(Spin::LEFT)){
                        type += 2;
                    }
                    int i = base + turn * 16 * SCORE_LENGTH * 8
                    + no1PositionIndex * SCORE_LENGTH * 8
                    + type * SCORE_LENGTH + m;
                    FooC(EV_NO1_GUARD, i);
                }
            }
            /*{  // stone position
                constexpr int base = EV_IDX(EV_STONE_POS);
                iterateStone(bd, BLACK, [base, turn, m, &s, &bd, &ev](const auto& st)->void{
                    int i = base + turn * 16 * 2 * SCORE_LENGTH + getPositionIndex(st) * SCORE_LENGTH + m;
                    FooC(EV_STONE_POS, i);
                });
                iterateStone(bd, WHITE, [base, turn, m, &s, &bd, &ev](const auto& st)->void{
                    int i = base + turn * 16 * 2 * SCORE_LENGTH + 16 * SCORE_LENGTH + getPositionIndex(st) * SCORE_LENGTH + m;
                    FooC(EV_STONE_POS, i);
                });
            }*/
            
            /*{ // num of my stones
                constexpr int base = EV_IDX(EV_N_MY_STONES);
                int i = base + turn * SCORE_LENGTH * 2 + clean * SCORE_LENGTH + m;
                FooXC(EV_N_MY_STONES, i, bd.NInHouse(myColor));
            }
            { // num of opp stones
                constexpr int base = EV_IDX(EV_N_OPP_STONES);
                int i = base + turn * SCORE_LENGTH * 2 + clean * SCORE_LENGTH + m;
                FooXC(EV_N_OPP_STONES, i, bd.NInHouse(oppColor));
            }*/
            
            double exps = exp(s / ev.temperature());
            pev[m] = exps;
            *psum += exps;
            
            FASSERT(s,);
            FASSERT(exps, cerr << "s = " << s << " T = " << ev.temperature() << endl;);
            
            ev.template feedCandidateScore<M>(exps);
        }
    }
#undef Foo
#undef FooX
#undef FooC
#undef FooXC
}

#endif //DCURLING_AYUMU_ESTIMATOR_HPP_