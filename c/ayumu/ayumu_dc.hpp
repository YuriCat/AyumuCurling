/*
 ayumu_dc.hpp
 Katsuki Ohto
 */

// Digital Curling
// primitive values and classes for AYUMU

#ifndef DCURLING_AYUMU_DC_HPP_
#define DCURLING_AYUMU_DC_HPP_

#include "../dc.hpp"

#ifndef NO_SIMULATION
#include "../simulation/fastSimulator.hpp"
#endif

namespace DigitalCurling{
    
    /**************************戦略的なエンド、ターンの値**************************/
    
    // TODO: より根拠のある数値にする
    
    constexpr int END_B0_CLEAN = END_LAST + 3; // 同点の先手がクリーンに戦うべき最後のエンド
    constexpr int END_B1_CLEAN = END_LAST + 0; // 1点リードの先手がクリーンに戦うべき最後のエンド
    constexpr int END_B2_CLEAN = END_LAST + 0; // 2点リードの先手がクリーンに戦うべき最後のエンド
    constexpr int END_W0_CLEAN = END_LAST + 0; // 同点の後手がクリーンに戦うべき最初のエンド
    constexpr int END_W1_CLEAN = END_LAST + 3; // 1点リードの後手がクリーンに戦うべき最初のエンド
    constexpr int END_W2_CLEAN = END_LAST + 5; // 2点リードの後手がクリーンに戦うべき最初のエンド
    
    constexpr bool isCleanBetterScore(Color c, unsigned int e, int rs)noexcept{
        return (isBlack(c) && (((rs > 0) && (e >= END_B1_CLEAN)) || ((rs == 0) && (e >= END_B0_CLEAN))))
        || (isWhite(c) && (((rs < -1) && (e <= END_W2_CLEAN)) || ((rs < 0) && (e <= END_W1_CLEAN)) || ((rs == 0) && (e <= END_W0_CLEAN))));
    }
    
    /**************************着手ラベル定義**************************/
    
    namespace PostGuardLength{
        enum{
            SHORT = 0,
            MIDDLE = 1,
            LONG = 2,
            MAX = 3,
        };
    }
    
    namespace HitWeight{
        enum{
            TOUCH = 0,
            WEAK = 1,
            MIDDLE = 2,
            STRONG = 3,
            MAX = 4,
        };
    }
    namespace ComeAroundLength{
        enum{
            SHORT = 0,
            MIDDLE = 1,
            LONG = 2,
            TEE = 3,
            MAX = 4,
        };
    }
    
    namespace DrawPos{
        enum{
            TEE = 0,
            S0,
            S2,
            S4,
            S6,
            L0,
            L1,
            L2,
            L3,
            L4,
            L5,
            L6,
            L7,
            G3,
            G4,
            G5,
            MAX = 16,
        };
    }
    enum MoveType{
        NONE,
        WHOLE,
        ONE,
        TWO,
        MAX,
    };
    
    constexpr int typeMovesTable[] = {
        // 各タイプの着手の数
        1,
        2 + 1,
        5 + 1 + 1,
        2,
    };
    
    enum Standard{
        // 定跡手番号
        // パス 0
        PASS = 0,
        // 0石 1 ~ 4
        DRAW,
        PREGUARD,
        L1DRAW,
        // 1石 4 ~ 10
        FREEZE,
        COMEAROUND,
        POSTGUARD,
        HIT,
        PEEL,
        DRAWRAISE,
        // ROLLIN,
        
        // WICK,
        TAKEOUT,
        // 2石 11 ~ 10
        DOUBLE,
        RAISETAKEOUT,
        // FRONTTAKE,
        // BACKTAKE,
        // HITROLL,
        // DOUBLEROLLIN,
        //
        // MIDDLEGUARD,
        N_STANDARD_MOVES,
    };
    
    constexpr int N_TYPE_MOVES(unsigned int t){
        return typeMovesTable[t];
    }
    constexpr int IDX_TYPE_MOVE(unsigned int t){
        return (t == 0) ? 0 : (IDX_TYPE_MOVE(t - 1) + N_TYPE_MOVES(t - 1));
    }
    constexpr int N_MOVES_ALL = IDX_TYPE_MOVE(4);
    
    // 範囲付き着手をルートで生成していたころの名残
    constexpr int RANGE_ROOT_DRAW = 10;
    constexpr int RANGE_ROOT_TEESHOT = 11;
    constexpr int RANGE_ROOT_HIT = 10;
    constexpr int RANGE_ROOT_COMEAROUND = 10;
    constexpr int RANGE_ROOT_POSTGUARD = 10;
    
    const std::string shotString[N_STANDARD_MOVES] = {
        // enum Standard と合わせる
        "Pass", "Draw", "PreGuard", "L1Draw", "Freeze",
        "ComeAround", "PostGuard", "Hit", "Peel",
        "DrawRaise", "TakeOut", "Double", "RaiseTakeOut",
    };
    
    /**************************石の座標関連追加アルゴリズム**************************/
    
    // TODO: もっと根拠のある数式にする or ちゃんとテーブルを作る
    
    constexpr fpn_t FR_ACTIVE_LINE = FR_HOUSE_RAD + 3 * FR_STONE_RAD;
    constexpr fpn_t F_ROLLIN_RATE = 1.5;
    constexpr fpn_t FR_TAIL_RADIUS_MIN = 2 * FR_STONE_RAD + 0.1;
    constexpr fpn_t FR_TAIL_RADIUS_MAX = 8 * FR_STONE_RAD + 0.1;
    
    template<int IN_HOUSE = _BOTH>
    bool isInGuardZone(const fpn_t& x, const fpn_t& y)noexcept{ // フロントガードと認めるゾーン
        if(IN_HOUSE == _YES){ return false; }
        if(y > FY_TEE){ return false; }
        if(fabs(x - FX_TEE) > FR_HOUSE_RAD){ return false; }
        if(IN_HOUSE == _BOTH){
            if(isInHouse(x, y)){ return false; }
        }
        return true;
    }
    
    template<int IN_HOUSE = _BOTH>
    bool isInRollInZone(const fpn_t& x, const fpn_t& y)noexcept{ // ダブルロールイン可能なゾーン
        if(IN_HOUSE == _YES){ return false; }
        if(IN_HOUSE == _BOTH){
            if(isInHouse(x, y)){ return false; }
        }
        return (y < FY_TEE) && XYtoR2(x - FX_TEE, (y - FY_TEE) / F_ROLLIN_RATE) < FR2_IN_HOUSE;
    }
    template<int IN_HOUSE = _BOTH>
    bool isInActiveZone(const fpn_t& x, const fpn_t& y)noexcept{ // 試合に関係ありそうなゾーン
        if(IN_HOUSE == _YES){ return true; }
        if(y < FY_TEE){
            return (fabs(x - FX_TEE) <= FR_ACTIVE_LINE);
        }else{
            return XYtoR2(x - FX_TEE, y - FY_TEE) <= pow(FR_ACTIVE_LINE, 2);
        }
    }
    bool isInDrawRaiseZone(const fpn_t& x, const fpn_t& y)noexcept{ // ドローレイズを狙うゾーン
        if ((y < FY_TEE - FR_STONE_RAD)
            && (y > FY_TEE - (FR_HOUSE_RAD + 5 * FR_STONE_RAD))
            && (fabs(x - FX_TEE) < min(FR_HOUSE_RAD * 0.6, (FY_TEE - y) * sin(M_PI / 6)))){
            return true;
        }
        return false;
    }
    
    template<class pos_t, int IN_HOUSE = _BOTH>
    bool isInGuardZone(const pos_t& pos)noexcept{ // フロントガードと認めるゾーン
        if(IN_HOUSE == _YES){ return false; }
        if(IN_HOUSE == _BOTH){
            if(isInHouse(pos)){ return false; }
        }
        if(pos.y > FY_TEE){ return false; }
        if(fabs(pos.x - FX_TEE) > FR_HOUSE_RAD){ return false; }
        return true;
    }
    
    template<class pos_t, int IN_HOUSE = _BOTH>
    bool isInRollInZone(const pos_t& pos)noexcept{ // ダブルロールイン可能なゾーン
        if(IN_HOUSE == _YES){ return false; }
        if(IN_HOUSE == _BOTH){
            if(isInHouse(pos)){ return false; }
        }
        return (pos.y < FY_TEE) && XYtoR2(pos.x - FX_TEE, (pos.y - FY_TEE) / F_ROLLIN_RATE) < FR2_IN_HOUSE;
    }
    template<class pos_t, int IN_HOUSE = _BOTH>
    bool isInActiveZone(const pos_t& pos)noexcept{ // 試合に関係ありそうなゾーン
        if(IN_HOUSE == _YES){ return true; }
        if(pos.y < FY_TEE){
            return (fabs(pos.x - FX_TEE) <= FR_ACTIVE_LINE);
        }else{
            return isNearTee(pos, FR_ACTIVE_LINE);
        }
    }
    template<class pos_t>
    bool isInDrawRaiseZone(const pos_t& pos)noexcept{
        return isInDrawRaiseZone(pos.x, pos.y);
    }
    
    template<class board_t>
    int countNInActiveZone(const board_t& bd)noexcept{
        return countStones(bd, [](const auto& st)->bool{ return isInActiveZone(st); });
    }
    template<class board_t>
    int countNInActiveZone(const board_t& bd, Color c){
        return countStones(bd, c, [](const auto& st)->bool{ return isInActiveZone(st); });
    }
    
    /**************************定石ドローの生成**************************/
    
    template<class pos_t>
    void realizeDrawPosition(pos_t *const ppos, int n){
        
        constexpr fpn_t r_s = 0.9;
        constexpr fpn_t r_l = FR_HOUSE_RAD - 2 * FR_STONE_RAD;
        
        switch(n){
            case 0: ppos->set(FX_TEE, FY_TEE); break;
            case 1: ppos->set(FX_TEE, FY_TEE + 4 * FR_STONE_RAD); break;
            case 2: ppos->set(FX_TEE + r_s, FY_TEE); break;
            case 3: ppos->set(FX_TEE, FY_TEE + 2 * FR_STONE_RAD); break;
            case 4: ppos->set(FX_TEE - r_s, FY_TEE); break;
            case 5: ppos->set(FX_TEE, FY_TEE + r_l); break;
            case 6: ppos->set(FX_TEE + r_l / sqrt(2), FY_TEE + r_l / sqrt(2)); break;
            case 7: ppos->set(FX_TEE + r_l, FY_TEE); break;
            case 8: ppos->set(FX_TEE + r_l / sqrt(2), FY_TEE - r_l / sqrt(2)); break;
            case 9: ppos->set(FX_TEE, FY_TEE - FR_HOUSE_RAD); break;
            case 10: ppos->set(FX_TEE - r_l / sqrt(2), FY_TEE - r_l / sqrt(2)); break;
            case 11: ppos->set(FX_TEE - r_l, FY_TEE); break;
            case 12: ppos->set(FX_TEE - r_l / sqrt(2), FY_TEE + r_l / sqrt(2)); break;
            case 13: ppos->set(FX_TEE + 0.6, FY_TEE - 2.17); break;
            case 14: ppos->set(FX_TEE, FY_TEE - 3.0); break;
            case 15: ppos->set(FX_TEE - 0.6, FY_TEE - 2.17); break;
            default: UNREACHABLE; break;
        }
    }
    
    /**************************16ビット離散座標**************************/
    
    constexpr int RESOLUTION_I16X = 15;
    constexpr int RESOLUTION_I16Y = 15;
    
    constexpr uint32_t I16R_IN_HOUSE = (1U << 12);
    
    constexpr uint32_t I16X_MID = 1U << (RESOLUTION_I16X - 1);
    constexpr uint32_t I16Y_MID = 1U << (RESOLUTION_I16Y - 1);
    
    constexpr int RESOLUTION_I16TH = 15;
    constexpr uint32_t I16TH_MIN = 0;
    constexpr uint32_t I16TH_MAX = (1U << RESOLUTION_I16TH) - 1U;
    
    /*************************位置の連続座標と離散座標の変換**************************/
    
    // 座標変換
    
    // r,x,yのステップ
    constexpr fpn_t FR_I16R_STEP = (FR_HOUSE_RAD + FR_STONE_RAD) / (double)I16R_IN_HOUSE;
    constexpr fpn_t FR_I16R_INVSTEP = (1.0 / FR_I16R_STEP);
    
    // thetaのステップ
    constexpr fpn_t FTH_I16TH_STEP = 2.0 * M_PI / (double)(1 << RESOLUTION_I16TH);
    constexpr fpn_t FTH_I16TH_INVSTEP = (1.0 / FTH_I16TH_STEP);
    
    constexpr fpn_t FX_XMID = (FX_PA_MIN + FX_PA_MAX) / 2.0;
    constexpr fpn_t FY_YMID = (FY_PA_MIN + FR_STONE_RAD + FY_PA_MAX + FR_STONE_RAD) / 2.0;
    
    constexpr fpn_t FX_I16X0 = FX_XMID - FR_I16R_STEP * (double)I16X_MID;
    constexpr fpn_t FY_I16Y0 = FY_YMID - FR_I16R_STEP * (double)I16Y_MID;
    
    constexpr fpn_t I16RtoFR(uint32_t r)noexcept{
        return (fpn_t)r * FR_I16R_STEP;
    }
    constexpr fpn_t I16XtoFX(uint32_t x)noexcept{
        return (fpn_t)x * FR_I16R_STEP + FX_I16X0;
    }
    constexpr fpn_t I16YtoFY(uint32_t y)noexcept{
        return (fpn_t)y * FR_I16R_STEP + FY_I16Y0;
    }
    
    constexpr uint32_t FRtoI16R(fpn_t f)noexcept{
        return (uint32_t)(f * FR_I16R_INVSTEP);
    }
    constexpr uint32_t FXtoI16X(fpn_t fx)noexcept{
        return uint32_t((int32_t)(I16X_MID) + (int32_t)((fx - FX_XMID) * FR_I16R_INVSTEP));
    }
    constexpr uint32_t FYtoI16Y(fpn_t fy)noexcept{
        return uint32_t((int32_t)(I16Y_MID) + (int32_t)((fy - FY_YMID) * FR_I16R_INVSTEP));
    }
    
    constexpr uint32_t FR2toI16R2(float f2)noexcept{
        return (uint32_t)(f2 * static_cast<float>(FR_I16R_INVSTEP * FR_I16R_INVSTEP));
    }
    constexpr uint32_t FR2toI16R2(double f2)noexcept{
        return (uint32_t)(f2 * static_cast<double>(FR_I16R_INVSTEP * FR_I16R_INVSTEP));
    }
    constexpr uint32_t FR2toI16R2(long double f2)noexcept{
        return (uint32_t)(f2 * static_cast<long double>(FR_I16R_INVSTEP * FR_I16R_INVSTEP));
    }
    constexpr uint32_t FTtoI16T(fpn_t ft)noexcept{
        return I16TH_MIN + (uint32_t)((ft + M_PI) * FTH_I16TH_INVSTEP);
    }
    
    /*************************16ビット離散座標でのルール**************************/
    
    constexpr uint32_t I16X_TEE = FXtoI16X(FX_TEE);
    constexpr uint32_t I16Y_TEE = FYtoI16Y(FY_TEE);
    
    // リンク内
    constexpr bool isOnRink(uint32_t x, uint32_t y)noexcept{
        return (FXtoI16X(FX_RINK_MIN + FR_STONE_RAD) < x && x < FXtoI16X(FX_RINK_MAX - FR_STONE_RAD))
        && (FYtoI16Y(FY_RINK_MIN + FR_STONE_RAD) < y && y < FYtoI16Y(FY_RINK_MAX - FR_STONE_RAD));
    }
    
    // プレーエリア(石が無効にならない範囲)内
    constexpr bool isInPlayArea(uint32_t x, uint32_t y)noexcept{
        return (FXtoI16X(FX_PA_MIN + FR_STONE_RAD) < x && x < FXtoI16X(FX_PA_MAX - FR_STONE_RAD))
        && (FYtoI16Y(FY_PA_MIN + FR_STONE_RAD) < y && y < FYtoI16Y(FY_PA_MAX + FR_STONE_RAD));
    }
    
    // ハウス内
    constexpr bool isInHouse(uint32_t x, uint32_t y)noexcept{
        return (XYtoR2(x - I16X_TEE, y - I16Y_TEE)
                < FR2toI16R2((FR_HOUSE_RAD + FR_STONE_RAD) * (FR_HOUSE_RAD + FR_STONE_RAD)));
    }
    
    // ティーラインの前
    constexpr bool isThisSideOfTeeLine(uint32_t y)noexcept{
        return (y < FYtoI16Y(FY_TEE - FR_STONE_RAD));
    }
    
    // フリーガードゾーン内
    template<int OUT_HOUSE = 0, int IN_PLAYAREA = 0>
    bool isInFreeGuardZone(uint32_t x, uint32_t y){
        if(!IN_PLAYAREA){
            if(!(FXtoI16X(FX_PA_MIN + FR_STONE_RAD) < x
                 && x < FXtoI16X(FX_PA_MAX - FR_STONE_RAD)
                 && FYtoI16Y(FY_PA_MIN + FR_STONE_RAD) < y)){
                return false;
            }
        }else{
            assert(isInPlayArea(x, y));
        }
        if(!isThisSideOfTeeLine(y)){ return false; }
        if(OUT_HOUSE){
            assert(!isInHouse(x, y));
        }else{
            if(isInHouse(x, y)){ return false; }
        }
        return true;
    }
    
    /*************************16ビット離散座標系での盤面情報**************************/
    
    constexpr uint32_t STONE_MASK_X = (1U << 16) - 1U;
    constexpr uint64_t STONE_MASK_COLOR = 1ULL << 32;
    
    struct StoneXY{
        // 離散ストーン位置(直交座標) + 追加情報
        uint64_t st;
        
        constexpr StoneXY() :st(){}
        constexpr StoneXY(uint64_t ast) : st(ast){}
        constexpr StoneXY(const StoneXY& ast) : st(ast.st){}
        
        constexpr operator uint64_t()const noexcept{ return st; }
        
        // 成分取り出し
        constexpr uint32_t getX()const noexcept{ return ((uint32_t)st) & STONE_MASK_X; }
        constexpr uint32_t getY()const noexcept{ return (((uint32_t)st) >> 16) & STONE_MASK_X; }
        constexpr uint32_t getXY()const noexcept{ return (uint32_t)st; }
        constexpr uint32_t getCol()const noexcept{ return (uint32_t)(st >> 32); }
        
        // 性質
        constexpr uint64_t isWhite()const noexcept{ return st & STONE_MASK_COLOR; }
        constexpr bool isBlack()const noexcept{ return !(st & STONE_MASK_COLOR); }
        
        // 分割
        /*template<int N>StoneXY div(int n); // N分割のn番目を計算する
         
         template<>Move div<4>(int n){ // 4分割
         assert(0 <= n && n < 4);
         assert(getRange() > 0);
         
         uint32_t r = 1U << getRange();
         uint32_t half_r = r >> 1; // 範囲の半分
         
         uint64_t tmp = mv;
         switch(n){
         case 0: tmp += half_r | (half_r << 16);
         case 1: tmp -= (half_r << 16) - half_r;
         case 2: tmp += (half_r << 16) - half_r;
         case 3: tmp -= half_r | (half_r << 16);
         default: assert(0);
         }
         tmp -= (1ULL << 32); // rangeを1減らす
         
         return Move(tmp);
         }*/
    };
    
    struct StoneRT{
        // 離散ストーン位置(極座標)
        uint64_t st;
    };
    
    /**************************16ビット離散座標系での着手表現**************************/
    
    constexpr int RESOLUTION_I16VX = 15;
    constexpr int RESOLUTION_I16VY = 15;
    constexpr int RESOLUTION_I16VRANGE = 15;
    constexpr int RESOLUTION_SPIN = 1;
    
    constexpr uint32_t I16VX_MIN = 0;
    constexpr uint32_t I16VY_MIN = 0;
    constexpr uint32_t I16VX_MAX = (1U << RESOLUTION_I16VX) - 1U;
    constexpr uint32_t I16VY_MAX = (1U << RESOLUTION_I16VY) - 1U;
    constexpr uint32_t I16VX_MID = 1U << (RESOLUTION_I16VX - 1);
    constexpr uint32_t I16VY_MID = 1U << (RESOLUTION_I16VY - 1);
    
    //constexpr uint32_t V_RANGE_MAX = (1U << RESOLUTION_VRANGE);
    
    constexpr int MOVE_LCT_X = 0;
    constexpr int MOVE_LCT_Y = 16;
    constexpr int MOVE_LCT_SPIN = 32;
    constexpr int MOVE_LCT_RELATIVITY = 34;
    constexpr int MOVE_LCT_CONTACT = 35;
    constexpr int MOVE_LCT_RANGE = 36;
    constexpr int MOVE_LCT_NUM0 = 40;
    constexpr int MOVE_LCT_NUM1 = 44;
    constexpr int MOVE_LCT_NUM2 = 48;
    constexpr int MOVE_LCT_TYPE = 56;
    
    constexpr uint32_t MOVE_MASK_X = (1U << 16) - 1U;
    constexpr uint32_t MOVE_MASK_Y = MOVE_MASK_X << MOVE_LCT_Y;
    constexpr uint64_t MOVE_MASK_RANGE = 15ULL << MOVE_LCT_RANGE;
    constexpr uint64_t MOVE_MASK_SPIN = 1ULL << MOVE_LCT_SPIN;
    constexpr uint64_t MOVE_MASK_RELATIVITY = 1ULL << MOVE_LCT_RELATIVITY;
    constexpr uint64_t MOVE_MASK_CONTACT = 1ULL << MOVE_LCT_CONTACT;
    constexpr uint64_t MOVE_MASK_NUM0 = 15ULL << MOVE_LCT_NUM0;
    constexpr uint64_t MOVE_MASK_NUM1 = 15ULL << MOVE_LCT_NUM1;
    constexpr uint64_t MOVE_MASK_NUM2 = 255ULL << MOVE_LCT_NUM2;
    constexpr uint64_t MOVE_MASK_TYPE = 255ULL << MOVE_LCT_TYPE;
    
    constexpr uint64_t MOVE_PASS = MOVE_MASK_RELATIVITY;
    constexpr uint64_t MOVE_ILLEGAL = uint64_t(-1);
    
    struct MoveXY{
        // レンジなし、あり両用の離散着手型(直交座標)
        //  0 - 15 X
        // 16 - 31 Y
        // 32 spin
        // 34 relativity
        // 35 contact
        // 36 - 39 range
        // 40 - 43 num0
        // 44 - 47 num1
        // 48 - 55 num2
        // 56 - 63 type
        
        uint64_t mv;
        
        constexpr MoveXY() :mv(){}
        constexpr MoveXY(uint64_t amv) : mv(amv){}
        constexpr MoveXY(const MoveXY& amv) : mv(amv.mv){}
        
        constexpr operator uint64_t()const noexcept{ return mv; }
        
        constexpr uint64_t data()const noexcept{ return mv; }
        uint64_t& data()noexcept{ return mv; }
        
        void setNR(uint32_t ax, uint32_t ay, uint64_t s)noexcept{ // ノーレンジ着手をセット
            mv = ax | (ay << MOVE_LCT_Y) | (s << MOVE_LCT_SPIN);
        }
        void clear()noexcept{ mv = 0ULL; }
        
        // 成分取り出し
        constexpr uint32_t vx()const noexcept{ return (((uint32_t)mv) >> MOVE_LCT_X) & (MOVE_MASK_X >> MOVE_LCT_X); }
        constexpr uint32_t vy()const noexcept{ return (((uint32_t)mv) >> MOVE_LCT_Y) & (MOVE_MASK_Y >> MOVE_LCT_Y); }
        constexpr uint32_t vxy()const noexcept{ return (uint32_t)mv; }
        constexpr Spin getSpin()const noexcept{ return static_cast<Spin>(((uint32_t)(mv >> MOVE_LCT_SPIN)) & 1U); }
        constexpr Spin spin()const noexcept{ return getSpin(); }
        constexpr uint32_t getRange()const noexcept{ return ((uint32_t)(mv >> MOVE_LCT_RANGE)) & 15U; }
        constexpr uint32_t getNum0()const noexcept{ return ((uint32_t)(mv >> MOVE_LCT_NUM0)) & 15U; }
        constexpr uint32_t getNum1()const noexcept{ return ((uint32_t)(mv >> MOVE_LCT_NUM1)) & 15U; }
        constexpr uint32_t getNum2()const noexcept{ return ((uint32_t)(mv >> MOVE_LCT_NUM2)) & 255U; }
        constexpr uint32_t getType()const noexcept{ return ((uint32_t)(mv >> MOVE_LCT_TYPE)); }
        
        constexpr uint32_t getPartVX()const noexcept{ return ((uint32_t)mv) & MOVE_MASK_X; }
        constexpr uint32_t getPartVY()const noexcept{ return ((uint32_t)mv) & MOVE_MASK_Y; }
        
        constexpr uint64_t anyRange()const noexcept{ return mv & MOVE_MASK_RANGE; }
        constexpr uint64_t isLeftSpin()const noexcept{ return mv & MOVE_MASK_SPIN; }
        constexpr uint64_t isRelative()const noexcept{ return mv & MOVE_MASK_RELATIVITY; }
        constexpr uint64_t hasContact()const noexcept{ return mv & MOVE_MASK_CONTACT; }
        
        bool isPASS()const noexcept{ return (mv == MOVE_PASS); }
        bool isILLEGAL()const noexcept{ return (mv == MOVE_ILLEGAL); }
        
        void setSpin(uint64_t s)noexcept{ mv |= (s << MOVE_LCT_SPIN); }
        void setRelativity()noexcept{ mv |= MOVE_MASK_RELATIVITY; }
        void setContact()noexcept{ mv |= MOVE_MASK_CONTACT; }
        void setType(uint64_t at)noexcept{ mv |= (at << MOVE_LCT_TYPE); }
        void setNum0(uint64_t an)noexcept{ mv |= (an << MOVE_LCT_NUM0); }
        void setNum1(uint64_t an)noexcept{ mv |= (an << MOVE_LCT_NUM1); }
        void setNum2(uint64_t an)noexcept{ mv |= (an << MOVE_LCT_NUM2); }
        
        void setWR(Spin as)noexcept{ // 全範囲着手
            mv = (I16VX_MID << MOVE_LCT_X)
            | (I16VY_MID << MOVE_LCT_Y)
            | (((uint64_t)as) << MOVE_LCT_SPIN)
            | ((uint64_t)(RESOLUTION_I16VRANGE) << MOVE_LCT_RANGE);
        }
        // pass
        void setPASS(Spin s = Spin::RIGHT, int n0 = 0, int n1 = 0)noexcept{ // パス
            mv = MOVE_PASS;
        }
        
        // 0 stone
        void setDraw(Spin s, int n0, int n1 = 0)noexcept{
            clear(); setType(Standard::DRAW); setSpin(s);
            setNum0(n0); setRelativity();
        }
        void setPreGuard(Spin s, int n0, int n1 = 0)noexcept{
            clear(); setType(Standard::PREGUARD); setSpin(s);
            setNum0(n0); setRelativity();
        }
        
        // 1 stone
        void setHit(Spin s, int n0, int n1)noexcept{
            clear(); setType(Standard::HIT); setSpin(s);
            setNum0(n0); setNum1(n1); setRelativity(); setContact();
        }
        void setFreeze(Spin s, int n0, int n1)noexcept{
            clear(); setType(Standard::FREEZE); setSpin(s);
            setNum0(n0); setNum1(n1); setRelativity();
        }
        void setPeel(Spin s, int n0, int n1)noexcept{
            clear(); setType(Standard::PEEL); setSpin(s);
            setNum0(n0); setNum1(n1); setRelativity(); setContact();
        }
        void setComeAround(Spin s, int n0, int n1)noexcept{
            clear(); setType(Standard::COMEAROUND); setSpin(s);
            setNum0(n0); setNum1(n1); setRelativity();
        }
        void setPostGuard(Spin s, int n0, int n1)noexcept{
            clear(); setType(Standard::POSTGUARD); setSpin(s);
            setNum0(n0); setNum1(n1); setRelativity();
        }
        void setDrawRaise(Spin s, int n0, int n1 = 0)noexcept{
            clear(); setType(Standard::DRAWRAISE); setSpin(s);
            setNum0(n0); setRelativity(); setContact();
        }
        
        // 2 stones
        void setDouble(Spin s, int n0, int n1)noexcept{
            clear(); setType(Standard::DOUBLE); setSpin(s);
            setNum0(n0); setNum1(n1); setRelativity(); setContact();
        }
        void setRaiseTakeOut(Spin s, int n0, int n1)noexcept{
            clear(); setType(Standard::RAISETAKEOUT); setSpin(s);
            setNum0(n0); setNum1(n1); setRelativity(); setContact();
        }
        
        // all
        void setL1Draw(Spin s, int n0 = 0, int n1 = 0)noexcept{
            clear(); setType(Standard::L1DRAW); setSpin(s);
            setRelativity();
        }
        
        // 1 stone - all
        void setTakeOut(Spin s, int n0, int n1 = 0)noexcept{
            clear(); setType(Standard::TAKEOUT); setSpin(s);
            setNum0(n0); setRelativity(); setContact();
        }
        
        // 未実装
        /*
         
         void setRollIn(int s, int n0, int n1)noexcept{
         clear(); setType(Standard::ROLLIN); setSpin(s);
         setNum0(n0); setNum1(n1); setRelativity(); setContact();
         }
         void setFrontTake(int s, int n0, int n1)noexcept{
         clear(); setType(Standard::FRONTTAKE); setSpin(s);
         setNum0(n0); setNum1(n1); setRelativity(); setContact();
         }
         void setBackTake(int s, int n0, int n1)noexcept{
         clear(); setType(Standard::BACKTAKE); setSpin(s);
         setNum0(n0); setNum1(n1); setRelativity(); setContact();
         }
         void setMiddleGuard(int s, int n0, int n1)noexcept{
         clear(); setType(Standard::MIDDLEGUARD); setSpin(s);
         setNum0(n0); setNum1(n1); setRelativity();
         }
         void setHitAndRoll(int s, int n0, int n1)noexcept{
         clear(); setType(Standard::HITROLL); setSpin(s);
         setNum0(n0); setNum1(n1); setRelativity(); setContact();
         }
         void setDoubleRollIn(int s, int n0, int n1)noexcept{
         clear(); setType(Standard::DOUBLEROLLIN); setSpin(s);
         setNum0(n0); setNum1(n1); setRelativity(); setContact();
         }
         */
        
        //分割
        template<int N>
        MoveXY part(int n)const{
            ASSERT(getRange() > 1, cerr << getRange() << endl;);
            assert(0 <= n && n < N);
            uint64_t tmp = mv;
            
            switch (N){
                case 4:{
                    uint32_t half_r = 1U << (getRange() - 2); // レンジ幅の1/4
                    switch (n){
                        case 0: tmp += half_r | (half_r << MOVE_LCT_Y); break;
                        case 1: tmp -= (half_r << MOVE_LCT_Y) - half_r; break;
                        case 2: tmp += (half_r << MOVE_LCT_Y) - half_r; break;
                        case 3: tmp -= half_r | (half_r << MOVE_LCT_Y); break;
                        default: UNREACHABLE; break;
                    }
                    tmp -= (1ULL << MOVE_LCT_RANGE); // rangeを1減らす
                    break;
                }
                default: UNREACHABLE; break;
            }
            return MoveXY(tmp);
        }
        
        template<class dice_t>
        MoveXY genNR(dice_t *const dice)const{
            ASSERT(getRange() > 0, cerr << getRange() << endl;);
            
            // 乱数を掛けて、範囲0(no range)の着手を1つ返す
            MoveXY ret(mv & (~MOVE_MASK_RANGE));
            //uint32_t range = 1U << (getRange() - 1U);
            //uint32_t josu = (range << 1) + 1U;
            
            uint32_t range = 1U << getRange();
            uint32_t josu = range + 1;
            range >>= 1;
            
            uint32_t r = dice->rand();
            
            ret.mv -= range + (range << 16); // 角の座標に合わせる
            ret.mv += (r & MOVE_MASK_X) % josu;
            ret.mv += ((r >> MOVE_LCT_Y) % josu) << 16;
            
            return ret;
        }
        
        int wherePart(int N, const MoveXY& amv)const{
            // amvの中心がN分割中どの分割に属するか返す
            // どの分割にも属さない場合は、分割の方向を返す
            switch (N){
                case 4:{
                    if (amv.getPartVX() > getPartVX()){
                        if (amv.getPartVY() > getPartVY()){
                            return 0;
                        }
                        else{
                            return 1;
                        }
                    }
                    else{
                        if (amv.getPartVY() > getPartVY()){
                            return 2;
                        }
                        else{
                            return 3;
                        }
                    }break;
                }default: UNREACHABLE; break;
            }
            UNREACHABLE;
        }
        
        void mean(MoveXY amv)noexcept{
            // X, Y をamvとの間で平均化する
            constexpr uint32_t mask = (MOVE_MASK_X >> 1) | ((MOVE_MASK_X >> 1) << MOVE_LCT_Y);
            uint32_t nxy = vxy() + amv.vxy();
            mv = (mv & (~(MOVE_MASK_X | MOVE_MASK_Y))) | ((nxy >> 1) & mask);
        }
        
        // short func
        uint32_t n0()const noexcept{ return getNum0(); }
        uint32_t n1()const noexcept{ return getNum1(); }
    };
    
    std::ostream& operator<<(std::ostream& out, const MoveXY& arg){
        if(!arg.isRelative()){ // 絶対着手
            out << std::hex << arg.data() << std::dec;
            out << "MV< vx:" << arg.vx() << " vy:" << arg.vy()
            << " vr:+-" << ((1U << (arg.getRange())) >> 1) << "(" << arg.getRange()
            << ") s:" << arg.getSpin() << " >";
        }else{ // 相対着手
            out << shotString[arg.getType()];
            if(!arg.isPASS()){
                if(arg.hasContact()){
                    out << "*";
                }
                out << " s:" << arg.spin();
                out << "[" << arg.n0() << "," << arg.n1() << "]";
            }
        }
        return out;
    }
    
    struct MovePolicy : public MoveXY{
        fpn_t size;
        fpn_t eval_sum;
        fpn_t weight;
        fpn_t mu;
        
        fpn_t mean()const{
            return eval_sum / size;
        }
        
        void initUCB()noexcept{
            size = 0; eval_sum = 0; mu = 0;
        }
    };
    
    /**************************着手の連続座標と16ビット離散座標の変換**************************/
    
    
    constexpr fpn_t FV_I16V_STEP = (FV_RANGE_MAX * 2) / (double)(I16VX_MAX - I16VX_MIN + 1);
    constexpr fpn_t FV_I16V_INVSTEP = (1.0 / FV_I16V_STEP);
    
    // 速度演算のため、STEPはx,y同じ幅である必要がある
    
    constexpr fpn_t I16VtoFV(uint32_t i)noexcept{
        return ((fpn_t)i) * FV_I16V_STEP;
    }
    constexpr fpn_t I16VXtoFVX(uint32_t x)noexcept{
        return ((fpn_t)x) * FV_I16V_STEP + FVX_MIN;
    }
    constexpr fpn_t I16VYtoFVY(uint32_t y)noexcept{
        return ((fpn_t)y) * FV_I16V_STEP + FVY_MIN;
    }
    
    template<typename float_t>
    constexpr uint32_t FVtoI16V(float_t f)noexcept{
        return (uint32_t)(f * FV_I16V_INVSTEP);
    }
    template<typename float_t>
    constexpr uint32_t FVXtoI16VX(float_t fx)noexcept{
        return I16VX_MIN + (uint32_t)((fx - FVX_MIN) * FV_I16V_INVSTEP);
    }
    template<typename float_t>
    constexpr uint32_t FVYtoI16VY(float_t fy)noexcept{
        return I16VY_MIN + (uint32_t)((fy - FVY_MIN) * FV_I16V_INVSTEP);
    }
    template<typename float_t>
    constexpr uint32_t FV2toI16V2(float_t f2)noexcept{
        return (uint32_t)(f2 * static_cast<float_t>(FV_I16V_INVSTEP * FV_I16V_INVSTEP));
    }
    
    template<class movexy_t>
    void convMove(const MoveXY& arg, movexy_t *const dst)noexcept{
        dst->x = I16VXtoFVX(arg.vx());
        dst->y = I16VYtoFVY(arg.vy());
        dst->s = arg.getSpin();
    }
    
    template<typename movexy_t>
    void convMove(const movexy_t& arg, MoveXY *const dst)noexcept{
        dst->mv = (uint64_t)((FVXtoI16VX(arg.vx()) << MOVE_LCT_X) | (FVYtoI16VY(arg.vy()) << MOVE_LCT_Y))
        | (((uint64_t)arg.getSpin()) << MOVE_LCT_SPIN);
    }
    
    template<typename float_t>
    void convRMove(const MoveXY& arg, fRMoveXY<float_t> *const dst)noexcept{
        dst->x = I16VXtoFVX(arg.vx());
        dst->y = I16VYtoFVY(arg.vy());
        dst->rx = I16VtoFV((1U << arg.getRange()) >> 1);
        dst->ry = I16VtoFV((1U << arg.getRange()) >> 1);
        dst->s = arg.getSpin();
    }
    
    template<typename float_t>
    void convRMove(const fRMoveXY<float_t>& arg, MoveXY *const dst)noexcept{
        dst->mv = (uint64_t)((FVXtoI16VX(arg.vx()) << MOVE_LCT_X) | (FVYtoI16VY(arg.vy()) << MOVE_LCT_Y))
        | (((uint64_t)(bsr64((uint64_t)FVtoI16V(arg.getRangeVX())))) << MOVE_LCT_RANGE) | (((uint64_t)arg.getSpin()) << MOVE_LCT_SPIN);
    }
    
    /**************************盤上のオーダー**************************/
    
    using OrderBits = BitSet32;
    
    int countScore(OrderBits o)noexcept{
        if (!o.data()){
            return 0;
        }
        else if (o.data() & 1){ // black scored
            return +bsf(o.data() + 1);
        }
        else{ // white scored
            return -bsf(o.data());
        }
        UNREACHABLE;
    }
    int count2ndScore(OrderBits o, Color c)noexcept{
        // 該当の色の中でNo1の石が無かった場合の点
        if (!o.data()){
            return 0;
        }else{
            OrderBits to = o;
            if(!isBlack(c)){ o.flip(); }
            to.remove(to.bsf());
            return countScore(to);
        }
        UNREACHABLE;
    }
    
    /*int countScore(OrderBits o, Color c)noexcept{
        int sc = countScore(o);
        if(isWhite(c)){
            sc = -sc;
        }
        return sc;
    }*/
    
    OrderBits getFlippedPosition(OrderBits o)noexcept{
        int i = o; // 最上位ビットを考えsignedにする
        return OrderBits(((i >> 1) ^ o) << 1);
    }
    
    OrderBits maskUpper(OrderBits o, int n){
        // 色の反転回数が上の方は得点に関わる可能性が低いので、隠す
        OrderBits fp = getFlippedPosition(o);
        //cerr << "fp = " << fp << endl;
        uint32_t b = NthLowestBit(fp, n + 1);
        if(b){
            //cerr << "lbi = " << bsf(b) << endl;
            const uint32_t color = (-((o >> (bsf(b) - 1)) & 1));
            const uint32_t mask = b - 1; // iより下の部分のマスク
            //cerr << "clr = " << color << endl;
            //cerr << "msk = " << mask << endl;
            return OrderBits((o & mask) | (color & (~mask)));
        }else{
            return o;
        }
    }
    
    /**************************ハッシュテーブル**************************/
    
    // 位置は手番ごとに、2次元の位置要素にそれぞれにハッシュ値を与える
    // 同じ色の石を区別するか? 絶対石番以外のノード内一意性のある
    // 順序を使用して手を設定するならば区別しなくてよい
    
    enum HashPosVariable{
        XY = 0,
        RT = 1,
    };
    
    //uint64_t hash_table_pos[2][2][16][2];
    uint64_t hash_table_pos[N_STONES][2][16][2];
    
    constexpr bool examAB(int ab)noexcept{ return (ab == 0 || ab == 1); }
    
    // 中央からの石順序
    //uint64_t hash_table_order[2][16];
    
    // 絶対着手は(vx,vy,回転)の3次元
    // それにvx,vyの範囲を含めた4次元のそれぞれにハッシュ値を与える
    // 回転は現在2値なので、ハッシュ値も0と1とする
    
    uint64_t hash_table_v[2][16][2];
    
    //uint64_t hash_table_s[2];
    std::array<uint64_t, 16> hash_table_vr;
    
    
    /**************************石の位置ハッシュ関数**************************/
    
    uint64_t genHash_Pos(uint32_t idx, int ab, uint32_t val, int r = 0){
        assert(examIndex(idx));
        assert(examAB(ab));
        idx = getColor(idx);
        //if(ab){ r += 1; } // Y座標の認識精度を下げる
        if(r < 16){
            val >>= r;
            uint64_t ret = hash_table_pos[idx][ab][r][val & 1U];
            for (int i = r + 1; i < 16; ++i){
                val >>= 1;
                //ret ^= hash_table_pos[idx][ab][i][val & 1U];
                ret += hash_table_pos[idx][ab][i][val & 1U];
            }
            return ret;
        }else{
            return 0ULL;
        }
    }
    
    /*void addHash_Pos(uint64_t *const dst, uint32_t idx, uint32_t a, uint32_t b, int rmin = 0, int rmax = 15, int rbase = 0){
        // 1個の石について、広いレンジのハッシュ値をまとめて加算
        assert(examIndex(idx));
        assert(0 <= rmin && rmax < 16 && rmin <= rmax);
        int i = rmin;
        a >>= (rmin + rbase); b >>= (rmin + rbase);
        // rmax以下の分はそこまで全部をxorする
        for (int i = rmin + rbase; i <= rmax; ++i){
            uint64_t hd = hash_table_pos[idx][0][i][a & 1U] ^ hash_table_pos[idx][1][i][b & 1U];
            int j = rmin;
            do{
                dst[j] ^= hd;
            } while (j++ < i);
            a >>= 1; b >>= 1;
        }
        // rmax以上の分はrmaxまでをxorする
        for (int i = max(rmin + rbase + 1, rmax + 1); i < 16; ++i){
            uint64_t hd = hash_table_pos[idx][0][i][a & 1U] ^ hash_table_pos[idx][1][i][b & 1U];
            int j = rmin;
            do{
                dst[j] ^= hd;
            } while (j++ < rmax);
            a >>= 1; b >>= 1;
        }
    }*/
    
    
    /**************************石のオーダーハッシュ関数**************************/
    
    
    uint64_t genHash_OrderBits(OrderBits o)noexcept{
        //cerr << o << " -> ";
        OrderBits oo = o; //maskUpper(o, 3);
        //cerr << oo << endl; if(o != oo){ getchar(); }
        return ((uint64_t(oo & (0x0FFFU))) << 17);
    }
    
    /**************************着手成分ハッシュ関数**************************/
    
    uint64_t genHash_V(int ab, uint32_t a){
        assert(examAB(ab));
        uint64_t ret = hash_table_v[ab][0][a & 1U];
        for (int i = 1; i < 16; ++i){
            a >>= 1;
            ret ^= hash_table_v[ab][i][a & 1U];
        }
        return ret;
    }
    
    uint64_t genHash_VRange(uint32_t r){
        return hash_table_vr[r];
    }
    constexpr uint32_t genHash_S(uint32_t s)noexcept{
        return (s << 16);
    }
        
    void subtrHash_VRange(uint64_t *const dst, int org_range){
        //レンジを狭くする
        *dst ^= hash_table_vr[org_range];
        *dst ^= hash_table_vr[org_range - 1];
    }
    
    uint64_t genHash_VAB(uint32_t val){
        // 下半分がa、上半分がbとする形
        uint64_t ret = hash_table_v[0][0][val & 1U];
        for (int i = 1; i < 16; ++i){
            val >>= 1;
            ret ^= hash_table_v[0][i][val & 1U];
        }
        for (int i = 0; i < 16; ++i){
            val >>= 1;
            ret ^= hash_table_v[1][i][val & 1U];
        }
        return ret;
    }
    
    /**************************着手ハッシュ関数**************************/
    
    uint64_t genHash_AbsoluteMove(const MoveXY mv){
        //CERR << mv.vxy() << " , " << mv.getSpin() << " , " << mv.getRange() << " -> ";
        //CERR << (genHash_VAB(mv.vxy()) ^ genHash_S(mv.getSpin()) ^ genHash_VRange(mv.getRange())) << endl;
        return genHash_VAB(mv.vxy()) ^ genHash_S(mv.getSpin()) ^ genHash_VRange(mv.getRange());
    }
    
    constexpr uint64_t genHash_RelativeMove(const MoveXY mv)noexcept{
        return ((uint64_t)mv) & (MOVE_MASK_TYPE | MOVE_MASK_NUM0 | MOVE_MASK_NUM1 | MOVE_MASK_SPIN) & (~MOVE_MASK_RANGE);
    }
    
    void initHash(){
        // ハッシュ関連初期化
        
        const uint32_t seed = 0x0e4a0fe9;
        XorShift64 dice;
        
        dice.srand(seed);
        
        for (int idx = 0; idx < N_STONES; ++idx){
            for (int ab = 0; ab < 2; ++ab){
                for (int i = 0; i < 16; ++i){
                    for (int bit = 0; bit < 2; ++bit){
                        hash_table_pos[idx][ab][i][bit] = dice.rand();
                    }
                }
            }
        }
        for (int ab = 0; ab < 2; ++ab){
            for (int i = 0; i < 16; ++i){
                for (int bit = 0; bit < 2; ++bit){
                    hash_table_v[ab][i][bit] = dice.rand();
                }
            }
        }
        for (int i = 0; i < 16; ++i){
            hash_table_vr[i] = dice.rand();
        }
    }
    
    /**************************解析用ショットログ**************************/
    
    struct AyumuShotLog : public ShotLog{
        MoveXY vmv;
        AyumuShotLog() : ShotLog(){}
        AyumuShotLog(const ShotLog& a) : ShotLog(a){}
    };
    
    /**************************渦巻きビットボート**************************/
    
    using VortexBitBoard = BitSet64;
    
    constexpr fpn_t VItoR(int vi)noexcept{ return (FR_IN_HOUSE - 3 * FR_STONE_RAD) / (64 - 1) * vi; }
    constexpr fpn_t VItoRawTh(int vi)noexcept{ return 6 * (2 * M_PI) / (64 - 1) * vi; }
    
    constexpr int RtoVI(fpn_t r)noexcept{ return cmin(63, int(r * ((64 - 1) / (FR_IN_HOUSE - 3 * FR_STONE_RAD)))); }
    
    template<class pos_t>
    inline void genVortexDrawPos(pos_t *const ppos, int vi){
        ASSERT(0 <= vi && vi < 64,);
        ppos->setPCS(VItoR(vi), VItoRawTh(vi)); // 極座標を指定して設定
    }
    
    // フリードロー出来るゾーンのテーブルサイズ
    constexpr int GVBBTABLE_WIDTH = 128;
    constexpr int GVBBTABLE_LENGTH = 128;
    
    // フリードロー出来る確率
    double guardProbTable[GVBBTABLE_WIDTH + 1][GVBBTABLE_LENGTH + 1][64];
    // フリードロー出来るゾーン
    VortexBitBoard guardVBBTable[GVBBTABLE_WIDTH][GVBBTABLE_LENGTH][4];

    int initGuardProb(const std::string& path){
        std::ifstream ifs(path + "guard_prob.dat");
        if(!ifs){ cerr << "failed to import guard_prob.dat!" << endl; return -1; }
        for(int w = 0; w <= GVBBTABLE_WIDTH; ++w){
            for(int l = 0; l <= GVBBTABLE_LENGTH; ++l){
                for(int i = 0; i < 64; ++i){
                    ifs >> guardProbTable[w][l][i];
                }
            }
        }
        return 0;
    }
    
    int initGuardVBB(){
        // 周囲の全点のヒット確率最大値が一定以下ならば採択
        for(int l = 0; l < GVBBTABLE_LENGTH; ++l){
            for(int w = 0; w < GVBBTABLE_WIDTH; ++w){
                BitSet64 bs50(0), bs75(0), bs95(0), bs99(0);
                for(int i = 0; i < 64; ++i){
                    double maxHitProb = max(max(guardProbTable[w][l][i],
                                                guardProbTable[w][l + 1][i]),
                                            max(guardProbTable[w + 1][l][i],
                                                guardProbTable[w + 1][l + 1][i]));
                    if(maxHitProb < 0.5){
                        bs50.set(i);
                    }
                    if(maxHitProb < 0.25){
                        bs75.set(i);
                    }
                    if(maxHitProb < 0.05){
                        bs95.set(i);
                    }
                    if(maxHitProb < 0.01){
                        bs99.set(i);
                    }
                }
                guardVBBTable[w][l][0] = bs99.data();
                guardVBBTable[w][l][1] = bs95.data();
                guardVBBTable[w][l][2] = bs75.data();
                guardVBBTable[w][l][3] = bs50.data();
                //cerr << guardVBBTable99[w][l] << endl;
            }
        }
        return 0;
    }
    
    /**************************データテーブル**************************/
    
    fPosXY<fpn_t> drawTable[2][DrawPos::MAX]; // 標準ドロー位置
    
    // ここにはないが、各ファイルに含まれるテーブルで初期化、インポート必要なものリスト
    // 高速シミュレータテーブル(simulation/fastSimulatorFunc.hpp)
    // ダブルテイクテーブル(ayumu/shot/double.hpp)
    // 得点統計テーブル(ayumu/eval/estimator.hpp)
    // 着手方策重みテーブル(ayumu.hpp)
    // 評価関数テーブル(ayumu.hpp)
    // ドロー位置の情報テーブル(ayumu/structure/stoneInfo.hpp)
    // 石に当たるかの角度テーブル
    
    /**************************スレッドの持ち物**************************/
    
    struct ThreadTools{
        // 各スレッドの持ち物
        using dice64_t = XorShift64;
        using ddice_t = DSFMT;
        
        // スレッド番号
        int threadId;
        
        // サイコロ
        dice64_t dice; // 64ビット整数サイコロ
        ddice_t ddice; // 64ビット浮動小数サイコロ
        
        ThreadTools(){}
    };
    
    /**************************設定ファイルの読み込み**************************/
    
    std::string DIRECTORY_PARAMS_IN("");
    std::string DIRECTORY_PARAMS_OUT("");
    std::string DIRECTORY_DCLOG("");
    std::string DIRECTORY_SHOTLOG("");
    std::string DIRECTORY_IMAGELOG("");
    
    int readConfigFile(){
#if defined(VERSION)
        const std::string configFileName = "config" + MY_VERSION + ".txt";
#else
        const std::string configFileName = "config.txt";
#endif
        std::ifstream ifs;
        
        ifs.open(configFileName);
        
        if(!ifs){
            cerr << "failed to open config file." << endl; return -1;
        }
        ifs >> DIRECTORY_PARAMS_IN;
        ifs >> DIRECTORY_PARAMS_OUT;
        ifs >> DIRECTORY_DCLOG;
        ifs >> DIRECTORY_SHOTLOG;
        ifs >> DIRECTORY_IMAGELOG;
        
        ifs.close();
        return 0;
    }
    
    struct ConfigReader{
        ConfigReader(){
            readConfigFile();
        }
    };
    
    ConfigReader configReader;
}

#endif // DCURLING_AYUMU_DC_HPP_