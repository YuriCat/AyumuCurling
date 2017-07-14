/*
 leaf.hpp
 Katsuki Ohto
 */

// デジタルカーリング
// モンテカルロリーフノード

#ifndef DCURLING_AYUMU_MC_LEAF_HPP_
#define DCURLING_AYUMU_MC_LEAF_HPP_

#include "monteCarlo.h"
#include "mcFunction.hpp"

namespace DigitalCurling{
    namespace Ayumu{
        
        struct PlayoutResult{
            MoveXY mv; // 離散着手
            eval_t terminalEval_; // 末端評価(先手から見た)
            eval_t staticEval_; // 途中局面評価
            //eval_t eval_;
            
            ContactTree ct; // 衝突グラフ
            fpn_t dvx, dvy; // ずれ
            //RecursiveNode<>
            
            void setTerminalEval(eval_t aev)noexcept{
                terminalEval_ = aev;
            }
            void setStaticEval(eval_t aev)noexcept{
                staticEval_ = aev;
            }
            void addTerminalEval(eval_t aev)noexcept{
                terminalEval_ += aev;
            }
            eval_t terminalEval(Color c)const noexcept{
                return isBlack(c) ? terminalEval_ : (-terminalEval_);
            }
            eval_t staticEval(Color c)const noexcept{
                return isBlack(c) ? staticEval_ : (-staticEval_);
            }
            eval_t eval(Color c, fpn_t lambda)const noexcept{
                const eval_t bev = lambda * staticEval_ + (1 - lambda) * terminalEval_;
                return isBlack(c) ? (bev) : (-bev);
            }
            /*
            void setEval(eval_t aev)noexcept{
                eval_ = aev;
            }
            void addEval(eval_t aev)noexcept{
                eval_ += aev;
            }
            eval_t eval(Color c)const noexcept{
                return isBlack(c) ? (eval_) : (-eval_);
            }*/
        };
        
        template<class board_t, class node_t>
        void diveBoardTree(PlayoutResult *const, board_t&, node_t&, const int, const int, const int, ThreadTools *const);
        
        template<class board_t>
        ContactTree playTurnPolicy(board_t& bd, ThreadTools *const ptools){
            // 方策関数で1ターンプレーする
            fMoveXY<> mv;
            playWithPolicy(&mv, bd, gPolicy, ptools);
            DERR << toColorChar(bd.getTurnColor()) << " " << mv << endl;
            return makeMove(&bd, mv, &ptools->ddice); // mvをプレーしてみる
        }
        
        template<class board_t>
        fpn_t doPlayout(board_t& bd, ThreadTools *const ptools){
            // 通常プレイアウト
            // 末端で得点を返す
            while (bd.getTurn() > TURN_LAST){
                playTurnPolicy(bd, ptools);
                bd.procTurn();
            }
            playTurnPolicy(bd, ptools);
            bd.updateInfoLast();
            return bd.countScore();
        }
        
        template<class board_t, class move_t>
        void doUCT(PlayoutResult *const presult, board_t& bd, const move_t& nrmv,
                   int exDepth, int playDepth, ThreadTools *const ptools){
            // 与えられた局面と着手からプレイアウトを行う
            // 乱数によるぶれを掛けない
            const int end = bd.getEnd();
            const int startTurn = bd.getTurn();
            const Color ownerColor = bd.getTurnColor();
            //eval_t ev = 0;
            
            //DERR << "turn = " << turn << endl;
            ContactTree ct = makeMoveNoRand<1>(&bd, bd.getTurn(), nrmv); // nrmvをプレーしてみる
            presult->ct = ct;
            
            // 評価点修正分を計算
#if defined(USE_SUBEVAL)
            eval_t addedEv = (exDepth == BOARD_EX_DEPTH_MAX) ? gEvaluator.evalSub(ownerColor, bd) : 0;
#else
            eval_t addedEv = 0;
#endif
            
            if (bd.getTurn() == TURN_LAST || playDepth <= 0){
                //プレイアウト終了
            }
            else{//depthが0になるまで
                const int pto_turn = max(TURN_LAST, bd.getTurn() - playDepth);
                assert(pto_turn == TURN_LAST);
                do{
                    // 局面更新(シミュレーション結果に応じて)
                    bd.procTurn();
                    
                    // 自明な局面では評価を返す
#ifdef USE_PLAYOUT_LEAVING
                    if (end == END_LAST){
                        // 最終エンド
                        // この時点で必勝ほぼ決定の場合には、その結果を返す
                        Color mateColor = judgeFuzzyMate(bd);
                        if (mateColor != COLOR_NONE){
                            DERR << "mate." << endl;
#ifdef USE_MATE_TIMING_REWARD
                            // 優勢時に手抜きな手ばかり読まないように、早くMATEが出る事に加点する
                            constexpr eval_t MATE_TIMING_SCORE_STEP = 0.004;
                            presult->setTerminalEval(gEvaluator.rewardMateWithTiming(BLACK, mateColor, bd.getTurn(), MATE_TIMING_SCORE_STEP));
#else
                            presult->setTerminalEval(gEvaluator.rewardMate(BLACK, mateColor));
#endif
                            goto TERMINAL_END;
                        }
                    }
                    if (bd.getTurn() == TURN_LAST){
                        // ラストストーン
                        // 分かりやすい局面では、シミュレーションを行わずに報酬を確定させる
                        if (bd.countScore() == 0){ // ハウス内に石がない
                            DERR << "0-1,";
                            // 白が0か1か選ぶ事が出来る
                            presult->setTerminalEval(gEvaluator.reward01(BLACK));
                            goto TERMINAL_END;
                        }
                        if (countNInHouse(bd, BLACK) == 0){ // 邪魔な黒石がない
                            // 白は必ずドロー
                            DERR << "easydraw" << bd.NInHouse(WHITE) << endl;
                            const int sc = -(bd.NInHouse(WHITE) + 1);
                            presult->setTerminalEval(gEvaluator.reward(BLACK, sc));
                            goto TERMINAL_END;
                        }
                    }
#endif
                    
                    assert(bd.getTurn() >= TURN_LAST);
                    
                    Color col = bd.getTurnColor();
                    
                    if (bd.isAlmostNull()){
                        // ほぼ空場なら局面認識の必要なし
                        ASSERT(!bd.NInHouse(), cerr << bd << endl;);
                        auto& node = *bdNullTT.read(bd.getTurn());
                        PlayoutResult tresult;
                        tryNode(&tresult, bd, node, exDepth - 1, playDepth - 1, ptools);
                        
                        ASSERT(!tresult.mv.hasContact(), cerr << bd << endl << tresult.mv << endl;);
                        
                        node.startMaking();
                        //node.attenuate();
                        node.feedEval(col, tresult);
                        node.finishMaking();
                        
                        presult->setTerminalEval(tresult.terminalEval(BLACK));
                        presult->setStaticEval(tresult.staticEval(BLACK));
                        goto END;
                    }
                    else if(exDepth > 0){
                        // 緩い局面認識から手を検索して、あったらそのまま潜り、無かったらノード登録して潜る
                        // 登録出来なかったら、プレイアウトを一手進める
                        for (int r = RANGE_MAX_LEAFNODE; r >= RANGE_MIN_LEAFNODE; --r){
                            uint64_t hashValue = bd.getHash(r);
                            uint64_t identityValue = tt_t::page_t::knitIdentityValue(hashValue, bd.getTurn(), r);
                            bool found;
                            auto *pnode = pbdTT->read(hashValue, identityValue, &found);
                            if(pnode != nullptr){
                                if(!found){ // 新規登録
                                    if(pbdTT->isFilled()){ continue; } // 置き換え表が一杯なので新規登録しない
                                    pnode = pbdTT->registAndStartMaking(identityValue, pnode); // 登録(この時に場所変化の可能性あり)
                                    if(pnode == nullptr){ continue; } // 登録失敗
                                    pnode->set(bd);
                                    pnode->finishMaking();
                                }else{
                                    //pnode->parent = nullptr; // 親付け替え
                                }
                                PlayoutResult tresult;
                                
                                // このノードから潜る
                                diveBoardTree(&tresult, bd, *pnode, r, exDepth, playDepth, ptools);
                                presult->setTerminalEval(tresult.terminalEval(BLACK));
                                presult->setStaticEval(tresult.staticEval(BLACK));
                                goto END;
                            }
                        }
                        // 置換表参照が不可能だったので、1手通常のプレイアウトとする
                    }
                    
                    playTurnPolicy(bd, ptools);
                    
                    --exDepth;
                    --playDepth;
                } while (bd.getTurn() > pto_turn);
            }
            
            bd.updateInfoLast();
            presult->setTerminalEval(gEvaluator.reward(BLACK, bd.countScore()));
        TERMINAL_END:
            presult->setStaticEval(0);
        END:
            presult->addTerminalEval(addedEv); // 修正用評価点を加算
        }
        
        template<class board_t, class move_t>
        void doErrorUCT(PlayoutResult *const presult, board_t& bd, const move_t& nrmv,
                        int exDepth, int playDepth, ThreadTools *const ptools){
            // 与えられた局面と着手からプレイアウトを行う
            // 乱数によるぶれを掛け、ぶれの程度も報告する
            
            // 乱数をかける
            fpn_t dvx, dvy;
            genRandToMoveXY(&dvx, &dvy, nrmv.spin(), &ptools->ddice);
            
            presult->dvx = dvx;
            presult->dvy = dvy;
            fMoveXY<> fmv(nrmv.vx() + dvx, nrmv.vy() + dvy, nrmv.spin());
            
            doUCT(presult, bd, fmv, exDepth, playDepth, ptools);
        }
        
        template<class board_t, class node_t>
        void tryNode(PlayoutResult *const presult, board_t& bd, const node_t& node,
                     const int exDepth, const int playDepth,
                     ThreadTools *const ptools){
            // 与えられた候補局面からプレイアウトを行う
            
            MoveXY mvBuf[256]; // 着手ラベル生成バッファ
            MovePolicy buf[256]; // 着手を名前順ソートして行動価値関数を計算するためのバッファ
            
            MoveXY decidedMv; // 決定した着手インデックス
            
            // 現在の絶対局面にて候補言語手を生成
            //const int NMoves = genAllVMove(buf, bd);
#ifdef USE_HANDMADE_MOVES
            const int NMoves = genChosenVMove(mvBuf, bd);
#else
            const int NMoves = genSimpleVMove(mvBuf, bd);
#endif
            
            ASSERT(0 < NMoves && NMoves < 256, cerr << NMoves << endl;);
            
            constexpr int r = 1; // デバッグ出力フラグ
            //int r = (ptools->dice.rand() % 20000);
            
            int best = 0;
            if(NMoves == 1){ // 着手が1つしか生成されていない場合は確定
                decidedMv = mvBuf[0];
            }else{ // 着手が複数生成された場合
                for(int i = 0; i < NMoves; ++i){
                    DERR << mvBuf[i].data() << " ";
                }DERR << endl;
                
                // mvBufの着手を名前順に並べ替えてbufに入れる
                int cnt = 0;
                int tmp = NMoves;
                while(tmp){
                    MoveXY minMV = mvBuf[0];
                    int minIdx = 0;
                    for(int j = 1; j < tmp; ++j){
                        if(mvBuf[j].data() < minMV.data()){
                            minIdx = j;
                            minMV = mvBuf[j];
                        }
                    }
                    buf[cnt].mv = minMV;
                    ++cnt;
                    --tmp;
                    mvBuf[minIdx] = mvBuf[tmp];
                }
                
                for(int i = 0; i < NMoves; ++i){
                    DERR << buf[i].data() << " ";
                    // それぞれの着手のジェネレータに予想評価を出してもらう
                    //buf[i].weight = estimate(MoveXY(buf[i]));
                }DERR << endl;
                //getchar();
                
                // それぞれの手の基礎方策点を計算
#ifdef USE_POLICY_SCORE // ポリシーあり
                calcPolicyScore(buf, NMoves, bd, gPolicy);
                // ここでpolicy funcの出した結果の加算と、評価合算の準備
                // ただしここでは方策点をeval_sumとsizeには足しこまない(これらの値はあとでいじられるので)
                fpn_t polSum = 0;
                for(int i = 0; i < NMoves; ++i){
                    buf[i].initUCB();
                    buf[i].weight = exp(buf[i].weight / gPolicy.temperature());
                    polSum += buf[i].weight;
                }
#else // ポリシーなし
                for(int i = 0; i < NMoves; ++i){
                    buf[i].weight = 0;
                }
                fpn_t polSum = 1;
                for(int i = 0; i < NMoves; ++i){
                    buf[i].initUCB();
                    buf[i].weight = 0;
                }
#endif
                
                
                // ノードの記録をたどる
                if(node.size != 0 || node.parent != nullptr){ // 本当に何の情報もないとき以外
                    
                    const node_t *ptnode = &node;
                    
                    fpn_t allSizeSum = 0; // 全着手のトライアル回数の重みつけ合計
                    // 各着手のトライアル回数
                    fpn_t nodeActionSize[256][17] = {0};
                    fpn_t nodeActionEval[256][17];
                    fpn_t allNodeSize[256];
                    
                    while(1){ // 上位のノードを辿っていくループ
                        ptnode->startReading();
                        if(ptnode->size > 0){ // 何も記録がないノードはとばす
                            ASSERT(!std::isnan(ptnode->eval_sum),
                                   cerr << ptnode->size << endl);
                            if(!r){
                                cerr << "r" << ptnode->range() << " : "
                                << ptnode->size << " " << ptnode->lockValue() << " " << bd.NInActiveZone() << endl;
                            }
                            
                            // このノードに記録されている結果にかける重み
                            const int dpth = RANGE_MAX_LEAFNODE - ptnode->range();
                            
                            ASSERT(!std::isnan(ptnode->mean()),
                                   cerr << node << endl << *ptnode << endl;
                                   cerr << ptnode->eval_sum << " / " << ptnode->size << endl;);

                            
                            // 全ての深さのノードで同じラベルの着手を探す
                            // 高速に検索するために全てのノードの着手と現局面で生成された着手は名前順に並べられてある
                            // i0, n0が現局面 i1, n1がノード上
                            int i0 = 0, i1 = 0;
                            const int n0 = NMoves, n1 = ptnode->NChilds();
                            
                            while(1){
                                if(buf[i0].data() == ptnode->child[i1].mv.data()){ // 一致
                                    
                                    nodeActionSize[i0][dpth] += ptnode->child[i1].size;
                                    nodeActionEval[i0][dpth] += ptnode->child[i1].eval_sum;
                                    allNodeSize[i0] += ptnode->child[i1].size;
                                    
                                    ASSERT(!std::isnan(buf[i0].mean()),);
                                    
                                    ++i0; ++i1;
                                    if(i0 >= n0){ break; } // 終了
                                    if(i1 >= n1){ break; } // 終了
                                }else if(buf[i0].data() > ptnode->child[i1].mv.data()){ // buf(現局面の着手)の方が進んだ
                                    ++i1;
                                    if(i1 >= n1){ break; } // 終了
                                }else{ // node(ノードに記録されている着手)の方が進んだ
                                    ++i0;
                                    if(i0 >= n0){ break; } // 終了
                                }
                            }
                        }
                        ptnode->finishReading();
                        if(ptnode->parent == nullptr){ break; }
                        ptnode = ptnode->parent; // 親ノードを辿る
                    } // ノード上の結果の合算ループここまで
                    
                    // トライアル回数からカーネル近似で値を算出
                    const int leafDepth = RANGE_MAX_LEAFNODE - ptnode->range();
                    for(int i = 0; i < NMoves; ++i){
                        for(int d = 0; d <= topDepth; ++d){
                            double dn = nodeActionSize[i][d + 1] - nodeActionSize[i][d];
                            double de = nodeActionSize[i][d + 1] - nodeActionSize[i][d];
                            
                        }
                    }
                    
                    // トライアル回数の調整
                    const fpn_t modifiedAllSizeSum = modifySize(allSizeSum, 4);
                    const fpn_t modifyRate = modifiedAllSizeSum / allSizeSum;

                    // 方策点の分のトライ回数も足して計算
                    const fpn_t logAllSize = log(modifiedAllSizeSum + POL_SIZE_LEAFCHILD * NMoves);
                    const fpn_t sqrtAllSize = sqrt(modifiedAllSizeSum + POL_SIZE_LEAFCHILD * NMoves);
                    if(!r){
                        cerr << "all size " << allSizeSum << " -> " << modifiedAllSizeSum << endl;
                    }
                    
                    // 各着手の重みを計算
                    for(int i = 0; i < NMoves; ++i){
                        
                        // トライアル回数の調整
                        buf[i].eval_sum *= modifyRate;
                        buf[i].size *= modifyRate;

#ifdef USE_POLICY_SCORE // ポリシーあり
                        // ここで方策点を足す
                        buf[i].weight /= polSum; // 確率合計が1になるように正規化
                        buf[i].weight = gEvaluator.wpToReward(bd.getTurnColor(), buf[i].weight); // 勝率から末端報酬にスケールを変更
                        buf[i].eval_sum += POL_SIZE_LEAFCHILD * buf[i].weight;
                        buf[i].size += POL_SIZE_LEAFCHILD;
#endif
                        
                        fpn_t pev = buf[i].weight;
                        //buf[i].weight = buf[i].mean() + K_UCB_LEAF * sqrt(max(0.0, logAllSize + mixBeta * pev) / buf[i].size);
                        buf[i].weight = buf[i].mean() + K_UCB_LEAF * sqrt(logAllSize / buf[i].size);
                        //buf[i].weight = buf[i].mean() + K_PUCB_LEAF * pev * sqrt(allSizeSum) / (1 + buf[i].size);
                        
                        if(!r){
                            cerr << buf[i] << "UCB " << buf[i].weight << " mean " << buf[i].mean() << " pos " << pev << " size " << buf[i].size << endl;
                        }
                    }
                    if(!r){ getchar(); }
                }
                
                // 試す着手を選ぶ
                
                int bestCnt = 2;
                fpn_t bestWeight = -9999;
                for(int i = 0; i < NMoves; ++i){
                    if(node.size){
                        //DERR << buf[i].weight << endl;
                    }
                    if(buf[i].weight == bestWeight){
                        if(ptools->dice.rand() % bestCnt == 0){
                            best = i;
                        }
                        ++bestCnt;
                    }else if(buf[i].weight > bestWeight){
                        best = i;
                        bestWeight = buf[i].weight;
                        bestCnt = 2;
                    }
                }
                if(node.size){
                    //DERR << best << endl;
                }
                decidedMv = buf[best].mv;
            }
            
            DERR << NMoves << " " << decidedMv << endl;
            
            ASSERT(decidedMv.data(), cerr << best << " in " << NMoves;);
            ASSERT(!(bd.isAlmostNull() && decidedMv.hasContact()), cerr << bd << endl << decidedMv << endl;);
            
            fpn_t zure = 0;
#ifdef USE_KERNEL_REGRESSION
            // Kernel Regression を行う
            if(node.size > 0){
                fpn_t reward[N_KR_ERROR_BOXES] = {0};
                fpn_t n[N_KR_ERROR_BOXES] = {0};
                //reward[N_KR_ERROR_BOXES / 2] = 2;
                //n[N_KR_ERROR_BOXES / 2] = FIRST_SIZE_LEAFCHILD;
                
                const node_t* ptnode = &node;
                do{
                    ptnode->startReading();
                    
                    // このノードに記録されている結果にかける重み
                    const int dpth = RANGE_MAX_LEAFNODE - ptnode->range();
                    const double dw = depthWeight(dpth);
                    
                    int i = ptnode->searchChild(decidedMv);
                    if(i >= 0){
                        // 確率分布で畳み込む
                        for(int j = MID_INDEX_KR - N_KR_INNER_W; j <= MID_INDEX_KR + N_KR_INNER_W; ++j){
                            for(int k = j - ZW_KR; k <= j + ZW_KR; ++k){
                                fpn_t dvj = KR_ItoDV(j);
                                fpn_t dvk = KR_ItoDV(k);
                                
                                fpn_t pdf = pdfDeltaVxVy(dvk - dvj, 0, decidedMv.spin());
                                
                                reward[j] += ptnode->child[i].eval_sum__[k] * pdf * dw;
                                n[j] += ptnode->child[i].size__[k] * pdf * dw;
                            }
                        }
                    }
                    
                    ptnode->finishReading();
                    ptnode = ptnode->parent;
                }while(ptnode != nullptr);
                
                // ベストな位置を探す
                int bestJ = MID_INDEX_KR;
                fpn_t bestReward = -99999;
                for(int j = MID_INDEX_KR - N_KR_INNER_W; j <= MID_INDEX_KR + N_KR_INNER_W; ++j){
                    fpn_t mr = reward[j] / n[j];
                    if(mr > bestReward){
                        bestJ = j;
                        bestReward = mr;
                    }
                }
                fpn_t zure = KR_ItoDV(bestJ);
                
                if(!r){
                    cerr << "kr : dvx = " << zure << " {";
                    for(int j = 0; j < N_KR_ERROR_BOXES; ++j){
                        cerr << reward[j] / n[j] * 10 << ", ";
                    }cerr << "}" << endl;
                    getchar();
                }
            }
#else
#endif
            
            // 言語手を実着手形式で生成
            fMoveXY<> fmv;
            realizeMove(&fmv, bd, decidedMv);
            
            // ずらしを加算
            fmv.addVx(zure);

            // 離散着手を記録
            presult->mv = decidedMv;
            
            // 現局面の静的評価値を計算
#ifdef USE_STATIC_VALUE_FUNCTION
            eval_t scoreProb[SCORE_LENGTH];
            eval_t scoreProbSum;
            calcEvalScore<0>(scoreProb, &scoreProbSum, bd, gEstimator);
            
            eval_t staticEv = gEvaluator.evaluateProb(BLACK, scoreProb, scoreProbSum);
#else
            eval_t staticEv = 0;
#endif
            
            // ここで1ターン進める(乱数あり)
            doErrorUCT(presult, bd, fmv, exDepth - 1, playDepth - 1, ptools);
            
            ASSERT(!std::isnan(presult->terminalEval(BLACK)),);
            ASSERT(!std::isnan(presult->staticEval(BLACK)),);
            
            // 報酬由来の評価値と局面の静的評価値を混ぜる
            //const double alpha = 0.3 / (1 + log(1 + nodeSize));
            
            //cerr << "simulationEval = " << presult->eval(BLACK) << " static eval = " << staticEv << endl;
            
            //presult->setTerminalEval(presult->eval(BLACK) * (1 - alpha) + staticEv * alpha);
            presult->setStaticEval(presult->staticEval(BLACK) * 0.5 + staticEv * 0.5);
        }
        
        template<class board_t, class node_t>
        void diveBoardTree(PlayoutResult *const presult, board_t& bd, node_t& node, const int range,
                           const int exDepth, const int playDepth, ThreadTools *const ptools){
            
            const Color c = bd.getTurnColor();
            
            if(isExpansionCondition(node, range)){ // さらに深く潜るとき
                const int nextRange = range - 1;
                uint64_t hashValue = bd.getHash(nextRange);
                uint64_t identityValue = tt_t::page_t::knitIdentityValue(hashValue, bd.getTurn(), nextRange);
                bool found;
                auto *pnext = pbdTT->read(hashValue, identityValue, &found);
                if(pnext != nullptr){
                    if(!found){ // 新規登録
                        if(pbdTT->isFilled()){ goto TRY; } // 置き換え表が一杯なので新規登録しない
                        pnext = pbdTT->registAndStartMaking(identityValue, pnext); // 登録(この時に場所変化の可能性あり)
                        if(pnext == nullptr){ goto TRY; } // 登録失敗
                        pnext->succeed(bd, node);
                        //pnext->set(bd);
                        pnext->finishMaking();
                    }else{
                        //pnext->parent = &node; // 親付け替え
                    }
                    diveBoardTree(presult, bd, *pnext, nextRange, exDepth, playDepth, ptools);
                    goto FEED;
                }
            }
        TRY:
            // これ以上深く潜れなかった
            tryNode(presult, bd, node, exDepth, playDepth, ptools);
        FEED:
            // 着手の結果を報告
            node.startMaking();
            //node.attenuate();
            node.feedEval(c, *presult);
            node.finishMaking();
        }
        
        template<class board_t, class move_t>
        void doSimulation(PlayoutResult *const presult, board_t& bd, const move_t& nrmv, ThreadTools *const ptools){
            // 木探索のないシミュレーション
            const Color ownerColor = bd.getTurnColor();
            ContactTree ct = makeMoveNoRand<1>(&bd, bd.getTurn(), nrmv); // nrmvをプレーしてみる
            presult->ct = ct;
#ifdef USE_SUBEVAL
            // 修正用評価点を計算
            eval_t ev = gEvaluator.evalSub(ownerColor, bd);
#else
            eval_t ev = 0;
#endif
            while(bd.getTurn() > TURN_LAST){
                bd.procTurn();
                playTurnPolicy(bd, ptools);
            }
            bd.updateInfoLast();
            
            DERR << toColorChar(ownerColor) << " " << bd.countScore() << " " << gEvaluator.wp(ownerColor, bd.countScore()) << endl;
            
            presult->setTerminalEval(ev + gEvaluator.reward(BLACK, bd.countScore()));
            presult->setStaticEval(0);
        }
    }
    
}

#endif // DCURLING_AYUMU_MC_LEAF_HPP_