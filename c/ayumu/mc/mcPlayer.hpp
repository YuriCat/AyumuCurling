// デジタルカーリング
// モンテカルロ着手決定機関

#ifndef DCURLING_AYUMU_MC_MCPLAYER_HPP_
#define DCURLING_AYUMU_MC_MCPLAYER_HPP_

#include "../ayumu_dc.hpp"
#include "root.hpp"

// じりつくん関数
#include "../../jiritsu/search.hpp"

namespace DigitalCurling{
    namespace Ayumu{
        
        template<class root_t, class board_t>
        int MonteCarloThread(int threadId, root_t *const proot, const board_t *const pbd){
            
            // モンテカルロスレッド
            // マルチスレッディングにしない場合は関数として開く
            
            ClockMS clms; // 時計(millisec単位)
            clms.start();
            
            // グローバルに置いてあるこのスレッド用の道具
            auto& tools = threadTools[threadId];
            
            ThinkBoard tb; // プレイアウタ空間
            
            int thread_nplayouts = 0; // このスレッドのプレイアウト回数
            
            int best = 0;
            eval_t bestEval = -9999;
            /*for(int i = 0; i < proot->drawChilds; ++i){
             startPlayoutAllGrids(*proot, proot->drawChild[i], *pbd, *pevaluator, &tools);
             }
             for(int i = 0; i < proot->hitChilds; ++i){
             startPlayoutAllGrids(*proot, proot->hitChild[i], *pbd, *pevaluator, &tools);
             }*/
            /*proot->iterateChild([&](int i, auto& ch)->void{
             startPlayoutAllGrids(*proot, ch, *pbd, *pevaluator, &tools);
             });*/
            for(int i = threadId, n = proot->childs; i < n && !(gState & FLAG_STOP); i += N_THREADS){
                startPlayoutAllGridsFirst(*proot, proot->child[i], *pbd, &tools);
                
                if (clms.stop() > proot->limitTime
                    || proot->trials >= proot->limitPlayouts){ // 制限時間オーバー or シミュレーション回数オーバー
                    gState |= FLAG_STOP;
                    return 0;
                }
            }
            if(pbd->getTurn() == TURN_LAST){ return 0; }
            while (!(gState & FLAG_STOP)){ // 終了判定が出ていない
                int best = proot->getBestUCBIndex(&tools.ddice); // 検討する着手を選ぶ
                startPlayoutAllGridsAfter(*proot, proot->child[best], *pbd, &tools);
                
                if (clms.stop() > proot->limitTime
                    || proot->trials >= proot->limitPlayouts){ // 制限時間オーバー or シミュレーション回数オーバー
                    gState |= FLAG_STOP;
                    return 0;
                }
                if(proot->trials > 100000000){
                    cerr << "MonteCarloThread() : enndress mcts-search." << endl; exit(1);
                }
            }
            
            /*
             while (!(gState & FLAG_STOP)){//終了判定が出ていない
             
             DERR << "playout " << thread_nplayouts << ".";
             
             int best = proot->getBestUCBIndex(&tools.ddice);//検討する着手を選ぶ
             auto& child = proot->child[best];
             
             constexpr fpn_t epsilon = 0.1;
             if(tools.ddice.rand() < epsilon){
             //ランダムな探索
             proot->child[i].
             proot->child[i].startPlayout()
             }else{
             //bestPointの周囲に実際の乱数で
             
             ++proot->trials;//atomic命令
             ++thread_nplayouts;
             
             if (clms.stop() > proot->limit_time){//制限時間オーバー
             gState |= FLAG_STOP;
             }
             }*/
            //終了判定が出た
            return 0;
        }
        
        
        
        class MonteCarloPlayer{
        private:
            int settledPlayouts;
        public:
            
            template<class board_t>
            std::array<uint64_t, 2> decideLimits(const board_t& bd);
            
            template<class board_t>
            int play(fMoveXY<fpn_t> *const pdst, const board_t& bd, fpn_t *const pexpWP = nullptr);
            
            void settlePlayouts(int n)noexcept{
                settledPlayouts = n;
            }
            
            MonteCarloPlayer(){
                settledPlayouts = -1;
            }
            
            ~MonteCarloPlayer(){}
        };
        
        template<class board_t>
        std::array<uint64_t, 2> MonteCarloPlayer::decideLimits(const board_t& bd){
            // 自分の着手決定限度時間、プレイアウト回数を設定
            
            constexpr uint64_t byoyomiMargin = 100;
            
            uint64_t tmpTimeLimit;
            const auto& myTimeLimit = bd.getTimeLimit(bd.getTurnColor());
            
            if (myTimeLimit.isUnlimited()){ // 持ち時間無制限
                // 計算量レベルに合わせて決定する予定
                tmpTimeLimit = 10000; // とりあえず10秒
            }
            else{
                // 基本的に1投ごとの時間をだいたい同じ、比較的後半を長めにしている
                
                const uint64_t insurance = 4000;
                const uint64_t insuranceByShot = 500;
                //const uint32_t shots_remained = getGameRemStones(bd.getTurnColor(), bd.getEnd(), bd.getTurn());
                const fpn_t shotsRemained = bd.getEnd() * (N_COLOR_STONES - 0.5) + getEndRemStones(bd.getTurnColor(), bd.getTurn());
                
                //cerr << "remained = " << shots_remained << endl;
                
                const uint64_t byoyomi = (myTimeLimit.byoyomi_ms > byoyomiMargin) ?
                (myTimeLimit.byoyomi_ms - byoyomiMargin) : 0;
                
                // エンド、ターンから決定
                if (myTimeLimit.limit_ms <= insuranceByShot * shotsRemained + insurance){
                    tmpTimeLimit = myTimeLimit.limit_ms / shotsRemained + byoyomi;
                }
                else{
                    tmpTimeLimit = (myTimeLimit.limit_ms - insuranceByShot * shotsRemained - insurance) / shotsRemained + byoyomi;
                }
            }
            // cerr << tmpTimeLimit << endl;
            uint64_t tmpPlayoutLimit;
            if(settledPlayouts <= 0){
                tmpPlayoutLimit = -1;
            }else{
                tmpPlayoutLimit = settledPlayouts;
            }
            return {tmpTimeLimit, tmpPlayoutLimit};
        }
        
        template<class board_t>
        int MonteCarloPlayer::play(fMoveXY<fpn_t> *const pdst, const board_t& bd, fpn_t *const pexpWP){
            // モンテカルロで着手決定
            
            auto& root = gRootNode; // TODO: グローバル変数である必要はない
            ThinkBoard tb;
            fpn_t wp; // 予測勝率
            
            tb.set(bd); // 局面情報を思考用にセット
            tb.ownerTurn_ = tb.getTurnColor();
            
            // 候補手(言語型)の生成
            MoveXY kmv[512];
            
            // 末端報酬設定
            gEvaluator.setRewardInBound(tb.getTurnColor(), tb.getTurnColor(), tb.getRelScore(), tb.getEnd(), tb.getTurn(),
                                        [](Color c, int e, int s, int nrs)->double{
                                            return getEndWP(c, e, s, nrs);
                                        });
            gEvaluator.normalize([](int s)->double{
                return dScoreProb_EASY[s + N_COLOR_STONES];
            });
            
            CERR << gEvaluator.toWPString();
            CERR << gEvaluator.toRewardString();
            
            // まず詰みを判定
            Color mateColor = judgePerfectMate(tb);
            if (mateColor == bd.getTurnColor()){
                // 自分の必勝
#ifdef USE_NULLMOVE
                genPASS(pdst);
                cerr << "Mate " << toColorString(mateColor) << endl;
                return 1;
#endif
            }
            else if (mateColor == flipColor(bd.getTurnColor())){
                // 相手の必勝
#ifdef USE_NULLMOVE
                genPASS(pdst);
                cerr << "Mate " << toColorString(mateColor) << endl;
                return 1;
#endif
            }
            
            // 定跡から手の候補を入れる
            const int NMoves = genRootVMove(kmv, tb, &dice);
            
            ASSERT(-1 <= NMoves && NMoves < 512, cerr << NMoves << endl;);
            
            // まず定跡からの決定手があればそれに決定
#ifdef USE_ROOT_ONLY_MOVE
            if (NMoves < 0){
                realizeMove(pdst, tb, kmv[0]);
#ifdef MONITOR
                cerr << "Book Move." << endl;
#endif
                return 1;
            }
#endif
            
            // ルートノードに候補着手を用意
            root.init();
            fMoveXY<> fmv;
            
            // 定跡のレンジ手を入れる
            for (int i = 0; i < NMoves; ++i){
                realizeMove(&fmv, tb, kmv[i]);
                //convRMove(m, &tmpm);
                root.addChild(tb, kmv[i], fmv);
            }
            
#ifdef USE_ROOT_ONLY_MOVE
            // もしこの時点で候補着手が無い場合、ティーショットをする
            if (root.getNChilds() <= 0){
                genDraw(pdst, FPOSXY_TEE);
                return 0;
            }
#endif
            
            // 時間制限、プレイアウト制限を決める
            auto limits = decideLimits(bd);
            CERR << "MCTS Thinking Time Limit = " << limits[0] << " millisec, ";
            CERR << "Playout Limit = " << limits[1] << " playouts." << endl;
            root.setLimitTime(limits[0]);
            root.setLimitPlayouts(limits[1]);
            
            ClockMS clms;
            clms.start();
            
            if(bd.getTurn() == TURN_LAST){ // ラストストーン(L1)専用の解法
                /*auto r = L1(tb);
                *pdst = std::get<0>(r);
                root.trials = std::get<2>(r);
                wp = gEvaluator.rewardToWP(tb.getTurnColor(), std::get<1>(r));*/
                
#ifndef MINIMUM
                // L1のとき、その手が本当にその程度の評価か確かめる
                /*if(tb.getTurn() == TURN_LAST){
                 MiniBoard mbd;
                 iterateStoneWithIndex(tb, [&mbd](uint32_t idx, const auto& st){
                 mbd.locateNewStone(idx, st);
                 });
                 fpn_t wpSum = 0;
                 for(int i = 0; i < 2000; ++i){
                 MiniBoard tmp = mbd;
                 makeMove<1>(&tmp, TURN_LAST, *pdst, &threadTools[0].ddice);
                 wpSum += gEvaluator.wp(WHITE, countScore(tmp));
                 }
                 CERR << "MC wp = " << wpSum / 2000 << endl;
                 }*/
#endif
                
            }else{ // ラストストーン以外
                
                // 空場置換表を初期化
                if(bdNullTT.end() != bd.getEnd()){ // このエンドで初めて呼ばれる
                    bdNullTT.init(bd.getEnd(), bd.getRelScore());
                }
                bdNullTT.setEnd(bd.getEnd());
                // 空場置換表の整理(過ぎたターンを消す)
                for(int t = tb.getTurn(); t < N_TURNS; ++t){
                    bdNullTT.read(t)->size = 0;
                    bdNullTT.read(t)->childs = 0;
                }
                // 通常置換表整理
                //tickMicS();
                if(pbdTT->end() < INT_MAX){
                    if(bd.isNull() || pbdTT->end() != bd.getEnd()){
                        pbdTT->init();
                    }else{
                        //cerr << "proc" << endl;
                        //bdTT.proceed(bd.getTurn());
                        pbdTT->init();
                    }
                }
                pbdTT->setEnd(bd.getEnd());
                //tockMicS();
                /*
#ifdef MULTI_THREADING
                // open threads
                thread_t thr[N_THREADS];
                for (int th = 0; th < N_THREADS; ++th){
                    thr[th] = thread_t(&MonteCarloThread<RootNode, ThinkBoard>,
                                     th, &root, &tb);
                }
                for (int th = 0; th < N_THREADS; ++th){
                    thr[th].join();
                }
#else
                // call function
                MonteCarloThread<RootNode, ThinkBoard>(0, &root, &tb);
                
#endif //MULTI_THREADING
                
                // 最高評価の手を選ぶ
                //int best_id = root.choose_best_id(); // 最高評価のもの
                //move_t bestMove = root.child[best_id].dive_largest();
                // bestNRMove = bestMove.genNR(&threadTools[0].ddice);
                
                auto result = root.getBestMoveReward();
                *pdst = std::get<0>(result);
                //cerr << "reward = " << std::get<1>(result) << endl;
                wp = gEvaluator.rewardToWP(tb.getTurnColor(), std::get<1>(result));
                */
            }
            
            // じりつくん法
            std::vector<std::array<std::array<int, 16>, N_TURNS>> survey;
            std::vector<std::array<int, N_TURNS>> trees;
            auto best = Jiritsu::doMonteCarloSearch(tb, settledPlayouts,
                                                    [&](const auto& nd, const auto& bd, const auto& move)->eval_t{
                
                /*if(nd.trials % 10000 == 0){
                    survey.emplace_back(bdTT.analyze());
                    trees.emplace_back(bdTT.trees());
                }*/
                
                ThinkBoard tbd = bd;
                PlayoutResult result;
                //cerr << tbd << endl;
#ifdef USE_MCTS
                doUCT(&result, tbd, move, BOARD_EX_DEPTH_MAX, 1024, &threadTools[0]);
#else
                doSimulation(&result, tbd, move, &threadTools[0]);
#endif // USE_MCTS
                return result.eval(bd.getTurnColor(), 0);
            }, &threadTools[0].dice);
            *pdst = std::get<0>(best);
            wp = gEvaluator.rewardToWP(tb.getTurnColor(), std::get<1>(best));
            /*if(tb.getTurn() == TURN_FIRST){
                for(const auto& s : survey){
                    for(int t = 0; t < N_TURNS; ++t){
                        cerr << toString(s[t], ",") << endl;
                    }
                }
                cerr << endl;
                for(const auto& tr : trees){
                    cerr << toString(tr, ",") << endl;
                }
            }*/
            
#ifdef MONITOR
            //bdTT.print(10);
            //bdNullTT.print(0.5);
            pbdTT->ana.report(); // 置換表スタッツを表示
            //root.print();
            
            //cerr << "MCTS : " << clms.stop() << " millisec, " << root.trials << " playouts, " << bdTT.pages() << " nodes." << endl;
            
            //cerr << evaluator.rewardToWP(WHITE, evaluator.reward(WHITE, -8)) << endl;
            //cerr << evaluator.rewardToWP(WHITE, evaluator.reward(WHITE, +8)) << endl;
            
            // 期待勝率表示
            cerr << "Expected Winning Percentage = " << wp * 100 << " %" << endl;
            
            //cerr << "MCP : best move - " << bestMove << endl;
            //cerr << "MCP : best no-range move - " << bestNRMove << endl;
            
#endif // MONITOR
            
            
            //getchar();
            //convMove(bestNRMove, pdst);
            
            if(CONCEDE_MIN_END <= tb.end && tb.end <= CONCEDE_MAX_END ){ // 投了を検討
                if(wp < CONCEDE_WP){ // 予測勝率が投了ラインより低い
                    return -1; // 投了
                }
            }
            
            if(pexpWP != nullptr){
                // 予測勝率を返す
                *pexpWP = wp;
            }
            return 0;
        }
    }
}

#endif // DCURLING_AYUMU_MC_MCPLAYER_HPP_