# -*- coding: utf-8 -*-
# dc.py
# Katsuki Ohto

import math
import numpy as np

# constant
BLACK = 1
WHITE = 0

N_ENDS = 10
END_LAST = 0

N_TURNS = 16
TURN_LAST = 0

def to_turn_color(t):
    return t % 2

N_COLOR_STONES = 8
N_STONES = N_COLOR_STONES * 2

SCORE_MIN = -N_COLOR_STONES
SCORE_MAX = +N_COLOR_STONES
SCORE_LENGTH = SCORE_MAX - SCORE_MIN + 1

def StoIDX(s):
    return s - SCORE_MIN

STONE_RADIUS = 0.145
HOUSE_RADIUS = 1.83

W_SPIN = 0.066696

X_TEE = 0
Y_TEE = 0

PLAYAREA_WIDTH = 4.75
PLAYAREA_LENGTH = 8.23

X_PLAYAREA_MIN = X_TEE - PLAYAREA_WIDTH / 2
X_PLAYAREA_MAX = X_TEE + PLAYAREA_WIDTH / 2
Y_PLAYAREA_MIN = Y_TEE + HOUSE_RADIUS - PLAYAREA_LENGTH
Y_PLAYAREA_MAX = Y_TEE + HOUSE_RADIUS

X_THROW = X_TEE
Y_THROW = Y_PLAYAREA_MIN - 30.0

R_IN_HOUSE = HOUSE_RADIUS + STONE_RADIUS
R2_IN_HOUSE = R_IN_HOUSE * 2

XY_TEE = (X_TEE, Y_TEE)
XY_THROW = (X_THROW, Y_THROW)

VX_TEE_SHOT_R = -0.99073974
VY_TEE_SHOT = +29.559775

RIGHT = 0
LEFT = 1

TEE_SHOT_R = (VX_TEE_SHOT_R, VY_TEE_SHOT, RIGHT)

ERROR_SIGMA = 0.145
ERROR_SCALE_X = 0.5 # gat version
ERROR_SCALE_Y = 2.0 # gat version
    
VX_ERROR_SIGMA = 0.117659 * ERROR_SCALE_X
VY_ERROR_SIGMA = 0.0590006 * ERROR_SCALE_Y

def official_to_ayumu_turn(t):
    return N_TURNS - 1 - t

def official_to_ayumu_position(p):
    return (p[0] - 2.375, 4.88 - p[1])

def ayumu_to_official_move(mv):
    return (mv[0], -mv[1], mv[2])

def is_in_house_r(r):
    return bool(r < R_IN_HOUSE)

def is_in_house_r2(r2):
    return bool(r2 < R2_IN_HOUSE)

def is_in_house_xy(x, y):
    dx = x - X_TEE
    dy = y - Y_TEE
    return is_in_house_r2(dx * dx + dy * dy)

def is_in_house(pos):
    return is_in_house_xy(pos[0], pos[1])

def is_in_play_area_xy(x, y):
    return bool((X_PLAYAREA_MIN < x) and (x < X_PLAYAREA_MAX) and (Y_PLAYAREA_MIN < y) and (y < Y_PLAYAREA_MAX))

def is_in_play_area(pos):
    return is_in_play_area_xy(pos[0], pos[1])

def calc_r2(a, b):
    return (b[0] - a[0]) ** 2 + (b[1] - a[1]) ** 2
def calc_r(a, b):
    return np.hypot(b[0] - a[0], b[1] - a[1])
def calc_th(a, b = None):
    if b is None:
        return np.arctan2(a[0], a[1])
    else:
        return np.arctan2(b[0] - a[0], b[1] - a[1])

def calc_v2(vxy):
    return (vxy[0] ** 2) + (vxy[1] ** 2)

def calc_v(vxy):
    return np.hypot(vxy[0], vxy[1])

class Board:
    def __init__(self):
        self.init();
    
    def init(self):
        self.end = END_LAST
        self.turn = TURN_LAST
        self.rscore = 0
        self.stone = np.empty(N_STONES, dtype = tuple)
    
    def locate_in_throw_point(self):
        for i in range(N_STONES):
            self.stone[i] = XY_THROW

def count_in_house_a(sa, color = None): # count num of stones in house
    cnt = 0
    if color is None:
        lst = range(N_STONES)
    else:
        lst = range(color, N_STONES, 2)
    
    for i in lst:
        if is_in_house(sa[i]):
            cnt += 1
    return cnt

def count_in_play_area_a(sa, color = None): # count num of stones in play area
    cnt = 0
    if color is None:
        lst = range(N_STONES)
    else:
        lst = range(color, N_STONES, 2)

    for i in lst:
        if is_in_play_area(sa[i]):
            cnt += 1
    return cnt

def count_score_a(sa): # count stone score by array
    bmin2 = R2_IN_HOUSE
    wmin2 = R2_IN_HOUSE
    for i in range(BLACK, N_STONES, 2):
        st = sa[i]
        if is_in_play_area(st):
            r2 = calc_r2(st, XY_TEE)
            bmin2 = min(bmin2, r2)
    for i in range(WHITE, N_STONES, 2):
        st = sa[i]
        if is_in_play_area(st):
            r2 = calc_r2(st, XY_TEE)
            wmin2 = min(wmin2, r2)
    cnt = 0
    if bmin2 > wmin2:
        for i in range(WHITE, N_STONES, 2):
            st = sa[i]
            if is_in_play_area(st):
                r2 = calc_r2(st, XY_TEE)
                if r2 < bmin2:
                    cnt -= 1
    elif bmin2 < wmin2:
        for i in range(BLACK, N_STONES, 2):
            st = sa[i]
            if is_in_play_area(st):
                r2 = calc_r2(st, XY_TEE)
                if r2 < wmin2:
                    cnt += 1
    return cnt

def count_score(bd): # count stone score on board
    return count_score_a(bd.stone)

def is_caving_in_pp(p0, p1):
    return (calc_r2(p0, p1) < ((2 * STONE_RADIUS) ** 2))

def is_caving_in_bp(bd, p):
    for i in range(N_STONES):
        if is_caving_in_pp(bd.stone[i], p):
            return True
    return False

def locate_in_play_area_p():
    return (X_PLAYAREA_MIN + np.random.rand() * PLAYAREA_WIDTH,
            Y_PLAYAREA_MIN + np.random.rand() * PLAYAREA_LENGTH)

def locate_in_house_p():
    r = np.random.rand() * R_IN_HOUSE
    th = np.random.rand() * 2 * math.pi
    return (X_TEE + r * math.sin(th), Y_TEE + r * math.cos(th))

def locate_in_play_area_b(nb, nw):
    bd = Board()
    bd.locate_in_throw_point()
    for i in range(nb): # black
        while True:
            pos = locate_in_play_area_p()
            if not is_caving_in_bp(bd, pos): # ok
                bd.stone[N_STONES - 1 - 2 * i] = pos
                break
    for i in range(nw): # white
        while True:
            pos = locate_in_play_area_p()
            if not is_caving_in_bp(bd, pos): # ok
                bd.stone[N_STONES - 2 - 2 * i] = pos
                break
    return bd

"""SHOTLOG_NORMAL_VARIABLE =(
                   ('player', type(string)),
                   ('opp_player', string),
                   ('draw_game', int),
                   ('random', float),
                   ('end', int),
                   ('turn', int),
                   ('rel_score', int),
                   ('score', int),
                   ('rest_time', int),
                   ('used_time', int))"""

SHOTLOG_NORMAL_VARIABLE =(
                          'player',
                          'opp_player',
                          'draw_game',
                          'random',
                          'end',
                          'turn',
                          'rscore',
                          'escore',
                          'rest_time',
                          'used_time')

def shotlog_to_string(sl):
    lst = []
    for var in SHOTLOG_NORMAL_VARIABLE:
        lst.append(str(sl[var]))
    cmv = sl['chosen_move']
    rmv = sl['run_move']
    for v in cmv:
        lst.append(str(v))
    for v in rmv:
        lst.append(str(v))
    prvs = sl['previous_stone']
    afts = sl['after_stone']
    for s in prvs:
        for v in s:
            lst.append(str(v))
    for s in afts:
        for v in s:
            lst.append(str(v))
    return ' '.join(lst);

def string_to_shot_log(str):
    v = str.split(' ')
    sl = {}
    sl['player'] = v[0]
    sl['opp_player'] = v[1]
    sl['draw_game'] = int(v[2])
    sl['random'] = float(v[3])
    sl['end'] = int(v[4])
    sl['turn'] = int(v[5])
    sl['rscore'] = int(v[6])
    sl['escore'] = int(v[7])
    sl['rest_time'] = int(v[8])
    sl['used_time'] = int(v[9])
    sl['chosen_move'] = (float(v[10]), float(v[11]), int(v[12]))
    sl['run_move'] = (float(v[13]), float(v[14]), int(v[15]))
    p = np.empty(N_STONES, dtype = tuple)
    a = np.empty(N_STONES, dtype = tuple)
    for i in range(0, N_TURNS):
        index = 16 + i * 2
        x = float(v[index])
        y = float(v[index + 1])
        p[i] = (x, y)

    for i in range(0, N_TURNS):
        index = 16 + N_TURNS * 2 + i * 2
        x = float(v[index])
        y = float(v[index + 1])
        a[i] = (x, y)
    sl['previous_stone'] = p
    sl['after_stone'] = a
    return sl

def load_shot_log(file_path):
    # read log
    f = open(file_path)
    logs = []
    for line in f:
        line = line.rstrip()
        #print(line)
        sl = string_to_shot_log(line)
        logs.append(sl)
        #print shotlog_to_string(sl)
    return logs

def shot_log_to_board(sl):
    bd = Board()
    bd.end = sl['end']
    bd.turn = sl['turn']
    bd.rel_score = sl['rscore']
    ps = sl['previous_stone']
    for i in range(0, N_STONES):
        bd.stone[i] = ps[N_STONES - 1 - i]
    return bd

#IMAGE_WIDTH = 28
#IMAGE_LENGTH = 28
#IMAGE_PLAINS = 1

IMAGE_WIDTH = 27
IMAGE_LENGTH = 51
IMAGE_PLAINS = 5

IMAGE_SIZE = IMAGE_WIDTH * IMAGE_LENGTH

STEP_W_TO_X = PLAYAREA_WIDTH / (IMAGE_WIDTH - 1)
STEP_W_TO_Y = PLAYAREA_LENGTH / (IMAGE_LENGTH - 1)

def WtoX(w):
    return X_PLAYAREA_MIN + w * STEP_W_TO_X
def LtoY(l):
    return Y_PLAYAREA_MIN + l * STEP_W_TO_Y

NORM_SIGMA = STONE_RADIUS / 2
def norm(m, o):
    return math.exp(-(((o[0] - m[0]) ** 2) + ((o[1] - m[1]) ** 2)) / (2 * (NORM_SIGMA ** 2))) / (math.sqrt(2 * math.pi) * NORM_SIGMA)
                                                                                                               
def board_to_image(bd):
    img = np.zeros((IMAGE_WIDTH, IMAGE_LENGTH, IMAGE_PLAINS), dtype = float)
    for w in range(IMAGE_WIDTH):
        for l in range(IMAGE_LENGTH):
            for p in range(IMAGE_PLAINS):
                v = 0.0
                m = (WtoX(w), LtoY(l))
                # white
                for i in range(WHITE, N_STONES, 2):
                    o = bd.stone[i]
                    if is_in_play_area(o):
                        v -= norm(m, o)
                # black
                for i in range(BLACK, N_STONES, 2):
                    o = bd.stone[i]
                    if is_in_play_area(o):
                        v += norm(m, o)
            img[w][l][p] = v
    return img
                        

