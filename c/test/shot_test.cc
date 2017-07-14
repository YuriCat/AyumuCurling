/*
 shot_test.cc
 Katsuki Ohto
 */

// ショットの出来のテスト
#include "../ayumu/ayumu_dc.hpp"

#include "../simulation/fastSimulator.hpp"

#include "../simulation/primaryShot.hpp"
#include "../ayumu/shot/allShots.hpp"

#include "../ayumu/logic/logic.hpp"
#include "../ayumu/structure/thinkBoard.hpp"

#include "../ayumu/initialize.hpp"

namespace DigitalCurling{
    namespace Tester{

        XorShift64 dice;
        DSFMT ddice;
        
        using testBoard_t = ThinkBoard;
        
        constexpr int NSTR = 3; // num of shot-types

        template<class shot_t>
        void testShotThread(int threadId,
                           fpn_t (*scoreSum)[2], uint64_t (*timeSum)[2],
                           shot_t *const pshotgen,
                           int NTHREADS, int NSAMPLES, int NTRIALS){

            Clock cl;

            for (int s = 0; s < NSAMPLES; ++s){
                //making sample

                fMoveXY<fpn_t> fmv[NSTR][2];
                fpn_t expectedScore[NSTR][2] = {0};

                fpn_t score[NSTR][2] = {0};
                uint64_t time[NSTR][2] = {0};

                testBoard_t bd;
                bd.init();
                
                pshotgen->setSampleState(&bd, &ddice);
                
                // cerr << bd << endl;
                
                bd.updateInfo();
                
                constexpr int dim = pshotgen->dimension();
                
                BitArray64<4, 16> index(0);
                iterateExcept(bd.sb(), 15, [&index](int idx){
                    index.insert(0, idx);
                });
                if(bd.sb().test(15)){
                    index.insert(0, 15);
                }
                
                index = pshotgen->orderRefStoneIndex(bd, index);
                
                MoveXY vmv[2];
                for(int si = 0; si < 2; ++si){
                    const Spin s = static_cast<Spin>(si);
                    // 着手によって引数の与え方がことなるので分岐
                    switch(shot_t::number()){
                        case Standard::L1DRAW: vmv[s].setL1Draw(s); break;
                        case Standard::PEEL: vmv[s].setPeel(s, index[0], 0); break;
                        case Standard::DRAWRAISE: vmv[s].setDrawRaise(s, index[0]); break;
                        case Standard::TAKEOUT: vmv[s].setTakeOut(s, index[0]); break;
                        case Standard::DOUBLE: vmv[s].setDouble(s, index[0], index[1]); break;
                        case Standard::RAISETAKEOUT: vmv[s].setRaiseTakeOut(s, index[1], index[0]); break;
                        default: cerr << "testShotThread() : unprepared move." << endl; exit(1); break;
                    }
                }
                
                // Placebo Move
                for(int s = 0; s < 2; ++s){
                    cl.start();
                    fmv[2][s].setSpin(static_cast<Spin>(s));
                    if (shot_t::hasContact()){
                        genHit(&fmv[2][s], bd.stone(vmv[s].getNum0()), shot_t::baseSpeed());
                    }else{
                        genDraw(&fmv[2][s], FPOSXY_TEE);
                    }
                    time[2][s] += cl.stop();
                }

                //cerr << index << endl;
                for(int s = 0; s < 2; ++s){
                    cl.start();
                    pshotgen->realize(&fmv[1][s], bd, vmv[s]);
                    double est = pshotgen->estimate(bd, TURN_WHITE_LAST, vmv[s]);
                    expectedScore[1][s] = est;
                    if(est < 0){//failed to generate
                        if (shot_t::hasContact()){
                            genHit(&fmv[1][s], bd.stone(index[0]), shot_t::baseSpeed());
                        }else{
                            genDraw(&fmv[1][s], FPOSXY_TEE);
                        }
                    }
                    time[1][s] += cl.stop();
                }
                
                assert(isValidMove(fmv[1][0]) && isValidMove(fmv[1][1]));

                // 強化学習ソルバ
                constexpr int FLEXIBILITY = shot_t::flexibility();


                std::array<double, 2> rew = {0.0, 1.0};

                double br = -99999;

                double solverMoveReward[2];
                
#if 0
                for (int si = 0; si < 2; ++si){
                    const Spin s = static_cast<Spin>(si);
                    // ソルバに投げる変数と定義域を設定
                    std::array<std::array<double, 2>, FLEXIBILITY> zone;
                    if (shot_t::hasContact()){
                        fMoveXY<> mv[3];
                        mv[0].setSpin(s); mv[1].setSpin(s); mv[2].setSpin(s);
                        std::array<std::array<double, 2>, FLEXIBILITY> tzone;
                        tzone[0] = {M_PI / 2, 3 * M_PI / 2};
                        if (FLEXIBILITY > 1){
                            tzone[1] = {FV_MIN + 0.3, FV_MAX - 0.3};
                        }
                        FastSimulator::rotateToPassPointCircumferenceF(&mv[0], bd.stone(index[0]), 32.5, tzone[0]);
                        FastSimulator::rotateToPassPointCircumferenceF(&mv[1], bd.stone(index[0]), 32.5, tzone[1]);
                        FastSimulator::rotateToPassPointCircumferenceF(&mv[2], bd.stone(index[0]), 32.5, (tzone[0] + tzone[1])/2);
                        zone = {{mv[0].x, mv[1].x}, {mv[0].y, mv[1].y}};
                    }
                    else{
                        zone = {{FVX_MIN, FVX_MAX}, {FVY_MIN, FVY_MAX}};
                    }
                    
                    //MCTSSolver<FLEXIBILITY> ms(zone, rew);

                    for (int t = 0; t < 100000; ++t){
                        auto var = solver.play();
                        fMoveXY<> mv;
                        mv.setSpin(s);

                        //cerr<<mv<<endl;
                        testBoard_t tbd = bd;
                        
                        auto ct = makeMove<1>(&tbd, TURN_WHITE_LAST, mv, &ddice);
                        
                        double ev = ct.isPut() ? pshotgen->evaluate(bd, tbd, TURN_WHITE_LAST, vmv[s]) : 0;
                        solver.feed(var, ev);
                        //if (ev > 0.5){ cerr << ev << ", "; }
                    }
                    auto ans = solver.answer();

                    if (solver.mean() > br){
                        br = solver.mean();
                        fmv[0].setSpin(s);
                        if (shot_t::hasContact()){
                            FastSimulator::rotateToPassPointCircumferenceF(&fmv[0], bd.stone(index[0]), 32.5, ans[0]);
                        } else{
                            fmv[0].x = ans[0];
                            fmv[0].y = ans[1];
                        }
                    }
                }
#endif
                
                // grid solver

                for (int si = 0; si < 2; ++si){
                    cl.start();
                    
                    const Spin s = static_cast<Spin>(si);
                    UniformGridSolver<2> gs;
                    
                    std::array<std::array<double, 2>, 2> zone;
                    if (shot_t::hasContact()){
                        fMoveXY<> mv[3];
                        mv[0].setSpin(s); mv[1].setSpin(s); mv[2].setSpin(s);
                        std::array<std::array<double, 2>, 2> tzone;
                        tzone[0] = {M_PI / 2, 3 * M_PI / 2};
                        if (FLEXIBILITY > 1){
                            tzone[1] = {FV_MIN + 0.3, FV_MAX - 0.3};
                        }else{
                            tzone[1] = {shot_t::baseSpeed(), shot_t::baseSpeed()};
                        }
                        FastSimulator::rotateToPassPointCircumferenceF(&mv[0], bd.stone(index[0]), shot_t::baseSpeed(), tzone[0][0]);
                        FastSimulator::rotateToPassPointCircumferenceF(&mv[1], bd.stone(index[0]), shot_t::baseSpeed(), tzone[0][1]);
                        FastSimulator::rotateToPassPointCircumferenceF(&mv[2], bd.stone(index[0]), shot_t::baseSpeed(), (tzone[0][0] + tzone[0][1])/2);
                        zone[0] = {min(mv[0].x, mv[1].x) - 0.6, max(mv[0].x, mv[1].x) + 0.6}; zone[1] = {tzone[1][0], tzone[1][1]};
                    }
                    else{
                        if(isRight(s)){
                            zone[0] = {FVX_LEFT_HOUSE, FVX_RIGHT_HOUSE}; zone[1] = {FVY_FRONT_HOUSE, FVY_BACK_HOUSE};
                        }else{
                            zone[0] = {-FVX_RIGHT_HOUSE, -FVX_LEFT_HOUSE}; zone[1] = {FVY_FRONT_HOUSE, FVY_BACK_HOUSE};
                        }
                    }
                    //cerr << zone[0][0] << ", " << zone[0][1] << endl;
                    
                    std::array<long long, 2> length;
                    for(int d = 0; d < 2; ++d){
                        length[d] = min((1 << 6), max(1, (int)((zone[d][1] - zone[d][0]) / (FR_STONE_RAD / 8))));
                        //cerr<< length[d] << endl;
                    }
                    
                    gs.setGridSize(length);
                    gs.setVariableZone(zone);
                    gs.evalAllGrids([pshotgen, index, s, &bd, vmv](const auto& var)->double{
                        fMoveXY<> mv(var[0], var[1], s);
                        //cerr << mv << endl;
                        
                        testBoard_t tbd = bd;
                        
                        ContactTree ct = makeMoveNoRand<1>(&tbd, TURN_WHITE_LAST, mv);
                        
                        tbd.updateInfo(2);
                        
                        double ev = pshotgen->evaluate(bd, tbd, TURN_WHITE_LAST, vmv[s], ct);
                        //cerr << ret << ev << endl;
                        return ev;
                    });
                    auto result = gs.getBestIntegratedGridVariable([s](const auto& base, const auto& var)->double{
                        return relativePdfDeltaVxVy(var[0] - base[0], var[1] - base[1], s);
                        //return pdfDeltaVxVy(var[0] - base[0], var[1] - base[1]);
                    });
                    auto bestVar = std::get<0>(result);
                    fmv[0][s] = fMoveXY<>(bestVar[0], bestVar[1], s);
                    solverMoveReward[s] = std::get<1>(result);
                    
                    time[0][s] += cl.stop();
                }
                int bestSpin = (solverMoveReward[0] >= solverMoveReward[1]) ? 0 : 1;
                expectedScore[0][1] = solverMoveReward[bestSpin];
                //cerr<<fmvMC<<endl;

                // calculate evaluation
                for (int str = 0; str < NSTR; ++str){
                    for (int s = 0; s < 2; ++s){
                        ASSERT(isValidMove(fmv[str][s]),);
                        for (int t = 0; t < NTRIALS; ++t){
                            testBoard_t tbd = bd;
                            
                            ContactTree ct = makeMove<1>(&tbd, TURN_WHITE_LAST, fmv[str][s], &ddice);
                            
                            tbd.updateInfo(2);
                            
                            score[str][s] += pshotgen->evaluate(bd, tbd, TURN_WHITE_LAST, vmv[s], ct);
                        }
                        score[str][s] /= NTRIALS;
                        
                        scoreSum[str][s] += score[str][s];
                        timeSum[str][s] += time[str][s];
                    }
                }

                cerr << " Shot Test ... sample " << (s * NTHREADS + threadId) << endl;
                cerr << " expected op-reward" << endl;
                for(int s = 0; s < 2; ++s){
                    cerr << " Spin : " << toSpinString(s) << endl;
                    cerr << " [Solver :" << score[0][s] << " (" << time[0][s] << " clock)] " << fmv[0][s] << " (exp : " << expectedScore[0][s] << " )" << endl;
                    cerr << " [Test   :" << score[1][s] << " (" << time[1][s] << " clock)] " << fmv[1][s] << endl;
                    cerr << " [Placebo:" << score[2][s] << " (" << time[2][s] << " clock)] " << fmv[2][s] << endl;
                }
            }
        }

        template<class shot_t>
        int testShot(shot_t *const pshotgen, int NSAMPLES, int NTHREADS){

            // shot generating test
            const int NTRIALS = 2000;

            fpn_t scoreSum[NSTR][2] = { 0 };
            uint64_t timeSum[NSTR][2] = { 0 };

            fpn_t scoreThreadSum[N_THREADS][NSTR][2] = {0};
            uint64_t timeThreadSum[N_THREADS][NSTR][2] = {0};
            int threadNSamples[N_THREADS] = {0};

            //decide num of samples for each thread
            for (int s = 0; s < NSAMPLES; ++s){
                threadNSamples[s % NTHREADS]++;
            }

            if (NTHREADS > 1){
                std::thread thr[N_THREADS];
                for (int th = 0; th < NTHREADS; ++th){
                    thr[th] = std::thread(&testShotThread<shot_t>,
                               th,
                               scoreThreadSum[th],
                               timeThreadSum[th],
                               pshotgen,
                               NTHREADS,
                               threadNSamples[th],
                               NTRIALS
                               );
                }
                for (int th = 0; th < NTHREADS; ++th){
                    thr[th].join();
                }
            }
            else{
                int th = 0;
                testShotThread<shot_t>(
                                       th,
                                       scoreThreadSum[th],
                                       timeThreadSum[th],
                                       pshotgen,
                                       NTHREADS,
                                       threadNSamples[th],
                                       NTRIALS
                                       );
            }

            for (int str = 0; str < NSTR; ++str){
                for(int s = 0; s < 2; ++s){
                    for (int th = 0; th < NTHREADS; ++th){
                        scoreSum[str][s] += scoreThreadSum[th][str][s];
                        timeSum[str][s] += timeThreadSum[th][str][s];
                    }
                    scoreSum[str][s] /= NSAMPLES;
                    timeSum[str][s] /= NSAMPLES;
                }
            }
            cerr << " Shot Test ... all results" << endl;
            cerr << " expected op-reward "<<endl;
            for(int s = 0; s < 2; ++s){
                cerr << " Spin : " << toSpinString(s) << endl;
                cerr << " [Solver  : " << scoreSum[0][s] << " ( " << timeSum[0][s] << " clock )]"<<endl;
                cerr << " [Test    : " << scoreSum[1][s] << " ( " << timeSum[1][s] << " clock )]"<<endl;
                cerr << " ev distance : " << (scoreSum[0][s] - scoreSum[1][s]) << endl;
            }
            cerr << " [Placebo : " << scoreSum[2][0] << " ( " << timeSum[2][0] << " clock )]"<<endl;
            return 0;
        }
    }
}

int main(int argc, char* argv[]){
    
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    using namespace DigitalCurling;
    
    initShot(DIRECTORY_PARAMS_IN); // 初期化

    std::random_device seed;

    Tester::dice.srand(seed() * (unsigned int)time(NULL));
    Tester::ddice.srand(seed() * (unsigned int)time(NULL));
    
    int samples = 1000;
    int threads = N_THREADS;

    for (int i = 1; i < argc; ++i){
        if (!strcmp(argv[i], "-dr")){ // Draw Raise
            DrawRaise sg;
            return DigitalCurling::Tester::testShot(&sg, samples, threads);
        }else if (!strcmp(argv[i], "-d")){ // Double
            Double sg;
            return DigitalCurling::Tester::testShot(&sg, samples, threads);
        }else if (!strcmp(argv[i], "-l1d")){ // L1Draw
            L1Draw sg;
            return DigitalCurling::Tester::testShot(&sg, samples, threads);
        }else if (!strcmp(argv[i], "-n")){ // trials
            samples = atoi(argv[i + 1]);
        }else if (!strcmp(argv[i], "-p")){ // Peel
            Peel sg;
            return DigitalCurling::Tester::testShot(&sg, samples, threads);
        }else if(!strcmp(argv[i], "-rto")){ // RaiseTakeOut
            RaiseTakeOut sg;
            return DigitalCurling::Tester::testShot(&sg, samples, threads);
        }else if(!strcmp(argv[i], "-th")){ // num oh threads
            threads = atoi(argv[i + 1]);
        }else if(!strcmp(argv[i], "-to")){ // TakeOut
            TakeOut sg;
            return DigitalCurling::Tester::testShot(&sg, samples, threads);
        }

#if 0
        else if (!strcmp(argv[i], "-fto")){ // FrontTakeOut test
            //FrontTakeOut sg;
            //return DigitalCurling::Tester::testShot(&sg);
        }
        else if (!strcmp(argv[i], "-bto")){ // BackTakeOut test
            //BackTakeOut sg;
            //return DigitalCurling::Tester::testShot(&sg);
        }
#endif
        
    }
    return 0;
}