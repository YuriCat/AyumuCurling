// dcl形式のログファイル読み込み
// 20150913時点のログ

#include <dirent.h>
#include <fstream>

#include "../dc.hpp"

using namespace std;

namespace DigitalCurling{
    namespace DCL{

#define Get if(!ifs){ return pLog; }getline(ifs, str, '\n');\
if(str[str.size() - 1] == '\n'){ str.erase(str.size() - 1); }\
if(str[str.size() - 1] == '\r'){ str.erase(str.size() - 1); }
        
#define ToV v = split(str, ' ');
#define ToF(str) (atof((str).c_str()))
#define ToI(str) (atoi((str).c_str()))
   /*
        template<class log_t,class callback_t>
        int readToGameLog(log_t *const pLog,const std::string& fName,const callback_t& callback){
            
            ifstream ifs(fName);
            string str;
            vector<string> v;
            
            Get;
            pLog->setName(BLACK,trim(trim(str,'='),'\n'));
            Get;
            pLog->setTimeLimit(BLACK,trim(trim(str,'='),'\n'));
            Get;
            pLog->setName(WHITE,ToI(trim(trim(str,'='),'\n')));
            Get;
            pLog->setTimeLimit(WHITE,ToI(trim(trim(str,'='),'\n')));
            Get;
            pLog->setRandom(ToF(trim(trim(str,'='),'\n')));
            
            int ends = 10;
            int turns = 16;

            for(int e=0;e<=ends;++e){
                for(int t=0;t<turns;++t){

                    Get;
                    Get; ToV;//POSITION
                    for (int i = 0; i < 16; ++i){
                        pLog->setStonePosition(e, t, i, ToF(v[1 + i * 2]), ToF(v[1 + i * 2 + 1]));
                        pLog->setAfter
                    }
                    Get;//SETSTATE
                    Get;ToV//BESTSHOT
                    pLog->setChosenShot(e, t, i, ToF(v[1]), ToF(v[1 + 1]), ToI(v[1+2]));
                    Get; ToV;//RUNSHOT
                    pLog->setRunShot(e, t, i, ToF(v[1]), ToF(v[1 + 1]), ToI(v[1+2]));
                }

                Get; ToV;//POSITION (after last shot)
                for (int i = 0; i < 16; ++i){
                    pLog->setStonePosition(e, turns, i, ToF(v[1 + i * 2]), ToF(v[1 + i * 2 + 1]));
                }
                Get; ToV;//SCORE
                int sc = ToI(v[1 + i * 2]);
                pLog->setEndScore(e, sc);
            }
            Get; ToV;
            pLog->setTotalScore(ToI(v[1]), ToI(v[2]));
        }
        return 0;
	}
*/
        template<class callback_t>
        ShotLog* readDCLogToShotLog(ShotLog *const pLog0, const string& fName, int ends){
            
            cerr << fName << endl;
            
            ifstream ifs(fName);
            
            if(!ifs){ return pLog0; }
            
            ShotLog *pLog = pLog0;
            
            string str;
            vector<string> v;
            
            fPosXY<> officialPos;
            fPosXY<> ayumuPos;
            fMoveXY<> officialMove;
            fMoveXY<> ayumuMove;
            
            // 何度も使う変数
            string name[2];
            fpn_t random;
            int reversed = 0;
            int fcRelScore = 0;
            int relScore = 0;
            
            int inverseGame = -1; // 得点記録が反対になっている試合でないか
            
            Get;
            Get;
            name[BLACK] = str.substr(str.find_first_of("=") + 1, str.size());
            Get;
            //pLog->setTimeLimit(BLACK, trim(trim(str, '='), '\n'));
            Get;
            name[WHITE] = str.substr(str.find_first_of("=") + 1, str.size());
            Get;
            //pLog->setTimeLimit(WHITE, ToI(trim(trim(str, '='), '\n')));
            Get;
            
            //cerr<<str.substr(str.find_first_of("=") + 1, str.size()) << endl;
            
            random = ToF(str.substr(str.find_first_of("=") + 1, str.size()));
            
            int end_max = ends + END_LAST - 1;
            int turn_max = TURN_FIRST;
            
            const int turns = turn_max - TURN_LAST + 1;
            
            for (int e = end_max; e >= END_LAST; --e){
                for (int t = turn_max; t >= TURN_LAST; --t){
                    
                    pLog->setPlayer(name[reversed]);
                    pLog->setOppPlayer(name[1 ^ reversed]);
                    
                    pLog->random = random;
                    pLog->end = e;
                    pLog->turn = t;
                    pLog->rscore = relScore;
                    
                    //cerr << e << " " << t << endl;
                    
                    Get;
                    Get; ToV; // POSITION
                    for (int i = 0; i < 16; ++i){
                        officialPos.set(ToF(v[1 + i * 2]), ToF(v[1 + i * 2 + 1]));
                        ayumuPos = convPosition_Official_Ayumu(officialPos);
                        if(!isInPlayArea(ayumuPos)){
                            ayumuPos = FPOSXY_THROW;
                        }
                        pLog->previousStone[i].set(ayumuPos);
                        if (t != turn_max){
                            (pLog - 1)->afterStone[i].set(ayumuPos);
                        }
                    }
                    Get; // SETSTATE
                    if(inverseGame < 0){ // まだ先後どちらが先に書かれているか分からない
                        ToV;
                        int fc = ToI(v[4]);
                        if(fc == 0){
                            inverseGame = 0;
                        }else{
                            inverseGame = 1;
                        }
                    }
                    Get; ToV; // BESTSHOT
                    officialMove.set(ToF(v[1]), ToF(v[1 + 1]), static_cast<Spin>(ToI(v[1 + 2])));
                    ayumuMove = convMove_Official_Ayumu(officialMove);
                    pLog->chosenMove.set(ayumuMove);
                    Get; ToV; // RUNSHOT
                    officialMove.set(ToF(v[1]), ToF(v[1 + 1]), static_cast<Spin>(ToI(v[1 + 2])));
                    ayumuMove = convMove_Official_Ayumu(officialMove);
                    pLog->runMove.set(ayumuMove);
                    ++pLog;
                }
                
                Get;
                Get; ToV; // POSITION (after last shot)
                for (int i = 0; i < 16; ++i){
                    officialPos.set(ToF(v[1 + i * 2]), ToF(v[1 + i * 2 + 1]));
                    ayumuPos = convPosition_Official_Ayumu(officialPos);
                    if(!isInPlayArea(ayumuPos)){
                        ayumuPos = FPOSXY_THROW;
                    }
                    pLog->previousStone[i].set(ayumuPos);
                    (pLog - 1)->afterStone[i].set(ayumuPos);
                }
                Get; ToV; // SCORE
                int sc = ToI(v[1]);
                           
                if(inverseGame == 1){
                    sc = -sc;
                }
                           
                fcRelScore += sc;
                
                if (reversed){ sc = -sc; }
                
                for (auto *p = pLog - turns; p != pLog; ++p){
                    //cerr << sc;
                    p->escore = sc;
                }//cerr << endl;
                
                reversed ^= (sc < 0);
                if (reversed){ relScore = -fcRelScore; }
            }
            Get; ToV; // TOTAL SCORE
            int tsc = ToI(v[1]) - ToI(v[2]);
            if(inverseGame == 1){
                tsc = -tsc;
            }
            assert(tsc == fcRelScore);
            return pLog;
        }
#undef ToI
#undef ToF
#undef ToV
#undef Get
        
        template<class callback_t>
        int makeShotLog(const string& ipath, const string& opath, const int e, const int mode, const callback_t& callback){
            
            std::random_device seed;
            std::mt19937 mt(seed() ^ (unsigned int)time(NULL));
            
            ShotLog shot[1024];
            std::vector<ShotLog> v;
            
            vector<string> dclFileName;
            
            //DERR<<path<<endl;
            
            DIR *pdir;
            dirent *pentry;
            
            pdir = opendir(ipath.c_str());
            if (pdir == nullptr){
                return -1;
            }
            do {
                pentry = readdir(pdir);
                if (pentry != nullptr){
                    dclFileName.emplace_back(ipath + string(pentry->d_name));
                }
            } while (pentry != nullptr);
            
            for(auto& s : dclFileName){
                // cerr << s << " " << endl;
            }
            
            int loadedGames = dclFileName.size() - 2;
            
            cerr << loadedGames << " games were loaded." << endl;
            
            // ランダム並べ替え
            std::shuffle(dclFileName.begin(), dclFileName.end(), mt);
            
            for (int i = 0; i < dclFileName.size(); ++i){
                if(dclFileName.at(i).find(".dcl") != string::npos){
                    ShotLog *last = readDCLogToShotLog<ShotLog>(shot, dclFileName.at(i), e);
                    
                    for (ShotLog *pl = shot; pl != last; ++pl){
                        
                        if(callback(*pl)){
                            v.emplace_back(*pl);
                            pl->flip(); // 反転局面
                            v.emplace_back(*pl);
                        }
                    }
                }
            }
            if(mode & 4){
                // train & testモード
                int H = v.size() * 0.8;
                // ランダム並べ替え
                std::shuffle(v.begin(), v.begin() + H, mt);
                std::shuffle(v.begin() + H, v.end(), mt);
                // 文字列出力
                ofstream ofs;
                ofs.open(opath + "_train.dat", ios::out);
                for(int i = 0; i < H; ++i){
                    ofs << v[i].toString() << endl;
                }
                ofs.close();
                ofs.open(opath + "_test.dat", ios::out);
                for(int i = H; i < v.size(); ++i){
                    ofs << v[i].toString() << endl;
                }
                ofs.close();
            }else{
                // 通常
                // ランダム並べ替え
                std::shuffle(v.begin(), v.end(), mt);
                
                if(mode & 2){
                    // 文字列出力
                    ofstream ofs;
                    ofs.open(opath + ".dat", ios::out);
                    for(const auto& l : v){
                        ofs << l.toString() << endl;
                    }
                    ofs.close();
                }else{
                    // バイナリ出力
                    int N = v.size();
                    FILE *pf = fopen((opath + ".bin").c_str(), "wb");
                    fwrite(v.data(), sizeof(ShotLog) * N, 1, pf);
                    fclose(pf);
                    cerr << N << " states (in " << loadedGames << " games) were dumped." << endl;
                }
            }
            
            return 0;
        }
    }
}

int main(int argc, char *argv[]){
    
    using namespace DigitalCurling;
    using namespace DigitalCurling::DCL;
    string DIRECTORY_PARAMS_IN(""), DIRECTORY_PARAMS_OUT(""), DIRECTORY_DCLOG(""), DIRECTORY_SHOTLOG("");
    
    ifstream ifs("config.txt");
    ifs >> DIRECTORY_PARAMS_IN;
    ifs >> DIRECTORY_PARAMS_OUT;
    ifs >> DIRECTORY_DCLOG;
    ifs >> DIRECTORY_SHOTLOG;
    
    string ipath = DIRECTORY_DCLOG;
    
    string opath = DIRECTORY_SHOTLOG + "shotlog";
    string opath_ev = DIRECTORY_SHOTLOG + "shotlog_ev";
    string opath_pol = DIRECTORY_SHOTLOG + "shotlog_pol";
    
    string ipath_test = DIRECTORY_DCLOG + "../dclog_tmp/";
    string opath_test = DIRECTORY_SHOTLOG + "shotlog_tmp";
    
    int e = 10;
    int mode = 0; // normal
    int test = 0;
    
    for (int i = 1; i < argc; ++i){
        if(!strcmp(argv[i], "-c")){ // numcheck mode
            mode |= 1;
        }if(!strcmp(argv[i], "-d")){ // dat mode
            mode |= 2;
        }else if(!strcmp(argv[i], "-e")){ // num of ends
            e = atoi(argv[i + 1]);
        }else if(!strcmp(argv[i], "-i")){ // dcl directory path
            ipath = string(argv[i + 1]);
        }else if(!strcmp(argv[i], "-o")){ // shot log path
            opath = string(argv[i + 1]);
        }else if(!strcmp(argv[i], "-t")){ // test mode
            test = 1;
        }else if(!strcmp(argv[i], "-v")){ // validation mode
            mode |= 4;
        }
    }
    
    if(test){
        makeShotLog(ipath_test, opath_test, e, mode, [](const auto& shot)->bool{ // 全部
            return true;
        });
    }else{
        makeShotLog(ipath, opath, e, mode, [](const auto& shot)->bool{ // 全部
            return true;
        });
        makeShotLog(ipath, opath_ev, e, mode, [](const auto& shot)->bool{ // 評価値学習用
            if(shot.end == END_LAST && shot.turn < 8){ return false; } // 最終盤は得点差の影響が大きいので除外
            if(abs(shot.rscore) > min(5, (shot.end + 1) * 2)){ return false; } // 得点差が大きい場合は適当な手が多いので除外
            return true;
        });
        makeShotLog(ipath, opath_pol, e, mode, [](const auto& shot)->bool{ // 方策学習用
            if(shot.end == END_LAST && shot.turn < 8){ return false; } // 最終盤は得点差の影響が大きいので除外
            if(abs(shot.rscore) > min(5, (shot.end + 1) * 2)){ return false; } // 得点差が大きい場合は適当な手が多いので除外
            if(!(contains(shot.player(), "umu")
                 || contains(shot.player(), "kun")
                 || contains(shot.player(), "GCCS")
                 || contains(shot.player(), "CSACE"))){ // 指定したプレーヤーのみ
                return false;
            }
            return true;
        });
    }
    
    return 0;
}
