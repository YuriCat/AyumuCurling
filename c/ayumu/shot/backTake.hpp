//デジタルカーリング
//フロントガードを避けてテイクアウト

namespace DigitalCurling{
    struct FrontTakeOut : public ShotGenerator{
        
        constexpr static int dimension(){
            return 2;
        }
        constexpr static int flexibility(){
            return 1;
        }
        constexpr static bool hasContact(){
            return true;
        }
        
        constexpr static int RESOLUTION_R=64;
        constexpr static int RESOLUTION_TH=32;
        
        constexpr static int RESOLUTION_HASU=1<<12;
        
        constexpr static fpn_t FR_MIN = 2 * FR_STONE_RAD+0.1;
        constexpr static fpn_t FR_MAX = 10 * FR_STONE_RAD+0.1;
        constexpr static fpn_t FTH_MIN = -M_PI/2;
        constexpr static fpn_t FTH_MAX = +M_PI/2;
        
        static uint32_t FRtoID(fpn_t fr){
            return (uint32_t)((int)((fr - FR_MIN)*((RESOLUTION_R-1)*RESOLUTION_HASU)) / (FR_MAX - FR_MIN),0);
        }
        static uint32_t FThtoID(fpn_t fth){
            return (uint32_t)((fth - FTH_MIN)*((RESOLUTION_TH-1)*RESOLUTION_HASU) / (FTH_MAX - FTH_MIN));
        }
        
        double evaluateMin()const{ return -0.000000001; }
        double evaluateMax()const{ return +1.150000001; }
        
        template<class board_t>
        double evaluateMin(const board_t& bd)const{ return -0.000000001; }
        
        template<class board_t>
        double evaluateMax(const board_t& bd)const{ return +1.150000001; }
        
        template<class board0_t,class board1_t>
        double evaluate(const board0_t&,const board1_t&,Color);
        
        template<class move_t,class pos_t>
        int generate2(move_t *const pmv,const pos_t& front,const pos_t& back);
        
        template<class dice_t>
        void setSampleState(MinBoard *const pbd,dice_t *const pdice);
        
        void init();
        
        FrontTakeOut(){
            init();
        }
    };
    
    float ftoTable[FrontTakeOut::RESOLUTION_R*FrontTakeOut::RESOLUTION_TH*2][2];
    
    void FrontTakeOut::init(){
        float tmpFtoTable[FrontTakeOut::RESOLUTION_R*FrontTakeOut::RESOLUTION_TH*2][2];
        std::ifstream ifs(DIRECTORY_PARAMS_IN+"param_fronttake.dat");
        for(int i=0;ifs && i<FrontTakeOut::RESOLUTION_R*FrontTakeOut::RESOLUTION_TH*2;++i){
            for(int j=0;ifs && j<2;++j){
                ifs>>tmpFtoTable[i][j];
            }
        }
        memcpy(ftoTable,tmpFtoTable,sizeof(ftoTable));
    }
    
    template<class board0_t,class board1_t>
    double FrontTakeOut::evaluate(const board0_t& pre,const board1_t& post,Color col){
        //ハウス内の相手の石の増加 -1.0点
        //ハウス内の自分の石の増加 +0.15点
        double ret =
        + max(-1, min(0, countDiffNInHouse(pre, post,flipColor(col)))) * (-1.0)
        + max(0, min(1, countDiffNInPlayArea(pre, post, flipColor(col)))) * (+0.15);
        
        assert(evaluateMin(pre)<=ret && ret<=evaluateMax(pre));
        return ret;
    }
    
    template<class move_t,class pos_t>
    int FrontTakeOut::generate2(move_t *const pmv,const pos_t& front,const pos_t& back){
        //石の位置関係によっては狙えない可能性がある
        //CERR<<front<<back<<endl;
        
        constexpr int ID_SPIN_BAND=1;
        constexpr int ID_IR_BAND=RESOLUTION_TH*2;
        constexpr int ID_ITH_BAND=2;
        
        fpn_t r=calcDistance(front,back);
        if( r < FR_MAX ){
            fpn_t th=calcRelativeAngle(front,back);
            
            int ir=FRtoID(r);
            int ith=FThtoID(th);
            
            int idBase=((ir/RESOLUTION_HASU)*RESOLUTION_TH+(ith/RESOLUTION_HASU))*2+0;
            int hasu[2]={ir%RESOLUTION_HASU,ith%RESOLUTION_HASU,};
            double invDist[4]={
                1/sqrt((hasu[0])*(hasu[0])+(hasu[1])*(hasu[1])),
                1/sqrt((hasu[0])*(hasu[0])+(RESOLUTION_HASU-hasu[1])*(RESOLUTION_HASU-hasu[1])),
                1/sqrt((RESOLUTION_HASU-hasu[0])*(RESOLUTION_HASU-hasu[0])+(hasu[1])*(hasu[1])),
                1/sqrt((RESOLUTION_HASU-hasu[0])*(RESOLUTION_HASU-hasu[0])+(RESOLUTION_HASU-hasu[1])*(RESOLUTION_HASU-hasu[1])),
            };
            double invDistSum=0;
            for(int i=0;i<4;++i){ invDistSum+=invDist[i]; }
            
            
            double sc[2]={
                (
                 ftoTable[idBase][1]*invDist[0]
                 +ftoTable[idBase+ID_ITH_BAND][1]*invDist[1]
                 +ftoTable[idBase+ID_IR_BAND][1]*invDist[2]
                 +ftoTable[idBase+ID_IR_BAND+ID_ITH_BAND][1]*invDist[3]
                 )/invDistSum,
                (
                 ftoTable[idBase+ID_SPIN_BAND][1]*invDist[0]
                 +ftoTable[idBase+ID_SPIN_BAND+ID_ITH_BAND][1]*invDist[1]
                 +ftoTable[idBase+ID_SPIN_BAND+ID_IR_BAND][1]*invDist[2]
                 +ftoTable[idBase+ID_SPIN_BAND+ID_IR_BAND+ID_ITH_BAND][1]*invDist[3]
                 )/invDistSum,
            };
            
            int s=(sc[0]>=sc[1])?0:1;
            
            //if(sc[s]>-1.1){//予測成功率高い
            idBase+=s*ID_SPIN_BAND;
            double hth=(
                        ftoTable[idBase][0]*invDist[0]
                        +ftoTable[idBase+ID_ITH_BAND][0]*invDist[1]
                        +ftoTable[idBase+ID_IR_BAND][0]*invDist[2]
                        +ftoTable[idBase+ID_IR_BAND+ID_ITH_BAND][0]*invDist[3]
                        )/invDistSum;
            //CERR<<"dto!"<<endl;
            FastSimulator::rotateToPassPointCircumferenceF(pmv, front, FV_MAX - 0.5, hth);
            
            assert( isValidMove(*pmv) );
            
            return 0;
            
            //}
        }
        //No1を狙う
        const pos_t& no1=front;
        genHit(pmv, no1, HitWeight::MIDDLE, 1);
        return 0;
    }
    
    template<class dice_t>
    void FrontTakeOut::setSampleState(MinBoard *const pbd,dice_t *const pdice){
        do{
            putVecRandInHouse(&pbd->at(BLACK),1,pdice);
            fpn_t r,th,x,y;
            r=pdice->drand()*(FR_MAX-FR_MIN)+FR_MIN;
            th=pdice->drand()*(FTH_MAX-FTH_MIN)+FTH_MIN;
            x=accessStone(*pbd,BLACK,0).getX() + r*sin(th);
            y=accessStone(*pbd,BLACK,0).getY() + r*cos(th);
            pbd->at(WHITE).push_back(fPosXY<>(x,y));
        }while(!isInHouse(pbd->at(WHITE).back()));
    }
    
    template<class dice_t>
    void initFrontTakeOut(dice_t *const pdice){
        
        FrontTakeOut strategy;
        
        float tmpFtoTable[FrontTakeOut::RESOLUTION_R*FrontTakeOut::RESOLUTION_TH*2][2];
        
        //テーブル作り
        cerr << "ftoTable = {"<<endl;
        for (int ir = 0; ir < FrontTakeOut::RESOLUTION_R; ++ir){
            for (int ith = 0; ith < FrontTakeOut::RESOLUTION_TH; ++ith){
                fpn_t fr = FrontTakeOut::FR_MIN + ir*(FrontTakeOut::FR_MAX - FrontTakeOut::FR_MIN) / (FrontTakeOut::RESOLUTION_R-1);
                fpn_t fth = FrontTakeOut::FTH_MIN + ith*(FrontTakeOut::FTH_MAX - FrontTakeOut::FTH_MIN) / (FrontTakeOut::RESOLUTION_TH-1);
                
                tuple<float, float> best;
                
                for (int s = 0; s < 2; ++s){
                    
                    array<pair<double, double>, FrontTakeOut::flexibility()> arr;
                    arr[0] = pair<double, double>(M_PI / 2, 3 * M_PI / 2);
                    pair<double, double> rew(-2.0, 0.0);
                    
                    Solver<1> solver(arr, rew);
                    for (int t = 0; t < 200000; ++t){
                        MinBoard bd;
                        while (1){
                            putVecRandInHouse(&bd[BLACK], 1, pdice);
                            fPosXY<> back = fPosXY<>(bd[BLACK][0].x + fr*sin(fth), bd[BLACK][0].y + fr*cos(fth));
                            if (isInHouse(back)){
                                bd[BLACK].push_back(back);
                                break;
                            }
                            bd[BLACK].clear();
                        }
                        fMoveXY<> mv;
                        mv.setSpin(s);
                        
                        auto smp = solver.play();
                        
                        FastSimulator::rotateToPassPointCircumferenceF(&mv, bd[BLACK][0], FV_MAX - 0.5, smp[0]);
                        
                        //CERR<<OutMinBoard(bd)<<"->"<<endl;
                        MinBoard tbd=bd;
                        makeMove(&tbd, WHITE, mv, pdice);
                        //CERR<<OutMinBoard(bd)<<endl;
                        
                        double ev = strategy.evaluate(bd,tbd,WHITE);
                        //CERR<<ev<<endl;
                        solver.feed(smp, ev);
                        //CERR<<smp[0]<<endl;
                    }
                    
                    auto ans=solver.answer();
                    //CERR<<ans[0]<<" "<<solver.mean()<<endl;
                    
                    best = tuple<float, float>(ans[0], solver.mean());
                    
                    int id=(ir*FrontTakeOut::RESOLUTION_TH+ith)*2+s;
                    tmpFtoTable[id][0] = get<0>(best);
                    tmpFtoTable[id][1] = get<1>(best);
                    cerr << "{" << get<0>(best) << "," << get<1>(best) << "}," << endl;
                }
            }
        }
        cerr << "}" << endl;
        
        std::ofstream ofs(DIRECTORY_PARAMS+"param_fronttake.dat");
        for(int i=0;i<FrontTakeOut::RESOLUTION_R*FrontTakeOut::RESOLUTION_TH*2;++i){
            for(int j=0;j<2;++j){
                ofs<<tmpFtoTable[i][j]<<" ";
            }
            ofs<<std::endl;
        }
    }
}
