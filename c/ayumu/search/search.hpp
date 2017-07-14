/*
 search.hpp
 Katsuki Ohto
 */

// 座標を離散化しての alpha-beta search

namespace DigitalCurling{
    
    constexpr int N_MOVES = 8192;
    
    struct NodeBoard : public ThinkBoard{
        fMoveXY<> fmv;
        ContactTree ct;
        double ev;
        int depth;
        int around;
        double iev;
    };
    
    struct MiniNode{
        int id; double ev;
    };
    
    struct SearchStackElement{
        std::tuple<int, int> killerIndex[2]; // キラー着手のインデックス
        eval_t passEval;
        int currentIndex;
        
        void clear()noexcept{
            
        }
    };
    
    struct Searcher{
        //MiniNode mn[N_MOVES];
        //MoveXY buf[N_MOVES];
        int NMoves;
        int NMiniNodes;
        int depth;
        
        // 探索スタック
        std::array<SearchStackElement, N_TURNS> stack;
    };
	
    template<class board_t>
    std::tuple<int, eval_t> search(Searcher *const psr, const board_t& bd){
        
        // 探索スタックの初期化
        
        // ハッシュ値でオーダリング
        
        // 探索
    }
    
    template<class board_t>
    std::tuple<fMoveXY<>, eval_t> iterativeDeepen(){
        
    }
    
    template<class board_t>
    std::tuple<fMoveXy<>,eval_t> play(const board_t& bd){

        Searcher sr;
        //メッシュを初期化
        for(int i=0;i<256;++i){
            for(int j=0;j<256;++j){
                NodeBoard& nd=node[i][j];
                nd.depth=0;
                nd.around=25*25;
                nd.iev=0;
            }
        }
        
        // 離散化した着手候補を生成
        sr.NMoves = genAllVMove(sr.buf, bd);
        
        // 着手に得点を付けてオーダリング
        
        // 探索
        search(&sr,bd);
        
        return make_tuple
    }
    
}

