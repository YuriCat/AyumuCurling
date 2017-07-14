# -*- coding: utf-8 -*-
# gaussian_net.py
# Katsuki Ohto

import numpy as np
import chainer.functions as F

import dc

N_LAYERS = 4
N_FILTERS = 64

# 変数
#model = FunctionSet(#lin0 = [F.Linear(4, 4) for i in range(N_FILTERS)],
#                    )

w0 = chainer.Variable(np.array(4, 4, N_FILTERS), dtype = np.float32)
b0 = chainer.Variable(np.array(4, N_FILTERS), dtype = np.float32)
sig0 = chainer.Variable(np.array(N_FILTERS), dtype = np.float32)


# 線形

# RBF関数
def rbf_func(i, s):
    return F.exp(-F.sum(i * i) / s)

# フィルター
def rbf_filter(i, w, b, s):
    j = F.matmul(i, w) + b
    return rbf_func(j, s)

# 第一層
# 入力は石の位置(r, th) ~ 相対位置(r, th)
def l0(input, n):
    next = [chainer.Variable(np.float32) for _ in n]
    for j in N_FILTERS:
        for i in range(n):
            next[i] = rbf_filter(input[i], w0[j], )
    return
