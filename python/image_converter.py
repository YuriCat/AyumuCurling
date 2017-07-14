# -*- coding: utf-8 -*-
# image_converter.py
# Katsuki Ohto
# 2016/4/1

# 文字列の画像を学習時のnumpy形式に保存

import sys

import numpy as np
import dc
import image_log as il

BATCH = 256

def convert_to_np_turn(path, t):
    file_name = path + "_" + str(t) + ".dat"
    img, end, turn, rsc, esc = il.load_image_log_array(file_name, 250000)
    
    img_name = path + "_np_img"
    esc_name = path + "_np_esc"
    
    """# ターンごとに分けて保存
    imgs = [[] for _ in range(dc.N_TURNS)]
    escs = [[] for _ in range(dc.N_TURNS)]
    
    for i in range(len(img)):
        print(turn[i])
        t = np.argmax(turn[i])
        imgs[t].append(img[i])
        escs[t].append(esc[i])
    
    for t in range(dc.N_TURNS):
        # 細切れに保存"""
    cnt = 0
    for i in range(0, len(img), BATCH):
        np.savez_compressed(img_name + "_" + str(t) + "_" + str(cnt), img[i : (i + BATCH)])
        np.savez_compressed(esc_name + "_" + str(t) + "_" + str(cnt), esc[i : (i + BATCH)])
        cnt += 1

if __name__ == "__main__":
    args = sys.argv
    path = args[1]
    for t in range(dc.N_TURNS):
        convert_to_np_turn(path, t)
