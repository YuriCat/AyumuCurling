//avoid tableの作成

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
    
    typedef LinearlyInterpolatedFreeTable<tuple<double,double>,double> table_t;
    
    table_t table(256,0,FY_PA_MAX-FY_THROW+FR_STONE_RAD);
    
    cerr<<table.toDebugString();
    
    for(int itr=0;itr<table.arraySize();++itr){
        fpn_t tr=table.getKey(itr);
        //cerr<<"key = "<<tr<<endl;
        ThinkBoard bd;
        bd.init();
        
        fPosXY<> pos(FX_THROW,FY_THROW + table.getKey(itr));
        pushStone(&bd,BLACK,pos);
        
        fMoveXY<> hit;
        hit.setSpin(Spin::RIGHT);
        genHit(&hit, pos, V, 1);
        
        double left,right;
        {
            biSolver solver(-M_PI/2,hit.th());
            
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
            biSolver solver(hit.th(),+M_PI/2);
            
            //cerr<<"[ "<<solver.alpha()<<", "<<solver.beta()<<" ]"<<endl;
            
            for(int i=0;i<100;++i){
                ThinkBoard tbd=bd;
                
                fpn_t val=solver.play();
                
                fMoveVTh<> mv(V,val,Spin::RIGHT);
                
                int ret=FastSimulator::simulateF<0>(&tbd,mv);
                
                //cerr<<val<<mv<<ret<<tbd.toString();
                
                solver.feed((ret==2)?false:true);
            }
            right=solver.answer();
        }
        table.assign(itr,make_tuple(left,right));
        
        //cerr<<tth<<","<<endl;
        //cerr<<"tr = "<<tr<<" tth = "<<tth<<" r*th = "<<tr*tth<<endl;
    }
    
    for(int i=0;i<table.arraySize();++i){
        cerr<<"{";
        cerr<<get<0>(table.accessByIndex(i))<<", ";
        cerr<<get<1>(table.accessByIndex(i))<<", ";
        cerr<<"},";
        cerr<<endl;
    }
    
    return 0;
}