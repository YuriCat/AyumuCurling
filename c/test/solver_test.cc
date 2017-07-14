/*
solver_tester.cc
Katsuki Ohto
*/

#include "../dc.hpp"
#include "../ayumu/ayumu_dc.hpp"

#include "../ayumu/structure/thinkBoard.h"
#include "../simulation/b2dsimulator.hpp"
#include "../simulation/fastSimulator.hpp"
#include "../simulation/primaryShot.hpp"
#include "../ayumu/logic/logic.hpp"
#include "../ayumu/structure/thinkBoard.hpp"
#include "../solver/gridsolver.hpp"
#include "../solver/gridmc.hpp"

namespace DigitalCurling{
    namespace Tester{

        XorShift64 dice;
        DSFMT ddice;

        template<class dice_t>
        void setSampleState(MiniBoard *const pbd,dice_t *const pdice){
            locateInHouse(pbd, BLACK, 2, pdice);
            locateInHouse(pbd, WHITE, 1, pdice);
            locateInPlayArea(pbd, BLACK, 2, pdice);
            locateInPlayArea(pbd, BLACK, 2, pdice);
        }

        double evaluate(const MiniBoard& bd, Color c){
            return countScore(bd, c);
        }

        int solverTestThread(int threadId, fpn_t *scoreSum, uint64_t *timeSum,
            int NThreads, int NSamples, int NSolvers, int NTrials){

            Clock cl;

            //<Solver>
            //0 : MCT with randomness
            //1 : grid search and convolution

            for (int s = 0; s < NSamples; ++s){
                //making sample

                fMoveXY<fpn_t> fmv[NSolvers];
                eval_t eval[NSolvers];//subjective ev value

                fpn_t score[NSolvers] = { 0 };
                uint64_t time[NSolvers] = { 0 };
                uint64_t trials[NSolvers] = { 0 };

                MiniBoard bd;
                setSampleState(&bd, &ddice);

                
                cl.start();
#if 0
                //乱数情報を使わない強化学習ソルバ
                array<pair<double, double>, 2> arr;

                //ソルバに投げる変数と定義域を設定
                arr[0] = pair<double, double>(FVX_MIN, FVX_MAX);
                arr[1] = pair<double, double>(FVY_MIN, FVY_MAX);

                pair<double, double> rew(-3, +3);

                double br = -99999;

                fMoveXY<> fSolverMove[2];
                double solverMoveReward[2];
                for (int s = 0; s < 2; ++s){
                    Solver<2> solver(arr, rew);

                    for (int t = 0; t < 200000; ++t){
                        auto var = solver.play();
                        fMoveXY<> mv(var[0], var[1], s);
                        mv.setSpin(s);
                        MiniBoard tbd = bd;
                        FastSimulator::makeMove(&tbd, TURN_LAST, mv, &ddice);
                        double ev = evaluate(tbd, WHITE);
                        solver.feed(var, ev);
                        //cerr<<ev<<endl;
                    }
                    auto ans = solver.answer();

                    //solver.info();

                    fSolverMove[s].set(ans[0], ans[1], s);
                    solverMoveReward[s] = solver.mean();
                }
                if (solverMoveReward[0] > solverMoveReward[1]){
                    fmv[0] = fSolverMove[0]; eval[0] = solverMoveReward[0];
                }
                else{
                    fmv[0] = fSolverMove[1]; eval[0] = solverMoveReward[1];
                }
#endif
                
                // Grid Monte Carlo
                eval[0] = solveWithIntegratedGridWithMC<(1 << 8) - 1, (1 << 6) - 1>
                (&fmv[0], bd, WHITE, (1 << 8) * (1 << 6) * 100, &ddice, [&bd](const auto& mv)->eval_t{
                    // making move
                    auto tbd = bd;
                    FastSimulator::makeMoveNoRand(&tbd, TURN_LAST, mv);
                    
                    // evaluate after board
                    return evaluate(tbd, WHITE);
                });
                
                time[0] += cl.stop();

                //Grid Search
                cl.start();
                eval[1] = solveBySimpleGrid<(1 << 8) - 1, (1 << 6) - 1>(&fmv[1], bd, WHITE, evaluate);
                time[1] += cl.stop();

                //calculate evaluation
                for (int sv = 0; sv < NSolvers; ++sv){
                    for (int t = 0; t < NTrials; ++t){
                        MiniBoard tbd = bd;
                        FastSimulator::makeMove(&tbd, TURN_LAST, fmv[sv], &ddice);
                        score[sv] += evaluate(tbd, WHITE);
                    }
                    score[sv] /= NTrials;

                    scoreSum[sv] += score[sv];
                    timeSum[sv] += time[sv];
                }

                cerr << " Solver Test ... sample " << (s * NThreads + threadId) << endl;
                cerr << " expected op-reward" << endl;
                cerr << " [MC :" << score[0] << " (" << time[0] << " clock)] " << fmv[0] << " " << eval[0] << endl;
                cerr << " [GS :" << score[1] << " (" << time[1] << " clock)] " << fmv[1] << " " << eval[1] << endl;
            }
        }

        int testSolver(int NThreads){

            //shot generating test
            constexpr int NSamples = 100;
            constexpr int NTrials = 5000;
            constexpr int NSolvers = 2;//num of solvers

            fpn_t scoreSum[NSolvers] = { 0 };
            uint64_t timeSum[NSolvers] = { 0 };

            fpn_t scoreThreadSum[NThreads][NSolvers] = { 0 };
            uint64_t timeThreadSum[NThreads][NSolvers] = { 0 };
            int threadNSamples[NThreads] = { 0 };

            //decide num of samples for each thread
            for (int s = 0; s<NSamples; ++s){
                threadNSamples[s%NThreads]++;
            }

            //cerr << threadNSamples[2] << endl;

            std::thread thr[NThreads];
            
            if(NThreads>1){
                for (int th = 0; th < NThreads; ++th){
                    thr[th] = std::thread(&solverTestThread,
                                          th,
                                          scoreThreadSum[th],
                                          timeThreadSum[th],
                                          NThreads,
                                          threadNSamples[th],
                                          NSolvers,
                                          NTrials);
                }
                for (int th = 0; th < NThreads; ++th){
                    thr[th].join();
                }
            }else{
                int th = 0;
                solverTestThread
                (th,
                scoreThreadSum[th],
                timeThreadSum[th],
                NThreads,
                threadNSamples[th],
                NSolvers,
                NTrials);
            }

            for (int sv = 0; sv < NSolvers; ++sv){
                for (int th = 0; th < NThreads; ++th){
                    scoreSum[sv] += scoreThreadSum[th][sv];
                    timeSum[sv] += timeThreadSum[th][sv];
                }
                scoreSum[sv] /= NSamples;
                timeSum[sv] /= NSamples;
            }
            cerr << " Solver Test ... all results" << endl;
            cerr << " [MC : " << scoreSum[0] << " ( " << timeSum[0] << " clock )]" << endl;
            cerr << " [GS : " << scoreSum[1] << " ( " << timeSum[1] << " clock )]" << endl;
            cerr << " ev distance : " << (scoreSum[0] - scoreSum[1]) << endl;

            return 0;
        }
    }
}

int main(int argc, char* argv[]){

    using namespace DigitalCurling;

    std::random_device seed;

    Tester::dice.srand(seed()*(unsigned int)time(NULL));
    Tester::ddice.srand(seed()*(unsigned int)time(NULL));

#ifdef DCURLING_SIMULATION_FASTSIMULATOR_HPP_
    FastSimulator::init();
#endif

    int NThreads=1;

    for (int i = 1; i < argc; ++i){
        if (strstr(argv[i], "-th")){ // num of threads
            if (i == argc - 1)break;
            NThreads = atoi(argv[i + 1]);
        }        
    }
    cerr << "Number Of Threads = " << NThreads << endl;
    return Tester::testSolver(NThreads);
}