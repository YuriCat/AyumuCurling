/*
 dc.hpp
 Katsuki Ohto
 */

// デジタルカーリング
// 基本的な定数とデータ構造

#ifndef DCURLING_DC_HPP_
#define DCURLING_DC_HPP_

#include <cstdio>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <errno.h>
#include <random>
#include <bitset>
#include <utility>
#include <tuple>
#include <vector>
#include <array>
#include <valarray>

#include <sys/time.h>

#include <thread>
#include <mutex>
#include <atomic>

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#else

#include <sys/param.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <netinet/in.h>
#include <netdb.h>

#endif

#include "settings.h"

#include "../CppCommon/src/common_all.h"

#include "../../../lib/dSFMT-src-2.2.3/dSFMT.h"
#include "../../../lib/dSFMT-src-2.2.3/dSFMT.c"

// サイコロの定義(浮動小数点乱数の生成が高速なはず)
struct DSFMT{
    dsfmt_t dsfmt;
    uint32_t rand(){ // 整数
        //static AtomicCounter c("DSFMT::rand()");
        //c += 1;
        return dsfmt_genrand_uint32(&dsfmt);
    }
    double drand(){
        //static AtomicCounter c("DSFMT::drand()");
        //c += 1;
        return dsfmt_genrand_open_open(&dsfmt);
    }
    void srand(uint32_t s){
        dsfmt_init_gen_rand(&dsfmt, s);
    }
    
    DSFMT(uint32_t s) :dsfmt(){ dsfmt_init_gen_rand(&dsfmt, s); }
    DSFMT() :dsfmt(){}
};

using std::swap;
using std::size_t;

namespace DigitalCurling{

    /**************************先攻後攻**************************/

    // エンド内の先攻後攻
    // turnColor = turn % 2 とでき、
    // さらに静止時の盤上に0番の石が存在しないようにできるため、先手 1 後手 0 としている
    enum Color : int{
        COLOR_NONE = -1,
        BLACK = 1,
        WHITE = 0,
    };

    constexpr Color flipColor(Color c)noexcept{
        return static_cast<Color>(BLACK + WHITE - c);
    }
    constexpr bool examColor(Color c)noexcept{
        return (c == BLACK || c == WHITE);
    }
    
    // 黒白判定
    // 黒白が入れ替わっても 0 との比較になるようにこちらの関数を使う
    constexpr int isBlack(Color c)noexcept{ return static_cast<int>(c); }
    constexpr bool isWhite(Color c)noexcept{ return (!c); }

    const char *colorChar = "WB";
    const char *colorString[2] = { "WHITE", "BLACK" };
    
    char toColorChar(int c)noexcept{
        if(examColor(static_cast<Color>(c))){
            return colorChar[c];
        }
        return '*';
    }
    std::string toColorString(int c)noexcept{
        if(examColor(static_cast<Color>(c))){
            return colorString[c];
        }
        return "NONE";
    }
    
    /**************************エンド**************************/

    // 最終エンドを0とする減数型
    enum{
        END_LAST = 0, // 最終エンド
        N_ENDS = 10, // 延長除く最大エンド数
        END_JOBAN = 5, // このエンドまでを前半と捉える
    };
    // 延長戦は全て「延長ありの最終エンド」として扱えばよい
    // (延長戦の時間制限のかかり方がエンドごと独立でない場合、時間配分を考えるために延長戦エンド数情報が必要)

    /**************************エンド内のターン**************************/

    // 先攻後攻含めて 15 ~ 0 で表現
    constexpr int N_TURNS = 16;
    
    constexpr int TURN_LAST = 0; // ラストトーン
    constexpr int TURN_BEFORE_LAST = TURN_LAST + 1;
    constexpr int TURN_FIRST = N_TURNS - 1; // 1手目
    
    constexpr int TURN_BLACK_LAST = TURN_BEFORE_LAST; // 黒(先手)の最終ターン
    constexpr int TURN_WHITE_LAST = TURN_LAST; // 白(後手)の最終ターン
    
    constexpr int TURN_BLACK_FIRST = TURN_FIRST; // 黒(先手)の第1手ターン
    constexpr int TURN_WHITE_FIRST = TURN_FIRST - 1; // 白(後手)の第1手ターン
    
    constexpr int TURN_FREEGUARD = TURN_FIRST - 4 + 1; // このターンまでフリーガードルール適用

    constexpr bool examTurn(int t)noexcept{ return (TURN_LAST <= t && t <= TURN_FIRST); }

    constexpr Color toTurnColor(uint32_t t)noexcept{
        return static_cast<Color>(t & 1);
    }

    constexpr bool isFreeGuardTurn(int t)noexcept{ // フリーガードルール適用ターンかどうか
        return (t >= TURN_FREEGUARD);
    }

    /**************************石、石数**************************/
    
    // 石の番号は、その石が投げられるターンと同一としている
    // もし石の性質が全て一緒ではなくなる場合には、そうならなくなるので注意
    constexpr int N_STONES = 16;
    constexpr int N_COLOR_STONES = 8;
    
    // 先手後手合わせた石番(0 ~ N_STONES - 1)から手番、色内石番(0 ~ N_COLOR_STONES - 1)への変換
    constexpr Color getColor(uint32_t id)noexcept{
        return static_cast<Color>(id & 1);
    }
    constexpr uint32_t getIndexInColor(uint32_t id)noexcept{ return (id >> 1); }
    
    constexpr bool examIndex(uint32_t id)noexcept{
        return (0 <= id && id < N_STONES);
    }
    constexpr bool examIndexInColor(uint32_t id)noexcept{
        return (0 <= id && id < N_COLOR_STONES);
    }
    
    // 手番、石番から先手後手合わせた石番(色判別可能)への変換
    constexpr uint32_t CItoTurn(Color c, uint32_t idx)noexcept{
        return (idx << 1) | c;
    }
    
    // 石番から色が同じか調べる
    constexpr bool isSameColorIndex(uint32_t id0, uint32_t id1)noexcept{
        return !((id0 + id1) & 1);
    }
    
    constexpr int getEndRemStones(Color c, int t)noexcept{ // c がこのエンド中投げられる石数
        return (t - c + 2) / 2;
    }
    constexpr int getGameRemStones(Color c, int e, int t)noexcept{ // c がこの試合中投げられる石数
        return e * N_COLOR_STONES + getEndRemStones(c, t);
    }
    
    /**************************得点**************************/
    
    constexpr int SCORE_MIN = -N_COLOR_STONES;
    constexpr int SCORE_MAX = +N_COLOR_STONES;
    
    // 得点の幅
    constexpr int SCORE_LENGTH = SCORE_MAX - SCORE_MIN + 1;

    // 得点インデックスからエンド得点を出す
    constexpr int IDXtoS(int i)noexcept{ return i + SCORE_MIN; }
    
    // エンド得点から得点インデックスを出す
    constexpr int StoIDX(int s)noexcept{ return s - SCORE_MIN; }

    /**************************制限時間**************************/

    constexpr uint64_t TIME_LIMIT_UNLIMITED = 0xffffffffffffffff;

    struct TimeLimit{
        uint64_t limit_ms;
        uint64_t byoyomi_ms;

        void set(uint64_t aleft, uint64_t aby = 0ULL)noexcept{
            limit_ms = aleft;
            byoyomi_ms = aby;
        }
        void subtr(uint64_t aused)noexcept{
            limit_ms -= aused;
        }
        void add(uint64_t aadded)noexcept{
            limit_ms += aadded;
        }
        bool isUnlimited()const noexcept{
            // 時間無制限
            return (limit_ms == TIME_LIMIT_UNLIMITED);
        }
    };

    /**************************連続座標系でのルール(公式)**************************/

    // 公式が使用している座標系そのまま
    // 現実のカーリングでの 1 メートルが 1 に相当
    // 歩の諸々の判定関数はこの系に対応していないので、座標変換してから使用すること

    constexpr fpn_t FX_OFFICIAL_TEE = 2.375f;
    constexpr fpn_t FY_OFFICIAL_TEE = 4.88f;


    /**************************連続座標系でのルール(自分)**************************/

    // 自分の思考用に座標変換したもの
    // 石を投げる方向のY座標が+となって欲しいこと、
    // ティーの座標(投げる点の座標の方が良いかも)を(0, 0)とするために
    // 座標変換している
    // 座標を変換すると物理計算内の判定関数も符号が逆転したりする場合があるので注意

    // 石の半径
    constexpr fpn_t FR_STONE_RAD = 0.145f;
    constexpr fpn_t FR2_STONE_RAD = FR_STONE_RAD * FR_STONE_RAD;
    constexpr fpn_t FR2_STONE_CONTACT = 4 * FR2_STONE_RAD;
    
    // ハウスの半径
    constexpr fpn_t FR_HOUSE_RAD = 1.83f;

    // ティー(ハウス中心)の座標
    constexpr fpn_t FX_TEE = 0;
    constexpr fpn_t FY_TEE = 0;

    // プレーエリアの縦横
    constexpr fpn_t FR_PA_WIDTH = 4.75f;
    constexpr fpn_t FR_PA_LENGTH = 8.23f;

    // プレーエリアの座標
    constexpr fpn_t FX_PA_MAX = FX_TEE + FR_PA_WIDTH / 2;
    constexpr fpn_t FY_PA_MAX = FY_TEE + FR_HOUSE_RAD;
    constexpr fpn_t FX_PA_MIN = FX_PA_MAX - FR_PA_WIDTH;
    constexpr fpn_t FY_PA_MIN = FY_PA_MAX - FR_PA_LENGTH;

    // 線座標
    constexpr fpn_t FY_HOGLINE = FY_PA_MIN;

    // スロー位置
    constexpr fpn_t FX_THROW = FX_TEE;
    constexpr fpn_t FY_THROW = FY_HOGLINE - 30.0f;

    // リンクの縦横
    constexpr fpn_t FR_RINK_WIDTH = FR_PA_WIDTH;
    //constexpr fpn_t F_RINK_LENGTH = F_PA_LENGTH;

    // リンクの端の座標(公式のものと定義が異なるので注意)
    constexpr fpn_t FX_RINK_MIN = FX_PA_MIN;
    constexpr fpn_t FY_RINK_MIN = FY_THROW - FR_STONE_RAD * 2; // 後ろに投げる事は無いだろう
    constexpr fpn_t FX_RINK_MAX = FX_PA_MAX;
    constexpr fpn_t FY_RINK_MAX = FY_PA_MAX + FR_STONE_RAD * 2;


    // ハウス中心の極座標系
    constexpr fpn_t FR_TEE = 0;

    // 石とリンクの動摩擦
    constexpr fpn_t F_FRIC_RINK_STONE = 12.009216f; // ( = g * mu )
    constexpr fpn_t F_STD_GRAV = 9.80665; // 重力加速度 g
    constexpr fpn_t F_DCOF = F_FRIC_RINK_STONE / F_STD_GRAV; // 動摩擦係数 mu

    // 石同士の摩擦係数(公式の設定ミスで石とリンクの摩擦の値と一緒になってしまったと思われる)
    constexpr fpn_t F_FRIC_STONES = 12.009216f;

    // 石同士の反発係数
    constexpr fpn_t F_REST_STONES = 1.0f; // 完全弾性衝突

    // 石の密度(box2dでは 0 にすると質量 0 になって固定物体になる)
    constexpr fpn_t F_DENS_STONE = 10.0f;

    // 石の質量
    constexpr fpn_t F_MASS_STONE = M_PI * FR_STONE_RAD * FR_STONE_RAD * F_DENS_STONE;

    // 石の到達位置のX,Y座標にかかる正規乱数の標準偏差
    // 公式の定義ではティー中心にドローする着手の(X, Y)座標にかかる
    constexpr fpn_t F_ERROR_SIGMA = 0.145f;
    
#ifdef RULE_ERROR_GAT
    // 2016年GAT杯にて採用されたルール
    constexpr fpn_t F_ERROR_SCALE_X = 0.5;
    constexpr fpn_t F_ERROR_SCALE_Y = 2.0;
#else
    // 第1回UEC杯からのルール
    constexpr fpn_t F_ERROR_SCALE_X = 1;
    constexpr fpn_t F_ERROR_SCALE_Y = 1;
#endif
    
    // (x, y)のスケール
    constexpr fpn_t FX_ERROR_SIGMA = F_ERROR_SIGMA * F_ERROR_SCALE_X;
    constexpr fpn_t FY_ERROR_SIGMA = F_ERROR_SIGMA * F_ERROR_SCALE_Y;
    
    // (dVx, dVx)のスケールは大体(Vx, Vy)に比例するオーダー(石の速さ|V|が相対的に大きいため)
    constexpr fpn_t F_ERROR_SCALE_VX = F_ERROR_SCALE_X;
    constexpr fpn_t F_ERROR_SCALE_VY = F_ERROR_SCALE_Y;
    
    // 歩内の計算では(Vx, Vy)系で正規分布であるとして誤差計算を行う
    // ただし実際には直線運動でないためゆがんだ形をしているし標準偏差も少し違う
    constexpr fpn_t FVX_ERROR_SIGMA = 0.117659 * F_ERROR_SCALE_VX;
    constexpr fpn_t FVY_ERROR_SIGMA = 0.0590006 * F_ERROR_SCALE_VY;

    // 石の初期回転角速度(rad per sec)
    constexpr fpn_t FANGV_ORG = 0.066696f;
    
    // ルール上の最大Vy、指定着手(外乱がかかる前)の速度が
    // この値をオーバーしていればパス扱いになる(2016年GAT杯で追加されたルール)
    constexpr fpn_t FVY_LEGAL_MAX = 33.7149;
    
    // ハウス内と判定される距離
    constexpr fpn_t FR_IN_HOUSE = FR_HOUSE_RAD + FR_STONE_RAD;
    constexpr fpn_t FR2_IN_HOUSE = FR_IN_HOUSE * FR_IN_HOUSE;

    // リンク内
    template<typename float_t>
    bool isOnRink(float_t x, float_t y){
        return (static_cast<float_t>(FX_RINK_MIN + FR_STONE_RAD) < x
            && x < static_cast<float_t>(FX_RINK_MAX - FR_STONE_RAD))
            && (static_cast<float_t>(FY_RINK_MIN + FR_STONE_RAD) < y
            && y < static_cast<float_t>(FY_RINK_MAX - FR_STONE_RAD));
    }
    // プレーエリア(静止した石が無効にならない範囲)内
    // TODO: 実際のカーリングのルールではプレーエリアより手前側のストーンでも
    // プレーエリア内のストーンに接触した場合には有効になる
    // デジタルカーリング公式ではまだ実装されていない
    template<typename float_t>
    bool isInPlayArea(float_t x, float_t y){
        return (static_cast<float_t>(FX_PA_MIN + FR_STONE_RAD) < x
            && x < static_cast<float_t>(FX_PA_MAX - FR_STONE_RAD))
            && (static_cast<float_t>(FY_PA_MIN + FR_STONE_RAD) < y
            && y < static_cast<float_t>(FY_PA_MAX + FR_STONE_RAD));
    }
    // ハウス内 石の一部でもハウスにかかっていればハウス内と判定
    template<typename float_t>
    bool isInHouse(float_t x, float_t y)noexcept{
        return (XYtoR2(x - FX_TEE, y - FY_TEE) < static_cast<float_t>(FR2_IN_HOUSE));
    }

    // ティーラインより手前側(かかっていない)
    template<typename float_t>
    bool isThisSideOfTeeLine(float_t y)noexcept{
        return (y < static_cast<float_t>(FY_TEE - FR_STONE_RAD));
    }

    // 石の中心がティーの近傍|dist|内にある
    template<typename float_t>
    bool isNearTee(float_t x, float_t y, float_t dist)noexcept{
        return (XYtoR2(x - FX_TEE, y - FY_TEE) < dist * dist);
    }

    // フリーガードゾーン内
    // 石がティーラインより完全に手前側にあり、プレーエリア内かつハウス外
    // ハウス外であることが確定 ... OUT_HOUSE を 1 に
    // プレーエリア内であることが確定 ... IN_PLAYAREA を 1 に
    template<int OUT_HOUSE = 0, int IN_PLAYAREA = 0, typename float_t>
    bool isInFreeGuardZone(float_t x, float_t y)noexcept{
        if(!IN_PLAYAREA){
            if(!((static_cast<float_t>(FX_PA_MIN + FR_STONE_RAD) < x)
               && (x < static_cast<float_t>(FX_PA_MAX - FR_STONE_RAD))
               && (static_cast<float_t>(FY_RINK_MIN + FR_STONE_RAD) < y))){
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

    // 指定した範囲内でランダムに石の座標を設定
    template<class float_t, class dice_t>
    void locateInHouse(float_t *const px, float_t *const py, dice_t *const pdice){
        fpn_t r0 = pdice->drand();
        fpn_t r1 = pdice->drand();
        *px = FX_TEE + FR_IN_HOUSE * r1 * cos(2.0 * M_PI * r0);
        *py = FY_TEE + FR_IN_HOUSE * r1 * sin(2.0 * M_PI * r0);
        assert(isInHouse(*px, *py));
    }

    template<class float_t, class dice_t>
    void locateInPlayArea(float_t *const px, float_t *const py, dice_t *const pdice){
        fpn_t r0 = pdice->drand();
        fpn_t r1 = pdice->drand();
        *px = FX_PA_MIN + FR_STONE_RAD + (FX_PA_MAX - FX_PA_MIN - 2 * FR_STONE_RAD) * r0;
        *py = FY_PA_MIN + FR_STONE_RAD + (FY_PA_MAX - FY_PA_MIN) * r1;
        assert(isInPlayArea(*px, *py));
    }

    /**************************連続座標系での代表的な値**************************/

    // 公式提供のヒューリスティック関数で導いたティーショットの速度ベクトル
    constexpr fpn_t FVX_TEESHOT_OFFICIAL[2] = { -0.99073974, +0.99073974 }; // 右回転, 左回転
    constexpr fpn_t FVY_TEESHOT_OFFICIAL = +29.559775;
    
    constexpr fpn_t FV_TEESHOT = XYtoR(FVX_TEESHOT_OFFICIAL[0], FVY_TEESHOT_OFFICIAL);
    
    // ハウスの端4点へのドローの速度ベクトル
    // 右回転のみ
    constexpr fpn_t FVX_FRONT_HOUSE = -0.958646;
    constexpr fpn_t FVY_FRONT_HOUSE = +28.7467;
    
    constexpr fpn_t FVX_BACK_HOUSE = -1.01215;
    constexpr fpn_t FVY_BACK_HOUSE = +30.3512;
    
    constexpr fpn_t FVX_RIGHT_HOUSE = +0.61765;
    constexpr fpn_t FVY_RIGHT_HOUSE = +29.5916;
    
    constexpr fpn_t FVX_LEFT_HOUSE = -2.58773;
    constexpr fpn_t FVY_LEFT_HOUSE = +29.4847;

    /**************************石の回転**************************/

    // 現在、回転は右回り、左回りの2値しかない
    enum Spin : int{
        SPIN_NONE = -1,
        RIGHT = 0,
        LEFT = 1,
        SPIN_BOTH = 2,
    };
    
    const char* spinChar = "RL";
    const char* spinString[2] = {"RIGHT", "LEFT"};
    
    // 0 と比較して判定するための関数
    constexpr bool isRight(Spin s)noexcept{ return (s == Spin::RIGHT); }
    constexpr int isLeft(Spin s)noexcept{ return static_cast<int>(s); }
    
    constexpr Spin flipSpin(Spin s)noexcept{
        return static_cast<Spin>(Spin::RIGHT + Spin::LEFT - s);
    }
    constexpr bool examSpin(Spin s)noexcept{ return !(s & (~1)); }

    // 回転の向きと初期回転角速度の間の変換
    // TODO: 回転角速度が完全に0のときの扱いがもし重要になりそうならば対応
    constexpr bool isLeftFW(fpn_t fw)noexcept{ return (fw < 0); }
    constexpr Spin FWtoSpin(fpn_t fw)noexcept{ return (fw >= 0) ? Spin::RIGHT : Spin::LEFT; }
    constexpr fpn_t SpintoThrowFW(Spin s)noexcept{ return isRight(s) ? FANGV_ORG : (-FANGV_ORG); }
    
    char toSpinChar(int s)noexcept{
        if(examSpin(static_cast<Spin>(s))){
            return spinChar[s];
        }
        return '-';
    }
    
    std::string toSpinString(int s)noexcept{
        if(examSpin(static_cast<Spin>(s))){
            return spinString[s];
        }
        return "NONE";
    }

    /**************************連続座標系での公式系と歩系の座標変換**************************/

    // 原点をずらしてYを反転させる
    template<class fpos_t>
    fpos_t convPosition_Official_Ayumu(const fpos_t& arg)noexcept{
        return fpos_t((arg.getX() - FX_OFFICIAL_TEE) + FX_TEE, -(arg.getY() - FY_OFFICIAL_TEE) + FY_TEE);
    }

    template<class fpos_t>
    fpos_t convPosition_Ayumu_Official(const fpos_t& arg)noexcept{
        return fpos_t((arg.getX() - FX_TEE) + FX_OFFICIAL_TEE, -(arg.getY() - FY_TEE) + FY_OFFICIAL_TEE);
    }

    template<class fmove_t>
    fmove_t convMove_Official_Ayumu(const fmove_t& arg)noexcept{
        return fmove_t(arg.vx(), -arg.vy(), arg.getSpin());
    }

    template<class fmove_t>
    fmove_t convMove_Ayumu_Official(const fmove_t& arg)noexcept{
        return fmove_t(arg.vx(), -arg.vy(), arg.getSpin());
    }

    /**************************連続座標系での盤面情報**************************/
    /*
    template<typename float_t = fpn_t>
    struct fVecXY{
    //ストーンの位置を表す連続型
    float_t x;
    float_t y;

    constexpr fVecXY() :
    x(), y(){}

    constexpr fVecXY(float_t ax, float_t ay)
    : x(ax), y(ay){}

    void set(float_t ax, float_t ay){
    x = ax; y = ay;
    }

    void setX(float_t ax){ x = ax; }
    void setY(float_t ay){ y = ay; }

    float_t getX()const{ return x; }
    float_t getY()const{ return y; }
    };
    */

    template<typename float_t = fpn_t>
    struct fPosXY{
        // ストーンの(X, Y)座標での位置を表す連続型の基本表現
        float_t x, y;

        constexpr fPosXY() :
            x(), y(){}

        constexpr fPosXY(float_t ax, float_t ay)
            : x(ax), y(ay){}

        constexpr fPosXY(const fPosXY<float_t>& apos)
            : x(apos.x), y(apos.y){}
        
        constexpr fPosXY<float_t> operator+(const fPosXY<float_t>& ap)noexcept{
            return fPosXY<float_t>(x + ap.x, y + ap.y);
        }
        constexpr fPosXY<float_t> operator-(const fPosXY<float_t>& ap)noexcept{
            return fPosXY<float_t>(x - ap.x, y - ap.y);
        }
        constexpr void operator+=(const fPosXY<float_t>& ap)noexcept{
            add(ap.x, ap.y);
        }
        constexpr void operator-=(const fPosXY<float_t>& ap)noexcept{
            add(-ap.x, -ap.y);
        }

        void set(float_t ax, float_t ay)noexcept{
            x = ax; y = ay;
        }
        template<typename f_t>
        void set(const fPosXY<f_t>& apos)noexcept{
            x = apos.x; y = apos.y;
        }
        void setRCS(float_t ax, float_t ay)noexcept{
            x = ax; y = ay;
        }
        void setPCS(float_t ar, float_t ath)noexcept{
            x = ar * ::sin(ath); y = ar * ::cos(ath);
        }
        
        void setX(float_t ax)noexcept{ x = ax; }
        void setY(float_t ay)noexcept{ y = ay; }
        
        void add(float_t ax, float_t ay)noexcept{
            x += ax; y += ay;
        }
        void addX(float_t ax)noexcept{ x += ax; }
        void addY(float_t ay)noexcept{ y += ay; }

        constexpr float_t getX()const noexcept{ return x; }
        constexpr float_t getY()const noexcept{ return y; }
        
        void flip()noexcept{ x = 2 * FX_TEE - x; }
        
        std::string toString()const{
            std::ostringstream oss;
            oss << "(" << getX() << ", " << getY() << ")";
            return oss.str();
        }
    };

    template<typename float_t>
    ostream& operator<<(ostream& out, const fPosXY<float_t>& arg){
        out << arg.toString();
        return out;
    }

    // 重要な座標定数
    constexpr fPosXY<fpn_t> FPOSXY_TEE = fPosXY<fpn_t>(FX_TEE, FY_TEE);
    constexpr fPosXY<fpn_t> FPOSXY_THROW = fPosXY<fpn_t>(FX_THROW, FY_THROW);
    constexpr fPosXY<fpn_t> FPOSXY_PA_CORNER_NL = fPosXY<fpn_t>(FX_PA_MIN, FY_PA_MIN); // near, left
    constexpr fPosXY<fpn_t> FPOSXY_PA_CORNER_NR = fPosXY<fpn_t>(FX_PA_MAX, FY_PA_MIN); // near, right
    constexpr fPosXY<fpn_t> FPOSXY_PA_CORNER_FL = fPosXY<fpn_t>(FX_PA_MIN, FY_PA_MAX); // far, left
    constexpr fPosXY<fpn_t> FPOSXY_PA_CORNER_FR = fPosXY<fpn_t>(FX_PA_MAX, FY_PA_MAX); // far, right

    /**************************位置座標の基本アルゴリズム**************************/

    template<class pos_t>
    bool isInPlayArea(const pos_t& pos){
        return isInPlayArea(pos.getX(), pos.getY());
    }

    template<class pos_t>
    bool isInHouse(const pos_t& pos){
        return isInHouse(pos.getX(), pos.getY());
    }

    constexpr bool isInHouse(const float r)noexcept{
        return (r < static_cast<float>(FR_IN_HOUSE));
    }
    constexpr bool isInHouse(const double r)noexcept{
        return (r < static_cast<double>(FR_IN_HOUSE));
    }
    
    // ハウスからの距離の2乗で判定
    constexpr bool isInHouseR2(const float r2)noexcept{
        return (r2 < static_cast<float>(FR2_IN_HOUSE));
    }
    constexpr bool isInHouseR2(const double r2)noexcept{
        return (r2 < static_cast<double>(FR2_IN_HOUSE));
    }
    
    /*
    template<>
    bool isInHouse(const fPosXYRT<float>& pos){
    return isInHouse<float>((float)pos.getR());//すでにRが計算されているので
    }
    template<>
    bool isInHouse(const fPosXYRT<double>& pos){
    return isInHouse<double>((double)pos.getR());//すでにRが計算されているので
    }
    */
    template<class pos_t, class float_t = fpn_t>
    bool isNearTee(const pos_t& pos, float_t dist){
        return isNearTee(pos.getX(), pos.getY(), dist);
    }

    template<>
    bool isNearTee(const float& r, float dist){
        return (r < dist);
    }

    template<>
    bool isNearTee(const double& r, double dist){
        return (r < dist);
    }
    /*
    template<>
    bool isNearTee(const fPosXYRT<float>& pos, float dist){
    return isNearTee(pos.getR(), dist);//すでにRが計算されているので
    }

    template<>
    bool isNearTee(const fPosXYRT<double>& pos, double dist){
    return isNearTee(pos.getR(), dist);//すでにRが計算されているので
    }
    */
    template<int OUT_HOUSE = 0, int IN_PLAYAREA = 0, class pos_t>
    bool isInFreeGuardZone(const pos_t& pos){
        return isInFreeGuardZone<OUT_HOUSE, IN_PLAYAREA>(pos.getX(), pos.getY());
    }
    
    // 相対的な位置関係の計算
    template<class pos0_t, class pos1_t>
    auto calcDistance(const pos0_t& p0, const pos1_t& p1){
        return XYtoR(p1.x - p0.x, p1.y - p0.y);
    }
    template<class pos0_t, class pos1_t>
    auto calcDistance2(const pos0_t& p0, const pos1_t& p1){
        return XYtoR2(p1.x - p0.x, p1.y - p0.y);
    }
    template<class pos0_t, class pos1_t>
    auto calcRelativeAngle(const pos0_t& p0, const pos1_t& p1){
        return XYtoT(p1.y - p0.y, p1.x - p0.x);
    }
    
    // ティーとの関係
    template<class pos_t>
    auto calcDistanceTee(const pos_t& p){
        return XYtoR(p.x - FX_TEE, p.y - FY_TEE);
    }
    template<class pos_t>
    auto calcDistance2Tee(const pos_t& p){
        return XYtoR2(p.x - FX_TEE, p.y - FY_TEE);
    }
    template<class pos_t>
    auto calcRelativeAngleTee(const pos_t& p){
        return XYtoT(p.y - FY_TEE, p.x - FX_TEE);
    }
    
    // 投げる点との関係
    template<class pos_t>
    auto calcDistanceThrow(const pos_t& p){
        return XYtoR(p.x - FX_THROW, p.y - FY_THROW);
    }
    template<class pos_t>
    auto calcDistance2Throw(const pos_t& p){
        return XYtoR2(p.x - FX_THROW, p.y - FY_THROW);
    }
    template<class pos_t>
    auto calcRelativeAngleThrow(const pos_t& p){
        return XYtoT(p.y - FY_THROW, p.x - FX_THROW);
    }
    
    // 順序関係
    template<class pos_t>
    bool isMoreFrontal(const pos_t& p0, const pos_t& p1){
        return (p0.y < p1.y);
    }
    // TODO: 左右判定を投げる点からの向きで判定する仕様にした方がいいかも
    template<class pos_t>
    bool isMoreLeft(const pos_t& p0, const pos_t& p1){
        return (p0.x < p1.x);
    }
    template<class pos_t>
    bool isMoreCentral(const pos_t& p0, const pos_t& p1){
        return (calcDistance2Tee(p0) < calcDistance2Tee(p1));
    }

    // 2つの石がめり込んでいる
    template<class pos0_t, class pos1_t>
    bool isCavingIn(const pos0_t& p0, const pos1_t& p1){
        return (calcDistance2(p0, p1) < 4.0 * FR2_STONE_RAD);
    }

    // プレーエリア外との最短距離
    template<class pos_t>
    double calcDistanceToOut(const pos_t& pos){
        if (!isInPlayArea(pos)){ return 0; }
        return min(min(pos.x - (FX_PA_MIN + FR_STONE_RAD), (FX_PA_MAX - FR_STONE_RAD) - pos.x),
            min(pos.y - (FY_PA_MIN + FR_STONE_RAD), (FY_PA_MAX + FR_STONE_RAD) - pos.y));
    }

    // ランダムに石をセット
    template<class pos_t, class dice_t>
    void locateInHouse(pos_t *const ppos, dice_t *const pdice){
        locateInHouse(&ppos->x, &ppos->y, pdice);
    }
    template<class pos_t, class dice_t>
    void locateInPlayArea(pos_t *const ppos, dice_t *const pdice){
        locateInPlayArea(&ppos->x, &ppos->y, pdice);
    }

    /**************************連続座標系での速度ベクトルによる着手表現**************************/

    // 着手を存在させる範囲
    constexpr fpn_t FVX_MIN = -3.1;
    constexpr fpn_t FVX_MAX = +3.1;
    constexpr fpn_t FVY_MIN = +27.1;
    constexpr fpn_t FVY_MAX = +33.3;

    constexpr fpn_t FVX_MID = (FVX_MIN + FVX_MAX) / 2;
    constexpr fpn_t FVY_MID = (FVY_MIN + FVY_MAX) / 2;

    constexpr fpn_t FV_RANGE_MAX = (FVX_MAX - FVX_MIN) / 2;

    // 速さの範囲(NullMoveは別)
    constexpr fpn_t FV_MIN = FVY_MIN;
    constexpr fpn_t FV_MAX = XYtoR(FVX_MAX, FVY_MAX);

    constexpr fpn_t FV2_MAX = FV_MAX * FV_MAX;

    template<typename float_t = fpn_t>
    struct fMoveXY{
        // ノーレンジ着手
        // 速度ベクトルのxy成分と回転
        float_t x;
        float_t y;
        Spin s;

        constexpr float_t vx()const noexcept{ return x; }
        constexpr float_t vy()const noexcept{ return y; }
        constexpr Spin getSpin()const noexcept{ return s; }
        constexpr Spin spin()const noexcept{ return s; }
        constexpr bool isLeftSpin()const noexcept{ return isLeft(s); }
        fpn_t w()const noexcept{ return SpintoThrowFW(s); }

        float_t v2()const noexcept{ return XYtoR2(x, y); } // 速さ^2
        float_t v()const noexcept{ return XYtoR(x, y); } // 速さ
        float_t th()const noexcept{ return XYtoT(y, x); } // 角度
        float_t cos()const noexcept{ return XYtoCosT(y, x); }
        float_t sin()const noexcept{ return XYtoSinT(y, x); }

        void set(float_t ax, float_t ay, Spin as)noexcept{
            setRCS(ax, ay, as);
        }
        template<typename f_t>
        void set(const fMoveXY<f_t>& amv)noexcept{
            setRCS(amv.x, amv.y, amv.s);
        }
        
        void setRCS(float_t ax, float_t ay, Spin as)noexcept{
            x = ax; y = ay; s = as;
        }
        void setPCS(float_t av, float_t ath, Spin as)noexcept{
            x = av * ::sin(ath); y = av * ::cos(ath); s = as;
        }

        void setVX(float_t ax)noexcept{ x = ax; }
        void setVY(float_t ay)noexcept{ y = ay; }
        void setVx(float_t ax)noexcept{ x = ax; }
        void setVy(float_t ay)noexcept{ y = ay; }
        void setSpin(Spin as)noexcept{ s = as; }

        void addVX(float_t ax)noexcept{ x += ax; }
        void addVY(float_t ay)noexcept{ y += ay; }
        void addVx(float_t ax)noexcept{ x += ax; }
        void addVy(float_t ay)noexcept{ y += ay; }
        
        void flip()noexcept{ x = -x; s = flipSpin(s); }

        constexpr fMoveXY() :x(), y(), s(){}
        constexpr fMoveXY(float_t ax, float_t ay, Spin as)
            : x(ax), y(ay), s(as){}
    };
    
    constexpr fMoveXY<> FMVXY_TEESHOT_R = fMoveXY<>(FVX_TEESHOT_OFFICIAL[Spin::RIGHT],
                                                    FVY_TEESHOT_OFFICIAL,
                                                    Spin::RIGHT);

    template<typename float_t = fpn_t>
    struct fMoveVTh{
        //ノーレンジ着手
        //速度ベクトルのr,th成分と回転
        float_t v_;
        float_t th_;
        Spin s;

        float_t vx()const noexcept{ return v_ * sin(); }
        float_t vy()const noexcept{ return v_ * cos(); }
        constexpr Spin getSpin()const noexcept{ return s; }
        constexpr Spin spin()const noexcept{ return s; }
        constexpr bool isLeftSpin()const noexcept{ return isLeft(s); }
        fpn_t w()const noexcept{ return SpintoThrowFW(s); }

        constexpr float_t v2()const noexcept{ return v_ * v_; }
        constexpr float_t v()const noexcept{ return v_; }
        constexpr float_t th()const noexcept{ return th_; }
        float_t cos()const noexcept{ return ::cos(th_); }
        float_t sin()const noexcept{ return ::sin(th_); }

        void set(float_t av, float_t ath, Spin as)noexcept{
            v_ = av; th_ = ath; s = as;
        }
        void setRCS(float_t ax, float_t ay, Spin as)noexcept{
            v_ = XYtoR(ax, ay); th_ = XYtoT(ay, ax); s = as;
        }
        void setPCS(float_t av, float_t ath, Spin as)noexcept{
            v_ = av; th_ = ath; s = as;
        }

        void setV(float_t av)noexcept{ v_ = av; }
        void setVTheta(float_t ath)noexcept{ th_ = ath; }
        void setSpin(Spin as)noexcept{ s = as; }

        void addV(float_t av)noexcept{ v_ += av; }
        void addVTheta(float_t ath)noexcept{ th_ += ath; }

        constexpr fMoveVTh() :v_(), th_(), s(){}
        constexpr fMoveVTh(float_t av, float_t ath, Spin as)
            : v_(av), th_(ath), s(as){}
    };

    template<typename float_t>
    std::ostream& operator<<(std::ostream& out, const fMoveXY<float_t>& arg){
        out << "fMV < vx:" << arg.vx() << " vy:" << arg.vy() << " s:" << arg.getSpin() << " >";
        return out;
    }

    template<typename float_t>
    std::ostream& operator<<(std::ostream& out, const fMoveVTh<float_t>& arg){
        out << "fMV < vx:" << arg.vx() << " vy:" << arg.vy() << " s:" << arg.getSpin() << " >";
        return out;
    }

    template<typename float_t = fpn_t>
    struct fRMoveXY{
        // 速度ベクトルのxy成分と回転、その範囲
        float_t x;
        float_t y;
        Spin s;
        float_t rx;
        float_t ry;

        float_t vx()const noexcept{ return x; }
        float_t vy()const noexcept{ return y; }
        Spin getSpin()const noexcept{ return s; }
        Spin spin()const noexcept{ return s; }
        float_t getRangeVX()const noexcept{ return rx; }
        float_t getRangeVY()const noexcept{ return ry; }
        bool isLeftSpin()const noexcept{ return isLeft(s); }
        fpn_t w()const noexcept{ return SpintoThrowFW(s); }

        float_t v()const noexcept{ return XYtoR(x, y); } // 速さ
        float_t v2()const noexcept{ return XYtoR(x, y); }
        float_t calcT()const{ return XYtoT(y, x); } // 角度
        float_t calcCosT()const{ return XYtoCosT(y, x); }
        float_t calcSinT()const{ return XYtoSinT(y, x); }

        void setVX(float_t ax)noexcept{ x = ax; }
        void setVY(float_t ay)noexcept{ y = ay; }
        void setSpin(Spin as)noexcept{ s = as; }
        void setRangeVX(float_t arx)noexcept{ rx = arx; }
        void setRangeVY(float_t ary)noexcept{ ry = ary; }
        void setRangeVXY(float_t ar)noexcept{ rx = ar; ry = ar; }

        void addVX(float_t ax)noexcept{ x += ax; }
        void addVY(float_t ay)noexcept{ y += ay; }

        void setWR(Spin as)noexcept{ // 全範囲着手
            x = FVX_MID; y = FVY_MID;
            s = as;
            rx = FV_RANGE_MAX; ry = FV_RANGE_MAX;
        }

        // 分割
        fRMoveXY<float_t> part4(int n){
            // 4分割のn番目を計算する
            assert(0 <= n && n < 4);

            fMoveXY<float_t> ret;
            ret.s = s;
            ret.rx = rx / 2; ret.ry = ry / 2;
            switch (n){
                case 0: ret.x = x - ret.rx; ret.y = y - ret.ry; break;
                case 1: ret.x = x - ret.rx; ret.y = y + ret.ry; break;
                case 2: ret.x = x + ret.rx; ret.y = y - ret.ry; break;
                case 3: ret.x = x + ret.rx; ret.y = y + ret.ry; break;
                default: UNREACHABLE; break;
            }
            return ret;
        }

        template<class ddice_t>
        fMoveXY<float_t> genNR(ddice_t *const dice)const{
            // 乱数を掛けて、範囲 0 (no range) の着手を1つ返す
            fMoveXY<float_t> ret;
            ret.s = s;
            ret.x = x + dice->drand() * rx;
            ret.y = y + dice->drand() * ry;
            return ret;
        }
    };

    template<typename float_t>
    std::ostream& operator<<(std::ostream& out, const fRMoveXY<float_t>& arg){
        out << "fMV < vx:" << arg.vx() << " vy:" << arg.vy() << " vr:" << arg.getRangeVX() << " s:" << arg.getSpin() << " >";
        return out;
    }

    // 合法性
    template<typename T>
    bool isValidVelocity(const T& v)noexcept{
        return (0 <= v && v <= FV_MAX);
    }
    template<typename T>
    bool isValidVelocity2(const T& v2)noexcept{
        return (0 <= v2 && v2 <= FV_MAX * FV_MAX);
    }

    template<class move_t>
    bool isValidMove(const move_t& mv)noexcept{
        if (!examSpin(mv.spin())){ // 回転
            return false;
        }
        //if (mv.v2() > FV2_MAX){ // 速さ
        if (mv.vy() > FVY_LEGAL_MAX){ // 速さ
            return false;
        }
        return true;
    }

    /**************************石の瞬間パラメータ**************************/

    template<typename float_t = fpn_t>
    struct fMStone{
        // 動いている石のパラメータ
        float_t x;
        float_t y;
        float_t vx_;
        float_t vy_;
        float_t w;

        void set(float_t ax, float_t ay, float_t avx, float_t avy, float_t aw)noexcept{
            x = ax; y = ay; vx_ = avx; vy_ = avy; w = aw;
        }

        void setX(float_t ax)noexcept{ x = ax; }
        void setY(float_t ay)noexcept{ y = ay; }
        void setVX(float_t avx)noexcept{ vx = avx; }
        void setVY(float_t avy)noexcept{ vy = avy; }
        void setW(float_t aw)noexcept{ w = aw; }

        constexpr float_t getX()const noexcept{ return x; }
        constexpr float_t getY()const noexcept{ return y; }
        constexpr float_t vx()const noexcept{ return vx_; }
        constexpr float_t vy()const noexcept{ return vy_; }
        constexpr float_t getW()const noexcept{ return w; }

        constexpr fMStone() :
            x(), y(), vx_(), vy_(), w(){}

        constexpr fMStone(float_t ax, float_t ay, float_t avx, float_t avy, float_t aw) :
            x(ax), y(ay), vx_(avx), vy_(avy), w(aw){}
    };



    /**************************石の状態**************************/

    struct StoneState : public BitSet32{
        using data_t = uint32_t;

        void setIndex(uint32_t idx)noexcept{ *this |= idx; }
        void setColor(Color col){ *this |= col; }
        
        Color getColor()const noexcept{ return static_cast<Color>(get(0)); }
        uint32_t getIndex()const noexcept{ return *this & 15; }

        // フリーガード
        void setFreeGuard()noexcept{ set(3); }
        void resetFreeGuard()noexcept{ reset(3); }
        data_t isFreeGuard()const noexcept{ return test(3); }
        // フリーガード予備
        void setInFreeGuardZone()noexcept{ set(2); }
        void resetInFreeGuardZone()noexcept{ reset(2); }
        data_t isInFreeGuardZone()const noexcept{ return test(2); }

        void init()noexcept{ reset(); }

        constexpr StoneState():BitSet32(){}
        constexpr StoneState(const StoneState& arg):BitSet32(arg){}
    };

    /**************************石のシンプルな集合**************************/
    
    using StoneSet = BitSet32;

    /**************************局面表現のアルゴリズムテンプレート**************************/

    // セットされた石を順に見ていく関数
    // 疎な系だと仮定して、石の存在ビットセットから1ビットずつ取り出して見る
    template<class board_t, typename callback_t>
    void iterateStone(const board_t& bd, const callback_t& callback)noexcept{
        iterate(bd.sb(), [bd, callback](const int idx)->void{
            callback(bd.stone(idx));
        });
    }
    template<class board_t, typename callback_t>
    void iterateStone(const board_t& bd, Color c, const callback_t& callback){
        iterate(bd.sbColor(c), [bd, callback](const int idx)->void{
            callback(bd.stone(idx));
        });
    }
    
    // 色やインデックスと一緒に見ていく場合
    template<class board_t, typename callback_t>
    void iterateStoneWithColor(const board_t& bd, const callback_t& callback)noexcept{
        iterate(bd.sb(), [bd, callback](const int idx)->void{
            callback(getColor(idx), bd.stone(idx));
        });
    }
    template<class board_t, typename callback_t>
    void iterateStoneWithIndex(const board_t& bd, const callback_t& callback)noexcept{
        iterate(bd.sb(), [bd, callback](const int idx)->void{
            callback(idx, bd.stone(idx));
        });
    }
    template<class board_t, typename callback_t>
    void iterateStoneWithIndex(const board_t& bd, Color c, const callback_t& callback){
        iterate(bd.sbColor(c), [bd, callback](const int idx)->void{
            callback(idx, bd.stone(idx));
        });
    }
    
    // 1つを除き、それ以外を見る
    template<class board_t, typename callback_t>
    void iterateStoneWithIndexExcept(const board_t& bd, uint32_t ex, const callback_t& callback){
        iterateExcept(bd.sb(), ex, [bd, callback](const int idx)->void{
            callback(idx, bd.stone(idx));
        });
    }
    template<class board_t, typename callback_t>
    void iterateStoneWithIndexExcept(const board_t& bd, Color c, uint32_t ex, const callback_t& callback){
        iterateExcept(bd.sbColor(c), ex, [bd, callback](const int idx)->void{
            callback(idx, bd.stone(idx));
        });
    }
    
    // 少なくとも1つ以上石があることを前提とする場合
    // もし無かった場合の動作は未定義
    template<class board_t, typename callback_t>
    void iterateStoneWithIndexWithoutFirstCheck(const board_t& bd, const callback_t& callback)noexcept{
        iterateWithoutFirstCheck(bd.sb(), [bd, callback](const int idx)->void{
            callback(idx, bd.stone(idx));
        });
    }
    template<class board_t, typename callback_t>
    void iterateStoneWithIndexWithoutFirstCheck(const board_t& bd, Color c, const callback_t& callback){
        iterateWithoutFirstCheck(bd.sbColor(c), [bd, callback](const int idx)->void{
            callback(idx, bd.stone(idx));
        });
    }
    template<class board_t, typename callback_t>
    void iterateStoneWithIndexWithoutFirstCheckExcept(const board_t& bd, uint32_t ex, const callback_t& callback){
        iterateWithoutFirstCheckExcept(bd.sb(), ex, [bd, callback](const int idx)->void{
            callback(idx, bd.stone(idx));
        });
    }
    template<class board_t, typename callback_t>
    void iterateStoneWithIndexWithoutFirstCheckExcept(const board_t& bd, Color c, uint32_t ex, const callback_t& callback){
        iterateWithoutFirstCheckExcept(bd.sbColor(c), ex, [bd, callback](const int idx)->void{
            callback(idx, bd.stone(idx));
        });
    }
    
    // 条件に合う石を探す
    template<class board_t, typename callback_t>
    int searchStone(const board_t& bd, const callback_t& callback)noexcept{
        return search(bd.sb(), [&](const int idx)->bool{
            return callback(bd.stone(idx));
        });
    }
    template<class board_t, typename callback_t>
    int searchStone(const board_t& bd, Color c, const callback_t& callback){
        return search(bd.sbColor(c), [bd, callback](const int idx)->bool{
            return callback(bd.stone(idx));
        });
    }
    template<class board_t, typename callback_t>
    int searchStoneWithIndex(const board_t& bd, const callback_t& callback)noexcept{
        return search(bd.sb(), [&](uint32_t idx)->bool{
            return callback(idx, bd.stone(idx));
        });
    }
    template<class board_t, typename callback_t>
    int searchStoneWithIndex(const board_t& bd, Color c, const callback_t& callback){
        return search(bd.sbColor(c), [&](uint32_t idx)->bool{
            return callback(idx, bd.stone(idx));
        });
    }
    
    // 条件を満たす石の数を数える
    template<class board_t, typename callback_t>
    int countStones(const board_t& bd, const callback_t& callback)noexcept{
        int cnt = 0;
        iterateStone(bd, [&cnt, callback](const auto& st)->void{
            if(callback(st)){ ++cnt; }
        });
        return cnt;
    }
    template<class board_t, typename callback_t>
    int countStones(const board_t& bd, Color c, const callback_t& callback){
        int cnt = 0;
        iterateStone(bd, c, [&cnt, callback](const auto& st)->void{
            if(callback(st)){ ++cnt; }
        });
        return cnt;
    }
    
    // ハウス内
    template<class board_t>
    int countNInHouse(const board_t& bd)noexcept{
        return countStones(bd, [](const auto& st)->bool{ return isInHouse(st); });
    }
    template<class board_t>
    int countNInHouse(const board_t& bd, Color c){
        return countStones(bd, c, [](const auto& st)->bool{ return isInHouse(st); });
    }

    // プレーエリア内
    template<class board_t>
    int countNInPlayArea(const board_t& bd)noexcept{
        return countStones(bd, [](const auto& st)->bool{ return isInPlayArea(st); });
    }
    template<class board_t>
    int countNInPlayArea(const board_t& bd, Color c){
        return countStones(bd, c, [](const auto& st)->bool{ return isInPlayArea(st); });
    }
    
    // 石の数の差分を数える(盤面クラスごとに, より高速な方法がある場合は各クラスで独自に実装する)
    template<class board0_t, class board1_t>
    int countDiffNInHouse(const board0_t& pre, const board1_t& post)noexcept{
        return countNInHouse(post) - countNInHouse(pre);
    }
    template<class board0_t, class board1_t>
    int countDiffNInHouse(const board0_t& pre, const board1_t& post, Color c){
        return countNInHouse(post, c) - countNInHouse(pre, c);
    }
    template<class board0_t, class board1_t>
    int countDiffNInPlayArea(const board0_t& pre, const board1_t& post)noexcept{
        return countNInPlayArea(post) - countNInPlayArea(pre);
    }
    template<class board0_t, class board1_t>
    int countDiffNInPlayArea(const board0_t& pre, const board1_t& post, Color c){
        return countNInPlayArea(post, c) - countNInPlayArea(pre, c);
    }
    
    // 指定した石と盤上の石が1つでも位置が被っているか調べる
    template<class board_t, class pos_t>
    bool isCavingInAny(const board_t& bd, const pos_t& p)noexcept{
        if(0 <= searchStone(bd, [p](const auto& st)->bool{
            return isCavingIn(p, st);
        })){ return true; }
        return false;
    }
    
    // 条件を満たす石を取り除く
    template<class board_t, class callback_t>
    void removeStones(board_t *const pbd, const callback_t& callback){
        iterateStoneWithIndex(*pbd, [pbd, &callback](int idx, const auto& st)->void{
            if(callback(st)){
                pbd->removeStone(idx);
            }
        });
    }
    
    // 乱数を用いて、石を置いていく
    // 石の番号は色ごとに, エンドの第1投から順番に割り振られる
    // 各色 8 個をオーバーした場合の動作は未定義
    template<class board_t, class dice_t, typename callback_t>
    void pushStones(board_t *const pbd, Color c, int n, dice_t *const pdice, const callback_t& callback){
        for(int i = 0; i < n; ++i){
            fPosXY<> pos;
            do{
                callback(&pos, pdice);
            }while(isCavingInAny(*pbd, pos));
            pbd->pushStone(c, pos);
        }
    }
    template<class board_t, class dice_t>
    void pushStonesInHouse(board_t *const pbd, Color c, int n, dice_t *const pdice){
        pushStones(pbd, c, n, pdice, [pdice](auto *const ppos, dice_t *const pd)->void{
            locateInHouse(ppos, pd);
        });
    }
    template<class board_t, class dice_t>
    void pushStonesInPlayArea(board_t *const pbd, Color c, int n, dice_t *const pdice){
        pushStones(pbd, c, n, pdice, [pdice](auto *const ppos, dice_t *const pd)->void{
            locateInPlayArea(ppos, pd);
        });
    }
    
    // 盤面の石同士の位置の差の和を求める
    
    // 同じ番号の石が必ずあることを前提とする場合
    template<class board0_t, class board1_t>
    fpn_t calcDistanceSumOneToOne(const board0_t& bd0, const board1_t& bd1, int *const pdist = nullptr){
        assert(bd0.sb() == bd1.sb());
        double sum = 0;
        iterate(bd0.sb(), [bd0, bd1, pdist, &sum](uint32_t idx)->void{
            double d = calcDistance(bd0.stone(idx), bd1.stone(idx));
            sum += d;
            if(pdist != nullptr){
                int l = int(d * 1000);
                pdist[(l < 1) ? 0 : (log2i(uint32_t(l)) + 1)]++;
            }
        });
        return sum;
    }
    
    template<class board0_t, class board1_t>
    fpn_t calcDistance2SumOneToOne(const board0_t& bd0, const board1_t& bd1, int *const pdist = nullptr){
        assert(bd0.sb() == bd1.sb());
        double sum = 0;
        iterate(bd0.sb(), [bd0, bd1, pdist, &sum](uint32_t idx)->void{
            double d2 = calcDistance2(bd0.stone(idx), bd1.stone(idx));
            sum += d2;
            if(pdist != nullptr){
                int l = int(d2 * 1000);
                pdist[(l < 1) ? 0 : (log2i(uint32_t(l)) + 1)]++;
            }
        });
        return sum;
    }
    
    // 同じ番号の石がなければプレーエリア境界との距離とする場合
    template<class board0_t, class board1_t>
    fpn_t calcDistanceSumNotOneToOne(const board0_t& bd0, const board1_t& bd1, int *const pdist = nullptr){
        double sum = 0;
        iterate(BitSet32(bd0.sb() & (~bd1.sb())), [bd0, pdist, &sum](uint32_t idx)->void{
            double d = calcDistanceToOut(bd0.stone(idx));
            sum += d;
            if(pdist != nullptr){
                int l = int(d * 1000);
                pdist[(l < 1) ? 0 : (log2i(uint32_t(l)) + 1)]++;
            }
        });
        iterate(BitSet32(bd1.sb() & (~bd0.sb())), [bd1, pdist, &sum](uint32_t idx)->void{
            double d = calcDistanceToOut(bd1.stone(idx));
            sum += d;
            if(pdist != nullptr){
                int l = int(d * 1000);
                pdist[(l < 1) ? 0 : (log2i(uint32_t(l)) + 1)]++;
            }
        });
        iterate(BitSet32(bd0.sb() & bd1.sb()), [bd0, bd1, pdist, &sum](uint32_t idx)->void{
            double d = calcDistance(bd0.stone(idx), bd1.stone(idx));
            sum += d;
            if(pdist != nullptr){
                int l = int(d * 1000);
                pdist[(l < 1) ? 0 : (log2i(uint32_t(l)) + 1)]++;
            }
        });
        return sum;
    }
    
    template<class board0_t, class board1_t>
    fpn_t calcDistance2NotOneToOne(const board0_t& bd0, const board1_t& bd1, int *const pdist = nullptr){
        double sum = 0;
        iterate(BitSet32(bd0.sb() & (~bd1.sb())), [bd0, pdist, &sum](uint32_t idx)->void{
            double d = calcDistanceToOut(bd0.stone(idx));
            double d2 = d * d;
            sum += d2;
            if(pdist != nullptr){
                int l = int(d2 * 1000);
                pdist[(l < 1) ? 0 : (log2i(uint32_t(l)) + 1)]++;
            }
        });
        iterate(BitSet32(bd1.sb() & (~bd0.sb())), [bd1, pdist, &sum](uint32_t idx)->void{
            double d = calcDistanceToOut(bd1.stone(idx));
            double d2 = d * d;
            sum += d2;
            if(pdist != nullptr){
                int l = int(d2 * 1000);
                pdist[(l < 1) ? 0 : (log2i(uint32_t(l)) + 1)]++;
            }
        });
        iterate(BitSet32(bd0.sb() & bd1.sb()), [bd0, bd1, pdist, &sum](uint32_t idx)->void{
            double d2 = calcDistance2(bd0.stone(idx), bd1.stone(idx));
            sum += d2;
            if(pdist != nullptr){
                int l = int(d2 * 1000);
                pdist[(l < 1) ? 0 : (log2i(uint32_t(l)) + 1)]++;
            }
        });
        return sum;
    }
    
    template<int ONE_TO_ONE = 0, class board0_t, class board1_t>
    fpn_t calcDistanceSum(const board0_t& bd0, const board1_t& bd1, int *const pdist  = nullptr){
        if(ONE_TO_ONE){
            return calcDistanceSumOneToOne(bd0, bd1, pdist);
        }else{
            return calcDistanceSumNotOneToOne(bd0, bd1, pdist);
        }
    }
    template<int ONE_TO_ONE = 0, class board0_t, class board1_t>
    fpn_t calcDistance2Sum(const board0_t& bd0, const board1_t& bd1, int *const pdist  = nullptr){
        if(ONE_TO_ONE){
            return calcDistance2SumOneToOne(bd0, bd1, pdist);
        }else{
            return calcDistance2SumNotOneToOne(bd0, bd1, pdist);
        }
    }
    
    // 同じ番号の付いた石の位置を線形補完
    template<class board0_t, class board1_t, class board2_t>
    void interpolate(const board0_t& bd0, const board1_t& bd1, board2_t *const pbd)noexcept{
        pbd->clearStones();
        iterateStoneWithIndex(bd0, [&](uint32_t idx, const auto& st){
            pbd->locateNewStone(idx, fPosXY<>((st.x + bd1.stone(idx).x) / 2.0, (st.y + bd1.stone(idx).y) / 2.0));
        });
    }
    
    /**************************局面進行人**************************/

    template<class board_t>
    class BoardMover{
        // TODO: 現在未使用
        
        //making move to board
    public:
        //pass
        void makePass(board_t *const pbd)const;

        //put 1 stone
        template<class pos_t>
        void makePut(board_t *const pbd, Color col, const pos_t& pos)const;

        //remove 1 stone
        void makeRemove(board_t *const pbd, int idx)const;

        //become null-field
        void makeNull(board_t *const pbd)const;

        //replace
        template<class pos_t>
        void makeReplace(board_t *const pbd, int idx, Color col, const pos_t& pos)const;

        //complex move
        template<class aboard_t, class container0_t, class container1_t>
        void makeComplexNoPut(board_t *const pbd, const aboard_t& aboard,
            const container0_t& removed,
            const container1_t& moved
            )const;

        //complex move
        template<class aboard_t, class container0_t, class container1_t, class pos_t>
        void makeComplexWithPut(board_t *const pbd, const aboard_t& aboard,
            const container0_t& removed,
            const container1_t& moved,
            Color col, const pos_t& pos
            )const;

        //renew field information
        template<class aboard_t>
        void makeRenew(board_t *const pbd, const aboard_t& aboard)const;
    };
  
    /**************************同一性確保した簡単な盤面表現**************************/
    
    class MiniBoard{
    public:
        using stone_t = fPosXY<fpn_t>;
        using sb_t = StoneSet;
        
        constexpr MiniBoard():
        st_(), sb_(0){}
        
        constexpr MiniBoard(const MiniBoard& abd):
        st_(abd.st_), sb_(abd.sb()){}
        
        constexpr sb_t sb()const noexcept{ return sb_; }
        constexpr sb_t sbColor(Color c)const{ return sb_t(sb_ & (0x5555 << c)); }
        constexpr sb_t sbSpace()const noexcept{ return sb_t(0xffff - sb_); }
        constexpr sb_t sbColorSpace(Color c)const{ return sb_t(sbSpace() & (0x5555 << c)); }
        
        stone_t& stone(uint32_t idx){ return st_[idx]; }
        stone_t& stone(Color c, uint32_t idx){ return st_[CItoTurn(c, idx)]; }
        
        const stone_t& stone(uint32_t idx)const{ return st_[idx]; }
        const stone_t& stone(Color c, uint32_t idx)const{ return st_[CItoTurn(c, idx)]; }
        
        // 石1個のときのアクセス
        stone_t& onlyStone()noexcept{ return st_[sb().bsf()]; }
        stone_t& onlyStone(Color c){ return st_[sbColor(c).bsf()]; }
        
        const stone_t& onlyStone()const noexcept{ return st_[sb().bsf()]; }
        const stone_t& onlyStone(Color c)const{ return st_[sbColor(c).bsf()]; }
        
        StoneState state(uint32_t idx)const{
            StoneState state;
            state.init();
            state.setIndex(idx);
            //if(isInFreeGuardZone(stone(idx).x, stone(idx).y)){
            //    state.setInFreeGuardZone();
            //}
            return state;
        }
        
        template<class st_t>
        void pushStone(Color c, const st_t& ast){
            // 1投目の分から詰めて置く
            sb_t tsb = sbColorSpace(c);
            ASSERT(tsb.any(),); // 石を置くことが可能
            uint32_t idx = tsb.bsr();
            stone(idx).set(ast.x, ast.y);
            sb_.set(idx);
        }
        template<class st_t>
        void locateStone(uint32_t idx, const st_t& ast){
            // 指定した番号の石を置く(前からその番号の石があるかもしれない)
            stone(idx).set(ast.x, ast.y);
            sb_.set(idx);
        }
        template<class st_t>
        void locateNewStone(uint32_t idx, const st_t& ast){
            // 指定した番号の新しい石を置く
            stone(idx).set(ast.x, ast.y);
            sb_.set(idx);
        }
        template<class st_t>
        void relocateStone(uint32_t idx, const st_t& ast){
            // すでに置かれていた石の位置を変更する
            ASSERT(sb().test(idx),);
            stone(idx).set(ast.x, ast.y);
        }
        void removeStone(uint32_t idx)noexcept{
            ASSERT(sb().test(idx),);
            sb_.reset(idx);
        }
        
        void clearStones()noexcept{ sb_.reset(); }
        void init()noexcept{ clearStones(); }
        
        std::array<stone_t, N_STONES> st_;
        sb_t sb_;
    };
    
    // No1ストーンを探す(色ごと、全体)
    int searchNo1Index(const MiniBoard& bd, Color c)noexcept{
        int minidx = -1;
        fpn_t mind2(10000);
        iterateStoneWithIndex(bd, c, [&minidx, &mind2](uint32_t idx, const auto& st)->void{
            double d2 = calcDistance2Tee(st);
            if(d2 < mind2){
                minidx = idx;
                mind2 = d2;
            }
        });
        return minidx;
    }
    
    int searchNo1Index(const MiniBoard& bd)noexcept{
        int minidx = -1;
        fpn_t mind2(10000);
        iterateStoneWithIndex(bd, [&minidx, &mind2](uint32_t idx, const auto& st)->void{
            double d2 = calcDistance2Tee(st);
            if(d2 < mind2){
                minidx = idx;
                mind2 = d2;
            }
        });
        return minidx;
    }
    
    // 全体No1ストーンの色
    template<typename callback_t>
    Color searchNo1Color(const MiniBoard& bd){
        return getColor(searchNo1Index(bd));
    }
    
    // エンド末端での得点計算
    int countScore(const MiniBoard& bd)noexcept{
        fpn_t bmin2(FR2_IN_HOUSE), wmin2(FR2_IN_HOUSE);
        std::array<fpn_t, N_STONES> r2a;
        iterateStoneWithIndex(bd, BLACK, [&bmin2, &r2a](uint32_t idx, const auto& st)->void{
            fpn_t r2 = calcDistance2Tee(st);
            r2a[idx] = r2;
            bmin2 = min(bmin2, r2);
        });
        iterateStoneWithIndex(bd, WHITE, [&wmin2, &r2a](uint32_t idx, const auto& st)->void{
            fpn_t r2 = calcDistance2Tee(st);
            r2a[idx] = r2;
            wmin2 = min(wmin2, r2);
        });
        if (bmin2 == wmin2){
            return 0;
        }else if (bmin2 > wmin2){
            int cnt = 0;
            iterateStoneWithIndex(bd, WHITE, [bmin2, &r2a, &cnt](uint32_t idx, const auto& st)->void{
                if (r2a[idx] < bmin2){ ++cnt; }
            });
            return -cnt;
        }else{
            int cnt = 0;
            iterateStoneWithIndex(bd, BLACK, [wmin2, &r2a, &cnt](uint32_t idx, const auto& st)->void{
                if (r2a[idx] < wmin2){ ++cnt; }
            });
            return +cnt;
        }
        UNREACHABLE;
    }
    
    int countScore(const MiniBoard& bd, Color c)noexcept{
        int sc = countScore(bd);
        if (isBlack(c)){
            return +sc;
        }
        else{
            return -sc;
        }
        UNREACHABLE;
    }
    
    Color searchNo1Color(const MiniBoard& bd)noexcept{
        fpn_t bmin2(10000), wmin2(10000);
        iterateStone(bd, BLACK, [&bmin2](const auto& st)->void{
            bmin2 = min(bmin2, calcDistance2Tee(st));
        });
        iterateStone(bd, WHITE, [&wmin2](const auto& st)->void{
            wmin2 = min(wmin2, calcDistance2Tee(st));
        });
        if (bmin2 > wmin2){
            return WHITE;
        }
        else{
            return BLACK;
        }
        UNREACHABLE;
    }
    
    std::ostream& operator<<(std::ostream& os, const MiniBoard& arg){
        os << "B";
        iterateStone(arg, BLACK, [&os](const auto& st)->void{ os << st; });
        os << "W";
        iterateStone(arg, WHITE, [&os](const auto& st)->void{ os << st; });
        return os;
    }
    
    /**************************衝突木**************************/
    
    // 石同士の衝突の流れをグラフとして保存
    // 簡略化のため、depth 0 ~ 3 まで保存
    // 最上位4bitを諸々の情報の保存に利用
    class ContactTree : public BitArray64<4, 15>{
    public:
        constexpr static uint64_t CHANGED_FLAG = (1ULL << 63); // 盤上に変化あり(draw含む)
        constexpr static uint64_t COMPLEX_FLAG = (1ULL << 61); // 複雑な衝突があったので記録が信用できない
        constexpr static uint64_t ERROR_FLAG = (1ULL << 60); // エラー
        
        constexpr static uint64_t PASS = 0ULL;
        constexpr static uint64_t PUT = CHANGED_FLAG;
        
        // depth, number から保存インデックスへの変換
        constexpr static unsigned int DNtoI(const unsigned int d, const unsigned int n)noexcept{
            return ((1U << d) - 1U) + n;
        }
        // 保存インデックスから深さへの変換
        static unsigned int ItoD(const unsigned int i)noexcept{
            return bsr(i + 1U);
        }
        // 保存インデックスからコンタクト番号への変換
        static unsigned int ItoN(const unsigned int i)noexcept{
            return i - DNtoI(ItoD(i), 0U);
        }
        // 現在の衝突インデックスから次の衝突インデックスへの変換
        constexpr static unsigned int ItoNextI(const unsigned int i)noexcept{
            return 2U * i + 1U;
        }
        
        constexpr ContactTree() : BitArray64<4, 15>(){}
        constexpr ContactTree(uint64_t arg) : BitArray64<4, 15>(arg){}
        
        // 性質
        constexpr bool isPass()const noexcept{ return !data(); }
        constexpr bool isFoul()const noexcept{
            // フリーガード違反
            return hasContact() && !(data() & CHANGED_FLAG);
        }
        
        constexpr bool isPassAct()const noexcept{
            // 結果としてパスとなった
            return !(data() & CHANGED_FLAG);
        }
        
        constexpr bool isPut()const noexcept{
            // 他の石に接触せずに盤上に置かれた
            return (data() == PUT);
        }
        
        constexpr uint64_t isChanged()const noexcept{ return (data() & CHANGED_FLAG); }
        constexpr uint64_t isComplex()const noexcept{ return (data() & COMPLEX_FLAG); }
        constexpr uint64_t isError()const noexcept{ return (data() & ERROR_FLAG); }
        
        constexpr bool hasContact()const noexcept{
            return (data() & (COMPLEX_FLAG | 0x0fffffffffffffffULL));
        }
        
        int countContacts()const noexcept{
            int cnt = 0;
            for(int i = 0; i < 15; ++i){
                if(any(i)){ ++cnt; }
            }
            return cnt;
        }
        
        void setChanged()noexcept{
            data() |= CHANGED_FLAG;
        }
        void setComplex()noexcept{
            data() |= COMPLEX_FLAG;
        }
        void setError()noexcept{
            data() |= ERROR_FLAG;
        }
        
        void setContact(unsigned int i, unsigned int num)noexcept{
            if(i < 16 - 1){
                set(i, num);
            }
        }
        
        std::string toString()const{
            std::ostringstream oss;
            oss << (*this);
            if(isChanged()){ oss << " -changed"; }
            if(isComplex()){ oss << " -complex"; }
            if(isError()){ oss << " -error"; }
            return oss.str();
        }
    };
    
    std::ostream& operator<<(std::ostream& ost, const ContactTree& ct){
        ost << ct.toString();
        return ost;
    }
    
    /**************************シミュレーション結果**************************/
    
    struct MovedStones{
        
        // 局面情報の差分更新のために、更新する必要のある石をまとめるためのクラス
        
        int added; // added な 石があれば
        StoneSet removed; // 除外された石の集合
        StoneSet relocated; // 移動した石があれば
        
        void setSdded(int index)noexcept{ added = index; }
        void setRemoved(const StoneSet& set)noexcept{ removed = set; }
        void setRelocated(const StoneSet& set)noexcept{ relocated = set; }
                                                       
        template<class callback_t>
        void iterateAdded(const callback_t& callback)const noexcept{
            if(added != -1){
                callback(added);
            }
        }
        
        template<class callback_t>
        void iterateRemoved(const callback_t& callback)const noexcept{
            iterate(removed, [&callback](const int index)->void{
                callback(index);
            });
        }
        
        template<class callback_t>
        void iterateRelocated(const callback_t& callback)const noexcept{
            iterate(relocated, [&callback](const int index)->void{
                callback(index);
            });
        }
    };
    
    struct SimulationInfo : public MovedStones{
        ContactTree tree_;
        
        ContactTree tree()const noexcept{ return tree_; }
    };

    /**************************局面+着手+結果ログ**************************/

    struct ShotLog{
        
        // プレーヤー名
        char player_[32];
        char oppPlayer_[32];

        // ルール
        int drawGame;
        float random;
        
        // 試合状況
        int end, turn, rscore, escore;
        
        uint32_t usedTime, restTime;

        fMoveXY<float> chosenMove, runMove; // 着手(提出時、エラー加算後)
        fPosXY<float> previousStone[N_TURNS], afterStone[N_TURNS]; // 石の位置(着手前後)
        
        const auto& stone(int i)const{ return previousStone[i]; }
        
        std::string player()const{ return std::string(player_); }
        std::string oppPlayer()const{ return std::string(oppPlayer_); }
        void setPlayer(const std::string& nm){
            snprintf(player_, 32, nm.c_str());
        }
        void setOppPlayer(const std::string& nm){
            snprintf(oppPlayer_, 32, nm.c_str());
        }
        
        void flip()noexcept{ // 左右の反転
            chosenMove.flip(); runMove.flip();
            for(int i = 0; i < N_TURNS; ++i){
                previousStone[i].flip();
                afterStone[i].flip();
            }
        }
        
        std::string toString()const{
            std::ostringstream oss;

            oss << player() << " " << oppPlayer() << " ";
            oss << drawGame << " " << random << " ";
            oss << end << " " << turn << " " << rscore << " " << escore << " ";
            oss << restTime << " " << usedTime << " ";
            oss << chosenMove.vx() << " " << chosenMove.vy() << " " << chosenMove.spin() << " ";
            oss << runMove.vx() << " " << runMove.vy() << " " << runMove.spin() << " ";

            for (int i = 0; i < N_TURNS; ++i){
                oss << previousStone[i].x << " " << previousStone[i].y << " ";
            }
            for (int i = 0; i < N_TURNS; ++i){
                oss << afterStone[i].x << " " << afterStone[i].y << " ";
            }
            return oss.str();
        }
    };
    
    int readShotLog(std::ifstream& ifs, ShotLog *const psl){
        std::string p, op;
        ifs >> p >> op;
        snprintf(psl->player_, 32, p.c_str());
        snprintf(psl->oppPlayer_, 32, p.c_str());
        ifs >> psl->drawGame >> psl->random;
        ifs >> psl->end >> psl->turn >> psl->rscore >> psl->escore;
        ifs >> psl->restTime >> psl->usedTime;
        int s;
        ifs >> psl->chosenMove.x >> psl->chosenMove.y >> s;
        psl->chosenMove.s = static_cast<Spin>(s);
        ifs >> psl->runMove.x >> psl->runMove.y >> s;
        psl->runMove.s = static_cast<Spin>(s);
        for (int i = 0; i < N_STONES; ++i){
            ifs >> psl->previousStone[i].x >> psl->previousStone[i].y;
        }
        for (int i = 0; i < N_STONES; ++i){
            ifs >> psl->afterStone[i].x >> psl->afterStone[i].y;
        }
        return 0;
    }
    
    template<class accessor_t, typename callback_t>
    int readShotLog(std::ifstream& ifs, accessor_t *const pdb,
                    const callback_t& callback = [](const auto& shot)->bool{ return true; }){
        typename accessor_t::value_type tmp;
        int games = 0;
        while(ifs){
            if(readShotLog(ifs, &tmp) < 0){
                return -1;
            }
            if(callback(tmp)){
                pdb->push_back(tmp);
            }
            if(tmp.end == END_LAST && tmp.turn == TURN_LAST){ ++games; }
        }
        return games;
    }
    
    template<class accessor_t, typename callback_t>
    int readShotLog(const std::string& fName, accessor_t *const pdb,
                    const callback_t& callback = [](const auto& shot)->bool{ return true; }){
        std::ifstream ifs;
        ifs.open(fName, std::ios::in);
        if(!ifs){ return -1; }
        return readShotLog(ifs, pdb, callback);
    }
}

#endif // DCURLING_DC_HPP_