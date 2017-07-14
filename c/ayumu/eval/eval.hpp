//デジタルカーリング
//局面絶対評価点や局面修正評価点の計算

namespace DigitalCurling{

    //空場評価
    //fpn_t evalNFTableOpening[16];//序盤評価
    //fpn_t evalNFTable[16];

    using EvalVector = std::array<eval_t,N_TURNS + 1>;

    constexpr int FR_1ST_HOUSE_STEP = FR_HOUSE_RAD / 8;
    constexpr int FTH_1ST_HOUSE_STEP = M_PI / 8;
    constexpr int FX_1ST_GUARD_STEP = FX_PA_LENGTH / 16;
    constexpr int FY_1ST_GUARD_STEP = FY_PA_WIDTH / 16;
    constexpr int FTH_2ND_STEP = FR_1ST_STEP;
    constexpr int FTH_2ND_STEP = 2 * M_PI / 16;

    enum{
        EV_PHASE_TURN = 0,//
        EV_PHASE_LEAD,
        EV_PHASE_ALL,
    };

    constexpr int evPhaseNumTable[EV_PHASE_ALL] = {
        16,//ターン
        4,//先手2点以上リード、先手リード、同点、後手リード
    };

    enum{
        EV_1ST_HOUSE=0,
        EV_1ST_GUARD,
        EV_2ST,
        EV_SCORE,
        EV_ALL,
    };

    //候補数(* EvalVectorのサイズ が必要)
    constexpr int evNumTable[EV_ALL] = {
        0,//8*8,
        0,//16*16,EV
        0,//12*8*8*16,
        17,
    };

    constexpr int EV_NUM(unsigned int fea){
        return evNumTable[fea];
    }
    constexpr int EV_IDX(unsigned int fea){
        return (fea == 0) ? 0 : (EV_IDX(fea - 1) + POL_NUM(fea - 1));
    }
    constexpr int EV_NUM_ALL = EV_IDX(POL_ALL);

    eval_t evalTable[evPhaseNumTable[EV_PHASE_LEAD] * evPhaseNumTable[EV_PHASE_TURN] * EV_NUM_ALL][N_TURNS + 1];

#define Foo(v,) s+=param[(i)];\
if( v!=nullptr ){(v)->push_back(std::pair<int,double>((i),1.0));}

#define FooX(s,i,x,v) s+=param[(i)]*(x);\
if( v!=nullptr ){(v)->push_back(std::pair<int,double>((i),(x)));\
    ASSERT(!std::isinf(x),cerr<<(x)<<endl;);ASSERT(!std::isnan(x),cerr<<(x)<<endl;);}

    template<class stone_t>
    void ST1_HOUSE(EvalVector& ev, const stone_t& st){
        ASSERT(isInHouse(st),);
        int index = EV_IDX(ST1_HOUSE) + (st.r / FR_ST1_HOUSE_STEP) * 8 + fabs(st.th) / FTH_1ST_HOUSE_STEP;
        for (int s = 0; s < ev.size(); ++s){
            ev[s] += evTable[index + s];
        }
    }

    template<class board_t>
    eval_t evaluate(const boatd_t& bd,const RewardVector& rewVector){
        
        EvalVector evVector = {0};

        //局面の情報からevVectorを計算
        /*iterateAllStones(bd, [&evVector](const auto& st)->{
            if (isInHouse(st)){
                ST1_HOUSE(evVector, st);
            }
            else{
                ST1_GURAD(evVector, st);
            }
        });
        iterateAllStones(bd, [&evVector](const auto& st0)->{
            iterateAllStones(bd, [&evVector](const auto& st1)->{
                if (!isSame(st0, st1)){
                    ST2(evVector, st);
                }
            });
        });*/

        //ターン、得点、第2得点、ガードの数、No1の堅さ
        const int score = countScore(bd);
        const int lead = (bd.rscore > 1) ? 0 :
            ((bd.rscore > 0) ? 1 :
            ((bd.rscore == 0) ? 2 :
            3));
        constexpr int base = (bd.getTurn()*EV_NUM(EV_PHASE_LEAD) + lead)*EV_NUM_ALL;

        Foo(evVector, base + score + 8);

        //softmaxをとって内積計算
        eval_t evSum = 0;
        eval_t ev = 0;
        for (int i = 0; i < rewVector.size(); ++i){
            eval_t tmp = exp(evEvctor[i]);
            ev += tmp*rewVector[i];
            evSum += tmp;
        }
        return ev / evSum;
    }

}