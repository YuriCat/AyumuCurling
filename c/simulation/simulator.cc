// Digital Curling
// Simulating by standard input

#include <iostream>

#include "../dc.hpp"
#include "simufunc.hpp"
#include "b2dsimulator.hpp"
#include "fastsimulator.hpp"

int main(int argc, char *argv[]){
    
    // input board information
    MiniBoard bd;
    for(int i = 0; i < N_STONES; ++i){
        fPosXY<> p;
        std::cin >> p.x >> p.y;
        if(isInPlayArea(p)){
            bd.locateStone(i, p);
        }
    }
    // input shot information
    int t;
    std::cin >> t;
    
    fMoveXY<> mv;
    std::cin >> mv.x >> mv.y >> mv.s;
    
    // simulation
    makeMoveNoRand(&bd, t, mv);
    
    // output board information
    for(int i = 0; i < N_STONES - 1; ++i){
        std::cout << bd.stone(i).x << " " << bd.stone(i).y << " ";
    }
    std::cout << bd.stone(N_STONES - 1).x << " " << bd.stone(N_STONES - 1).y;
    
    return 0;
}
