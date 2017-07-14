/*
 simulator_test.cc
 Katsuki Ohto
 */

// 物理演算のテスト

#include "../simulation/fastSimulator.hpp"
#include "../simulation/primaryShot.hpp"

#include "../ayumu/structure/thinkBoard.hpp"

namespace DigitalCurling{
    namespace Tester{

        XorShift64 dice;
        DSFMT ddice;
        
        using testBoard_t = ThinkBoard;
        //using testBoard_t = MiniBoard;
        
        int testCZTable(){
            // コンタクトゾーンテーブルの性能テスト
            // 高速物理演算がCZテーブルを使わないことが条件
            int r[2][2] = {0};
            constexpr int samples = 10000;
            for(int t = 0; t < samples; ++t){
                MiniBoard bd;
                fPosXY<> st;
                locateInHouse(&st, &dice);
                bd.locateStone(1, st);
                fMoveXY<> mv;
                mv.setSpin(static_cast<Spin>(dice.rand() % 2));
                genHit(&mv, st, 31.5);
                // ばらけさせる為の乱数
                mv.addVx(dice.drand() * 0.5 - 0.25);
                
                // ルール上の乱数
                fpn_t dvx, dvy;
                genRandToMoveXY(&dvx, &dvy, mv.spin(), &dice);
                
                mv.addVx(dvx); mv.addVy(dvy);
                
                //cerr << st << endl;
                //cerr << mv << endl;
                
                // 実際のシミュレーション
                ContactTree ct = makeMoveNoRand<0>(&bd, 0, mv);
                
                // CZテーブルの結果
                fPosXY<> dp;
                FastSimulator::FMVtoFXY(mv, &dp);
                //cerr << dp << endl;
                
                //MiniBoard nbd;
                //FastSimulator::simulateSoloF<0>(&nbd, 0, mv);
                //cerr << nbd.stone(0) << endl;
                
                //FastSimulator::FMoment mst;
                //mst.set(FX_THROW, FY_THROW, mv.v(), mv.th(), mv.w(), 0);
                
                bool cz = FastSimulator::hasCollisionDraw(FPOSXY_THROW, mv, dp, st);
                
                r[bool(ct.hasContact())][cz] += 1;
            }
            cerr << r[0][0] << " " << r[0][1] << endl;
            cerr << r[1][0] << " " << r[1][1] << endl;
            
            return 1;
        }

#ifdef DCURLING_SIMULATION_INT_SIMULATOR_HPP_

        int testIntSimulator(){
            // 物理シミュレーション(整数版)のテスト
            using namespace IntSimulator;

            // V -> R Conversion
            {
                cerr << "FV -> FR Test : ";
                fMStone<> org, dst;
                double sumErr2 = 0;
                for (int i = 0; i < 200; ++i){
                    double v = ddice.drand() * FV_MAX;
                    //cerr << " real : " << v << " -> ";
                    org.set(0, 0, 0, v, FANGV_ORG);
                    Simulator::simulateSolo(org, &dst, 0.00001);
                    double r = XYtoR(dst.x, dst.y);
                    //cerr << r;
                    //cerr << " array : " 
                    //<< v << " -> " << FVtoI32V(v)<<" -> "
                    //<<IVtoIR(FVtoI32V(v))<<" -> "<<I64RtoFR(IVtoIR(FVtoI32V(v)))<<endl;
                    double err = r - I64RtoFR(IVtoIR(FVtoI32V(v)));
                    sumErr2 += err * err;
                }
                sumErr2 /= 100;
                cerr << "Average Error = " << sqrt(sumErr2) << endl;
            }
            //R -> V Conversion
            {
                cerr << "FR -> FV Test : ";
                fMStone<> org, dst;
                double sumErr2 = 0;
                for (int i = 0; i < 200; ++i){
                    double v = ddice.drand() * FV_MAX;
                    //cerr << " real : ";
                    org.set(0, 0, 0, v, FANGV_ORG);
                    Simulator::simulateSolo(org, &dst, 0.00001);
                    double r = XYtoR(dst.x, dst.y);
                    //cerr << r << " <- " << v;
                    //cerr << " array : "
                    //<< r << " <- " << FRtoI64R(r)<<" <- "
                    //<<IRtoIV(FRtoI64R(r)) <<" <- "<<I32VtoFV(IRtoIV(FRtoI64R(r)))<<endl;
                    double err = v - I32VtoFV(IRtoIV(FRtoI64R(r)));
                    sumErr2 += err * err;
                }
                sumErr2 /= 100;
                cerr << "Average Error = " << sqrt(sumErr2) << endl;
            }
            //R_V変換
            {
                cerr << "FR -> FV Test : ";
                fMStone<> org, dst;
                double sumErr2 = 0;
                for (int i = 0; i < 200; ++i){
                    double v = ddice.drand() * FV_MAX;
                    //cerr << " real : ";
                    org.set(0, 0, 0, v, FANGV_ORG);
                    Simulator::simulateSolo(org, &dst, 0.00001);
                    double r = XYtoR(dst.x, dst.y);
                    double err = v - I32VtoFV(IRtoIV(FRtoI64R(r)));
                    sumErr2 += err*err;
                }
                sumErr2 /= 100;
                cerr << "Average Error = " << sqrt(sumErr2) << endl;
            }
        }

#endif

        int testBox2DSimulator(){

            //box2dデータ構造サイズをチェック
            {
                cerr << "size of b2Vec2 ... " << sizeof(b2Vec2) << endl;
                cerr << "size of b2Body ... " << sizeof(b2Body) << endl;
                cerr << "size of b2World ... " << sizeof(b2World) << endl;
            }
            return 0;
        }

#ifdef DCURLING_SIMULATION_FASTSIMULATOR_HPP_
        template<int RINK_ONLY>
        int testFastSimulator(){
            //物理シミュレーションのテスト

            using namespace Simulator;
            using namespace FastSimulator;

            Clock cl;

            // V <-> R Conversion
            {
                cerr << "V<->R Test" << endl;
                const int N = 400;
                fMStone<pfpn_t> org, dst;
                double sumErr2[2] = { 0 };
                double maxErr2[2] = { 0 };
                uint64_t time[2] = { 0 };
                for (int i = 0; i < N; ++i){
                    double v = ddice.drand() * FV_MAX;
                    //cerr << " real : " << v << " -> ";
                    org.set(0, 0, 0, v, FANGV_ORG);
                    Simulator::simulateSolo<pfpn_t>(org, &dst, pfpn_t(0.00002));
                    double r = XYtoR(dst.x, dst.y);
                    //cerr << r;
                    //cerr << " array : " << v << " -> " << FVtoFR(v) <<endl;
                    cl.start();
                    fpn_t RbyV = FVtoFR(v);
                    time[0] += cl.stop();
                    cl.start();
                    fpn_t VbyR = FRtoFV(r);
                    time[1] += cl.stop();
                    double err[2] = { r - RbyV, v - VbyR };
                    double err2[2] = { err[0] * err[0], err[1] * err[1] };
                    sumErr2[0] += err2[0];
                    maxErr2[0] = max(maxErr2[0], err2[0]);
                    sumErr2[1] += err2[1];
                    maxErr2[1] = max(maxErr2[1], err2[1]);

                    //if (fabs(err[1]) > 0.01){
                    //    cerr << v << " -> " << FVtoFR(v) << endl;
                    //    cerr << r << " -> " << FRtoFV(r) << endl;
                    //}
                }
                sumErr2[0] /= N; sumErr2[1] /= N;
                //cerr << "FV -> FR Array Step : " << FV_FR_TABLE_STEP << endl;
                //cerr << "FR -> FV Array Step : " << FR_FV_TABLE_STEP << endl;
                cerr << "FVtoFR() : " << time[0] / N << " clock/trial  FRtoFV() : " << time[1] / N << " clock/trial" << endl;
                cerr << "FV -> FR Test : ";
                cerr << "Average Error = " << sqrt(sumErr2[0]) << " Maximum Error = " << sqrt(maxErr2[0]) << endl;
                cerr << "FR -> FV Test : ";
                cerr << "Average Error = " << sqrt(sumErr2[1]) << " Maximum Error = " << sqrt(maxErr2[1]) << endl;
            }
            // V, Theta -> Position-Stopped Conversion
            {
                const int N = 2000;
                double sumErr2(0), maxErr2(0);
                uint64_t time[2] = { 0 };
                cerr << "V,Theta -> X,Y,R,Theta Test(Fast)" << endl;
                for (int i = 0; i < N; ++i){
                    fPosXY<> pos0, pos1;
                    fpn_t r, th;
                    fMoveVTh<> fmv;
                    fpn_t v = (FV_MAX - 5.0) * ddice.drand() + 5.0;
                    fpn_t vth = 2 * M_PI * ddice.drand();
                    fmv.set(v, vth, static_cast<Spin>(ddice.rand() % 2));
                    // Faster Calculation
                    cl.start();
                    FMVtoFXY(fmv, &pos0);
                    time[0] += cl.stop();
                    // Fastest Calculation
                    cl.start();
                    FVThtoFXYRTh(&pos1.x, &pos1.y, &r, &th, fmv.v(), fmv.th(), fmv.getSpin());
                    time[1] += cl.stop();
                    fpn_t dist2 = calcDistance2(pos0, pos1);
                    sumErr2 += dist2;
                    maxErr2 = max(maxErr2, dist2);
                }
                sumErr2 /= N;
                cerr << " FasterCalc : " << time[0] / N << " clock/trial  FastestCalc : " << time[1] / N << " clock/trial" << endl;
                cerr << "Average Error = " << sqrt(sumErr2) << " Maximum Error = " << sqrt(maxErr2) << endl;
            }
            {
                const int N = 2000;
                double sumErr2(0), maxErr2(0);
                uint64_t time[2] = { 0 };
                cerr << "V, Theta -> X, Y, R, Theta Test(Slow)" << endl;
                for (int i = 0; i < N; ++i){
                    fPosXY<> pos0, pos1;
                    fpn_t r, th;
                    fMoveVTh<> fmv;
                    fpn_t v = 5.0 * ddice.drand();
                    fpn_t vth = 2 * M_PI * ddice.drand();
                    fmv.set(v, vth, static_cast<Spin>(ddice.rand() % 2));
                    // Faster Calculation
                    cl.start();
                    FMVtoFXY(fmv, &pos0);
                    time[0] += cl.stop();
                    // Fastest Calculation
                    cl.start();
                    FVThtoFXYRTh(&pos1.x, &pos1.y, &r, &th, fmv.v(), fmv.th(), fmv.getSpin());
                    time[1] += cl.stop();
                    fpn_t dist2 = calcDistance2(pos0, pos1);
                    sumErr2 += dist2;
                    maxErr2 = max(maxErr2, dist2);
                }
                sumErr2 /= N;
                cerr << " FasterCalc : " << time[0] / N << " clock/trial  FastestCalc : " << time[1] / N << " clock/trial" << endl;
                cerr << "Average Error = " << sqrt(sumErr2) << " Maximum Error = " << sqrt(maxErr2) << endl;
            }

            //いろいろやりたいことをやる
            fPosXY<> pos;
            fMoveXY<> vec;

            //逆関数のはたらき調査

            {
                fPosXY<> v(2.43, 54);
                fMoveXY<> v2;
                DIRtoVEC(v, &v2);
                cerr << "DIRtoVEC" << OutXY<float>(v.x, v.y) << " -> " << OutXY<float>(v2.x, v2.y) << endl;
                VECtoDIR(v2, &v);
                cerr << "VECtoDIR" << OutXY<float>(v2.x, v2.y) << " -> " << OutXY<float>(v.x, v.y) << endl;

                pos.set(FX_TEE, FY_TEE);
                vec.set(1, 2, Spin::RIGHT);

                calcDeparture(pos, &vec);
                cerr << "calcDeparture" << pos << " -> " << vec << endl;
                calcDestination(vec, &pos);
                cerr << "calcDestination" << vec << " -> " << pos << endl;
            }

            //testSimulator();

            //シミュレータの精度テスト


            //ヒットショットの最高速度を調べる

#if 0
            //角速度による運動の変化を求める
            {

                fMStone<> org;
                fMoveXY<> mv;
                for (int i = 0; i < 4; ++i){
                    testBoard_t bd;
                    bd.init();
                    bd.push(BLACK, FX_TEE, FY_TEE + 1);
                    mv.setSpin(Spin::RIGHT);
                    genHit(&mv, bd.stone(BLACK, 0), 33.1);
                    cerr << mv << endl;
                    org.set(FX_THROW, FY_THROW, mv.vx(), mv.vy(), -FANGV_ORG * i);
                    b2dSimulator::simulateb2d<0>(&bd, org, 0.0001);
                    cerr << bd.toString();
                }
            }
#endif
            // ドローショットがどの程度目標点に近づくかを判定(近づくかの方)
            {
                const int N = 2000;
                double sumErr2(0), maxErr2(0);
                cerr << "Drawing Test (In House)" << endl;
                for (int i = 0; i < N; ++i){
                    fPosXY<> mato, dst;
                    fMoveXY<> mv;

                    locateInHouse(&mato, &ddice);

                    mv.setSpin(static_cast<Spin>(ddice.rand() % 2));
                    genDraw(&mv, mato);
                    FastSimulator::FMVtoFXY(mv, &dst);
                    fpn_t dist2 = calcDistance2(mato, dst);

                    sumErr2 += dist2;
                    maxErr2 = max(maxErr2, dist2);
                }
                sumErr2 /= N;
                cerr << "Average Error = " << sqrt(sumErr2) << " Maximum Error = " << sqrt(maxErr2) << endl;
            }
            // 指定点を通るショットの精度判定
            {
                cerr << "Passing Point Test : ";
                const int N = 400;
                double sumErr2 = 0;
                double maxErr2 = 0;
                uint64_t time = 0;
                for (int i = 0; i < N; ++i){
                    fPosXY<> dst;
                    fPosXY<> pos;
                    fMoveXY<> mv;
                    fpn_t v;
                    double err2 = 9999;
                    locateInHouse(&dst, &ddice);
                    const fpn_t r2(calcDistance2(FPOSXY_THROW, dst)), r(calcDistance(FPOSXY_THROW, dst));
                    do{
                        v = ddice.drand() * (FV_MAX - FV_MIN) + FV_MIN;
                    } while (FVtoFR(v) - 0.1 <= r);

                    {
                        mv.setSpin(static_cast<Spin>(ddice.rand() % 2));
                        cl.start();
                        rotateToPassPointF(&mv, dst, v);
                        time += cl.stop();
                    }
                    {
                        fMStone<pfpn_t> mst;
                        mst.set(FX_THROW, FY_THROW, mv.vx(), mv.vy(), SpintoThrowFW(mv.getSpin()));
                        while (1){
                            Simulator::stepSolo<pfpn_t>(&mst, pfpn_t(0.0001));
                            pfpn_t fr2 = XYtoR2(mst.x - FX_THROW, mst.y - FY_THROW);
                            if (fr2 > r2){
                                fMStone<pfpn_t> lmst = mst;
                                Simulator::invStepSolo<pfpn_t>(&lmst, pfpn_t(0.0001));
                                pfpn_t fr = XYtoR(mst.x - FX_THROW, mst.y - FY_THROW);
                                pfpn_t lfr = XYtoR(lmst.x - FX_THROW, lmst.y - FY_THROW);
                                pfpn_t frac;
                                if (fr != lfr){
                                    frac = (r - lfr) / (fr - lfr);// lfr < r <= fr
                                }
                                else{
                                    frac = static_cast<pfpn_t>(0.5);
                                }
                                fPosXY<> tpos(lmst.x * ((pfpn_t)1 - frac) + mst.x * frac, lmst.y * ((pfpn_t)1 - frac) + mst.y * frac);
                                pfpn_t dr2 = calcDistance2(dst, pos);
                                if(dr2 < err2){
                                    pos = tpos;
                                    err2 = dr2;
                                }
                            }
                            if(!(mst.vx() || mst.vy())){break;}
                        }
                    }
                    if (err2 > 0.00001){
                        cerr << dst << endl;
                        cerr << mv << endl;
                        cerr << pos << endl;
                    }

                    sumErr2 += err2;
                    maxErr2 = max(maxErr2, err2);
                }
                sumErr2 /= N; sumErr2 /= N;
                cerr << time / N << " clock/trial" << endl;
                cerr << "Average Error = " << sqrt(sumErr2) << " Maximum Error = " << sqrt(maxErr2) << endl;
            }
            // 衝突処理チェック
            {
                const int N = 2000;
                int realN[2] = { 0 };
                double sumErr2[12] = { 0 };
                double maxErr2[12] = { 0 };
                uint64_t time[4] = { 0 };
                // 動-静
                cerr << "AW_AS Collision Test" << endl;
                FMoment aw, as, taw, tas;
                fPosXY<> pos_aw, pos_as;
                fpn_t dtheta, reltheta, w, v;
                StoneState state;
                for (int i = 0; i < N; ++i){
                    uint64_t tmpTime[2];
                    pos_aw.set(0, 0);
                    double r = ddice.drand();
                    pos_as.set(pos_aw.getX() + 2 * FR_STONE_RAD * cos(2 * r * M_PI), pos_aw.getX() + 2 * FR_STONE_RAD * sin(2 * r * M_PI));
                    reltheta = (M_PI - 0.05) * (ddice.drand() - 0.5); // 2石の中心点間を結ぶ向きからのずれ
                    dtheta = calcRelativeAngle(pos_aw, pos_as) + reltheta;
                    w = ddice.drand() - 0.5;
                    v = FV_MAX * ddice.drand();

                    state.init();
                    state.setColor(BLACK);

                    aw.set(pos_aw.getX(), pos_aw.getY(), v, dtheta, w, 0);
                    as.set(pos_as.getX(), pos_as.getY(), 0, 0, 0, 0);

                    //高速シミュレート
                    taw = aw; tas = as;

                    cl.start();
                    FastSimulator::collisionAw_As(&taw, &tas);
                    tmpTime[0] = cl.stop();
#if 0
                    cerr << "AW ";
                    cerr << " vx,vy " << fPosXY<>(taw.vx(), taw.vy())
                        << " " << taw.getW() << endl;
                    cerr << "AS ";
                    cerr << " vx,vy " << fPosXY<>(tas.vx(), tas.vy())
                        << " " << tas.getW() << endl;
#endif
                    //通常シミュレート
                    b2dSimulator::b2Board bd;
                    bd.setAwakeStone(0, aw);
                    bd.setAsleepStone(1, as);

                    bool collided = false;

                    cl.start();
                    for (int i = 0; i < 30; ++i){
                        b2dSimulator::step(&bd, 0.00001);
                        if (!(bd.stone(1).vx() == 0 && bd.stone(1).vy() == 0)
                            && (bd.stone(0).pos() - bd.stone(1).pos()).Length() > FR_STONE_RAD * 2){
                            collided = true; break;
                        }
                    }
                    tmpTime[1] = cl.stop();
#if 0                    
                    cerr << "AW ";
                    cerr << " vx,vy " << bd.stone(0).vel();
                        << " " << bd.stone(0).w() << endl;
                    cerr << "AS ";
                    cerr << " vx,vy " << bd.stone(1).vel();
                        << " " << bd.stone(1).w() << endl;
#endif                    
                    if (collided){
                        double err[6] = {
                            taw.vx() - bd.stone(0).vx(),
                            taw.vy() - bd.stone(0).vy(),
                            taw.getW() - bd.stone(0).w(),
                            tas.vx() - bd.stone(1).vx(),
                            tas.vy() - bd.stone(1).vy(),
                            tas.getW() - bd.stone(1).w(),
                        };
                        double err2[6];
                        for (int i = 0; i < 6; ++i){
                            err2[i] = err[i] * err[i];
                        }
                        int d = (fabs(reltheta) < M_PI / 4) ? 0 : 1;
                        ++realN[d];
                        for (int i = 0; i < 6; ++i){
                            int id = d * 6 + i;
                            sumErr2[id] += err2[i];
                            maxErr2[id] = max(maxErr2[i], err2[i]);
                        }
                        for (int i = 0; i<2; ++i){
                            time[d * 2 + i] += tmpTime[i];
                        }
                    }
                }
                const std::string fu[2] = { "<Frontal>", "<Unfrontal>" };
                const std::string awas[2] = { "Awake ", "Asleep " };
                const std::string param[3] = { "VX", "VY", "W " };
                for (int d = 0; d < 2; ++d){
                    cerr << fu[d];
                    if (realN[d] == 0){
                        cerr << " No Collided Sample." << endl;
                    }
                    else{
                        cerr << " " << realN[d] << " Samples.";
                        cerr << " Fast : " << time[d * 2] / realN[d] << " clock/trial  Box2D : " << time[d * 2 + 1] / realN[d] << " clock/trial" << endl;

                        for (int st = 0; st < 2; ++st){
                            for (int p = 0; p < 3; ++p){
                                int id = d * 6 + 3 * st + p;
                                sumErr2[id] /= realN[d];
                                cerr << awas[st] << param[p] << " : ";
                                cerr << "Average Error = " << sqrt(sumErr2[id]) << " Maximum Error = " << sqrt(maxErr2[id]) << endl;
                            }
                        }
                    }
                }
            }
            {
                const int N = 1000;
                int realN = 0;
                double sumErr2[6] = { 0 };
                double maxErr2[6] = { 0 };
                uint64_t time[2] = { 0 };
                // 動-動
                cerr << "2AW Collision Test" << endl;
                FMoment aw[2], taw[2];
                fPosXY<> pos[2];
                fpn_t dtheta[2], w[2], v[2];
                for (int i = 0; i < N; ++i){
                    uint64_t tmpTime[2];
                    pos[0].set(0, 0);
                    double r = ddice.drand();
                    pos[1].set(pos[0].getX() + (2 * FR_STONE_RAD + 0.00001) * cos(2 * r * M_PI), pos[0].getX() + (2 * FR_STONE_RAD + 0.00001) * sin(2 * r * M_PI));
                    dtheta[0] = XYtoT(pos[1].getY() - pos[0].getY(), pos[1].getX() - pos[0].getX()) + (M_PI - 0.2) * (ddice.drand() - 0.5);
                    dtheta[1] = XYtoT(pos[0].getY() - pos[1].getY(), pos[0].getX() - pos[1].getX()) + (M_PI - 0.2) * (ddice.drand() - 0.5);
                    w[0] = ddice.drand() - 0.5;
                    w[1] = ddice.drand() - 0.5;
                    v[0] = (FV_MAX - 5.0) * ddice.drand();
                    v[1] = (FV_MAX - 5.0) * ddice.drand();

                    aw[0].set(pos[0].getX(), pos[0].getY(), v[0], dtheta[0], w[0], 0);
                    aw[1].set(pos[1].getX(), pos[1].getY(), v[1], dtheta[1], w[1], 0);

                    // 高速シミュレート
                    taw[0] = aw[0]; taw[1] = aw[1];

                    cl.start();
                    FastSimulator::collision2Aw(&taw[0], &taw[1]);
                    tmpTime[0] = cl.stop();

                    //cerr << "AW0 ";
                    //cerr << " vx,vy " << fPosXY<>(taw[0].vx(), taw[0].vy())
                    //<< " " << taw[0].getW() << endl;
                    //cerr << "AW1 ";
                    //cerr << " vx,vy " << fPosXY<>(taw[1].vx(), taw[1].vy())
                    //<< " " << taw[1].getW() << endl;

                    //通常シミュレート
                    b2dSimulator::b2Board bd;
                    bd.setAwakeStone(0, aw[0]);
                    bd.setAwakeStone(1, aw[1]);

                    fpn_t orgw0 = w[0];
                    bool collided = false;
                    cl.start();
                    for (int i = 0; i < 40; ++i){
                        b2dSimulator::step(&bd, 0.00001);
                        if (fabs(bd.stone(0).w() - orgw0) > 0.00001
                            && (bd.stone(0).pos() - bd.stone(1).pos()).Length() > FR_STONE_RAD * 2){
                            collided = true; break;
                        }
                    }
                    tmpTime[1] = cl.stop();

                    //cerr << "AW0 ";
                    //cerr << " vx,vy " << bd.stone(0).vel() << " " << bd.stone(0).w() << endl;
                    //cerr << "AW1 ";
                    //cerr << " vx,vy " << bd.stone(1).vel() << " " << bd.stone(1).w() << endl;

                    if (collided){
                        ++realN;
                        double err[6];
                        double err2[6];
                        for (int i = 0; i < 2; ++i){
                            err[0 + 3 * i] = taw[i].vx() - bd.stone(i).vx();
                            err[1 + 3 * i] = taw[i].vy() - bd.stone(i).vy();
                            err[2 + 3 * i] = taw[i].getW() - bd.stone(i).w();
                        }
                        for (int i = 0; i < 6; ++i){
                            err2[i] = err[i] * err[i];
                        }
                        for (int i = 0; i < 6; ++i){
                            sumErr2[i] += err2[i];
                            maxErr2[i] = max(maxErr2[i], err2[i]);
                        }
                        for (int i = 0; i < 2; ++i){
                            time[i] += tmpTime[i];
                        }
                    }
                }
                if (realN == 0){
                    cerr << "No Collided Sample." << endl;
                }
                else{
                    cerr << realN << " Samples.";
                    cerr << " Fast : " << time[0] / realN << " clock/trial  Box2D : " << time[1] / realN << " clock/trial" << endl;
                    for (int i = 0; i < 6; ++i){
                        sumErr2[i] /= realN;
                    }
                    for (int i = 0; i < 2; ++i){
                        cerr << "Awake" << i << " VX : ";
                        cerr << "Average Error = " << sqrt(sumErr2[3 * i + 0]) << " Maximum Error = " << sqrt(maxErr2[3 * i + 0]) << endl;
                        cerr << "Awake" << i << " VY : ";
                        cerr << "Average Error = " << sqrt(sumErr2[3 * i + 1]) << " Maximum Error = " << sqrt(maxErr2[3 * i + 1]) << endl;
                        cerr << "Awake" << i << " W  : ";
                        cerr << "Average Error = " << sqrt(sumErr2[3 * i + 2]) << " Maximum Error = " << sqrt(maxErr2[3 * i + 2]) << endl;
                    }
                }
            }

            // simulation torelance test
            const int N[4] = {1000, 1000, 1000, 1000};
            for (int t = 0; t < 4; ++t){
                if (t == 0){
                    cerr << "1_0 Fast Simulation Test" << endl;
                }
                else if (t == 1){
                    cerr << "1_1 Fast Simulation Test" << endl;
                }
                else if (t == 2){
                    cerr << "1_N Fast Simulation Test" << endl;
                }
                else if (t == 3){
                    cerr << "Double_Contact Fast Simulation Test" << endl;
                }
                double sumErr[3] = { 0 };
                int dist[3][32] = { 0 };
                uint64_t time[3] = { 0 };
                testBoard_t bd[3];
                fMStone<> org;
                fMoveXY<> mv;
                for (int i = 0; i < N[t]; ++i){

                    int nst = 1;

                    for (int s = 0; s < 3; ++s){
                        bd[s].init();
                    }

                    const int fr = (FR_HOUSE_RAD + FR_STONE_RAD);
                    if (t == 0){
                        mv.setSpin(static_cast<Spin>(ddice.rand() % 2));
                        double r0 = ddice.drand(); double r1 = ddice.drand();
                        fPosXY<> pos(FX_TEE + fr * r0 * cos(2 * r1 * M_PI), FY_TEE + fr * r0 * sin(2 * r1 * M_PI));
                        //cerr << "dst : ( " << x << ", " << y << " )" << endl;
                        genDraw(&mv, pos);
                        nst = 1;
                    }
                    else if (t == 1){
                        mv.setSpin(static_cast<Spin>(ddice.rand() % 2));
                        double r0 = ddice.drand(); double r1 = ddice.drand();
                        fPosXY<> pos(FX_TEE + fr * r0 * cos(2 * r1 * M_PI), FY_TEE + fr * r0 * sin(2 * r1 * M_PI));
                        //cerr << "dst : ( " << x << ", " << y << " )" << endl;
                        genHit(&mv, pos, 32.0);

                        for (int s = 0; s < 3; ++s){
                            bd[s].pushStone(BLACK, pos);
                            //bd[s].setState();
                            DERR << "settled" << endl;
                        }
                        nst = 2;
                    }
                    else if(t == 2){
                        for (int j = 0; j < 5; ++j){
                            Color col = (ddice.rand() % 2) ? BLACK : WHITE;
                            pushStonesInHouse(&bd[0], col, 1, &ddice);
                        }
                        mv.setSpin(static_cast<Spin>(ddice.rand() % 2));
                        //cerr << "dst : ( " << x << ", " << y << " )" << endl;
                        genDraw(&mv, fPosXY<>(FX_TEE, FY_TEE + 7));
                        for (int s = 1; s < 3; ++s){
                            bd[s] = bd[0];
                        }
                        nst = 6;
                    }
                    else if(t == 3){
                        fPosXY<> pos[2];
                        mv.setSpin(static_cast<Spin>(ddice.rand() % 2));
                        do{
                            locateInHouse(&pos[0], &ddice);
                            fpn_t fr=FR_STONE_RAD * 2 + FR_COLLISION / 3;
                            double r = ddice.drand();
                            pos[1].set(pos[0].x + fr*cos(2 * r * M_PI), pos[0].y + fr * sin(2 * r * M_PI));
                        }while(!isInHouse(pos[1]));
                        //cerr << "dst : ( " << x << ", " << y << " )" << endl;
                        genHit(&mv, pos[0], 32.0);
                        
                        for (int s = 0; s < 3; ++s){
                            bd[s].pushStone(BLACK, pos[0]);
                            bd[s].pushStone(BLACK, pos[1]);
                            DERR << "settled" << endl;
                        }
                        nst = 3;
                    }

                    org.set(FX_THROW, FY_THROW, mv.vx(), mv.vy(), SpintoThrowFW(mv.getSpin()));

                    constexpr int ONE_TO_ONE = 1 ^ RINK_ONLY;
                    
                    cl.start();
                    b2dSimulator::simulateb2d<RINK_ONLY>(&bd[0], TURN_WHITE_LAST, org, 1 / 3000.0); // 通常シミュレート(高精度)
                    time[0] += cl.stop();

                    cl.start();
                    b2dSimulator::simulateb2d<RINK_ONLY>(&bd[1], TURN_WHITE_LAST, org, BOX2D_TIMESTEP_OFFICIAL); // 通常シミュレート(公式)
                    time[1] += cl.stop();

                    cl.start();
                    ContactTree ct = FastSimulator::simulateF<RINK_ONLY>(&bd[2], TURN_WHITE_LAST, mv); // 高速シミュレート
                    //cerr << ct << endl;
                    time[2] += cl.stop();
                    
                    //tick();
                    //bd[2].updateInfo();
                    //tock();
                    
                    for(int i = 0; i < 3; ++i){
                        sumErr[i] += calcDistanceSum<ONE_TO_ONE>(bd[i], bd[(i + 1) % 3], dist[i]) / nst;
                        /*if(dist[i][9] > 0){
                            cerr << bd[i] << endl << bd[(i + 1) % 3] << endl; getchar();
                        }*/
                        /*if(i == 0 && calcDistanceSum<ONE_TO_ONE>(bd[i], bd[(i + 1) % 3]) > 0.1){
                            cerr << bd[i] << endl << bd[(i + 1) % 3] << endl;
                        }*/
                    }
                }
                for (int i = 0; i < 3; ++i){
                    sumErr[i] /= N[t];
                    time[i] /= N[t];
                }
                cerr << "Box2D_Hi : " << time[0] << " clock/trial" << endl;
                cerr << "Box2D_Of : " << time[1] << " clock/trial" << endl;
                cerr << "Array    : " << time[2] << " clock/trial" << endl;
                cerr << "Box2D_Hi - Box2D_Of " << sumErr[0] << endl;
                for (int i = 0; i < 32; ++i){
                    cerr << " " << dist[0][i];
                }cerr << endl;
                cerr << "Box2D_Of - Array " << sumErr[1] << endl;
                for (int i = 0; i < 32; ++i){
                    cerr << " " << dist[1][i];
                }cerr << endl;
                cerr << "Array - Box2D_Hi " << sumErr[2] << endl;
                for (int i = 0; i < 32; ++i){
                    cerr << " " << dist[2][i];
                }cerr << endl;
            }

            //ここから諸々の演算チェック
            //軌道距離
            /*{
            cerr << "Orbit Distance Check" << endl;
            fMStone<> org;
            fMoveXY<> mv;
            for (int s = 0; s<2; ++s){
            for (int i = 0; i<4; ++i){
            fPosXY<> stop(FX_TEE + i*1.0, FY_TEE - 3.0);
            fpn_t dr = sqrt(calcOrbitDist2AW_AS(fPosXY<>(FX_THROW, FY_THROW),
            fPosXY<>(FX_TEE, FY_TEE),
            s, stop));
            cerr << dr << " ";
            mv.setSpin(s);
            genDraw(&mv, fPosXY<>(FX_TEE, FY_TEE));
            org.set(FX_THROW, FY_THROW, mv.vx(), mv.vy(), (mv.getSpin() == Spin::RIGHT) ? FANGV_ORG : (-FANGV_ORG));

            fpn_t dist2 = 9999;
            while (XYtoR2(org.vx, org.vy)){
            Simulator::stepSolo(&org, 0.0001);
            dist2 = min(dist2, XYtoR2(org.x - stop.x, org.y - stop.y));
            }
            cerr << sqrt(dist2) << endl;
            }
            cerr << endl;
            }
            }*/
            return 0;
        }
#endif

#ifdef DCURLING_SIMULATION_FASTSIMULATOR_HPP_
        int testPrimaryShot(){
            //コーナー着手
            {
                fMoveXY<> mv;
                fPosXY<> pos;
                CERR << "Corner Move Check" << endl;
                pos.set(FX_PA_MIN + FR_STONE_RAD, FY_PA_MIN + FR_STONE_RAD);
                mv.setSpin(Spin::RIGHT);
                genDraw(&mv, pos);
                CERR << "PA LEFT-SHORT r (" << mv.x << "," << mv.y << ")" << endl;
                pos.set(FX_PA_MAX - FR_STONE_RAD, FY_PA_MIN + FR_STONE_RAD);
                mv.setSpin(Spin::RIGHT);
                genDraw(&mv, pos);
                CERR << "PA RIGHT-SHORT r (" << mv.x << "," << mv.y << ")" << endl;
                pos.set(FX_TEE, FY_TEE + FR_HOUSE_RAD + 6.0 * 1.52);
                mv.setSpin(Spin::RIGHT);
                genDraw(&mv, pos);
                CERR << "CENTER-LONG r (" << mv.x << "," << mv.y << ")" << endl;
                /*pos.set(FX_PA_MAX,FY_PA_MAX);
                vec.setSpin(0);
                calcDeparture(pos,&vec);
                CERR<<"PA RD 0 ("<<vec.x<<","<<vec.y<<")"<<endl;
                pos.set(FX_RINK_MIN,FY_RINK_MIN);
                vec.setSpin(0);
                CreateShot(pos,16,&vec);
                CERR<<"RINK LU 0 PowMAX("<<vec.x<<","<<vec.y<<") "<<XYtoR(vec.x,vec.y)<<endl;
                pos.set(FX_PA_MAX,FY_PA_MAX);
                vec.setSpin(0);
                CreateShot(pos,16,&vec);
                CERR<<"PA RD 0 PowMAX("<<vec.x<<","<<vec.y<<") "<<XYtoR(vec.x,vec.y)<<endl;
                pos.set(FX_RINK_MIN,FY_RINK_MIN);
                vec.setSpin(0);
                CreateShot(pos,0,&vec);
                CERR<<"RINK LU 0 PowMIN("<<vec.x<<","<<vec.y<<") "<<XYtoR(vec.x,vec.y)<<endl;
                pos.set(FX_PA_MAX,FY_PA_MAX);
                vec.setSpin(0);
                CreateShot(pos,0,&vec);*/
            }
            return 0;
        }
#endif
    }
}

int main(int argc, char* argv[]){
    
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    using namespace DigitalCurling;
    std::random_device seed;
    bool seedSettled = false;
    
    bool rinkOnly = false;
    
    for (int i = 1; i < argc; ++i){
        if (!strcmp(argv[i], "-s")){ // seed configuration
            uint64_t s = atoi(argv[i + 1]);
            Tester::dice.srand(s);
            Tester::ddice.srand(s);
            seedSettled = true;
        }else if (!strcmp(argv[i], "-ro")){ // rink only
            rinkOnly = true;
        }
    }
    
    if(!seedSettled){
        Tester::dice.srand(seed() * (unsigned int)time(NULL));
        Tester::ddice.srand(seed() * (unsigned int)time(NULL));
    }

    cerr << "FR_MAX = " << FastSimulator::FVtoFR(FV_MAX) << endl;

    
    //Tester::testCZTable();
    if(rinkOnly){
        Tester::testFastSimulator<1>();
    }else{
        Tester::testFastSimulator<0>();
    }

    return 0;
}