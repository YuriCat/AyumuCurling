/*
 error_test.cc
 Katsuki Ohto
 */

// 着手誤差のテスト
#include "../ayumu/ayumu_dc.hpp"

#include "../simulation/simuFunc.hpp"
#include "../simulation/b2dSimulator.hpp"
#include "../simulation/fastSimulator.hpp"
#include "../simulation/error.hpp"
//#include "../simulation/intSimulator.hpp"
#include "../simulation/primaryShot.hpp"

//#include "../ayumu/structure/thinkBoard.hpp"

namespace DigitalCurling{
    namespace Tester{

        XorShift64 dice;
        DSFMT ddice;
        
        //using testBoard_t = ThinkBoard;
        using testBoard_t = MiniBoard;

        int testError(){
            std::random_device seed;
            std::mt19937 dice(seed() ^ (unsigned int)time(NULL));
            std::normal_distribution<double> norm(0, F_ERROR_SIGMA);
            
            // 乱数の着手、手へのかかり方のテスト
            {
                fMoveXY<> teeShot;
                teeShot.setSpin(Spin::RIGHT);
                genDraw(&teeShot, FPOSXY_TEE);

                for (int ith = 0; ith < 16; ++ith){
                    fPosXY<> fpos(FX_TEE + 1.0 * sin(ith * 2 * M_PI / 16), FY_TEE + 1.0 * cos(ith * 2 * M_PI / 16));
                    fMoveXY<> draw;
                    draw.setSpin(Spin::RIGHT);
                    genDraw(&draw, fpos);

                    cerr << "dvx : " << (draw.x - teeShot.x) << " dvy : " << (draw.y - teeShot.y)
                        << " dv : " << XYtoR(draw.x - teeShot.x, draw.y - teeShot.y) << endl;

                }
            }

            // ショットのぶれの分布についてのテスト
            fPosXY<> fpos(FX_PA_MIN + 0.1, FY_PA_MAX + 2.0);
            //fPosXY<> fpos=FPOSXY_TEE;
            fMoveXY<> fmv;
            fmv.setSpin(Spin::RIGHT);
            FastSimulator::FXYtoFMV(fpos, &fmv);

            //cerr<<fpos<<" "<<fmv<<endl;

            constexpr int N = 20000;
            constexpr int R = 32;
            const double range = norm.stddev() * 6 / (double)R;

            int distNorm[R];
            int distRand[2][R] = {0};

            for (int n = 0; n < N; ++n){
                fMoveXY<> tmv;
                addRandToMove(&tmv, fmv, &ddice);
                fPosXY<> tpos;
                FastSimulator::FMVtoFXY(tmv, &tpos);

                int i;

                //cerr<<tmv<<" "<<tpos<<" "<<(tpos.x-fpos.x)<<endl;

                i = max(0, min(R - 1, int((tpos.x - fpos.x) / range + (double)R / 2)));
                distRand[0][i]++;
                i = max(0, min(R - 1, int((tpos.y - fpos.y) / range + (double)R / 2)));
                distRand[1][i]++;
            }

            auto func = [](double x, double m, double s)->double{
                return exp(-0.5 * (x - m) * (x - m) / (s * s)) / sqrt(2.0 * M_PI * s * s);
            };

            auto afunc = [](double x, double m, double s)->double{
                return 0.5 * (1 + erf((x - m) / sqrt(2.0 * s * s)));
            };

            for (int i = 0; i < R; ++i){
                distNorm[i] = int((-afunc((i - R / 2) * range, norm.mean(), norm.stddev())
                    + afunc((i - R / 2 + 1) * range, norm.mean(), norm.stddev()))
                    * N + 0.5);
            }
            for (int i = 0; i < R; ++i){
                cerr << distNorm[i] << " ";
            }cerr << endl;
            for (int d = 0; d < 2; ++d){
                for (int i = 0; i < R; ++i){
                    cerr << distRand[d][i] << " ";
                }cerr << endl;
            }
            
            // vx,vy空間での分布近似計算のテスト
            {
                for(int s = 0; s < 2; ++s){
                    cerr << "*** Spin = " << toSpinString(s) << " *** " << endl;
                    constexpr int N = 400000;
                    { // official function (rand generator)
                        double sum[2] = {0}, sum2[2] = {0};
                        int distribution[10][10] = {0}, distribution_else = 0;
                        fpn_t dvx, dvy;
                        for (int i = 0; i < N; ++i){
                            //genRandToMoveXY(&dvx, &dvy, s, &ddice);
                            fMoveXY<> fmv(0, 0, Spin::RIGHT), fmvNoise(0, 0, Spin::RIGHT);
                            fPosXY<> pos(FX_TEE - 1.22, FY_TEE);
                            Simulator::calcDeparture(pos, &fmv);
                            addRandToPos(&pos, &ddice);
                            //cerr << pos;
                            Simulator::calcDeparture(pos, &fmvNoise);
                            dvx = fmvNoise.x - fmv.x;
                            dvy = fmvNoise.y - fmv.y;
                            
                            /*FastSimulator::FMVtoFXY(fmvNoise, &pos);
                            cerr << " " << pos << endl;
                            dvx = pos.x - FX_TEE - 2;
                            dvy = pos.y - FY_TEE;*/
                            
                            sum[0] += dvx; sum2[0] += dvx * dvx;
                            sum[1] += dvy; sum2[1] += dvy * dvy;
                            int ix = floor(dvx / 0.0831305) + 5;
                            int iy = floor(dvy / 0.0832915) + 5;
                            if(0 <= ix && ix < 10 && 0 <= iy && iy < 10){
                                distribution[ix][iy] += 1;
                            }else{
                                distribution_else += 1;
                            }
                            //cerr << dvx << "," << dvy << endl;
                        }
                        cerr << "Official Error Generator (Vx, Vy)" << endl;
                        cerr << "mean  : " << sum[0] / N << ", " << sum[1] / N << endl;
                        cerr << "sigma : "
                        << sqrt((sum2[0] / N) - pow(sum[0] / N, 2)) << ", "
                        << sqrt((sum2[1] / N) - pow(sum[1] / N, 2)) << endl;
                        cerr << "distribution : " << endl;
                        for(int iy = 0; iy < 10; ++iy){
                            for(int ix = 0; ix < 10; ++ix){
                                cerr << std::setw(6) << distribution[ix][iy];
                            }cerr << endl;
                        }
                        cerr << "else = " << distribution_else << endl;
                    }
                    { // approximate function (pdf)
                        double sum[2] = {0}, sum2[2] = {0}, pdfSum = 0, pdfSumXY = 0;
                        //int distribution[9][9] = {0};
                        for (int i = 0; i < 400; ++i){
                            for (int j = 0; j < 400; ++j){
                                double dvx = -1 + i / (double)(400 - 1) * 2;
                                double dvy = -1 + j / (double)(400 - 1) * 2;
                                
                                double pdfVal = pdfDeltaVxVy(dvx, dvy, static_cast<Spin>(s));
                                //cerr << pdfVal << " ";
                                sum[0] += dvx * pdfVal; sum2[0] += dvx * dvx * pdfVal;
                                sum[1] += dvy * pdfVal; sum2[1] += dvy * dvy * pdfVal;
                                pdfSum += pdfVal;
                                pdfSumXY += pdfTeeDeltaXY(dvx, dvy);
                            }
                        }
                        //cerr << pdfSum * pow(10 / 200.0, 2) << endl;
                        //cerr << pdfSumXY * pow(10 / 200.0, 2) << endl;
                        //cerr << sum2[0] << endl;
                        sum[0] /= pdfSum; sum[1] /= pdfSum;
                        sum2[0] /= pdfSum; sum2[1] /= pdfSum;
                        cerr << "Approximate Error PDF (Vx, Vy)" << endl;
                        cerr << "mean  : " << sum[0] / N << ", " << sum[1] / N << endl;
                        cerr << "sigma : "
                        << sqrt(sum2[0] - pow(sum[0], 2)) << ", "
                        << sqrt(sum2[1] - pow(sum[1], 2)) << endl;
                        for(int iy = 0; iy < 10; ++iy){
                            for(int ix = 0; ix < 10; ++ix){
                                fpn_t dvx = (ix - 4.5) * 0.0831305;
                                fpn_t dvy = (iy - 4.5) * 0.0832915;
                                cerr << std::setw(6) << (int)(pdfDeltaVxVy(dvx, dvy, static_cast<Spin>(s)) / 160 * N);
                            }cerr << endl;
                        }
                    }
                }
            }
            return 0;
        }
        
        int testLogError(const std::string& logName){
            
            // ログをファイルから読み込み
            std::vector<ShotLog> db;
            
            cerr << "started reading " + logName << "." << endl;
            int games = readShotLog(logName, &db, [](const auto& shot)->bool{ return true; });
            cerr << "finished reading." << endl;
            
            for(int s = 0; s < 2; ++s){
                
                cerr << "*** Spin = " << toSpinString(s) << " *** " << endl;
                
                fpn_t sum[2] = {0}, sum2[2] = {0};
                int distribution[10][10] = {0}, distribution_else = 0;
                int N = 0;
                
                for(ShotLog& shot : db){
                    
                    if(s != shot.chosenMove.spin()){ continue; }
                    
                    fpn_t dvx = shot.runMove.vx() - shot.chosenMove.vx();
                    fpn_t dvy = shot.runMove.vy() - shot.chosenMove.vy();
                    
                    sum[0] += dvx; sum2[0] += dvx * dvx;
                    sum[1] += dvy; sum2[1] += dvy * dvy;
                    
                    int ix = floor(dvx / 0.0831305) + 5;
                    int iy = floor(dvy / 0.0832915) + 5;
                    
                    if(0 <= ix && ix < 10 && 0 <= iy && iy < 10){
                        distribution[ix][iy] += 1;
                    }else{
                        distribution_else += 1;
                    }
                    ++N;
                }
                
                if(N <= 0){ cerr << "no data." << endl; continue; }
                
                cerr << "Real(dclog) Error PDF (Vx, Vy)" << endl;
                cerr << "mean  : " << sum[0] / N << ", " << sum[1] / N << endl;
                cerr << "sigma : "
                << sqrt((sum2[0] / N) - pow(sum[0] / N, 2)) << ", "
                << sqrt((sum2[1] / N) - pow(sum[1] / N, 2)) << endl;
                cerr << "distribution : " << endl;
                for(int iy = 0; iy < 10; ++iy){
                    for(int ix = 0; ix < 10; ++ix){
                        cerr << std::setw(6) << distribution[ix][iy];
                    }cerr << endl;
                }
                cerr << "else = " << distribution_else << endl;
            }
            return 0;
        }
    }
}

int main(int argc, char *argv[]){
    
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    using namespace DigitalCurling;
    std::random_device seed;

    Tester::dice.srand(seed() * (unsigned int)time(NULL));
    Tester::ddice.srand(seed() * (unsigned int)time(NULL));

    std::string ipath = DIRECTORY_SHOTLOG + "shotlog.dat";
    
    for (int i = 1; i < argc; ++i){
        if(!strcmp(argv[i], "-l")){ // shot log path
            ipath = std::string(argv[i + 1]);
        }else if(!strcmp(argv[i], "-o")){ // shot log path
            //opath = std::string(argv[i + 1]);
        }
    }
    
    //FastSimulator::init();
    Tester::testError();
    //Tester::testLogError(ipath);

    return 0;
}