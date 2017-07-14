/*
 image_log_maker.cc
 Katsuki Ohto
 */

// 盤面の画像ログを作る
#include "../ayumu/ayumu_dc.hpp"
#include "../ayumu/model/image.hpp"

namespace DigitalCurling{
    
    // 使用する画像データの形式
    /*using Image = ImageVer0<28, 28, 1>;
    
    template<class board_t, class image_t>
    void genBoardImage(const board_t& bd, image_t *const pimg){
        genBoardImageVer0ByPixel<board_t, image_t>(bd, pimg);
    }*/
    using Image = ImageVer1;
    template<class board_t, class image_t>
    void genBoardImage(const board_t& bd, image_t *const pimg){
        genBoardImageVer1<board_t, image_t>(bd, pimg);
    }
    
    std::array<ImageLog<Image>, 1024> ila[N_TURNS];
    
    int makeImageLog(const std::string& slPath, const std::string& ilPath, int N){
        int nila[N_TURNS] = {0};
        std::ofstream ofs;
        
        std::string ilp[N_TURNS];
        for(int t = 0; t < N_TURNS; ++t){
            std::ostringstream oss;
            oss << ilPath << "_" << t << ".dat";
            ilp[t] = oss.str();
            
            // 画像データ保存先準備
            ofs.open(ilp[t], std::ios::out);
            ofs.close();
        }
        
        /*if(readShotLog(slPath, &slv, [](const auto& sl)->bool{ return true; }) < 0){
            return -1;
        }*/
        
        // shot log読み込み
        ShotLog *const psla = new ShotLog[N];
        FILE *const pf = fopen(slPath.c_str(), "rb");
        fread(psla, sizeof(ShotLog) * N, 1, pf);
        fclose(pf);
        
        for(int i = 0; i < N; ++i){
            const auto& sl = psla[i];
            
            ImageLog<Image> il;
            MiniBoard mbd;
            mbd.clearStones();
            for(int i = 0; i < N_STONES; ++i){
                if(isInPlayArea(sl.stone(i))){
                    mbd.locateNewStone(i, sl.stone(i));
                }
            }
            genBoardImage(mbd, &il);
            
            // generateの後にやる必要が有る
            il.end = sl.end;
            il.turn = sl.turn;
            il.rscore = sl.rscore;
            il.escore = sl.escore;
            
            ila[il.turn][nila[il.turn]++] = il;
            if(nila[il.turn] % 1024 == 0){
                std::ofstream ofs;
                ofs.open(ilp[il.turn], std::ios::app);
                for(int i = 0; i < nila[il.turn]; ++i){
                    ofs << ila[il.turn][i].toString() << endl;
                }
                nila[il.turn] = 0;
                ofs.close();
            }
        }
        for(int t = 0; t < N_TURNS; ++t){
            ofs.open(ilp[t], std::ios::app);
            for(int i = 0; i < nila[t]; ++i){
                ofs << ila[t][i].toString() << endl;
            }
            ofs.close();
        }
        return 0;
    }
}

int main(){
    
    using namespace DigitalCurling;
    
    //std::string slPath = DIRECTORY_SHOTLOG + "shotlog_mini.dat";
    //std::string ilPath = DIRECTORY_IMAGELOG + "imagelog_mini.dat";
    std::string slPath = DIRECTORY_SHOTLOG + "shotlog_ev.bin";
    std::string ilPath = DIRECTORY_IMAGELOG + "imagelog_ev";
    
    return makeImageLog(slPath, ilPath, 2700000);
}
