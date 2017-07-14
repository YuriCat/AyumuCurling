# -*- coding: utf-8 -*-
# image_log.py
# Katsuki Ohto

import sys
import numpy as np

import dc

def string_to_image_log(str):
    v = str.split(' ')
    e = int(v[0])
    t = int(v[1])
    rsc = int(v[2])
    esc = int(v[3])
    img = np.empty((dc.IMAGE_WIDTH, dc.IMAGE_LENGTH, dc.IMAGE_PLAINS), dtype = np.float32)
    for w in range(dc.IMAGE_WIDTH):
        for l in range(dc.IMAGE_LENGTH):
            for p in range(dc.IMAGE_PLAINS):
                img[w][l] = np.float32(v[4
                                         + w * dc.IMAGE_LENGTH * dc.IMAGE_PLAINS
                                         + l * dc.IMAGE_PLAINS
                                         + p])
    return (e, t, rsc, esc, img)


def load_image_log(file_path):
    #read image log
    f = open(file_path, 'r')
    logs = []
    for line in f:
        line = line.rstrip()
        il = string_to_image_log(line)
        logs.append(il)
    return logs

def load_image_log_array(file_path, size): # 画像のみロード
    f = open(file_path)
    img = np.empty((size, dc.IMAGE_WIDTH, dc.IMAGE_LENGTH, dc.IMAGE_PLAINS), dtype = np.float32)
    end = np.zeros((size, dc.N_ENDS), dtype = np.float32)
    turn = np.zeros((size, dc.N_TURNS), dtype = np.float32)
    rsc = np.zeros((size, dc.SCORE_LENGTH), dtype = np.float32)
    esc = np.zeros((size, dc.SCORE_LENGTH), dtype = np.float32)
    
    i = 0
    for line in f:
        if i >= size:
            break
        v = line.split(' ')
        end[i][int(v[0])] = 1
        turn[i][int(v[1])] = 1
        rsc[i][np.clip(int(v[2]), dc.SCORE_MIN, dc.SCORE_MAX) - dc.SCORE_MIN] = 1
        esc[i][int(v[3]) - dc.SCORE_MIN] = 1
        
        #print(esc[i])
        
        for w in range(dc.IMAGE_WIDTH):
            for l in range(dc.IMAGE_LENGTH):
                for p in range(dc.IMAGE_PLAINS):
                    img[i][w][l][p] = np.float32(v[4
                                                   + w * dc.IMAGE_LENGTH * dc.IMAGE_PLAINS
                                                   + l * dc.IMAGE_PLAINS
                                                   + p])
        i += 1
    if i < size:
        size = i

    return (img[0 : size], end[0 : size], turn[0 : size], rsc[0 : size], esc[0 : size])

def load_image_esc_np(path, ph, i):
    image_name = path + "_np_img_" + str(ph) + "_" + str(i) + ".npz"
    label_name = path + "_np_esc_" + str(ph) + "_" + str(i) + ".npz"
    imagez = np.load(image_name)
    labelz = np.load(label_name)
    ret = (imagez['arr_0'], labelz['arr_0'])
    imagez.close()
    labelz.close()
    return ret

if __name__ == "__main__":
    # make image log from shot log
    in_file = sys.argv[1]
    out_file = sys.argv[2]

    f = open(out_file, 'w')
    f.close()
    
    ifile = open(in_file, 'r')

    for line in ifile:
        line = line.rstrip()
        print(line)
        
        sl = dc.string_to_shot_log(line)
        
        f = open(out_file, 'a')
        
        bd = dc.shot_log_to_board(sl)
        img = dc.board_to_image(bd)
        e = sl['end']
        t = sl['turn']
        rsc = sl['rel_score']
        esc = sl['score']
        il = (e, t, rsc, esc, img)
    
        f.write(str(il[0]))
        for i in range(1, 3):
            f.write(' ')
            f.write(str(il[i]))
        img = il[4]
        for w in range(dc.IMAGE_WIDTH):
            for l in range(dc.IMAGE_LENGTH):
                for p in range(dc.IMAGE_PLAINS):
                    f.write(' ')
                    f.write(str(img[w][l]))
        f.write('\n')
        f.close()

    ifile.close()






