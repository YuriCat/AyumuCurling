//hit tableの作成

#include "../dc.hpp"
#include "../ayumu/ayumu_dc.hpp"

#include "../tools/thinkboard.h"
#include "../simulation/simulator.hpp"
#include "../simulation/b2dsimulator.hpp"
#include "../simulation/fastsimulator.hpp"
#include "../simulation/rand.hpp"
#include "../simulation/primaryshot.hpp"
#include "../shot/allshots.hpp"
#include "../logic/logic.hpp"
#include "../tools/thinkboard.hpp"

using namespace DigitalCurling;
using namespace std;

int main(int argc, char* argv[]){

    std::random_device seed;
    
    XorShift64 dice;
    DSFMT ddice;

    dice.srand(seed()*(unsigned int)time(NULL));
    ddice.srand(seed()*(unsigned int)time(NULL));

    FastSimulator::init();
    
    constexpr fpn_t V=31.5;
    
    fPosXY<> dst(FX_TEE,FY_TEE);
    
    //すれすれ点を求める
    fpn_t left,right;
    {
        ThinkBoard bd;
        bd.init();
        
        fPosXY<> pos=dst;
        pushStone(&bd,BLACK,pos);
        
        fMoveXY<> hit;
        hit.setSpin(Spin::RIGHT);
        genHit(&hit, pos, V, 1);
        
        biSolver solver(-0.1,hit.th());
        
        //cerr<<"[ "<<solver.alpha()<<", "<<solver.beta()<<" ]"<<endl;
        
        for(int i=0;i<100;++i){
            ThinkBoard tbd=bd;
            
            fpn_t val=solver.play();
            
            fMoveVTh<> mv(V,val,Spin::RIGHT);
            
            int ret=FastSimulator::simulateF<0>(&tbd,mv);
            
            //cerr<<val<<mv<<ret<<tbd.toString();
            
            solver.feed((ret==2)?true:false);
        }
        
        left=solver.answer();
    }
    {
        ThinkBoard bd;
        bd.init();
        
        fPosXY<> pos=dst;
        pushStone(&bd,BLACK,pos);
        
        fMoveXY<> hit;
        hit.setSpin(Spin::RIGHT);
        genHit(&hit, pos, V, 1);
        
        biSolver solver(hit.th(),0.1);
        
        //cerr<<"[ "<<solver.alpha()<<", "<<solver.beta()<<" ]"<<endl;
        
        for(int i=0;i<100;++i){
            ThinkBoard tbd=bd;
            
            fpn_t val=solver.play();
            
            fMoveVTh<> mv(V,val,Spin::RIGHT);
            
            int ret=FastSimulator::simulateF<0>(&tbd,mv);
            
            //cerr<<val<<mv<<ret<<tbd.toString();
            
            solver.feed(!((ret==2)?true:false));
        }
        
        right=solver.answer();
    }
    cerr<<"left = "<<left<<" right = "<<right<<endl;
    
    typedef LinearlyInterpolatedFreeTable<std::array<double,4>,double> table_t;

    table_t table(128,0,1);
    
    cerr<<table.toDebugString()<<endl;
    
    fMoveXY<> mv;
    mv.setSpin(Spin::RIGHT);
    genHit(&mv,dst,V,1);
    
    for(int ith=0;ith<table.arraySize();++ith){
        fpn_t th=left + table.getKey(ith)*(right-left) ;
        
        fpn_t dtth=th - mv.th();
        
        fMoveVTh<> tmv(V,mv.th()+dtth,mv.getSpin());
        
        ThinkBoard bd;
        bd.init();
        pushStone(&bd,BLACK,dst);
        bd.setState();
        
        int ret=FastSimulator::simulateF<0>(&bd,tmv);
        //int ret=b2dSimulator::simulate<0>(&bd,tmv);
        
        cerr<<bd.toString();
        
        const fPosXY<> st(accessStone(bd,BLACK,0).getX(),accessStone(bd,BLACK,0).getY());
        
        table.assign(ith,make_tuple(st.x-dst.x,st.y-dst.y,calcDistance(dst,st),calcRelativeAngle(dst,st)));
        
        //cerr<<st<<","<<calcDistance(st,dst)<<endl;
        
    }
    for(int i=0;i<table.arraySize();++i){
        cerr<<"{";
        cerr<<get<0>(table.accessByIndex(i))<<", ";
        cerr<<get<1>(table.accessByIndex(i))<<", ";
        cerr<<get<2>(table.accessByIndex(i))<<", ";
        cerr<<get<3>(table.accessByIndex(i))<<", ";
        cerr<<"},";
        cerr<<endl;
    }
    return 0;
}