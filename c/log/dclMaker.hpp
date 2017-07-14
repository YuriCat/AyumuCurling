/*
 dclMaker.hpp
 Katsuki Ohto
 */

// dcl形式のログファイルの作成

#include "../dc.hpp"
#include "../structure/field.hpp"

using namespace std;

namespace DigitalCurling{
    namespace DCL{
        
        std::string p6Float(float f){
            // dcl形式での不動小数点文字列
            char tmp[32];
            //cerr << f << endl;
            snprintf(tmp, sizeof(tmp), "%.6f", f);
            return std::string(tmp);
        }
        
        template<class field_t>
        std::string toDclString(const field_t& field){
            // dcl形式のログ文字列の作成
            std::ostringstream oss;
            char tmp[64];
            memset(tmp, 0, sizeof(tmp));
            
            oss << "[GameInfo]" << endl;
            oss << "First=" << field.name[0] << endl;
            oss << "FirstRemTime=" << field.limit[0] << endl;
            oss << "Second=" << field.name[1] << endl;
            oss << "SecondRemTime=" << field.limit[1] << endl;
            oss << "Random=" << field.random << endl;
            
            for(int e = 0; e < field.ends; ++e){
                for(int t = 0; t <= field.turns; ++t){
                    oss << "[" << (e / 10) << (e % 10) << (t / 10) << (t % 10) << "]" << endl;
                    // stone position
                    oss << "POSITION=POSITION";
                    for(int i = 0; i < N_STONES; ++i){
                        oss << " " << p6Float(field.stone[e][t][i][0]);
                        oss << " " << p6Float(field.stone[e][t][i][1]);
                    }oss << endl;
                    
                    if(t < field.turns){
                        oss << "SETSTATE=SETSTATE";
                        oss << " " << t << " " << e;
                        oss << " " << field.ends << " " << ((t % 2) ^ field.reversed[e]) << endl;
                        
                        oss << "BESTSHOT=BESTSHOT";
                        oss << " " << p6Float(field.bestShot[e][t].x);
                        oss << " " << p6Float(field.bestShot[e][t].y);
                        oss << " " << field.bestShot[e][t].s << endl;
                        
                        oss << "RUNSHOT=RUNSHOT";
                        oss << " " << p6Float(field.runShot[e][t].x);
                        oss << " " << p6Float(field.runShot[e][t].y);
                        oss << " " << field.runShot[e][t].s << endl;
                    }else{
                        oss << "SCORE=SCORE " << ((field.score[e][0] > 0) ? field.score[e][0] : (-field.score[e][1])) << endl;
                    }
                }
            }
            // game end
            oss << "TOTALSCORE=TOTALSCORE " << field.scoreSum[0] << " " << field.scoreSum[1] << endl;
            return oss.str();
        }
        
        template<class field_t>
        std::string toDclName(const field_t& field){
            // dcl形式のログファイル名の生成
            std::ostringstream oss;
            oss << field.name[0] << "_vs_" << field.name[1] << "[" << (unsigned int)time(NULL) << "]";
            return oss.str();
        }
    }
}