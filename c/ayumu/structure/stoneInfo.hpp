/*
 stoneInfo.hpp
 Katsuki Ohto
 */

// デジタルカーリング
// 思考用石表現

#ifndef DCURLING_STONEINFO_HPP_
#define DCURLING_STONEINFO_HPP_

#include "../../dc.hpp"
#include "../shot/params.h"
#include "../logic/logic.hpp"

namespace DigitalCurling{
    
    /**************************思考用の石情報**************************/
    
    struct StoneInfo{
        using float_t = fpn_t;
        
        // 位置情報
        float_t x;
        float_t y;
        float_t r;
        float_t th;
        float_t tr; // 投げる点から見ての距離
        float_t tth; // 投げる点から見ての角度
        
        // 状態
        StoneState state;
        
        // 離散化した相対位置にいる石ビット
        BitArray64<16, 4> rpsb_[4];
        BitSet16 tailStone_;
        
        int baseRange; // 最低の局面認識レンジ
        
        fpn_t hitLimit_[2][2]; // ベース初速で投げた場合に衝突しうる投げる角度の限界
        
        void remove(uint32_t idx){
            // idx 番の石がなくなった際の主観的な情報の更新
            //frontGuard[0].reset(idx); frontGuard[1].reset(idx);
            //backGuard.reset(idx);
            //tailStone.reset(idx);
        }
        
        BitSet16 rpsb(unsigned int rp)const{
            return rpsb_[rp / 4][rp % 4];
        }
        
        bool anyGuard()const noexcept{
            return bool(rpsb_[0] | rpsb_[1] | rpsb_[2] | rpsb_[3]);
        }
        BitSet16 guard()const noexcept{
            return BitSet16(rpsb_[0].sum() | rpsb_[1].sum() | rpsb_[2].sum() | rpsb_[3].sum());
        }
        BitSet16 guard(int s)const noexcept{
            return frontGuard(s) | backGuard(s);
        }
        
        bool anyFrontGuard()const noexcept{
            return bool(rpsb_[0] | rpsb_[1]);
        }
        BitSet16 frontGuard()const noexcept{
            return BitSet16(rpsb_[0] | rpsb_[1]);
        }
        BitSet16 frontGuard(int s)const{
            return BitSet16(rpsb_[s].sum() | rpsb_[1 - s][0] | rpsb_[1 - s][2]);
        }
        bool anyBackGuard()const noexcept{
            return bool(rpsb_[2] | rpsb_[3]);
        }
        BitSet16 backGuard()const noexcept{
            return BitSet16(rpsb_[2].sum() | rpsb_[3].sum());
        }
        BitSet16 backGuard(int s)const{
            return BitSet16(rpsb_[2 + 1 - s].sum() | rpsb_[2 + s][0] | rpsb_[2 + s][2]);
        }
        
        BitSet16 tailStone()const noexcept{ return tailStone_; }
        
        void setRelPosSB(uint32_t idx, uint32_t z){
            // 石 idx がラベル z の相対位置にある
            rpsb_[z / 4] |= 1ULL << ((z % 4) * 16 + idx);
        }
        
        void clearRelPosSB()noexcept{
            for(int i = 0; i < 4; ++i){
                rpsb_[i].clear();
            }
        }
        void setHitLimit(){
            fpn_t h0 = hitLimitTable.at<0>(tr), h1 = hitLimitTable.at<1>(tr);
            hitLimit_[Spin::RIGHT][0] = tth + h0;
            hitLimit_[Spin::RIGHT][1] = tth + h1;
            hitLimit_[Spin::LEFT][0] = tth - h1;
            hitLimit_[Spin::LEFT][1] = tth - h0;
        }
        
        fpn_t hitLimit(int s, int lr)const{
            return hitLimit_[s][lr];
        }
        
        template<class pos_t>
        void set(const pos_t& apos){
            // set absolute value
            x = apos.x;
            y = apos.y;
            r = calcDistanceTee(apos);
            th = calcRelativeAngleTee(apos);
            tr = calcDistanceThrow(apos);
            tth = calcRelativeAngleThrow(apos);
            setHitLimit();
        }
        
        template<typename f_t>
        void set(f_t ax, f_t ay){
            //set absolute value
            x = ax;
            y = ay;
            r = XYtoR(ax - FX_TEE, ay - FY_TEE);
            th = XYtoT(ay - FY_TEE, ax - FX_TEE);
            tr = XYtoR(ax - FX_THROW, ay - FY_THROW);
            tth = XYtoT(ay - FY_THROW, ax - FX_THROW);
            setHitLimit();
        }
        
        template<class pos_t>
        explicit StoneInfo(const pos_t& apos){ set(apos); }
        
        StoneInfo(fpn_t ax, fpn_t ay){
            set(fPosXY<>(ax, ay));
        }
        
        StoneInfo(){}
        
        float_t getX()const noexcept{ return x; }
        float_t getY()const noexcept{ return y; }
        float_t getR()const noexcept{ return r; }
        float_t getTh()const noexcept{ return th; }
        float_t getTR()const noexcept{ return tr; }
        float_t getTTh()const noexcept{ return tth; }
        
        std::string toString()const{
            std::ostringstream oss;
            oss << "(" << x << "," << y << ")";
            return oss.str();
        }
        
        std::string toDebugString()const{
            std::ostringstream oss;
            oss << "(" << x << "," << y << ")";
            return oss.str();
        }
        
        bool exam_rpsb()const noexcept{
            // 相対位置ビットの排他性
            BitSet16 tmp(0);
            for(int i = 0; i < 16; ++i){
                if(!isExclusive(tmp.data(), rpsb(0).data())){
                    return false;
                }
            }
        }
        
        bool exam()const noexcept{
            if(!exam_rpsb()){ return false; }
        }
    };
    
    StoneInfo drawPositionInfo[DrawPos::MAX];
    
    int initStoneInfo(){
        for(int i = 0; i < DrawPos::MAX; ++i){
            realizeDrawPosition(&drawPositionInfo[i], i);
        }
        return 0;
    }
    
    std::ostream& operator<<(std::ostream& os, const StoneInfo& arg){
        os << arg.toString();
        return os;
    }
    
    bool isInHouse(const StoneInfo& si)noexcept{
        return isInHouse(si.getR());
    }
    bool isNearTee(const StoneInfo& si, double dist)noexcept{
        return isNearTee(si.getR(), dist);
    }
    template<int OUT_HOUSE = 0, int IN_PLAYAREA = 0>
    bool isInFreeGuardZone(const StoneInfo& si){
        if (!IN_PLAYAREA){
            if(!((static_cast<float_t>(FX_PA_MIN + FR_STONE_RAD) < si.x)
                 && (si.x < static_cast<float_t>(FX_PA_MAX - FR_STONE_RAD))
                 && (static_cast<float_t>(FY_RINK_MIN + FR_STONE_RAD) < si.y))){
                return false;
            }
        }else{
            assert(isInPlayArea(si.x, si.y));
        }
        if(!isThisSideOfTeeLine(si.y)){ return false; }
        if(OUT_HOUSE){
            assert(!isInHouse(si.r));
        }else{
            if (isInHouse(si.r)){ return false; } // すでにRが計算されているので
        }
        return true;
    }
    template<int IN_HOUSE = _BOTH>
    bool isInRollInZone(const StoneInfo& si){ // ダブルロールイン可能なゾーン
        if(IN_HOUSE == _YES){ return false; }
        if(IN_HOUSE == _BOTH){
            if(isInHouse(si)){ return false; }
        }
        return (si.y < FY_TEE) && XYtoR2(si.x - FX_TEE, (si.y - FY_TEE) / F_ROLLIN_RATE) < pow(FR_HOUSE_RAD + FR_STONE_RAD, 2);
    }
    template<int IN_HOUSE = _BOTH>
    bool isInActiveZone(const StoneInfo& si){ // 試合に関係ありそうなゾーン
        if(IN_HOUSE == _YES){ return true; }
        if(si.y < FY_TEE){
            return (fabs(si.x - FX_TEE) <= FR_ACTIVE_LINE);
        }else{
            return si.r <= FR_ACTIVE_LINE;
        }
    }
    
    template<>auto calcDistanceTee<StoneInfo>(const StoneInfo& si){ return si.r; }
    template<>auto calcDistance2Tee<StoneInfo>(const StoneInfo& si){ return si.r * si.r; }
    template<>auto calcRelativeAngleTee<StoneInfo>(const StoneInfo& si){ return si.th; }
    
    template<>auto calcDistanceThrow<StoneInfo>(const StoneInfo& si){ return si.tr; }
    template<>auto calcDistance2Throw<StoneInfo>(const StoneInfo& si){ return si.tr * si.tr; }
    template<>auto calcRelativeAngleThrow<StoneInfo>(const StoneInfo& si){ return si.tth; }
    
    template<>bool isMoreCentral<StoneInfo>(const StoneInfo& si0, const StoneInfo& si1){ return si0.getR() < si1.getR(); }
    template<>bool isMoreFrontal<StoneInfo>(const StoneInfo& si0, const StoneInfo& si1){ return si0.getY() < si1.getY(); }
    template<>bool isMoreLeft<StoneInfo>(const StoneInfo& si0, const StoneInfo& si1){ return si0.getX() < si1.getX(); }
    
    /**************************思考用の石の相対情報**************************/
    
    struct RelativeStoneInfo{
        using float_t = float;
        
        float_t r;
        float_t th;
        int rp; // 相対位置
        
        void set(const StoneInfo& base, const StoneInfo& obj){
            r = calcDistance(base, obj);
            th = calcRelativeAngle(base, obj);
            rp = labelRelPos(base, obj, *this);
        }
    };
    
    
}

#endif // DCURLING_STONEINFO_HPP_