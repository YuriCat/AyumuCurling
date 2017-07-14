/*
 monteCarlo.h
 Katsuki Ohto
 */

// デジタルカーリング
// モンテカルロ関連定義

#ifndef DCURLING_AYUMU_MC_MONTECARLO_H_
#define DCURLING_AYUMU_MC_MONTECARLO_H_

#include "../ayumu_dc.hpp"

namespace DigitalCurling{
    namespace Ayumu{
        
        constexpr int N_DIV_TRIALS_ROOTCHILD = 8; // ルート着手分岐閾値
        constexpr fpn_t ATT_RATE_ROOTCHILD = 255.0 / 256.0; // 減衰率
        constexpr uint32_t MIN_TRIALS_ROOT = 1;
        constexpr fpn_t K_UCB_ROOT = 2.0;
        
        // leaf node, child
        constexpr int N_DIV_TRIALS_LEAFNODE = 10; // 局面分岐閾値
        constexpr fpn_t ATT_RATE_LEAFCHILD = 0.995; // 減衰率
        constexpr fpn_t POL_SIZE_LEAFCHILD = 4; // 方策関数が何試合分の意味を持つか
        constexpr fpn_t FIRST_SIZE_LEAFCHILD = 0; // プレイアウト回数の初期値
        constexpr fpn_t K_PUCB_LEAF = 2.0;
        constexpr fpn_t K_UCB_LEAF = 1; // UCBの定数
        constexpr fpn_t FF_RATE_LEAFCHILD = 0.3;
        
        // ノードを登録するレンジ(局面認識の細かさ)
        constexpr int RANGE_MAX_LEAFNODE = 15;
        constexpr int RANGE_MIN_LEAFNODE = 0;

        // 展開の最大手数
        constexpr int BOARD_EX_DEPTH_MAX = N_TURNS;
        
        // Kernel Regression 用の定数、関数
        constexpr int N_KR_ERROR_BOXES = 11;
        constexpr int N_KR_CANDIDATE_BOXES = 3;
        constexpr int MID_INDEX_KR = N_KR_ERROR_BOXES/ 2;
        constexpr int N_KR_INNER_W = N_KR_CANDIDATE_BOXES / 2;
        
        constexpr int ZW_KR = (N_KR_ERROR_BOXES - N_KR_CANDIDATE_BOXES) / 2;
        constexpr fpn_t FV_KR_WIDTH = FVX_ERROR_SIGMA * 4;
        constexpr fpn_t FV_KR_STEP = FV_KR_WIDTH * 2 / (N_KR_ERROR_BOXES - 1);
        
        
        int KR_DVtoI(fpn_t dv){ return (dv + FV_KR_WIDTH) / FV_KR_STEP; }
        fpn_t KR_ItoDV(int i){ return i * FV_KR_STEP - FV_KR_WIDTH; }
        
        
        template<class _move_t>
        struct Child{
            // リーフノードの子
            using move_t = _move_t;
            
            move_t mv;
            
            float size;
            float eval_sum;
            
            // 変化させるパラメータ
#ifdef USE_GAUSSIAN_OPTIMIZATION
            float a, b, mu;
            static constexpr fpn_t sigma = FX_ERROR_SIGMA;
#else
            static constexpr float mu = 0;
#endif
            
            
#ifdef USE_KERNEL_REGRESSION
            float eval_sum__[N_KR_ERROR_BOXES];
            float size__[N_KR_ERROR_BOXES];
            
            fpn_t mean__(int j)const{ return eval_sum__[j] / size__[j]; }
#endif
            
            void init()noexcept{
                size = FIRST_SIZE_LEAFCHILD;
                eval_sum = 0;

#ifdef USE_GAUSSIAN_OPTIMIZATION
                b = mu = 0;
                a = sqrt(2 * M_PI) * sigma;
#endif

#ifdef USE_KERNEL_REGRESSION
                for(int j = 0; j < N_KR_ERROR_BOXES; ++j){
                    eval_sum__[j] = 0;
                    //size__[j] = 0;
                    size__[j] = FIRST_SIZE_LEAFCHILD / 2;
                }
                eval_sum__[N_KR_ERROR_BOXES / 2] = 2 * FIRST_SIZE_LEAFCHILD / 2;
                //size__[N_KR_ERROR_BOXES / 2] = FIRST_SIZE_LEAFCHILD;
#endif
            }
            void set(move_t amv)noexcept{
                init();
                mv = amv;
            }
            void attenuate()noexcept{
                size *= ATT_RATE_LEAFCHILD;
                eval_sum *= ATT_RATE_LEAFCHILD;
            }
            
            void feedEval(eval_t aev, fpn_t dvx)noexcept{
                size++;
                eval_sum += aev;
                
#ifdef USE_GAUSSIAN_OPTIMIZATION
                // 評価分布予測を動かす
                fpn_t k = 1 / (sqrt(2 * M_PI) * sigma);
                fpn_t e = exp(-pow(dvx, 2) / sigma);
                fpn_t dvdm = sqrt(2 / M_PI) / pow(sigma, 2) * dvx * e;
                fpn_t dvda = 2 * k * e;
                fpn_t dvdb = 1;
                fpn_t vdist = aev - (a * k * e + b);
                
                fpn_t alpha = 0.005 * (1 + sqrt(size));
                
                fpn_t c = 0.03;
                
                mu += max(-c, min(+c, alpha * 2 * dvdm * vdist));
                a += max(-c, min(+c, alpha * 2 * dvda * vdist));
                b += max(-c, min(+c, alpha * 2 * dvdb * vdist));
                
                mu = max(-0.06, min(+0.06, (double)mu));
#endif
                
#ifdef USE_KERNEL_REGRESSION
                int j = KR_DVtoI(dvx);
                if(0 <= j && j < N_KR_ERROR_BOXES){
                    eval_sum__[j] += aev;
                    size__[j] += 1;
                }
#endif
            }
            
            double mean()const{ return eval_sum / size; }
            
            bool exam()const{
                if(std::isnan(eval_sum)){ cerr << "Child::exam() eval_sum = nan" << endl; return false; }
                if(std::isinf(eval_sum)){ cerr << "Child::exam() eval_sum = inf" << endl; return false; }
                if(std::isnan(size)){ cerr << "Child::exam() size = nan" << endl; return false; }
                if(std::isinf(size)){ cerr << "Child::exam() size = inf" << endl; return false; }
                return true;
            }
        };
        template<class move_t>
        std::ostream& operator<<(std::ostream& os, const Child<move_t>& arg){
            os << arg.mv << " " << arg.mean() << " (" << arg.size << ")";
            return os;
        }
        
        template<class node_t>std::pair<int, int> whereInTT(const node_t&);
        template<class node_t>void printWhereInTT(const node_t&);
        
        template<class _child_t>
        class RecursiveNode{
            // 再帰局面データ
            
        private:
            
            // スピンロック兼局面の同一性の弱確認
#ifdef MULTI_THREADING
            using plock_t = NodeLock<uint64_t>;
            //using plock_t = NodeRMLock<uint64_t, 7>;
            
            mutable plock_t lock_;
#else
            using plock_t = NullNodeLock;
            
            uint64_t lock_;
#endif
            
        public:
            using lock_t = plock_t;
            using child_t = _child_t;
            using move_t = typename _child_t::move_t;
            
            constexpr static int N_CHILDS = 32;
            
            // ターン、レンジ場所のマスク
            constexpr static uint64_t IDENTITY_MASK = ((1ULL << (4 + 4)) - 1ULL) << lock_t::lock_size();
            
            const RecursiveNode<child_t> *parent; // 認識精度を下げた親局面
            
            child_t child[N_CHILDS]; // 言語手による縮約
            
            // information of board
            // BitArray64<4, 16> info;
            
            int childs;
            
            //float alpha, beta;
            float eval_sum;
            
#ifdef USE_EVAL_INT
            eval_t size;
#else
            float size;
#endif
            
            RecursiveNode():
            lock_(0ULL){}
            
            constexpr static uint64_t knitIdentityValue(uint64_t ahash, uint32_t aturn, uint32_t arange)noexcept{
                //cerr << ahash << "," << aturn << "," << arange << endl;
                return (ahash & (~IDENTITY_MASK)) | (aturn << (lock_t::lock_size() + 4)) | (arange << lock_t::lock_size());
            }
            
            void init()noexcept{
                childs = 0;
                size = 0;
                eval_sum = 0;
            }
            
            void close()noexcept{
                childs = 0;
                parent = nullptr;
                forceRegist(-1);
            }
            
            int NChilds()const noexcept{ return childs; }
            void setNChilds(int nc)noexcept{ childs = nc; }
            void addNChilds()noexcept{ ++childs; }
            
            int turn()const noexcept{ return (lockValue() >> (lock_t::lock_size() + 4)) & 15; }
            int range()const noexcept{ return (lockValue() >> lock_t::lock_size()) & 15; }
            Color turnColor()const noexcept{ return getColor(turn()); }
            
            fpn_t mean()const{ return eval_sum / size; }
            
            template<class board_t>
            void set(const board_t& abd)noexcept{
                // nucleus value has set
                init();
                parent = nullptr;
            }
            
            void setNull(int e, int t, int rs)noexcept{
                // nucleus value has set
                init();
                forceRegist(knitIdentityValue(0, t, 0));
                parent = nullptr;
            }
            
            template<class board_t, class node_t>
            void succeed(const board_t& abd, node_t& anode)noexcept{
                // nucleus value has set
                init();
                parent = &anode;
            }
            
            template<class result_t>
            void feedEval(Color c, const result_t& result){
                // in making lock
                // こちらを減衰させるべきか??
                const auto& amv = result.mv;
                
                // 静的評価関数と末端報酬の重みつけ
#ifdef USE_STATIC_VALUE_FUNCTION
                const fpn_t lambda = 0.3 / (1 + log(1 + size));
#else
                const fpn_t lambda = 0;
#endif
                const eval_t ev = result.eval(c, lambda);
                
                // 言語手探索
                const int idx = searchChild(amv);
                if(idx < 0){ // 見つからなかった
                    if(NChilds() >= N_CHILDS){
                        //whereInTT(*this); cerr << *this << endl; getchar();
                        return;
                    }
                    //attenuate();
                    const int newIdx = -1 - idx;
                    insertChild(newIdx, amv);
                    child[newIdx].feedEval(ev, result.dvx);
                    eval_sum += ev;
                    ++size;
                }else{
                    //attenuate();
                    child[idx].feedEval(ev, result.dvx);
                    eval_sum += ev;
                    ++size;
                }
                ASSERT(exam(),);
            }
            
            void attenuate(){
                // ロックがかかっている間であることを仮定する
                for(int i = 0, n = NChilds(); i < n; ++i){
                    assert(i < N_CHILDS);
                    child[i].eval_sum *= ATT_RATE_LEAFCHILD;
                    child[i].size *= ATT_RATE_LEAFCHILD;
                }
                eval_sum *= ATT_RATE_LEAFCHILD;
                size *= ATT_RATE_LEAFCHILD;
            }
            
            void insertChild(int i, const move_t& amv){
                // ロックがかかっている間であることを仮定する
                for(int c = min(N_CHILDS - 1, NChilds()) - 1; c >= i; --c){
                    child[c + 1] = child[c];
                }
                child[i].set(amv);
                ASSERT(amv.isRelative(), cerr << amv << endl;); // 絶対着手は生成していない
                ASSERT(!(whereInTT(*this).first == 0 && amv.hasContact()), cerr << amv << endl;); // 空場でコンタクト着手
                //cerr << "inserted " << i << endl;
                addNChilds();
                size += FIRST_SIZE_LEAFCHILD;
                //cerr << NChilds();
            }
            int searchChildIn(const move_t& mv, int lb, int ub)const{
                for(int i = lb; i < ub; ++i){
                    if(mv.data() == child[i].mv.data()){
                        return i;
                    }else if(mv.data() < child[i].mv.data()){
                        return -1 - i;
                    }
                }
                return -1 - ub;
            }
            /*
             int searchChildIn(const move_t& mv, int lb, int ub)const{
             if(lb > ub){ return -1 - lb; }
             int next = (lb + ub) / 2;
             if(child[next].mv.data() > mv.data()){
             return searchChildIn(mv, next + 1, ub);
             }else if(child[next].data() < mv.data()){
             return searchChildIn(mv, lb, next - 1);
             }
             return next;
             }*/
            int searchChild(const move_t& mv)const{
                return searchChildIn(mv, 0, NChilds());
            }
            
            template<typename callback_t>
            void iterateChild(const callback_t& callback){
                for(int i = 0, n = NChilds(); i < n; ++i){
                    callback(i, child[i]);
                }
            }
            
            // 排他制御のための関数
#ifdef MULTI_THREADING
            uint64_t lockValue()const noexcept{ return lock_.data(); }
            void startReading()const noexcept{ lock_.start_read(); }
            void finishReading()const noexcept{ lock_.finish_read(); }
            void startFeeding()const noexcept{ lock_.start_feed(); }
            void finishFeeding()const noexcept{ lock_.finish_feed(); }
            void startMaking()const noexcept{ lock_.start_make(); }
            void finishMaking()const noexcept{ lock_.finish_make(); }
            bool isMaking()const noexcept{ return lock_.is_being_made(); }
            bool isReading()const noexcept{ return lock_.is_being_read(); }
            bool regist(uint64_t val)noexcept{ return lock_.regist(val); }
            bool registAndStartMaking(uint64_t val)noexcept{ return lock_.regist_start_make(val); }
            void forceRegist(uint64_t val)noexcept{ return lock_.force_regist(val); }
            bool compare(uint64_t val)const noexcept{ return lock_.compare(val); }
            constexpr static bool compare(uint64_t val0, uint64_t val1)noexcept{ return lock_t::compare(val0, val1); }
#else
            uint64_t lockValue()const noexcept{ return lock_; }
            void startReading()const noexcept{}
            void finishReading()const noexcept{}
            void startFeeding()const noexcept{}
            void finishFeeding()const noexcept{}
            void startMaking()const noexcept{}
            void finishMaking()const noexcept{}
            bool isMaking()const noexcept{ return false; }
            bool isReading()const noexcept{ return false; }
            bool regist(uint64_t val)noexcept{ lock_ = val; return true; }
            bool registAndStartMaking(uint64_t val)noexcept{ return regist(val); }
            void forceRegist(uint64_t val)noexcept{ lock_ = val; }
            bool compare(uint64_t val)const noexcept{ return (lock_ == val); }
            constexpr static bool compare(uint64_t val0, uint64_t val1)noexcept{ return (val0 == val1); }
#endif
            bool exam()const{
                // in making lock
                // 値のチェック
                if(!(TURN_LAST <= turn() && turn () < N_TURNS)){
                    cerr << "RecursiveNode::exam() : illegal turn(" << turn() << ")." << endl;
                    return false;
                }
                if(!(0 <= range() && range() < 16)){
                    cerr << "RecursiveNode::exam() : illegal range(" << range() << ")." << endl;
                    return false;
                }
                if(!(0 <= NChilds() && NChilds() <= N_CHILDS)){
                    cerr << "RecursiveNode::exam() : illegal number of childs(" << NChilds() << ")." << endl;
                    return false;
                }
                
                // 各着手のチェック
                for(int i = NChilds() - 1; i >= 0; --i){
                    if(!child[i].exam()){
                        cerr << "RecursiveNode::exam() : illegal child(" << i << " in " << NChilds() << ")." << endl;
                        return false;
                    }
                }
                // 同じ着手が二つ登録されていないかチェック
                for(int i = NChilds() - 1; i >= 0; --i){
                    for(int j = i - 1; j >= 0; --j){
                        if(child[i].mv.data() == child[j].mv.data()){
                            cerr << "RecursiveNode::exam() : same move child(" << j << ", " << i << " in " << NChilds() << ")." << endl;
                            cerr << "move = " << std::hex << child[i].mv.data() << std::dec << endl;
                            return false;
                        }
                    }
                }
                return true;
            }
        };
        
        template<class child_t>
        void feedForward(const RecursiveNode<child_t>& node, RecursiveNode<child_t> *const pnext){
            // nextに前向きフィード
            pnext->startMaking();
            int i0 = 0, i1 = 0, n0 = node.NChilds(), n1 = pnext->NChilds();
            while(1){
                if(node.child[i0].mv.data() == pnext->child[i1].mv.data()){
                    pnext->child[i1].feedChild(node.child[i0]);
                    ++i0; ++i1;
                    if(i0 >= n0 || i1 >= n1){ break; }
                }else if(node.child[i0].mv.data() > pnext->child[i1].mv.data()){
                    ++i1;
                    if(i1 >= n1){ break; }
                }else{
                    ++i0;
                    if(i0 >= n0){ break; }
                }
            }
            pnext->finishMaking();
        }
        
        template<class child_t>
        void copy(const RecursiveNode<child_t>& arg, RecursiveNode<child_t> *const pdst){
            pdst->forceRegist(arg.lockValue());
            pdst->parent = arg.parent;
            pdst->childs = arg.childs;
            pdst->eval_sum = arg.eval_sum;
            pdst->size = arg.size;
            for(int i = 0; i < arg.childs; ++i){
                pdst->child[i] = arg.child[i];
            }
        }
        
        template<class child_t>
        std::ostream& operator<<(std::ostream& os, const RecursiveNode<child_t>& arg){
            os << "lock_size = " << RecursiveNode<child_t>::lock_t::lock_size() << endl;
            os << "(t = " << arg.turn() << ", r = " << arg.range();
            os << ", h = " << std::hex << arg.lockValue() << std::dec;
            os << ", sz = " << arg.size << ", p = ";
            if(arg.parent == nullptr){
                os << "top)";
            }else{
                os << std::hex << (uint64_t)arg.parent << std::dec << ")";
            }
            os << endl;
            for (int c = 0; c < arg.NChilds(); ++c){
                os << arg.child[c] << endl;
            }
            return os;
        }
        
        template<class _page_t, int _SIZE>
        class BoardTranspositionTable{ // 局面置換表
            // 開番地法
        public:
            using page_t = _page_t;
            
        private:
            using index_t = int;
            
            constexpr static int SIZE_ = _SIZE;
            constexpr static uint64_t DELETED_ = 0xffffffffffffffff;
            constexpr static int REHASH_MAX_ = 16;
            constexpr static int REAL_SIZE_ = SIZE_ + REHASH_MAX_ * 2;
            
            constexpr static bool exam_index(index_t index)noexcept{
                return (0 <= index && index < REAL_SIZE_);
            }
            static void assert_index(const index_t index){
                ASSERT(exam_index(index),
                       cerr << "invalid index " << index << " in(0 ~ " << (REAL_SIZE_ - 1) << ")" << endl;);
            }
            
            constexpr static index_t convHash_Index(const uint64_t hash)noexcept{
                return index_t(hash % (unsigned int)SIZE_);
            }
            constexpr static index_t procIndex(const index_t index)noexcept{
                return index + 1;
            }
            
            bool examPagePointer(const page_t *const p)const noexcept{
                return (&this->page(0) <= p && p <= &this->page(REAL_SIZE_ - 1));
            }
            int convPointer_Index(const page_t *const p)const noexcept{
                return p - &this->page(0);
            }
            void assert_page_pointer(const page_t *const p)const{
                ASSERT(examPagePointer(p),
                       cerr << "invalid pointer index " << convPointer_Index(p);
                       cerr << " in(0 ~ " << (REAL_SIZE_ - 1) << ")" << endl;);
            }
            
            std::atomic<uint32_t> pages_;
            int oldest_age; // まだ消えていない最も古い年代
            int last_age; // 最も新しく登録されたと思われる年代
            int end_;
            
        public:
            HashBookAnalyzer ana;
            
            page_t page_[REAL_SIZE_];
            
            BoardTranspositionTable():
            ana("BoardTranspositionTable", REAL_SIZE_, sizeof(*this)){}
            
            bool is_over80()const noexcept{
                return (pages_ > (REAL_SIZE_ * 80) / 100);
            }
            bool is_over90()const noexcept{
                return (pages_ > (REAL_SIZE_ * 90) / 100);
            }
            bool is_over95()const noexcept{
                return (pages_ > (REAL_SIZE_ * 95) / 100);
            }
            
            bool isFilled()const noexcept{
                return is_over95();
            }
            
            uint32_t pages()const noexcept{ return pages_; }
            const page_t& page(const index_t index)const{ assert_index(index); return page_[index]; }
            page_t& page(const index_t index){ assert_index(index); return page_[index]; }
            int end()noexcept{ return end_; }
            
            void init()noexcept{
                memset(page_, 0, sizeof(page_));
                oldest_age = 1;
                last_age = 0;
                pages_ = 0;
                ana.init();
                end_ = INT_MAX;
            }
            void setEnd(int e){
                end_ = e;
            }
            void proceed(int t)noexcept{
                for(int i = 0; i < REAL_SIZE_; ++i){
                    if(page(i).turn() >= t){ // もう不要
                        page(i).forceRegist(0); // データが入っていないことを示す
                        --pages_;
                    }else{ // まだ使えるかもしれない
                        const uint64_t hash = page(i).lockValue();
                        const index_t bi = convHash_Index(hash);
                        for(int ii = bi; ii < i; ++ii){
                            if(page(ii).lockValue() == 0){ // 場所がある
                                copy(page(i), &page(ii)); // 移動
                                page(i).forceRegist(0); // データが入っていないことを示す
                                break;
                            }
                        }
                    }
                    page(i).parent = nullptr;
                }
            }
            
            void set_age(int aage)noexcept{
                last_age = aage;
            }
            
            page_t* registAndStartMaking(const uint64_t value, page_t *const pfirst)noexcept{
                ASSERT(0 <= (pfirst - page_) && (pfirst - page_) < (SIZE_ + REHASH_MAX_),);
                page_t *p = pfirst;
                if(p->registAndStartMaking(value)){ // 登録成功
                    //cerr << value << endl;
                    ++pages_; ana.addRegistration(); ana.addFilled();
                    assert_page_pointer(p);
                    return p;
                }
                // もともと登録したかった場所には登録できなかった
                // なので改めて別の場所を探す
                if(REHASH_MAX_ > 0){
                    const page_t *const plast = p + REHASH_MAX_ - 1;
                    for(;;){
                        ++p;
                        if(!p->lockValue()){
                            if(p->registAndStartMaking(value)){ // 登録成功
                                ++pages_; ana.addRegistration(); ana.addFilled();
                                assert_page_pointer(p);
                                return p;
                            }
                        }
                        if(p == plast){ ana.addRegistrationFailure(); return nullptr; }
                    }
                }
                UNREACHABLE;
            }
            
            page_t* regist(const uint64_t value, page_t *const pfirst)noexcept{
                ASSERT(0 <= (pfirst - page_) && (pfirst - page_) < (SIZE_ + REHASH_MAX_),);
                page_t *p = pfirst;
                if(p->regist(value)){ // 登録成功
                    //cerr << value << endl;
                    ++pages_; ana.addRegistration(); ana.addFilled();
                    assert_page_pointer(p);
                    return p;
                }
                // もともと登録したかった場所には登録できなかった
                // なので改めて別の場所を探す
                if(REHASH_MAX_ > 0){
                    const page_t *const plast = p + REHASH_MAX_ - 1;
                    for(;;){
                        ++p;
                        if(!p->lockValue()){
                            if(p->regist(value)){ // 登録成功
                                ++pages_; ana.addRegistration(); ana.addFilled();
                                assert_page_pointer(p);
                                return p;
                            }
                        }
                        if(p == plast){ ana.addRegistrationFailure(); return nullptr; }
                    }
                }
                UNREACHABLE;
            }
            
            page_t* read(const uint64_t ahash, const uint64_t avalue, bool *const pfound)noexcept{
                //int index = convHash_Index(ahash);
                int index = convHash_Index(avalue);
                page_t *p = &page_[index];
                const page_t *const plast = p + REHASH_MAX_ - 1;
                for(;;){
                    const uint64_t value = p->lockValue();
                    if(!value){ // 1行上の時点では空
                        *pfound = false; ana.addWhite();
                        assert_page_pointer(p);
                        return p;
                    }else if(p->compare(value, avalue)){ // 弱い同一性確認
                        //cerr << "same " << std::hex << value << ", " << avalue << std::dec << endl;
                        *pfound = true; ana.addHit();
                        assert_page_pointer(p);
                        return p;
                    }else if(p == plast){
                        *pfound = false; ana.addUnfounded();
                        return nullptr;
                    }
                    //cerr << "diff " << std::hex << value << ", " << avalue << std::dec << endl;getchar();
                    ++p;
                }
                UNREACHABLE;
            }
            /*
             page_t* readAndRegist(const uint64_t ahash, const uint64_t avalue, bool *const pfound)noexcept{
             int index = convHash_Index(ahash);
             page_t *p = &page[index];
             const page_t *const plast = p + REHASH_MAX_ - 1;
             for(;;){
             const uint64_t value = p->lockValue;
             if(!value){ // 1行上の時点では空
             page_t *pregisted = regist(avalue, p);
             *pfound = false; ana.addWhite();
             return pregisted;
             }else if(p->compare(value, avalue)){ // 弱い同一性確認
             *pfound = true; ana.addHit();
             return p;
             }else if(p == plast){
             *pfound = false; ana.addDiffHash();
             return nullptr;
             }
             ++p;
             }
             UNREACHABLE;
             }
             */
            void print(double line = 0.0)const{
                for (int i = 0; i < REAL_SIZE_; ++i){
                    if (page(i).lockValue() && page(i).lockValue() != DELETED_ && page(i).size >= line){
                        cerr << "node " << i << " " << page(i) << endl;
                    }
                }
            }
        };
        
        template<class page_t>
        struct NullBoardTranspositionTable{
            std::array<page_t, N_TURNS> page;
            int end_;
            
            int end()const noexcept{ return end_; }
            void setEnd(int e)noexcept{ end_ = e; }
            page_t* read(int t){
                return &page[t];
            }
            void init(int e, int rs)noexcept{
                memset(&page[0], 0, sizeof(page));
                for(int t = 0; t < N_TURNS; ++t){
                    page[t].setNull(e, t, rs);
                }
                end_ = INT_MAX;
            }
            void print(double line = 0.0)const{
                for (int t = 0; t < N_TURNS; ++t){
                    if (page[t].size >= line){
                        cerr << "null node " << t << " " << page[t] << endl;
                    }
                }
            }
        };
        
        using tt_t = BoardTranspositionTable<RecursiveNode<Child<MoveXY>>, (1 << 21) + 3>;
        
        tt_t *pbdTT;
        NullBoardTranspositionTable<RecursiveNode<Child<MoveXY>>> bdNullTT;
        
        struct TTInitializer{
            TTInitializer(){
                pbdTT = new BoardTranspositionTable<RecursiveNode<Child<MoveXY>>, (1 << 21) + 3>();
            }
            ~TTInitializer(){
                delete pbdTT;
            }
        };
        
        TTInitializer TTInitializer_;
        
        template<class node_t>
        std::pair<int, int> whereInTT(const node_t& nd){
            int i = &nd - &bdNullTT.page[0];
            if(0 <= i && i < N_TURNS){
                return std::make_pair(0, i);
            }else{
                i = &nd - &pbdTT->page(0);
                return std::make_pair(1, i);
            }
        }
        template<class node_t>
        void printWhereInTT(const node_t& nd){
            int i = &nd - &bdNullTT.page[0];
            if(0 <= i && i < N_TURNS){
                cerr << "NullTT : " << i << endl;
            }else{
                i = &nd - &pbdTT->page(0);
                cerr << "normalTT : " << i << endl;
            }
        }
    }
    
    
}

#endif // DCURLING_AYUMU_MC_MONTECARLO_H_
