/*
 ayumu_test.cc
 Katsuki Ohto
 */

// 歩の思考部テスト
#include "../ayumu/ayumu.hpp"

using namespace std;

namespace DigitalCurling{
    namespace Tester{

        using namespace Ayumu;

        XorShift64& dice = threadTools[0].dice;
        DSFMT& ddice = threadTools[0].ddice;

        ThreadTools& tools = threadTools[0];
        /*
        int testL1(){
            //test L1-method
            constexpr int samples = 100;
            constexpr int trials = 300;

            //L1-play
            fpn_t wpMCSum, wpTestSum, wpTestSlowSum;
            uint64_t clMCSum, clTestSum, clTestSlowSum;

            wpMCSum = wpTestSum = 0;
            clMCSum = clTestSum = 0;

            Clock cl;

            for (int s = 0; s<samples; ++s){

                //make sample
                ThinkField bd;

                fMoveXY<fpn_t> fmvMC, fmvTest, fmvTestSlow;

                fpn_t wpMC, wpTest, wpTestSlow;
                uint64_t clMC, clTest, clTestSlow;

                wpMC = wpTest = 0;
                clMC = clTest = 0;

                bd.init();

                int col[16];
                fPosXY<> pos[16];

                for (int j = 0; j<8; ++j){
                    col[j] = (ddice.rand() % 2) ? BLACK : WHITE;

                    while (1){
                        double r0 = ddice.drand(); double r1 = ddice.drand();
                        fpn_t x = FX_TEE + 2 * r0*cos(2 * r1*M_PI); fpn_t y = FY_TEE + 2 * r0*sin(2 * r1*M_PI);
                        int ok = 1;
                        for (int k = 0; k<j; ++k){
                            if (XYtoR(x - pos[k].x, y - pos[k].y) < FR_STONE_RAD * 2){
                                ok = 0; break;
                            }
                        }
                        if (ok){
                            pos[j].set(x, y);
                            break;
                        }
                    }
                }
                for (int j = 0; j<8; ++j){
                    bd.pushStone(col[j], pos[j].x, pos[j].y);
                }

                bd.end = END_JOBAN;
                bd.turn = TURN_LAST;

                bd.setState();
                bd.setTimeLimit(WHITE, 0, 1000);

                //MC Move
                MonteCarloPlayer mcp;
                gState = 0;
                mvTT.init();

                cl.start();
                mcp.play(&fmvMC, bd);
                clMC += cl.stop();

                //Test Move
                cl.start();
                L1::play(bd, &fmvTest, &tools);
                clTest += cl.stop();

                //Test(slow) Move
                cl.start();
                L1::playSlow(bd, &fmvTest, &tools);
                clTestSlow += cl.stop();

                //calculate evaluation
                for (int t = 0; t<trials; ++t){
                    ThinkBoard tbd = bd;
                    tbd.makeMove(fmvMC, &ddice);
                    wpMC += getOpeningEndReward(WHITE, tbd.countEndScore());
                }
                wpMC /= trials;

                for (int t = 0; t<trials; ++t){
                    ThinkBoard tbd = bd;
                    tbd.makeMove(fmvTest, &ddice);
                    wpTest += getOpeningEndReward(WHITE, tbd.countEndScore());
                }
                wpTest /= trials;

                for (int t = 0; t<trials; ++t){
                    ThinkBoard tbd = bd;
                    tbd.makeMove(fmvTestSlow, &ddice);
                    wpTest += getOpeningEndReward(WHITE, tbd.countEndScore());
                }
                wpTestSlow /= trials;

                CERR << " L1 tester ... sample " << s << endl;
                CERR << " expected op-reward  [MC : " << wpMC << " ( " << clMC << " clock )] [Test : " << wpTest << " ( " << clTest << " clock )]" << endl;

                wpMCSum += wpMC;
                wpTestSum += wpTest;
                wpTestSlowSum += wpTestSlow;
                clMCSum += clMC;
                clTestSum += clTest;
                clTestSlowSum += clTestSlow;

            }

            wpMCSum /= samples;
            wpTestSum /= samples;
            wpTestSlowSum /= samples;

            clMCSum /= samples;
            clTestSum /= samples;
            clTestSlowSum /= samples;

            CERR << " L1 tester ... all results" << endl;
            CERR << " expected op-reward  [MC : " << wpMCSum << " ( " << clMCSum << " clock )]" << endl;
            CERR << " [Test : " << wpTestSum << " ( " << clTestSum << " clock )]" << endl;
            CERR << " [TsSl : " << wpTestSlowSum << " ( " << clTestSlowSum << " clock )]" << endl;
            CERR << " ev distance : " << (wpMCSum - wpTestSum) << endl;

            return 0;
        }
        */
        
        int outputValue(){
            // 思考に用いる値を表示
            cerr << "FR_I16R_STEP = " << FR_I16R_STEP << endl;
            cerr << "FX_I16     = " << "[" << I16XtoFX(0) << ", " << I16XtoFX(32768) << "]" << endl;
            cerr << "FY_I16     = " << "[" << I16YtoFY(0) << ", " << I16YtoFY(32768) << "]" << endl;
            
            auto hashGen = [](fpn_t fx, fpn_t fy, uint32_t idx, int r)->uint64_t{
                return genHash_Pos(idx, 0, FXtoI16X(fx), r) ^ genHash_Pos(idx, 1, FYtoI16Y(fy), r);
            };
            
            cerr << "corner hash value = " << endl;
            for(int r = 0; r < 16; ++r){
                cerr << hashGen(FX_PA_MIN, FY_PA_MIN, TURN_FIRST, r) << " ";
                cerr << hashGen(FX_PA_MIN, FY_PA_MAX, TURN_FIRST, r) << " ";
                cerr << hashGen(FX_PA_MAX, FY_PA_MIN, TURN_FIRST, r) << " ";
                cerr << hashGen(FX_PA_MAX, FY_PA_MAX, TURN_FIRST, r) << " ";
                cerr << endl;
            }
            
            cerr << "center hash value = " << endl;
            for(int r = 0; r < 16; ++r){
                cerr << hashGen(FX_XMID - 0.001, FY_YMID - 0.001, TURN_FIRST, r) << " ";
                cerr << hashGen(FX_XMID - 0.001, FY_YMID + 0.001, TURN_FIRST, r) << " ";
                cerr << hashGen(FX_XMID + 0.001, FY_YMID - 0.001, TURN_FIRST, r) << " ";
                cerr << hashGen(FX_XMID + 0.001, FY_YMID + 0.001, TURN_FIRST, r) << " ";
                cerr << endl;
            }
            
            return 0;
        }

        int testGame(){

            cerr << "game test : " << endl;
            
            const Color myColor = (dice.rand() % 2) ? BLACK : WHITE;
            const int ends = 4;

            ThinkField bd;
            bd.init();
            bd.setTimeLimit(BLACK, 60000);
            bd.setTimeLimit(WHITE, 60000);
            bd.setEnd(END_LAST + ends - 1);
            bd.setTurn(TURN_FIRST);
            for (int e = END_LAST + ends - 1; e >= END_LAST; --e){
                fMoveXY<> mv;

                while (1){

                    cerr << bd.toDebugString();

                    ClockMS clms;
                    clms.start();

                    if (bd.getTurnColor() == bd.getCurrentColor(myColor)){
                        MonteCarloPlayer mcp;
                        gState = 0;
                        pbdTT->init();
                        //mvTT.init();
                        mcp.play(&mv, bd);
                    }
                    else{
                        mv.setVX(FVX_TEESHOT_OFFICIAL[Spin::RIGHT]);
                        mv.setVY(FVY_TEESHOT_OFFICIAL);
                        mv.setSpin(Spin::RIGHT);
                    }

                    bd.tl[bd.getTurnColor()].limit_ms -= clms.stop();

                    cerr << "my color = " << toColorString(bd.getCurrentColor(myColor)) << endl;
                    cerr << "move = " << mv << endl;
                    makeMove(&bd, mv, &ddice);
                    if (bd.getTurn() == TURN_LAST){
                        bd.updateInfoLast();
                        cerr << " score = " << countScore(bd) << endl;
                        bd.procEnd(countScore(bd));
                        break;
                    }
                    else{
                        bd.procTurn();
                    }
                }
                cerr << " black's relative score : " << bd.getRelScore() << endl;
                cerr << " my relative score : " << bd.getRelScore(myColor) << endl;
                getchar();
            }
            
            return 0;
        }
        /*
        int testStandardShot(){
            
            //コーナー着手
            {
                fMoveXY<> mv,vec;
                fRMoveXY<> rvec;
                fPosXY<> pos;
                CERR << "Corner Move Check" << endl;
                pos.set(FX_PA_MIN + FR_STONE_RAD, FY_PA_MIN + FR_STONE_RAD);
                mv.setSpin(Spin::RIGHT);
                genDraw(&mv, pos);
                CERR << "PA LEFT-SHORT r (" << mv.x << "," << mv.y << ")" << endl;
                pos.set(FX_PA_MAX - FR_STONE_RAD, FY_PA_MIN + FR_STONE_RAD);
                mv.setSpin(Spin::RIGHT);
                genDraw(&mv, pos);
                CERR << "PA RIGHT-SHORT r (" << mv.x << "," << mv.y << ")" << endl;
                pos.set(FX_TEE, FY_TEE + FR_HOUSE_RAD + 6.0*1.52);
                mv.setSpin(Spin::RIGHT);
                genDraw(&mv, pos);
                CERR << "CENTER-LONG r (" << mv.x << "," << mv.y << ")" << endl;
                pos.set(FX_PA_MAX,FY_PA_MAX);
                 vec.setSpin(0);
                 calcDeparture(pos,&vec);
                 CERR<<"PA RD 0 ("<<vec.x<<","<<vec.y<<")"<<endl;
                 pos.set(FX_RINK_MIN,FY_RINK_MIN);
                 vec.setSpin(0);
                 CreateShot(pos,16,&vec);
                 CERR<<"RINK LU 0 PowMAX("<<vec.x<<","<<vec.y<<") "<<XYtoR(vec.x,vec.y)<<endl;
                 pos.set(FX_PA_MAX,FY_PA_MAX);
                 vec.setSpin(0);
                 CreateShot(pos,16,&vec);
                 CERR<<"PA RD 0 PowMAX("<<vec.x<<","<<vec.y<<") "<<XYtoR(vec.x,vec.y)<<endl;
                 pos.set(FX_RINK_MIN,FY_RINK_MIN);
                 vec.setSpin(0);
                 CreateShot(pos,0,&vec);
                 CERR<<"RINK LU 0 PowMIN("<<vec.x<<","<<vec.y<<") "<<XYtoR(vec.x,vec.y)<<endl;
                 pos.set(FX_PA_MAX,FY_PA_MAX);
                 vec.setSpin(0);
                 CreateShot(pos,0,&vec);
            }
            
            {
                //着手作成の精度調査
                MoveXY lmv;
                fMoveXY<> mv;
                fRMoveXY<> rmv;
                ThinkBoard bd;
                int suc[16][8];
                const int N = 200;

                //ティーショット
                suc[0][0] = 0;
                for (int i = 0; i < N; ++i){
                    bd.init();
                    genDraw(&mv, FPOSXY_TEE);
                    bd.makeMove<1>(mv, &ddice);

                    //CERR<<bd.stone(WHITE,0)<<endl;

                    if (bd.NInHouse[WHITE]>0
                        && isNearTee(bd.stone(WHITE, 0), 1.0)){
                        ++suc[0][0];
                    }
                }

                //ショートティーショット
                suc[1][0] = 0;
                for (int i = 0; i < N; ++i){
                    bd.init();
                    mv.setSpin(dice.rand() % 2);
                    genStandardDraw(&mv,DrawPos::S4);
                    bd.makeMove<1>(mv, &ddice);

                    //CERR<<bd.stone(WHITE,0)<<endl;

                    if (bd.NInHouse[WHITE]>0
                        && isNearTee(bd.stone(WHITE, 0), FR_HOUSE_RAD + FR_STONE_RAD)
                        && isThisSideOfTeeLine(bd.stone(WHITE, 0).y)){
                        ++suc[1][0];
                    }
                }

                //レンジティーショット
                suc[2][0] = 0;
                for (int i = 0; i < N; ++i){
                    bd.init();
                    lmv.setDraw(dice.rand() % 2, DrawPos::TEE);
                    genRootMove(&rmv, bd, lmv);
                    mv = rmv.genNR(&ddice);
                    bd.makeMove<1>(mv, &ddice);

                    //CERR<<bd.stone(WHITE,0)<<endl;

                    if (bd.NInHouse[WHITE]>0
                        && isNearTee(bd.stone(WHITE, 0), 1.0)){
                        ++suc[2][0];
                    }
                }

                //ストロングヒットによるテイクアウト
                suc[3][0] = 0;
                suc[3][1] = 0;
                for (int i = 0; i < N; ++i){
                    bd.init();

                    double r0 = ddice.drand(); double r1 = ddice.drand();
                    fpn_t x = FX_TEE + FR_HOUSE_RAD *r0*cos(r1*M_PI); fpn_t y = FY_TEE + FR_HOUSE_RAD*r0*sin(r1*M_PI);

                    bd.pushStone(BLACK, x, y);

                    mv.setSpin(dice.rand() % 2);
                    genStandardHit(&mv, bd.stone(BLACK, 0), HitWeight::STRONG, 1);

                    bd.makeMove<1>(mv, &ddice);

                    if (bd.NInHouse[BLACK] == 0){//TakeOut
                        ++suc[3][0];
                    }
                    if (bd.NInHouse[WHITE] > 0){
                        ++suc[3][1];
                    }
                }

                //ミドルヒットによるテイクアウト
                suc[4][0] = 0;
                suc[4][1] = 0;
                for (int i = 0; i < N; ++i){
                    bd.init();

                    double r0 = ddice.drand(); double r1 = ddice.drand();
                    fpn_t x = FX_TEE + FR_HOUSE_RAD*r0*cos(r1*M_PI); fpn_t y = FY_TEE + FR_HOUSE_RAD*r0*sin(r1*M_PI);

                    bd.pushStone(BLACK, x, y);

                    mv.setSpin(dice.rand() % 2);
                    genStandardHit(&mv, bd.stone(BLACK, 0), HitWeight::MIDDLE, 1);

                    bd.makeMove<1>(mv, &ddice);

                    if (bd.NInHouse[BLACK] == 0){//TakeOut
                        ++suc[4][0];
                    }
                    if (bd.NInHouse[WHITE] > 0){
                        ++suc[4][1];
                    }
                }

                //ウィークヒットによるテイクアウト
                suc[5][0] = 0;
                suc[5][1] = 0;
                for (int i = 0; i < N; ++i){
                    bd.init();

                    double r0 = ddice.drand(); double r1 = ddice.drand();
                    fpn_t x = FX_TEE + FR_HOUSE_RAD*r0*cos(r1*M_PI); fpn_t y = FY_TEE + FR_HOUSE_RAD*r0*sin(r1*M_PI);

                    bd.pushStone(BLACK, x, y);

                    mv.setSpin(dice.rand() % 2);
                    genStandardHit(&mv, bd.stone(BLACK, 0), HitWeight::WEAK, 1);

                    bd.makeMove<1>(mv, &ddice);

                    if (bd.NInHouse[BLACK] == 0){//TakeOut
                        ++suc[5][0];
                    }
                    if (bd.NInHouse[WHITE] > 0){
                        ++suc[5][1];
                    }
                }


                CERR << "TeeShot      " << suc[0][0] << "/" << N << endl;
                CERR << "ShortTeeShot " << suc[1][0] << "/" << N << endl;
                CERR << "RTeeShot     " << suc[2][0] << "/" << N << endl;
                CERR << "StrongHit    " << suc[3][0] << "/" << N << " " << suc[3][1] << "/" << N << endl;
                CERR << "MiddleHit    " << suc[4][0] << "/" << N << " " << suc[4][1] << "/" << N << endl;
                CERR << "WeakHit      " << suc[5][0] << "/" << N << " " << suc[5][1] << "/" << N << endl;
                //CERR<<"TakeOut      "<<suc[1]<<"/"<<N<<endl;
                //CERR<<"TakeOut      "<<suc[1]<<"/"<<N<<endl;
            }
        }*/
    }
}

int main(int argc, char* argv[]){
    
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    
    using namespace DigitalCurling;
    std::random_device seed;
    
    Tester::dice.srand(seed() * (unsigned int)time(NULL));
    Tester::ddice.srand(seed() * (unsigned int)time(NULL));
    
    AyumuAI ai;
    if(ai.initAll() < 0){
        cerr << "failed to initialize engine." << endl;
        return 1;
    }
    
    Tester::outputValue();
    if(Tester::testGame() < 0){
        cerr << "failed game test." << endl;
    }
    
    return 0;
}