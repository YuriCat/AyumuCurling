/*
 eval_test.cc
 Katsuki Ohto
*/

// 局面評価やシミュレーションがどの程度それっぽい結果を出しているか調べる

#include "../ayumu/ayumu.hpp"

namespace DigitalCurling{
    namespace Ayumu{
        
        AyumuAI client;
        
        int testEvaluation(){
            
            // ログをファイルから読み込み
            RandomAccessor<AyumuShotLog> db;
            cerr << "started reading shot-log file." << endl;
            int games = readShotLog(DIRECTORY_SHOTLOG + "shotlog.dat", &db,
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
                
                // 評価関数
                {
                    cl.start();
                    eval_t score[SCORE_LENGTH];
                    eval_t scoreSum;
                    calcEvalScore<0>(score, &scoreSum, bd, gEstimator);
                    time[0] += cl.stop();
                    
                    accuracySum[0] += score[StoIDX(ans)] / scoreSum;
                    for(int s = SCORE_MIN; s < SCORE_MAX; ++s){
                        distHistgram[0][s - ans + SCORE_LENGTH - 1] += score[StoIDX(s)] / scoreSum;
                    }
                }
                
                // シミュレーション(温度1)
                {
                    tbd = bd;
                    
                    cl.start();
                    int sc = doPlayout(tbd, &threadTools[0]);
                    time[1] += cl.stop();
                    
                    accuracySum[1] += (sc == ans);
                    distHistgram[1][sc - ans + SCORE_LENGTH - 1] += 1;
                }
                
                // シミュレーション(温度0)
                {
                    tbd = bd;
                    cl.start();
                    while (tbd.getTurn() > TURN_LAST){
                        fMoveXY<> mv;
                        playWithBestPolicy(&mv, tbd, gPolicy, &threadTools[0]);
                        tbd.procTurn();
                    }
                    playTurnPolicy(tbd, &threadTools[0]);
                    tbd.updateInfoLast();
                    int sc =  tbd.countScore();
                    time[2] += cl.stop();
                    accuracySum[2] += (sc == ans);
                    distHistgram[2][sc - ans + SCORE_LENGTH - 1] += 1;
                }
                
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