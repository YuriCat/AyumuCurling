//前計算テーブルの作成

#include "../dc.hpp"
#include "../ayumu/ayumu_dc.hpp"

#include "../simulation/fastSimulator.hpp"
#include "../simulation/primaryShot.hpp"
#include "../ayumu/shot/allShots.hpp"

#include "../ayumu/logic/logic.hpp"
#include "../ayumu/structure/thinkBoard.hpp"

namespace DigitalCurling{
    
    XorShift64 dice;
    DSFMT ddice;
    
    /*int genDoubleTable(){
        cerr
        << Double::RESOLUTION_R * Double::RESOLUTION_TH * 2 << " "
        << Double::RESOLUTION_R * Double::RESOLUTION_TH * 2 * 2 / 3 << " "
        << Double::RESOLUTION_R * Double::RESOLUTION_TH * 2 - Double::RESOLUTION_R * Double::RESOLUTION_TH * 2 * 2 / 3 << " "
        << 3 << endl;
        
        float tmpDoubleTable[Double::RESOLUTION_R * Double::RESOLUTION_TH * 2][2];
#if N_THREADS > 1
        std::thread thr[N_THREADS];
        for (int th = 0; th < N_THREADS; ++th){
            thr[th] = thread(&genDoubleTableThread<DSFMT>, th, N_THREADS, tmpDoubleTable, &ddice);
        }
        for (int th = 0; th < N_THREADS; ++th){
            thr[th].join();
        }
#else
        genDoubleTableThread<DSFMT>(0, 1, tmpDoubleTable, &ddice);
#endif
    
        std::ofstream ofs(DIRECTORY_PARAMS_OUT + "param_double.dat");
        for (int i = 0; i<Double::RESOLUTION_R * Double::RESOLUTION_TH * 2; ++i){
            for (int j = 0; j < 2; ++j){
                ofs << tmpDoubleTable[i][j] << " ";
            }
            ofs << std::endl;
        }
        
        return 0;
    }*/
    
    int genTypicalValue(){
        cerr << "Move Value" << endl;
        fMoveXY<> fmv(0, 0, Spin::RIGHT);
        
        cerr << "tee " << endl;
        genDraw(&fmv, FPOSXY_TEE); cerr << fmv << endl;
        cerr << "front of house ";
        genDraw(&fmv, fPosXY<>(FX_TEE, FY_TEE - FR_IN_HOUSE)); cerr << fmv << endl;
        cerr << "back of house ";
        genDraw(&fmv, fPosXY<>(FX_TEE, FY_TEE + FR_IN_HOUSE)); cerr << fmv << endl;
        cerr << "right-side of house ";
        genDraw(&fmv, fPosXY<>(FX_TEE + FR_IN_HOUSE, FY_TEE)); cerr << fmv << endl;
        cerr << "left-side of house ";
        genDraw(&fmv, fPosXY<>(FX_TEE - FR_IN_HOUSE, FY_TEE)); cerr << fmv << endl;
        
        cerr << "Position Value" << endl;
        fPosXY<> pos(FX_TEE, FY_TEE);
        
        cerr << "Vx = 0, Vy = LEGAL_MAX ";
        FastSimulator::FMVtoFXY(fMoveXY<>(0, FVY_LEGAL_MAX, Spin::RIGHT), &pos);
        cerr << pos << endl;
        
        return 0;
    }
    
    int genDrawTable(){
        cerr << "start making draw table." << endl;
        
        std::ofstream ofs(DIRECTORY_PARAMS_OUT + "draw_prob.dat");
        
        // ノータッチでドロー出来る確率テーブル
        double tmpGuardProbTable[GVBBTABLE_WIDTH + 1][GVBBTABLE_LENGTH + 1][64] = {0};
        fPosXY<> drawPos[64];
        fMoveXY<> drawMv[64];
        
        for(int i = 0; i < 64; ++i){
            genVortexDrawPos(&drawPos[i], i);
            
            drawMv[i].setSpin(Spin::RIGHT);
            genDraw(&drawMv[i], drawPos[i]);
            cerr << i << " : " << drawPos[i] << drawMv[i] << endl;
        }
        getchar();
        
        for(int w = 0; w <= GVBBTABLE_WIDTH; ++w){
            for(int l = 0; l <= GVBBTABLE_LENGTH; ++l){
                
                // ガード石のポジション
                const fpn_t x = FX_PA_MIN + FR_PA_WIDTH / GVBBTABLE_WIDTH * w;
                const fpn_t y = FY_PA_MIN + (FR_PA_LENGTH + 2 * FR_STONE_RAD) / GVBBTABLE_LENGTH * l;
                
                // VxとVyの候補
                constexpr fpn_t minVx = FVX_LEFT_HOUSE - 5 * FVX_ERROR_SIGMA;
                constexpr fpn_t maxVx = FVX_RIGHT_HOUSE + 5 * FVX_ERROR_SIGMA;
                
                constexpr fpn_t minVy = FVY_FRONT_HOUSE - 5 * FVY_ERROR_SIGMA;
                constexpr fpn_t maxVy = FVY_BACK_HOUSE + 5 * FVY_ERROR_SIGMA;
                
                // それぞれのVx, Vyで衝突するかの情報
                bool tmpContactVxVyTable[256 + 1][256 + 1] = {0};
                for(int vw = 0; vw <= 256; ++vw){
                    for(int vl = 0; vl <= 256; ++vl){
                        fMoveXY<> mv(minVx + (maxVx - minVx) * vw / 256,
                                     minVy + (maxVy - minVy) * vl / 256,
                                     Spin::RIGHT);
                        //fPosXY<> dst;
                        //FastSimulator::FMVtoFXY(mv, &dst); // ドローした位置に置く
                        //bool con = FastSimulator::hasCollisionDraw(fPosXY<>(FX_THROW, FY_THROW),
                        //dst, fPosXY<>(x, y), Spin::RIGHT);
                        
                        MiniBoard mbd;
                        mbd.pushStone(BLACK, fPosXY<>(x, y));
                        bool con = FastSimulator::simulateF(&mbd, TURN_LAST, mv).hasContact();
                        
                        if(con){
                            // 衝突可能性ありと判定
                            tmpContactVxVyTable[vw][vl] = 1;
                        }
                    }
                }
                // 64個のドロー点に対して衝突確率を計算
                for(int i = 0; i < 64; ++i){
                    double pdsum = 0;
                    double sum = 0;
                    for(int vw = 0; vw <= 256; ++vw){
                        for(int vl = 0; vl <= 256; ++vl){
                            fpn_t vx = minVx + (maxVx - minVx) * vw / 256;
                            fpn_t vy = minVy + (maxVy - minVy) * vl / 256;
                            
                            double pd = pdfDeltaVxVy(vx - drawMv[i].vx(),
                                                     vy - drawMv[i].vy(),
                                                     Spin::RIGHT);
                            
                            pdsum += pd;
                            sum += pd * tmpContactVxVyTable[vw][vl];
                        }
                    }
                    assert(pdsum > 0);
                    tmpGuardProbTable[w][l][i] = sum / pdsum;
                    cerr << tmpGuardProbTable[w][l][i] << endl;
                    ofs << tmpGuardProbTable[w][l][i] << endl;
                }
            }
        }

        
        return 0;
    }
    
}

int main(int argc, char* argv[]){

    using namespace DigitalCurling;
    
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    
    std::random_device seed;

    dice.srand(seed() * (unsigned int)time(NULL));
    ddice.srand(seed() * (unsigned int)time(NULL));

    initShotParams();
    
    //genDoubleTable();
    //genTypicalValue();
    genDrawTable();
    
    return 0;
}