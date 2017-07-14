/*
 error.hpp
 Katsuki Ohto
 */

// デジタルカーリング
// ストーンの速度ベクトルにかかる外乱について検討

#ifndef DCURLING_SIMULATION_ERROR_HPP_
#define DCURLING_SIMULATION_ERROR_HPP_

namespace DigitalCurling{
    
    //struct fRandXY{fpn_t x,y;};
    //fRandXY randToMoveXYTable[1<<14];
    
    /**************************(X, Y)空間でのエラー乱数の確率密度関数(概念式)**************************/
    
    // 公式ではティーへのドローのときのみ正確にこの分布となる
    
    constexpr fpn_t pdfTeeDeltaXY(fpn_t dx, fpn_t dy)noexcept{
        return (1 / (2 * M_PI * pow(F_ERROR_SIGMA, 2)))
        * exp(-XYtoR2(dx / F_ERROR_SCALE_X, dy / F_ERROR_SCALE_Y) / (2 * pow(F_ERROR_SIGMA, 2)));
    }
    
    constexpr fpn_t relativePdfTeeDeltaXY(fpn_t dx, fpn_t dy)noexcept{
        return exp(-XYtoR2(dx / F_ERROR_SCALE_X, dy / F_ERROR_SCALE_Y) / (2 * pow(F_ERROR_SIGMA, 2)));
    }
    
    /**************************(Vx, Vy)空間でのエラー乱数の確率密度関数(近似式)**************************/
    
    fpn_t pdfDeltaVxVy(fpn_t dvx, fpn_t dvy, Spin spin)noexcept{
        constexpr fpn_t X = FX_TEE - FX_THROW + 1.22f;
        constexpr fpn_t Y = FY_TEE - FY_THROW;
        constexpr fpn_t X2_Y2 = X * X + Y * Y;
        //constexpr fpn_t X2_2Y2 = X*X + 2 * Y*Y;
        //constexpr fpn_t _2X2_mXY_2Y2 = 2 * X*X + -X*Y + 2 * Y*Y;
        
        //jacobian
        
        
        /*cerr << sqrt(2 * F_FRIC_RINK_STONE
         * (pow(X2_2Y2, 2) + pow(_2X2_mXY_2Y2, 2)))
         / 2 / X2_Y2 << endl;*/
        
        return 2 * (1 / (2 * M_PI * pow(F_ERROR_SIGMA, 2))) * (sqrt(X2_Y2) / (2 * F_FRIC_RINK_STONE))
        * exp(-XYtoR2(dvx * (1 / F_ERROR_SCALE_VX),
                      dvy * (2 / F_ERROR_SCALE_VY))
              / (2 * pow(F_ERROR_SIGMA, 2) / (sqrt(X2_Y2) / (2 * F_FRIC_RINK_STONE))));
    }
    
    fpn_t relativePdfDeltaVxVy(fpn_t dvx, fpn_t dvy, Spin spin)noexcept{
        
        // 格子点から積分するときに使う式
        
        constexpr fpn_t X = FX_TEE - FX_THROW + 1.22f;
        constexpr fpn_t Y = FY_TEE - FY_THROW;
        constexpr fpn_t X2_Y2 = X * X + Y * Y;
        return exp(-XYtoR2(dvx * (1 / F_ERROR_SCALE_VX),
                           dvy * (2 / F_ERROR_SCALE_VY))
                   / (2 * pow(F_ERROR_SIGMA, 2) / (sqrt(X2_Y2) / (2 * F_FRIC_RINK_STONE))));
    }
    
    class ApproximateErrorGenerator{
    private:
        constexpr static fpn_t X = FX_TEE - FX_THROW + 1.22f;
        constexpr static fpn_t Y = FY_TEE - FY_THROW;
        constexpr static fpn_t X2_Y2 = X * X + Y * Y;
        
        // ./constexpr static fpn_t SIGMA_ = F_ERROR_SIGMA *
        
    public:
        static fpn_t sigmaDeltaVx()noexcept{ return 0.0588636; }
        static fpn_t sigmaDeltaVy()noexcept{ return 0.117789; }
        
        template<class dice_t>
        static void genDeltaVxVy(fpn_t *const pdvx, fpn_t *const pdvy, Spin spin, dice_t *const pdice){
            NormalDistribution<fpn_t> norm(0, 1); // 正規分布
            fpn_t dvx, dvy;
            norm.rand(&dvx, &dvy, pdice);
            
        }
        static void addDeltaVxVy(){
            
        }
        
        static fpn_t pdfDeltaVxVy(fpn_t dvx, fpn_t dvy, Spin spin){
            return 2 * (1 / (2 * M_PI * pow(F_ERROR_SIGMA, 2))) * (sqrt(X2_Y2) / (2 * F_FRIC_RINK_STONE))
            * exp(-XYtoR2(dvx * (1 / F_ERROR_SCALE_VX), dvy * (2 / F_ERROR_SCALE_VY)) / (2 * pow(F_ERROR_SIGMA, 2) / (sqrt(X2_Y2) / (2 * F_FRIC_RINK_STONE))));
        }
        
    };
    
    /**************************(X, Y)空間でのエラー乱数の発生(概念式)**************************/
    
    template<class dice_t>
    void genRandToPosXY(fpn_t *const pdx, fpn_t *const pdy, dice_t *const pdice){
        NormalDistribution<fpn_t> norm(0, F_ERROR_SIGMA); // 正規分布
        norm.rand(pdx, pdy, pdice);
        *pdx *= F_ERROR_SCALE_X;
        *pdy *= F_ERROR_SCALE_Y;
    }
    
    template<class pos_t, class dice_t>
    void addRandToPos(pos_t *const pos, dice_t *const pdice){
        // 着手到達位置ベクトルに正規乱数をかける
        fpn_t dx, dy;
        genRandToPosXY(&dx, &dy, pdice);
        pos->addX(dx);
        pos->addY(dy);
    }
    
    /**************************(Vx, Vy)空間でのエラー乱数の発生(公式)**************************/
    
    class OfficialErrorGenerator{
        
        template<class dice_t>
        void genVxVy(fpn_t *const pdvx, fpn_t *const pdvy, Spin spin, dice_t *const pdice){
            
        }
        void addVxVy(){
            
        }
        
    };
    
    template<class dice_t>
    void genRandToMoveXY(fpn_t *const pdvx, fpn_t *const pdvy, Spin spin, dice_t *const pdice){
        
        // 実際に乱数をかけてみる操作
        // 公式と同じ式を使う
        
        NormalDistribution<fpn_t> norm(0, F_ERROR_SIGMA); // 正規分布
        
        fPosXY<> pos;
        fMoveXY<> shot;
        
        fpn_t a, b;
        norm.rand(&a, &b, pdice);
        
        pos.x = FX_TEE + a;
        pos.y = FY_TEE + b;
        
        shot.setSpin(spin);
        
        Simulator::calcDeparture(pos, &shot);
        
        *pdvx = (FVX_TEESHOT_OFFICIAL[spin] - shot.x) * sqrt(F_ERROR_SCALE_X);
        *pdvy = (FVY_TEESHOT_OFFICIAL - shot.y) * sqrt(F_ERROR_SCALE_Y);
        
        //constexpr fpn_t X = FX_TEE - FX_THROW - 1.22f;
        //constexpr fpn_t Y = FY_TEE - FY_THROW;
        
        //cerr << sqrt(2 * F_FRIC_RINK_STONE)*(
        //    - ((X + a) / pow(pow(X + a, 2) + pow(Y + b, 2), 0.25))
        //    + (X / pow(pow(X, 2) + pow(Y, 2), 0.25))) << endl;
        
        //cerr << XYtoR(*dvx, *dvy) / XYtoR(a, b) << endl;
    }
    
    template<class move_t, class dice_t>
    void addRandToMove(move_t *const pmv, dice_t *const pdice){
        // 着手直交速度ベクトルにエラー乱数をかける
        fpn_t dvx, dvy;
        genRandToMoveXY(&dvx, &dvy, pmv->spin(), pdice);
        pmv->addVX(dvx);
        pmv->addVY(dvy);
    }
    
    template<class move_t, class dice_t>
    void addRandToMove(move_t *const dst, const move_t& arg, dice_t *const pdice){
        // 着手直交速度ベクトルに正規乱数をかける
        fpn_t dvx, dvy;
        genRandToMoveXY(&dvx, &dvy, arg.spin(), pdice);
        dst->x = arg.x + dvx;
        dst->y = arg.y + dvy;
        dst->setSpin(arg.spin());
    }
    
    template<class dice_t>
    void initError(dice_t *const pdice){
        //for(int i=0;i<(1<<14);++i){
        //genRandToMoveXY(&randToMoveXYTable[i].x,&randToMoveXYTable[i].y,pdice);
        //}
    }
}

#endif // DCURLING_SIMULATION_ERROR_HPP_