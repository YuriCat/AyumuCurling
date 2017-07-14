/*
 logic_test.cc
 Katsuki Ohto
 */

// 各種ロジックが期待通りに働いているかのテスト

#include "../dc.hpp"
#include "../ayumu/ayumu_dc.hpp"

#include "../simulation/simuFunc.hpp"
#include "../simulation/b2dSimulator.hpp"
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
        
        template<class structure_t>
        void sz(const std::string& name){
            cerr << "sizeof(" << name << ") = " << sizeof(structure_t) << " bytes" << endl;
        }
        
        int testStructures(){
            
            // size of structures
            
            sz<fPosXY<fpn_t>>("fPosXY<fpn_t>");
            sz<MiniBoard>("MiniBoard");
            sz<StoneInfo>("StoneInfo");
            sz<RelativeStoneInfo>("RelativeStoneInfo");
            sz<ThinkBoard>("ThinkBoard");
            
            return 0;
        }
        
        int testPositionIndex(){
            std::ofstream ofs("position.txt");
            
            fPosXY<> pos;
            StoneInfo si;
            constexpr fpn_t stepX = FR_PA_WIDTH / (32 - 1);
            constexpr fpn_t stepY = FR_PA_LENGTH / (64 - 1);
            
            for(int l = 64 - 1; l >= 0; --l){
                for(int w = 0; w < 32; ++w){
                    si.set(FX_PA_MIN + w * stepX, FY_PA_MIN + l * stepY);
                    int lbl = getPositionIndex(si);
                    if(lbl >= 10){
                        ofs << lbl;
                    }else{
                        ofs << " " << lbl;
                    }
                    ofs << " ";
                }
                ofs << endl;
            }
            ofs << endl;
            return 0;
        }
        
        int testRelativePositionLabel(){
            // 石の相対位置の離散化が期待通りに動いているかのテスト
            std::ofstream ofs("label.txt");
            for(int t = 0; t < 10; ++t){
                
                fPosXY<> pos;
                locateInHouse(&pos, &ddice);
                StoneInfo si, si1;
                RelativeStoneInfo rel;
                si.set(pos);
                
                ofs << "sample " << t << " reference position : " << pos << endl;
                
                for(int l = 64; l >= 0; --l){
                    for(int w = 0; w <= 64; ++w){
                        if(l == 32 && w == 32){
                            ofs << " * ";
                        }else{
                            si1.set(pos.x + (w - 32) * FR_STONE_RAD / 2, pos.y + (l - 32) * FR_STONE_RAD / 2);
                            rel.set(si, si1);
                            int lbl = labelRelPos(si, si1, rel);
                            if(lbl == -1 || lbl >= 10){
                                ofs << lbl;
                            }else{
                                ofs << " " << lbl;
                            }
                            ofs << " ";
                        }
                    }
                    ofs << endl;
                }
                ofs << endl;
            }
            
            return 0;
        }
    }
}

int main(int argc, char* argv[]){

    using namespace DigitalCurling;
    std::random_device seed;

    Tester::dice.srand(seed() * (unsigned int)time(NULL));
    Tester::ddice.srand(seed() * (unsigned int)time(NULL));

    initAyumu(DIRECTORY_PARAMS_IN); // 初期化
    
    Tester::testRelativePositionLabel();
    
    Tester::testStructures();
    Tester::testPositionIndex();

    return 0;
}