// Digital Curling
// realizing verbal shots

#ifndef DCURLING_MOVE_REALIZER_HPP_
#define DCURLING_MOVE_REALIZER_HPP_

namespace DigitalCurling{
    
    // 手の生成のためのデータ
    
    int initStandardDrawTable(){
        // ドローテーブル初期化
        fPosXY<fpn_t> pos;
        fMoveXY<fpn_t> fmv;
        
        for (int n = 0; n < DrawPos::MAX; ++n){
            
            MoveXY vmv(0);
            realizeDrawPosition(&pos, n);
            
            for (int s = 0; s < 2; ++s){
                fmv.setSpin(static_cast<Spin>(s));
                FastSimulator::FXYtoFMV(pos, &fmv);
                drawTable[s][n].x = fmv.x;
                drawTable[s][n].y = fmv.y;
            }
        }
        return 0;
    }
    
    /**************************ノーレンジ着手の生成**************************/
    
    template<class move_t>
    void genStandardDraw(move_t *const pmv,int ipos){
        // 定跡系のドローショット
        int s = pmv->getSpin();
        pmv->x = drawTable[s][ipos].x;
        pmv->y = drawTable[s][ipos].y;
    }
    
    template<class pos_t, class move_t>
    void genStandardHit(move_t *const pmv, const pos_t& pos, int w, int free_spin = 1){
        // ヒット
        if (!free_spin){
            if (pos.getX() > FX_TEE){
                pmv->setSpin(Spin::RIGHT);
            }
            else{
                pmv->setSpin(Spin::LEFT);
            }
        }
        switch (w){
            case HitWeight::TOUCH:{
                FastSimulator::rotateToPassPointGoF(pmv, fPosXY<>(pos.getX(), pos.getY() - 0.015), 0.3);
            }break;
            case HitWeight::WEAK:{
                FastSimulator::rotateToPassPointGoF(pmv, fPosXY<>(pos.getX(), pos.getY() - 0.015), 1.7);
            }break;
            case HitWeight::MIDDLE:{
                FastSimulator::rotateToPassPointGoF(pmv, fPosXY<>(pos.getX(), pos.getY() - 0.015), 5.0);
            }break;
            case HitWeight::STRONG:{
                FastSimulator::rotateToPassPointF(pmv, fPosXY<>(pos.getX(), pos.getY() - 0.015), FV_MAX - 0.2);
            }break;
            default: UNREACHABLE; break;
        }
    }
    
    template<class move_t, class pos_t>
    void genFreeze(move_t *const pmv, const pos_t& pos, int p, int free_spin = 1){
        if (!free_spin){
            if (pos.getX() > FX_TEE){
                pmv->setSpin(Spin::RIGHT);
            }
            else{
                pmv->setSpin(Spin::LEFT);
            }
        }
        fPosXY<> dst;
        const auto x = pos.getX(), y = pos.getY();
        constexpr fpn_t ls = 2 * FR_STONE_RAD + 0.02;
        constexpr fpn_t ll = 3.5 * FR_STONE_RAD;
        constexpr fpn_t c45 = 1.0 / sqrt(2.0);
        constexpr fpn_t s45 = c45;
        constexpr fpn_t c225 = 1.0 / sqrt(4.0 - 2.0 * sqrt(2.0));
        constexpr fpn_t s225 = sqrt(1.0 - c225 * c225);
        
        switch(p){
            case 0: dst.set(x, y - ls); break;
            case 1: dst.set(x - s45 * ls, y - c45 * ls); break;
            case 2: dst.set(x + s45 * ls, y - c45 * ls); break;
            case 3: dst.set(x + s45 * ls, y - c45 * ls); break;
            case 4: dst.set(x, y - ll); break;
            case 5: dst.set(x - s225 * ll, y - c225  * ll); break;
            case 6: dst.set(x + s225 * ll, y - c225  * ll); break;
            case 7: dst.set(x - s45 * ll, y - c45 * ll); break;
            case 8: dst.set(x + s45 * ll, y - c45 * ll); break;
            default: UNREACHABLE; break;
        }
        genDraw(pmv, dst);
    }
    
    const fpn_t standardComeAroundLength[3] = {1, 2.2, 3.5};
    const fpn_t standardComeAroundY[1] = {FY_TEE};
    const fpn_t standardPostGuardLength[3] = {-1, -2, -3};
    
    /**************************着手型表現型から実着手型への変換**************************/
    
    template<class board_t, class move_t, class nrmove_t>
    void realizeMove(nrmove_t *const dst, const board_t& bd, const move_t& amv){
        // ある局面での言語手から実際の着手形式を生成
        // amvのノーレンジ性は保証されていると考えてよい
        
        if (!amv.isRelative()){
            // 絶対着手
            dst->setSpin(amv.getSpin());
            convMove(amv, dst);
        }
        else{
            // 相対着手
            Spin s = amv.getSpin();
            dst->setSpin(s);
            switch (amv.getType()){
                case Standard::PASS:{
                    genPASS(dst);
                }break;
                case Standard::DRAW: case Standard::PREGUARD:{
                    ASSERT(0 <= amv.getNum0() && amv.getNum0() < 16, cerr << amv.getNum0() << endl;);
                    fPosXY<fpn_t> *p = &drawTable[s][amv.getNum0()];
                    dst->x = p->x;
                    dst->y = p->y;
                }break;
                case Standard::FREEZE:{
                    int st = amv.getNum0(); // 狙う石
                    int p = amv.getNum1(); // 位置
                    genFreeze(dst, bd.stone(st), p, 1);
                }break;
                case Standard::HIT:{
                    int st = amv.getNum0(); // 狙う石
                    int w = amv.getNum1(); // 強さ
                    genStandardHit(dst, bd.stone(st), w, 1);
                }break;
                case Standard::COMEAROUND:{
                    int st = amv.getNum0(); // 狙う石
                    int l = amv.getNum1(); // 長さ
                    if(l < ComeAroundLength::TEE){
                        fpn_t fl = standardComeAroundLength[l];
                        genComeAround(dst, bd.stone(st), fl);
                    }else{
                        fpn_t fy = standardComeAroundY[l - ComeAroundLength::TEE];
                        genComeAroundByY(dst, bd.stone(st), fy);
                    }
                }break;
                case Standard::POSTGUARD:{
                    int st = amv.getNum0(); // 狙う石
                    int l = amv.getNum1(); // 長さ
                    genPostGuard(dst, bd.stone(st), standardPostGuardLength[l]);
                }break;
                case Standard::DOUBLE:{
                    int ifront = amv.getNum0(); // 狙う石1
                    int iback = amv.getNum1(); // 狙う石2
                    if(isMoreFrontal(bd.stone(iback), bd.stone(ifront))){
                        swap(ifront, iback);
                    }
                    gDouble.genReal2(dst, bd.stone(ifront), bd.stone(iback));
                }break;
                case Standard::RAISETAKEOUT:{
                    gRaiseTakeOut.realize(dst, bd, amv);
                }break;
                case Standard::PEEL:{
                    gPeel.realize(dst, bd, amv);
                }break;
                case Standard::DRAWRAISE:{
                    gDrawRaise.realize(dst, bd, amv);
                }break;
                case Standard::TAKEOUT:{
                    gTakeOut.realize(dst, bd, amv);
                }break;
                case Standard::L1DRAW:{
                    gL1Draw.realize(dst, bd, amv);
                }break;
                default: ASSERT(0, cerr << amv.getType() << endl;); break;
            }
        }
    }
    
    /*template<class board_t, class move_t, class rmove_t>
    int realizeRootMove(rmove_t *const dst, const board_t& bd, const move_t& amv){
        // ある局面での言語手から実際の着手形式を生成
        // ルート専用
        if (!amv.isRelative()){
            // 絶対着手
            dst->setSpin(amv.getSpin());
            convRMove(amv, dst);
        }
        else{
            // 相対着手
            int s = amv.getSpin();
            dst->setSpin(s);
            switch (amv.getType()){
                case Standard::PASS:{
                    genPASS(dst);
                }break;
                case Standard::DRAW:{
                    fPosXY<fpn_t> *p = &drawTable[s][amv.getNum0()];
                    dst->x = p->x;
                    dst->y = p->y;
                    if (amv.getNum0() == DrawPos::TEE){
                        dst->setRangeVXY(I16VtoFV(1 << RANGE_ROOT_TEESHOT));
                    }
                    else{
                        dst->setRangeVXY(I16VtoFV(1 << RANGE_ROOT_DRAW));
                    }
                }break;
                case Standard::HIT:{
                    int st = amv.getNum0(); // 狙う石
                    int w = amv.getNum1(); // 強さ
                    genStandardHit(dst, accessStone(bd, st), w, 1);
                    dst->setRangeVXY(I16VtoFV(1 << RANGE_ROOT_HIT));
                }break;
                case Standard::COMEAROUND:{
                    int st = amv.getNum0(); // 狙う石
                    int l = amv.getNum1(); // 長さ
                    genStandardComeAround(dst, accessStone(bd, st), l, 1);
                    dst->setRangeVXY(I16VtoFV(1 << RANGE_ROOT_COMEAROUND));
                }break;
                case Standard::POSTGUARD:{
                    int st = amv.getNum0(); // 狙う石
                    int l = amv.getNum1(); // 長さ
                    genStandardPostGuard(dst, accessStone(bd, st), l, 1);
                    dst->setRangeVXY(I16VtoFV(1 << RANGE_ROOT_POSTGUARD));
                }break;
                default: UNREACHABLE; break;
            }
        }
        return 0;
    }*/
    
}

#endif // DCURLING_MOVE_REALIZER_HPP_