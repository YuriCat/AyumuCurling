/*
 dcl_counter.cc
 Katsuki Ohto\
 */

// dcl形式のログファイル読み込み
// 20151202時点のログ
// 試合の勝敗と平均得点を数えるだけ

#include <dirent.h>
#include <fstream>

#include "../dc.hpp"

using namespace std;

namespace DigitalCurling{
    namespace DCL{

#define ToV v.clear();split(v, str, boost::algorithm::is_space(), boost::algorithm::token_compress_on);
#define ToF(str) (atof((str).c_str()))
#define ToI(str) (atoi((str).c_str()))
        
        struct GameWL{
            int b, w;
            int lsc;
            int score[N_ENDS];
            string first, second;
            
            string toString()const{
                std::ostringstream oss;
                oss << "black : " << b;
                oss << " white : " << w;
                if(b == w){
                    oss << " lsc = " << lsc;
                }
                return oss.str();
            }
        };
        struct MatchWL{
            int g;
            int b, w, db, dw;
            int sb, sw, sbmax, swmax;
            
            void clear()noexcept{
                g = 0;
                b = w = db = dw = 0;
                sb = sw = 0;
                sbmax = swmax = 0;
            }
            void add(const GameWL& gwl)noexcept{
                //cerr << gwl.toString() << endl;
                if(gwl.b > gwl.w){
                    ++b;
                }else if(gwl.b < gwl.w){
                    ++w;
                }else{
                    if(gwl.lsc > 0){
                        ++db;
                    }else if(gwl.lsc < 0){
                        ++dw;
                    }
                }
                sb += gwl.b;
                sw += gwl.w;
                sbmax = max(sbmax, gwl.b);
                swmax = max(swmax, gwl.w);
                ++g;
            }
            std::string toString()const{
                std::ostringstream oss;
                oss << "black : " << b;
                oss << " white : " << w;
                oss << " draw_b : " << db;
                oss << " draw_w : " << dw;
                oss << " score :";
                oss << " b = " << sb << "(avg = " << sb / (double)g << ", max = " << sbmax << ")";
                oss << " w = " << sw << "(avg = " << sw / (double)g << ", max = " << swmax << ")";
                return oss.str();
            }
            
            MatchWL(){ clear(); }
        };
        
        int checkWL(GameWL *const pWL, const string& fName){
            cerr << fName << endl;
            ifstream ifs(fName);
            if(!ifs){ return -1; }
            
            vector<string> v;
            string str, lstr;
            pWL->first = "";
            pWL->second = "";
            
            while(ifs){
                getline(ifs, str, '\n');
                if(str.size() > 0 && str[str.size() - 1] == '\r'){
                    str = str.substr(0, str.size() - 1);
                }
                if(contains(str, "First=")){
                    vector<string> v = split(str, '=');
                    if(v.size() > 1){
                        pWL->first = v[1];
                        //cerr << "df " << pWL->first << endl;
                    }
                }else if(contains(str, "Second=")){
                    vector<string> v = split(str, '=');
                    if(v.size() > 1){
                        pWL->second = v[1];
                        //cerr << "s " << v[1] << endl;
                    }
                }else if(contains(str, "TOTALSCORE")){
                    vector<string> v = split(str, ' ');
                    pWL->b = ToI(v.at(1));
                    pWL->w = ToI(v.at(2));
                    
                    //cerr << pWL->b << " " << pWL->w << endl;
                    
                    if(pWL->b == pWL->w){
                        if(contains(lstr, "SCORE")){
                            vector<string> tv = split(lstr, ' ');
                            pWL->lsc = ToI(tv.at(1));
                        }else{
                            return -1;
                        }
                    }
                    //cerr << "lf " << pWL->first << endl;
                    cerr << pWL->first << " " << pWL->second << endl;
                    //cerr << "lf " << pWL->first << endl;
                    return 0;
                }
                
                lstr = str;
                
                //cerr << "f " << pWL->first << endl;
            }
            return -1;
        }

#undef ToI
#undef ToF
#undef ToV
        
        int countWL(const string& path){
            
            map<pair<string, string>, MatchWL> mwlTree; // 試合ごと
            MatchWL mwl; // 総計
            
            vector<string> dclFileName;
            
            DIR *pdir;
            dirent *pentry;
            
            pdir = opendir(path.c_str());
            if (pdir == nullptr){
                return -1;
            }
            do {
                pentry = readdir(pdir);
                if (pentry != nullptr){
                    dclFileName.emplace_back(path + string(pentry->d_name));
                }
            } while (pentry != nullptr);
            
            for (int i = 0; i < dclFileName.size(); ++i){
                if(dclFileName.at(i).find(".dcl") != string::npos){
                    GameWL gwl;
                    if(checkWL(&gwl, dclFileName.at(i)) >= 0){
                        mwl.add(gwl);
                        cerr << gwl.first << " - " << gwl.second << endl;
                        mwlTree[make_pair(gwl.first, gwl.second)].add(gwl);
                    }
                }
            }
            
            for(const auto& l : mwlTree){
                cerr << l.first.first << " - " << l.first.second << endl;
                cerr << l.second.toString() << endl;
            }
            cerr << "sum" << endl << mwl.toString() << endl;
            
            return 0;
        }
    }
}

int main(int argc, char* argv[]){
    
    using namespace DigitalCurling::DCL;
    
    string path;
    if(argc > 1){
        path = argv[1];
    }else{
        path = "./";
    }
    
    countWL(path);
}
