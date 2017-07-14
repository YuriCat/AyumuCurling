/*
 interpolation_test.cc
 Katsuki Ohto
 */

// 局面情報の補完の有効性に対するテスト

#include "../simulation/fastSimulator.hpp"
#include "../simulation/primaryShot.hpp"

#include "../ayumu/structure/thinkBoard.hpp"

namespace DigitalCurling{
    namespace Tester{

        XorShift64 dice;
        DSFMT ddice;
        
        using testBoard_t = ThinkBoard;
        //using testBoard_t = MiniBoard;
        
        int testInterpolation(){
            // 連続関数の線形補完の妥当性を調べる
            
            cerr << "Interpolation Test : " << endl;
            for(double dr = 0.0001; dr < 1; dr *= 2){
                constexpr int N = 10000;
                
                int same[3] = {0};
                double d[3] = {0};
                
                for(int n = 0; n < N; ++n){
                    ThinkBoard obd;
                    ThinkBoard bd[4];
                    fMoveXY<> fmv[3];
                    ContactTree ct[3];
                    
                    obd.init();
                    pushStonesInHouse(&obd, BLACK, 4, &ddice);
                    pushStonesInHouse(&obd, WHITE, 3, &ddice);
                    
                    //obd.updateInfo();
                    
                    //cerr << obd << obd.front[0] << endl;
                    
                    const auto& fs = obd.stoneFront(0);
                    
                    double r = + M_PI / 2 + M_PI * ddice.drand();
                    
                    const fPosXY<> dst(fs.x + 2 * FR_STONE_RAD * sin(r), fs.y + 2 * FR_STONE_RAD * cos(r));
                    
                    // fmv[2] = (fmv[0] + fmv[1]) / 2 (interpolated)
                    
                    fmv[0].setSpin(static_cast<Spin>(dice.rand() % 2));
                    genHit(&fmv[0], dst, 32.5);
                    
                    fmv[2] = fmv[1] = fmv[0];
                    fmv[1].x = fmv[0].x + ((dice.rand() % 2) ? dr : (-dr));
                    
                    fmv[2].x = (fmv[0].x + fmv[1].x) / 2;
                    
                    //for(int i = 0; i < 3; ++i){ cerr << fmv[i] << endl; }
                    
                    for(int i = 0; i < 3; ++i){
                        bd[i] = obd;
                        ct[i] = makeMoveNoRand<0>(&bd[i], TURN_WHITE_LAST, fmv[i]);
                    }
                    
                    // interpolation
                    interpolate(bd[0], bd[1], &bd[3]);
                    
                    //cerr << endl;
                    //for(int i = 0; i < 4; ++i){ cerr << bd[i] << endl; }
                    
                    int result;
                    double dsum = calcDistanceSum(bd[2], bd[3]);
                    if((!ct[0].isComplex() && !ct[1].isComplex()) && ct[0].data() == ct[1].data()){
                        if(0 && dsum > 10){
                            cerr << dsum << ct[0] << ct[1] << endl;
                            cerr << obd << endl;
                            for(int i = 0; i < 4; ++i){ cerr << bd[i] << endl; }
                        }
                        if(ct[0].data() == ct[2].data()){
                            result = 0;
                        }else{
                            result = 1;
                        }
                    }else{
                        result = 2;
                    }
                    
                    d[result] += dsum;
                    same[result] += 1;
                    
                    //cerr << ct[0] << ct[1] << endl;
                }
                cerr << "dr = " << dr << endl;
                const std::string dis[] = {
                    "all same  : ",
                    "end same  : ",
                    "different : "
                };
                for(int i = 0; i < 3; ++i){
                     cerr << dis[i] << (same[i] ? (d[i] / same[i]) : 0) << " (in " << same[i] << " times)" << endl;
                }
            }
            return 0;
        }
    }
}

int main(int argc, char* argv[]){
    
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    using namespace DigitalCurling;
    std::random_device seed;
    bool seedSettled = false;
    
    for (int i = 1; i < argc; ++i){
        if (!strcmp(argv[i], "-s")){ // seed configuration
            uint64_t s = atoi(argv[i + 1]);
            Tester::dice.srand(s);
            Tester::ddice.srand(s);
            seedSettled = true;
        }
    }

    if(!seedSettled){
        Tester::dice.srand(seed() * (unsigned int)time(NULL));
        Tester::ddice.srand(seed() * (unsigned int)time(NULL));
    }
    Tester::testInterpolation();

    return 0;
}