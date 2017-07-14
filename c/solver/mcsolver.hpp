//デジタルカーリング
//モンテカルロ着手決定機関

namespace DigitalCurling{
    namespace Ayumu{
        class MonteCarloSolver{
        private:
        public:
            template<class board_t>
            int solve(fMoveXY<fpn_t> *const dst, const board_t& bd, int playouts, const fpn_t *const pexpWP = nullptr);
            
            MonteCarloSolver(){}
            ~MonteCarloSolver(){}
        };
        
        
        template<class board_t>
        int MonteCarloPlayer::solve(fMoveXY<fpn_t> *const pdst, const board_t& bd, fpn_t *const pexpWP){
            //モンテカルロで着手決定
            typedef MoveXY move_t;
            
            move_t bestNRMove;
            
            RootNode<move_t> root;
            ThinkBoard tb;
            
            tb.set(bd);//局面情報を思考用にセット
            
            //候補手(言語型)の生成
            MoveXY kmv[256];
            int NKillers = 0;
            
            //定跡から手の候補を入れる
            NKillers = pushStandardRoot(kmv, tb, &dice);
            
            //まず定跡からの決定手があればそれに決定
            if (NKillers < 0){
                genMove(pdst, tb, kmv[0]);
                return 1;
            }
            
            Evaluater evaluater;
            
            //ルートノードに候補着手を用意
            root.init();
            move_t mv_s0, mv_s1;
            mv_s0.setWR(0);
            mv_s1.setWR(1);
            
            move_t tmpm;
            fRMoveXY<> m;
            
            //定跡のレンジ手を入れる
            for (int i = 0; i<NKillers; ++i){
                genRootMove(&m, tb, kmv[i]);
                convRMove(m, &tmpm);
                root.addChild(tb, tmpm);
            }
            
            if (tb.isSymmetry()){
                //左右対称の時は、回転が正または負の着手のみ考えれば良い
                DERR << "Sym" << endl;
                if (threadTools[0].ddice.rand() % 2 == 0){
                    for (int i = 0; i<4; ++i){
                        tmpm = mv_s0.part<4>(i);
                        root.addChild(tb, tmpm);
                    }
                }
                else{
                    for (int i = 0; i<4; ++i){
                        tmpm = mv_s1.part<4>(i);
                        root.addChild(tb, tmpm);
                    }
                }
            }
            else{
                for (int i = 0; i<4; ++i){
                    tmpm = mv_s0.part<4>(i);
                    root.addChild(tb, tmpm);
                    tmpm = mv_s1.part<4>(i);
                    root.addChild(tb, tmpm);
                }
            }
            
            //もしこの時点で候補着手が無い場合、ティーショットをする
            if (root.childs <= 0){
                genDraw(pdst, FPOSXY_TEE);
                return 0;
            }
            
            //時間制限を決める
            uint64_t tmp_time_limit = decide_play_time(bd);
            
            CERR << "thinking time limit ... " << tmp_time_limit << " ms" << endl;
            
            root.set_limit_time(tmp_time_limit);
            
            
#ifdef MULTI_THREADING
            //detach threads
            std::thread thr[N_THREADS];
            for (int th = 0; th < N_THREADS; ++th){
                thr[th] = thread(&MonteCarloThread<RootNode<move_t>, ThinkBoard, Evaluator<ThinkBoard>>,
                                 th, &root, &tb, &evaluator);
            }
            for (int th = 0; th < N_THREADS; ++th){
                thr[th].join();
            }
#else
            //call function
            MonteCarloThread<RootNode<move_t>, ThinkBoard, Evaluator<ThinkBoard>>(th, &root, &tb, &evaluator);
            
#endif //MULTI_THREADING
            
            int best_id = root.choose_best_id();//最高評価のもの
            move_t bestMove = root.child[best_id].dive_largest();
            
#ifdef MONITOR
            cerr << "MCP : " << root.trials << " playouts were done." << endl;
            cerr << "MCP : " << mvTT.npages << " children were made." << endl;
            cerr << "MCP : " << bdTT.npages << " nodes were made." << endl;
            
            //bdTT.print();
            
            root.print();
            
            cerr << "MCP : best move - " << bestMove << endl;
            
#endif //MONITOR
            
            bestNRMove = bestMove.genNR(&threadTools[0].ddice);
            
#ifdef MONITOR
            cerr << "MCP : best no-range move - " << bestNRMove << endl;
#endif //MONITOR
            
            //getchar();
            convMove(bestNRMove, pdst);
            /*
             if( CONCEDE_MIN_END<=tb.end && tb.end<=CONCEDE_MAX_END ){//投了を検討
             double wp=(root.child[best_id].mean(root.child[best_id].tmpBest)+1.0)/2.0;
             if( wp < CONCEDE_WP ){//予測勝率が投了ラインより低い
             return -1;//投了
             }
             }
             */
            if( pexpWP!=nullptr ){
                //予測勝率を返す
                *pexpWP=0.5;
            }
            return 0;
        }
    }
}