# -*- coding: utf-8 -*-
# ayumu_dc.py
# Katsuki Ohto

import math
import numpy as np

import simulator

# move
VX_MIN = -3.1
VX_MAX = +3.1
VY_MIN = +27.1
VY_MAX = +33.3

# grid integration

GRID_WIDTH = 256 - 1
GRID_LENGTH = 256 - 1
GRID_ZSGMDVX = 4;
GRID_ZSGMDVY = 4;
GRID_ZWIDTH = GRID_WIDTH * GRID_ZSGMDVX * dc.VX_ERROR_SIGMA / (VX_MAX - VX_MIN);
GRID_ZLENGTH = GRID_LENGTH * GRID_ZSGMDVY * dc.VY_ERROR_SIGMA / (VY_MAX - VY_MIN);

def WtoVX(w):
    return VX_MIN + (VX_MAX - VX_MIN) * w / GRID_WIDTH

def LtoVY(l):
    return VY_MIN + (VY_MAX - VY_MIN) * l / GRID_LENGTH

pdf_table = np.empty(())

def init_grid_pdf():
                for(int dw = 0; dw <= ROOT_LAST_GRID_ZW; ++dw){
                    for(int dl = 0; dl <= ROOT_LAST_GRID_ZL; ++dl){
                        pdfTableLast[dw][dl] = relativePdfDeltaVxVy(lastRootChild_t::DWtoDVX(dw),
                                                                    lastRootChild_t::DLtoDVY(dl),
                                                                    Spin::RIGHT);
                }
                }
                eval_t sum = 0;
                for(int dw = -ROOT_LAST_GRID_ZW; dw <= ROOT_LAST_GRID_ZW; ++dw){
                    for(int dl = -ROOT_LAST_GRID_ZL; dl <= ROOT_LAST_GRID_ZL; ++dl){
                        sum += pdfTableLast[abs(dw)][abs(dl)];
                }
                }
                for(int dw = 0; dw <= ROOT_LAST_GRID_ZW; ++dw){
                    for(int dl = 0; dl <= ROOT_LAST_GRID_ZL; ++dl){
                        pdfTableLast[dw][dl] /= sum;
                }
                }
                CERR << "1 step dvx = " << lastRootChild_t::DWtoDVX(1) << " dvy = " << lastRootChild_t::DLtoDVY(1) << endl;
                CERR << "whole  dvx  = " << lastRootChild_t::DWtoDVX(lastRootChild_t::width()) << " dvy = " << lastRootChild_t::DLtoDVY(lastRootChild_t::length()) << endl;
                
                fMoveXY<> fmv[2];
                fPosXY<> pos[2];
                fmv[0].setSpin(Spin::RIGHT);
                FastSimulator::FXYtoFMV(FPOSXY_TEE, &fmv[0]);
                fmv[1] = fmv[0];
                fmv[1].x += lastRootChild_t::DWtoDVX(1);
                FastSimulator::FMVtoFXY(fmv[1], &pos[0]);
                fmv[1] = fmv[0];
                fmv[1].y += lastRootChild_t::DLtoDVY(1);
                FastSimulator::FMVtoFXY(fmv[1], &pos[1]);
                
                CERR << "1 step dx  = " << (pos[0].x - FX_TEE) << " dy = " << (pos[1].y - FY_TEE) << endl;
                CERR << "fuzzy  dx  = " << (pos[0].x - FX_TEE) * ROOT_LAST_GRID_ZW << " dy = " << (pos[1].y - FY_TEE) * ROOT_LAST_GRID_ZL << endl;
                CERR << "inner  dx  = " << (pos[0].x - FX_TEE) * (ROOT_LAST_GRID_W - ROOT_LAST_GRID_ZW) << " dy = " << (pos[1].y - FY_TEE) * (ROOT_LAST_GRID_L - ROOT_LAST_GRID_ZL) << endl;
                CERR << "whole  dx  = " << (pos[0].x - FX_TEE) * ROOT_LAST_GRID_W << " dy = " << (pos[1].y - FY_TEE) * ROOT_LAST_GRID_L << endl;
                
                CERR << "pdfTableLast[][] = {" << endl;
                for(int dw = 0; dw <= ROOT_LAST_GRID_ZW; ++dw){
                    for(int dl = 0; dl <= ROOT_LAST_GRID_ZL; ++dl){
                        CERR << pdfTableLast[dw][dl] << ", ";
                    }CERR << endl;
        }
            CERR << "}" << endl;
        }
                    }