//Digital Curling
//reading dclog

namespace DigitalCurling{
    
    struct MoveLog{
        //1ショットのログ
        fMoveXY<float> tried;//投げようとしたもの
        fMoveXY<float> run;//結果
        uint64_t time;//思考時間
    };
    
    struct TurnLog{
        //1エンドのログ
        fPosXY<float> pos[2][8];//石の配置
        MoveLog mv;//ショット
    };
    
    struct EndLog{
        int orgScore[2];
        int lastScore[2];
        TurnLog turn[16];
    };
    
    struct GameLog{
        //1ゲームのログ
        int orgScore[2];
        int lastScore[2];
        std::string name[2];
        EndLog end[16];
    };
    
    
    int readDCLog() 
    
    
}