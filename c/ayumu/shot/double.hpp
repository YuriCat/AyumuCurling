/*
 double.hpp
 Katsuki Ohto
 */

// デジタルカーリング
// double-takeout

#ifndef DCURLING_SHOT_DOUBLE_HPP_
#define DCURLING_SHOT_DOUBLE_HPP_

#include "shotGenerator.hpp"

namespace DigitalCurling{
    
    struct Double : public ShotGenerator{
        
        constexpr static int number()noexcept{ return Standard::DOUBLE; }
        constexpr static int dimension()noexcept{ return 2; }
        constexpr static int flexibility()noexcept{ return 1; }
        constexpr static bool hasContact()noexcept{ return true; }
        constexpr static double baseSpeed()noexcept{ return FV_MAX - 0.5; }
        
        constexpr static int64_t RESOLUTION_R = 32;
        constexpr static int64_t RESOLUTION_TH = 32;
        
        constexpr static int64_t RESOLUTION_HASU = 1LL << 6;
        
        constexpr static fpn_t FR_MIN = FR_TAIL_RADIUS_MIN;
        constexpr static fpn_t FR_MAX = FR_TAIL_RADIUS_MAX;
        constexpr static fpn_t FTH_MIN = -M_PI / 2;
        constexpr static fpn_t FTH_MAX = +M_PI / 2;
        
        constexpr static fpn_t FR2_MAX = FR_MAX * FR_MAX;
        
        static int64_t FRtoID(fpn_t fr){
            return max((int64_t)((fr - FR_MIN) * ((RESOLUTION_R - 1) * RESOLUTION_HASU) / (FR_MAX - FR_MIN)), (int64_t)0);
        }
        static int64_t FThtoID(fpn_t fth){
            return max((int64_t)((fth - FTH_MIN) * ((RESOLUTION_TH - 1) * RESOLUTION_HASU) / (FTH_MAX - FTH_MIN)), (int64_t)0);
        }
        
        template<class board_t, class vmove_t>
        double estimate(const board_t& bd, uint32_t stNum, const vmove_t& vmv){
            uint32_t obj0 = vmv.getNum0(), obj1 = vmv.getNum1();
            return 0.0;
        }
        
        template<class board0_t, class board1_t, class vmove_t>
        double evaluate(const board0_t& pre, const board1_t& post, uint32_t stNum, const vmove_t& vmv, const ContactTree& ct = 0)const{
            // テイクアウトしたい石のそれぞれに対して、
            // アクティブゾーン外に行った... + 0.5
            // ハウスからは出た... + 0.45
            const uint32_t obj0 = vmv.getNum0(), obj1 = vmv.getNum1(); // テイクアウトしたい石
            double ev = ((!post.sb().test(obj0) || !isInActiveZone(post.stone(obj0))) ? 0.5 : ((!isInHouse(post.stone(obj0))) ? 0.45 : 0))
            + ((!post.sb().test(obj1) || !isInActiveZone(post.stone(obj1))) ? 0.5 : ((!isInHouse(post.stone(obj1))) ? 0.45 : 0));
            return ev;
        }
        
        template<class move_t, class pos_t>
        double genReal2(move_t *const pmv, const pos_t& front, const pos_t& back);
        
        template<class move_t, class board_t, class vmove_t>
        void realize(move_t *const pmv, const board_t& bd ,const vmove_t& vmv){
            genReal2(pmv, bd.stone(vmv.getNum0()), bd.stone(vmv.getNum1()));
        }
        
        template<class board_t, class dice_t>
        void setSampleState(board_t *const pbd, dice_t *const pdice);
        
        template<class board_t>
        BitArray64<4, 16> orderRefStoneIndex(const board_t& bd, BitArray64<4, 16> index);
    };
    
    double doubleTable[Double::RESOLUTION_R * Double::RESOLUTION_TH * 2][2];
    
    int initDoubleTable(const std::string& path){
        double tmpDoubleTable[Double::RESOLUTION_R * Double::RESOLUTION_TH * 2][2];
        std::ifstream ifs(path + "param_double.dat");
        if(!ifs){ cerr << "failed to import param_double.dat!" << endl; return -1; }
        for (int i = 0; ifs && i < Double::RESOLUTION_R * Double::RESOLUTION_TH * 2; ++i){
            for (int j = 0; ifs && j < 2; ++j){
                ifs >> tmpDoubleTable[i][j];
                //cerr << tmpDoubleTable[i][j] << endl;
            }
        }
        memcpy(doubleTable, tmpDoubleTable, sizeof(doubleTable));
        CERR << "imported double table." << endl;
        return 0;
    }
    
    template<class move_t, class pos_t>
    double Double::genReal2(move_t *const pmv, const pos_t& front, const pos_t& back){
        // 石の位置関係によっては狙えない可能性がある
        DERR << shotString[number()] << " - FrontStone : " << front << " : BackStone : " << back << endl;
        //CERR<<front<<back<<endl;
        
        constexpr int ID_SPIN_BAND = 1;
        constexpr int ID_IR_BAND = RESOLUTION_TH * 2;
        constexpr int ID_ITH_BAND = 2;
        
        //cerr<<"front = "<<front<<" back = "<<back<<endl;
        fpn_t r = calcDistance(front, back);
        
        if (r < FR_MAX){
            fpn_t th = calcRelativeAngle(front, back);
            
            //cerr << "r = " << r << " th = " << th << endl;
            
            int64_t ir = FRtoID(r);
            int64_t ith = FThtoID(th);
            
            //cerr << "ir = " << ir << " ith = " << ith << endl;
            
            int64_t idBase = ((ir / RESOLUTION_HASU) * RESOLUTION_TH + (ith / RESOLUTION_HASU)) * 2 + 0;
            int64_t hasu[2] = { ir % RESOLUTION_HASU, ith % RESOLUTION_HASU, };
            
            double sc[2] = {
                (doubleTable[idBase][1] * (RESOLUTION_HASU - hasu[0]) * (RESOLUTION_HASU - hasu[1])
                 + doubleTable[idBase + ID_ITH_BAND][1] * (RESOLUTION_HASU - hasu[0]) * hasu[1]
                 + doubleTable[idBase + ID_IR_BAND][1] * hasu[0] * (RESOLUTION_HASU -  hasu[1])
                 + doubleTable[idBase + ID_IR_BAND + ID_ITH_BAND][1] * hasu[0] * hasu[1]
                 ) / (RESOLUTION_HASU * RESOLUTION_HASU),
                (doubleTable[idBase + ID_SPIN_BAND][1] * (RESOLUTION_HASU - hasu[0]) * (RESOLUTION_HASU - hasu[1])
                 + doubleTable[idBase + ID_SPIN_BAND + ID_ITH_BAND][1] * (RESOLUTION_HASU - hasu[0]) * hasu[1]
                 + doubleTable[idBase + ID_SPIN_BAND + ID_IR_BAND][1] * hasu[0] * (RESOLUTION_HASU -  hasu[1])
                 + doubleTable[idBase + ID_SPIN_BAND + ID_IR_BAND + ID_ITH_BAND][1] * hasu[0] * hasu[1]
                 ) / (RESOLUTION_HASU * RESOLUTION_HASU)
            };
            int s = (sc[0] >= sc[1]) ? 0 : 1;
            //cerr << sc[s] << endl;
            if (sc[s] > -1.1){ // 予測成功率高い
                idBase += s * ID_SPIN_BAND;
                double hth = (doubleTable[idBase][0] * (RESOLUTION_HASU - hasu[0]) * (RESOLUTION_HASU - hasu[1])
                              + doubleTable[idBase + ID_ITH_BAND][0] * (RESOLUTION_HASU - hasu[0]) * hasu[1]
                              + doubleTable[idBase + ID_IR_BAND][0] * hasu[0] * (RESOLUTION_HASU - hasu[1])
                              + doubleTable[idBase + ID_IR_BAND + ID_ITH_BAND][0] * hasu[0] * hasu[1]
                              ) / (RESOLUTION_HASU * RESOLUTION_HASU);
                //CERR<<"dto!"<<endl;
                
                //cerr << front << " " << hth << endl;
                
                FastSimulator::rotateToPassPointCircumferenceF(pmv, front, Double::baseSpeed(), hth);
                //cerr << front << " " << *pmv << endl;
                
                assert(isValidMove(*pmv));
                
                return (sc[s] + 2) / 2;
                
            }
        }
        // No1を狙う
        genHit(pmv, isMoreCentral(front, back) ? front : back, baseSpeed());
        return -1;
    }
    
    template<class board_t, class dice_t>
    void Double::setSampleState(board_t *const pbd, dice_t *const pdice){
        while (1){
            fPosXY<> ref, tail;
            fpn_t r, th;
            locateInHouse(&ref, pdice);
            r = FR_MIN + (FR_MAX - FR_MIN) * pdice->drand();
            th = -M_PI + pdice->drand() * 2 * M_PI;
            tail.set(ref.x + r * sin(th), ref.y + r * cos(th));
            if (isInHouse(tail)){
                pbd->pushStone(BLACK, ref);
                pbd->pushStone(BLACK, tail);
                break;
            }
        }
    }
    
    template<class board_t>
    BitArray64<4, 16> Double::orderRefStoneIndex(const board_t& bd, BitArray64<4, 16> index){
        if (!isMoreFrontal(bd.stone(index[0]), bd.stone(index[1]))){
            index.swap(0, 1);
        }
        return index;
    }
}

/*
 template<class dice_t>
 void genDoubleTableThread(int threadId, int threads, float tmpDoubleTable[][2], dice_t *const pdice){
 
 using std::array;
 
 Double strategy;
 
 //テーブル作り
 for (int ir = threadId; ir < Double::RESOLUTION_R; ir += threads){
 for (int ith = 0; ith < Double::RESOLUTION_TH; ++ith){
 const fpn_t fr = Double::FR_MIN + ir*(Double::FR_MAX - Double::FR_MIN) / (Double::RESOLUTION_R - 1);
 const fpn_t fth = Double::FTH_MIN + ith*(Double::FTH_MAX - Double::FTH_MIN) / (Double::RESOLUTION_TH - 1);
 
 tuple<float, float> best;
 
 for (int s = 0; s < 2; ++s){
 
 array<array<double, 2>, Double::flexibility()> arr;
 arr[0] = { M_PI / 2, 3 * M_PI / 2 };
 array<double, 2> rew = { 0, 1 };
 
 //MCTSSolver<1> solver(arr, rew);
 UniformGridSolver<1> gs;
 gs.setGridSize({128});
 
 evalAllGrids([&](const auto& var, const auto& index)->double{
 MiniBoard bd;
 fPosXY<> ref, tail;
 fpn_t ftth;
 
 // サンプル局面を生成
 while (1){
 locateInHouse(&ref, pdice);
 ftth = calcRelativeAngleThrow(ref);
 tail.x = ref.x + fr * sin(ftth + fth);
 tail.y = ref.y + fr * cos(ftth + fth);
 if (isInHouse(ref)){ break; }
 }
 
 bd.pushStone(BLACK, ref);
 bd.pushStone(BLACK, tail);
 
 fMoveXY<> mv;
 mv.setSpin(s);
 
 FastSimulator::rotateToPassPointCircumferenceF(&mv, ref, Double::baseSpeed(), var[0] + ftth);
 
 //CERR<<OutMinBoard(bd)<<"->"<<endl;
 MiniBoard tbd = bd;
 makeMove<0>(&tbd, TURN_WHITE_LAST, mv, pdice);
 //CERR<<OutMinBoard(bd)<<endl;
 MoveXY vmv;
 vmv.setDouble(s, 15, 13);
 return strategy.evaluate(bd, tbd, TURN_WHITE_LAST, vmv);
 }
 
 auto ans = gs.getBestIntegratedGridVariable([]()
 //CERR<<ans[0]<<" "<<solver.mean()<<endl;
 
 best = tuple<float, float>(ans[0], solver.mean());
 
 int id = (ir*Double::RESOLUTION_TH + ith) * 2 + s;
 tmpDoubleTable[id][0] = get<0>(best);
 tmpDoubleTable[id][1] = get<1>(best);
 cerr << s << " " << fr << " " << fth << " " << get<0>(best) << " " << get<1>(best) << endl;
 }
 }
 }
 }
 
 template<class dice_t>
 void makeDoubleTrainingData(dice_t *const pdice){
 
 using shot_t = Double;
 shot_t strategy;
 
 std::ofstream ofs(DIRECTORY_PARAMS_IN + "param_double.dat");
 
 ofs
 << 50000 << " "
 << 40000 << " "
 << 10000 << " "
 << 5 << " "
 << 2 << endl;
 
 //テーブル作り
 for (int i = 0; i<50000; ++i){
 fpn_t fr, fth;
 MiniBoard bd;
 while (1){
 fpn_t x0, y0, x1, y1;
 putRandInHouse(&x0, &y0, pdice);
 putRandInHouse(&x1, &y1, pdice);
 
 if (!isCavingIn(fPosXY<>(x0, y0), fPosXY<>(x1, y1))
 && calcDistance2(fPosXY<>(x0, y0), fPosXY<>(x1, y1))<1.0
 ){
 bd.pushStone(BLACK, fPosXY<>(x0, y0));
 bd.pushStone(BLACK, fPosXY<>(x1, y1));
 break;
 }
 }
 fMoveXY<> mv;
 mv.setSpin(pdice->rand() % 2);
 
 int ref[strategy.dimension()];
 //for (int d = 0; d<strategy.dimension(); ++d){
 //    ref[d] = strategy.chooseReferenceStoneIndex(bd, d);
 //}
 
 fr = calcDistance(bd[getColor(ref[0])][getIndex(ref[0])], bd[getColor(ref[1])][getIndex(ref[1])]);
 fth = calcRelativeAngle(bd[getColor(ref[0])][getIndex(ref[0])], bd[getColor(ref[1])][getIndex(ref[1])]);
 
 std::array<std::pair<double, double>, shot_t::flexibility()> arr;
 arr[0] = pair<double, double>(M_PI / 2, 3 * M_PI / 2);
 pair<double, double> rew(-2.0, 0.0);
 
 MCTSSolver<1> solver(arr, rew);
 
 //cerr<<bd;
 
 for (int t = 0; t<100000; ++t){
 auto smp = solver.play();
 
 FastSimulator::rotateToPassPointCircumferenceF(&mv, bd.stone(ref[0]), FV_MAX - 0.5, smp[0]);
 
 MiniBoard tbd = bd;
 makeMove<1>(&tbd, TURN_WHITE_LAST, mv, pdice);
 
 double ev = strategy.evaluate(bd, tbd, WHITE);
 solver.feed(smp, ev);
 
 //cerr<<smp[0]<<ev<<endl;
 }
 auto ans = solver.answer();
 
 ofs << mv.getSpin() << " "
 << bd[getColor(ref[0])][getIndex(ref[0])].getX() << " "
 << bd[getColor(ref[0])][getIndex(ref[0])].getY() << " "
 << fr << " " << fth << " " << ans[0] << " " << solver.mean() << endl;
 
 //getchar();
 }
 }
 */

#endif // DCURLING_SHOT_DOUBLE_HPP_