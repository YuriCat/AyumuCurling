/*
 variance_test.cc
 Katsuki Ohto
*/

// 同一局面の評価にどの程度揺らぎがあるか調べる

#include "../ayumu/ayumu.hpp"

namespace DigitalCurling{
    namespace Ayumu{
        
        AyumuAI client;
        
        int testEvaluation(){
            
            // ログをファイルから読み込み
            RandomAccessor<AyumuShotLog> db;
            cerr << "started reading shot-log file." << endl;
            int games = readShotLog(DIRECTORY_SHOTLOG + "shotlog_middle.dat", &db,
                                    [&](const auto& shot)->bool{ return true; });
            cerr << db.size() << " shots in " << games << " games were imported." << endl;
            
            uint64_t time[3] = {0};
            double accuracySum[3] = {0};
            double distHistgram[3][SCORE_LENGTH * 2 - 1] = {0};
            std::string str[3] = {"ev", "pt1", "pt1"};
            Clock cl;

            int cnt = 0;
            iterate(db, [&](const auto& shot)->void{
                ThinkBoard bd, tbd;
                bd.setShotLog(shot);
                int ans = shot.escore;
                
                
                
                ++cnt;
                if(cnt % 10000 == 0){
                    cerr << "accuracy =";
                    for(int i = 0; i < 3; ++i){
                        cerr << "  " << str[i] << " : " << (accuracySum[i] / cnt);
                    }cerr << endl;
                }
            });
            cerr << "finished." << endl;
            for(int i = 0; i < 3; ++i){
                cerr << "  " << str[i] << " : " << endl;
                cerr << "accuracy = " << (accuracySum[i] / cnt) << endl;
                cerr << "histgram = [";
                for(int s = 0; s < SCORE_LENGTH * 2 - 1; ++s){
                    cerr << (s - SCORE_LENGTH + 1) << " " << distHistgram[i][s] << endl;
                }cerr << "]" << endl;
            }cerr << endl;
            
            return 0;
        }
    }
}

int main(int argc, char* argv[]){

    using namespace DigitalCurling;

    Ayumu::client.initAll();
    Ayumu::testEvaluation();
    return 0;
}