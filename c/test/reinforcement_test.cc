/*
 reinforcement_test.cc
 Katsuki Ohto
 */

#include "../ayumu/ayumu_dc.hpp"
#include "../simulation/fastSimulator.hpp"
#include "../simulation/primaryShot.hpp"

#include "../ayumu/shot/takeOut.hpp"

#include "../ayumu/initialize.hpp"

namespace DigitalCurling{
    namespace Tester{

        XorShift64 dice;
        
        struct Gaussian{
            
            fpn_t mu, a, b;
            
            constexpr static fpn_t alpha = 0.0007;
            constexpr static fpn_t sigma = FR_STONE_RAD / 2;
            //constexpr static fpn_t sigma = 4 * sqrt(2 * F_FRIC_RINK_STONE * FR_STONE_RAD / 2);
            
            template<class fmove_t>
            void gen(fmove_t *const pmv)const{
                pmv->addVx(mu);
            }
            
            void feed(fpn_t tev, fpn_t dvx, fpn_t dvy)noexcept{
                const fpn_t k = 1 / (sqrt(2 * M_PI) * sigma);
                const fpn_t e = exp(-pow(dvx, 2) / sigma);
                const fpn_t dvdm = sqrt(2 / M_PI) / pow(sigma, 2) * dvx * e;
                const fpn_t dvda = 2 * k * e;
                const fpn_t dvdb = 1;
                const fpn_t vdist = tev - (a * k * e + b);
                
                mu += alpha * 2 * dvdm * vdist;
                a += alpha * 2 * dvda * vdist;
                b += alpha * 2 * dvdb * vdist;
            }
            void init()noexcept{
                mu = 0;
                a = sqrt(2 * M_PI) * sigma;
                b = 0;
            }
        };
        /*
        struct DiscreteBox{
            
            constexpr int N = 8;
            fpn_t r[N];
            
            constexpr static fpn_t alpha = 0.0005;
            
            template<class fmove_t>
            void gen(fmove_t *const pmv)const{
                pmv->addVx(mu);
            }
            
            void feed(fpn_t tev, fpn_t dvx, fpn_t dvy)noexcept{
                const fpn_t k = 1 / (sqrt(2 * M_PI) * sigma);
                const fpn_t e = exp(-pow(dvx, 2) / sigma);
                const fpn_t dvdm = sqrt(2 / M_PI) / pow(sigma, 2) * dvx * e;
                const fpn_t dvda = 2 * k * e;
                const fpn_t dvdb = 1;
                const fpn_t vdist = tev - (a * k * e + b);
                
                mu += alpha * 2 * dvdm * vdist;
                a += alpha * 2 * dvda * vdist;
                b += alpha * 2 * dvdb * vdist;
            }
            void init()noexcept{
                mu = 0;
                a = sqrt(2 * M_PI) * sigma;
                b = 0;
            }
        };*/
        
        template<class learner_t>
        int testFitting(learner_t *const plearner){
            // 最適点探索
            int samples = 1000;
            int trials = 500;
            
            double evSum[2][5] = {0};
            
            // 前方ガード
            for(int s = 0; s < samples; ++s){
                // テスト局面作成
                MiniBoard bd;
                fPosXY<> st, gd;
                locateInHouse(&st, &dice);
                bd.locateStone(15, st); // 狙う石
                
                genPostGuardPosition(&gd, st, dice.rand() % PostGuardLength::MAX);
                gd.x = gd.x + (dice.drand() - 0.5) * FR_STONE_RAD * 2;
                bd.locateStone(13, gd); // ガード
                
                MoveXY mv;
                mv.setTakeOut(dice.rand() % 2, 15);
                
                // 着手の生成
                fMoveXY<> fmv;
                fmv.setSpin(mv.spin());
                genHit(&fmv, st, 32.5);
                
                //cerr << bd << endl;
                //cerr << fmv << endl;
                
                // 評価
                double ev[2] = {0};
                
                // 学習なし
                for(int t = 0; t < trials; ++t){
                    MiniBoard tbd = bd;
                    makeMove<1>(&tbd, 0, fmv, &dice);
                    double tev = TakeOut::evaluate(bd, tbd, 0, mv);
                    
                    ev[0] += tev;
                    evSum[0][0] += tev;
                }
                
                // 学習あり
                plearner->init();
                
                for(int t = 0; t < trials; ++t){
                    MiniBoard tbd = bd;
                    fMoveXY<> tmv = fmv;
                    
                    plearner->gen(&tmv);
                    
                    fpn_t dvx, dvy;
                    genRandToMoveXY(&dvx, &dvy, tmv.spin(), &dice);
                    
                    tmv.addVx(dvx); tmv.addVy(dvy);
                    
                    makeMoveNoRand<1>(&tbd, 0, tmv);
                    double tev = TakeOut::evaluate(bd, tbd, 0, mv);
                    
                    ev[1] += tev;
                    evSum[1][0] += tev;
                    
                    // パラメータを更新
                    plearner->feed(tev, dvx, dvy);
                    
                    //cerr << mu << endl; getchar();

                }
                
                //cerr << "Settle : " << ev[0] / trials << endl;
                //cerr << "Learn  : " << ev[1] / trials << endl;
                
            }
            
            cerr << "Settle : " << evSum[0][0] / (trials * samples) << endl;
            cerr << "Learn  : " << evSum[1][0] / (trials * samples) << endl;
            
            // 後方ガード
            
            
            
            
            return 0;
        }
    }
}

int main(int argc, char *argv[]){

    using namespace DigitalCurling::Tester;
    std::random_device seed;
    
    dice.srand(seed() * (unsigned int)time(NULL));

    std::string ipath = DigitalCurling::DIRECTORY_SHOTLOG + "shotlog.dat";
    DigitalCurling::initShot(DigitalCurling::DIRECTORY_PARAMS_IN);
    
    for (int i = 1; i < argc; ++i){
        if(strstr(argv[i], "-l")){ // shot log path
            ipath = std::string(argv[i + 1]);
        }else if(strstr(argv[i], "-o")){ // shot log path
            //opath = string(argv[i + 1]);
        }
    }

    Gaussian g;
    //Beta;
    
    testFitting(&g);

    return 0;
}