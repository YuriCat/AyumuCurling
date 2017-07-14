/*
 ayumu.hpp
 Katsuki Ohto
 */

// デジタルカーリングクライアント
// 歩-Ayumu-
// Katsuki Ohto

#ifndef DCURLING_AYUMU_AYUMU_HPP_
#define DCURLING_AYUMU_AYUMU_HPP_

#include "../dc.hpp"
#include "ayumu_dc.hpp"

// shots
#include "shot/allShots.hpp"

namespace DigitalCurling{
    XorShift64 dice;
    DSFMT ddice;
    
    ThreadTools threadTools[N_THREADS];
}

#include "./eval/evaluator.hpp"

#include "./eval/stat.hpp"
//#include "../eval/eval.hpp"

#include "logic/mate.hpp"

namespace DigitalCurling{
    
    // ショット
    Peel gPeel;
    DrawRaise gDrawRaise;
    Double gDouble;
    RaiseTakeOut gRaiseTakeOut;
    TakeOut gTakeOut;
    L1Draw gL1Draw;
    
    // 局面評価
    Evaluator gEvaluator;
}

#include "logic/logic.hpp"

// structure
#include "structure/thinkBoard.h"
#include "structure/thinkBoard.hpp"
#include "structure/thinkField.hpp"
#include "../structure/grid.hpp"

// generation
#include "../simulation/primaryShot.hpp"
#include "move/generator.hpp"
#include "move/realizer.hpp"

// simulation
#include "../simulation/b2dSimulator.hpp"
#include "../simulation/fastSimulator.hpp"
//#include "../simulation/intSimulator.hpp"

// last play
//#include "../policy/L1.hpp"

// data base
#include "db/db.hpp"

// policy
#include "policy/heuristic.hpp"
#include "policy/policy.hpp"

// static evaluation
#include "eval/estimator.hpp"

namespace DigitalCurling{
    PlayPolicy gPolicy;
    ScoreEstimator gEstimator;
    
    enum{
        FLAG_PONDER = 1,
        FLAG_PLAY = 2,
        FLAG_STOP = 4,
    };
    
#if defined(CREATE_PONDER_THREAD)
    struct ClientState{
        std::atomic<uint32_t> st;
        
        uint32_t isPondering()const{ return st & 2U; }
        uint32_t isDecidingMove()const{ return st & 1U; }
        uint32_t requestedToStop()const{ return st & 4U; }
        uint32_t requestedToExit()const{ return st & 32U; }
        uint32_t requestedToPonder()const{ return st & 16U; }
        uint32_t requestedToDecideMove()const{ return st & 8U; }
        
        uint32_t isThinking()const{ return st & 3U; }
        
        void setPondering(){
            st |= 2U;
        }
        void setDecidingMove(){
            st |= 1U;
        }
        void requestToStop(){
            st |= 4U;
        }
        void requestToPonder(){
            st = (st & (~(4U | 32U))) | 16U;
        }
        void requestToDecideMove(){
            st = (st & (~(4U | 32U))) | 8U;
        }
        void requestToExit(){
            st |= 4U | 32U;
        }
        
        void resetRequestToPonder(){
            st &= (~16U);
        }
        void resetRequestToDecideMove(){
            st &= (~8U);
        }
        
        void resetPlay(){
            st &= (4U | 32U);
        }
        
        void init(){
            st = 0;
        }
    };
    
    ClientState gState;
#else
    
    std::atomic<uint32_t> gState;
    
#endif
    
    std::bitset<32> gRequirements;//クライアントに対しての要求
}

# if !defined(POLICY_ONLY)
// MCを使う場合
// montecarlo
#include "mc/monteCarlo.h"
#include "mc/leaf.hpp"
#include "mc/root.hpp"
#include "mc/mcPlayer.hpp"

#endif

#include "../structure/field.hpp"

#include "initialize.hpp"

namespace DigitalCurling{
    using namespace Ayumu;
    
    class AyumuAI{ // : public AI{
    private:
        int settledPlayouts; // プレイアウト回数固定の場合(実験用)
    public:
        AyumuAI(){
            settledPlayouts = -1;
        }
        
        ClientField field;
        
        virtual void settlePlayouts(int npo)noexcept{
            // プレイアウト回数固定(実験用)
            settledPlayouts = npo;
        }
        
        virtual void setRandomSeed(uint64_t s)noexcept{
            // 乱数シード固定(デバッグ用)
            dice.srand(s);
            ddice.srand((uint32_t)dice.rand());
            for (int th = 0; th < N_THREADS; ++th){ // 各スレッドの分も
                threadTools[th].dice.srand(dice.rand() * (th + 111));
                threadTools[th].ddice.srand(dice.rand() * (th + 111));
            }
        }
        
        virtual int initAll(){
            
            field.initAll();
            
            if(initAyumu(DIRECTORY_PARAMS_IN) < 0){
                cerr << "failed to initialize." << endl;
                //return -1;
            }
            gPolicy.fin(DIRECTORY_PARAMS_IN + "policy_param.dat");
            gPolicy.setTemperature(0.8);
            
            gEstimator.fin(DIRECTORY_PARAMS_IN + "estimator_param.dat");
            
            // 各スレッドの持ち物を初期化
            setRandomSeed((uint32_t)time(NULL));
            for (int th = 0; th < N_THREADS; ++th){
                threadTools[th].threadId = th;
            }
            
# if !defined(POLICY_ONLY)
            initGridPdf();
#endif // !POLICY_ONLY
            return 0;
        }
        virtual int initGame(){
            CERR << "AI::initGame()" << endl;
            field.initGame();
            // 1ゲーム開始時の初期化
#if !defined(POLICY_ONLY)
            pbdTT->init();
#endif
            return 0;
        }
        virtual int setName(const std::string& name0, const std::string& name1){
            CERR << "AI::setName()" << endl;
            field.setName(0, name0);
            field.setName(1, name1);
            return 0;
        }
        virtual int initEnd(){
            CERR << "AI::initEnd()" << endl;
            field.initEnd();
            return 0;
        }
        virtual int ready(){
            CERR << "AI::ready()" << endl;
            // ready 通達の際の処理
            initGame();
            initEnd();
            return 0;
        }
        
        virtual int prepare(){
            // 自分の思考終了後、次のターンの為に準備
            return 0;
        }
        virtual int ponder(){
            // 相手手番中の先読み
            return 0;
        }
        virtual int play(fMoveXY<fpn_t> *const dst){
            CERR << "AI::play()" << endl;
            int ret;
            ThinkField tf;
            tf.setClientField (field);
            
            // Decide Move
            fMoveXY<fpn_t> fmv;
            
#ifdef POLICY_ONLY
            //Heuristic::generate(&fmv, tf, &threadTools[0]); // ヒューリスティック
            //playWithPolicy(&fmv, tf, gPolicy, &threadTools[0]); // 温度1
            playWithBestPolicy(&fmv, tf, gPolicy, &threadTools[0]); // 温度0
            ret = 0;
#else
            MonteCarloPlayer mcp;
            if(settledPlayouts >= 0){
                mcp.settlePlayouts(settledPlayouts);
            }
            ret = mcp.play(&fmv, tf);

#endif // POLICY_ONLY
            //公式の座標系に変換
            *dst = convMove_Ayumu_Official(fmv);
            
            if (ret != -1){
                CERR << "My Move : " << *dst << endl;
                ret = 0;
            }
            
            return ret;
        }
        virtual int closeEnd(){
           field.closeEnd();
            return 0;
        }
        virtual int closeGame(int r){
            field.closeGame(r);
            printBRC();
            return 0;
        }
        virtual int closeAll(){
            return 0;
        }
    };
}

#endif // DCURLING_AYUMU_AYUMU_HPP_