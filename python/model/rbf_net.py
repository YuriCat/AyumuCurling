# -*- coding: utf-8 -*-
# gaussian_net.py
# Katsuki Ohto

import numpy as np
import tensorflow as tf

import dc

N_LAYERS = 4
N_FILTERS = 64

def make(x_placeholder, keep_prob):
    # モデルを作成する関数
    
    # 初期化関数
    def weight_variable(shape):
        initial = tf.truncated_normal(shape, stddev = 0.01, dtype = tf.float32)
        return tf.Variable(initial)
    
    def bias_variable(shape):
        initial = tf.zeros(shape = shape, dtype = tf.float32)
        return tf.Variable(initial)
    
    # 畳み込み層
    def conv2d(x, W):
        return tf.nn.conv2d(x, W, strides = [1, 1, 1, 1], padding = 'SAME')
    
    # 入力層
    with tf.name_scope('layer0') as scope:
        Ws0 = weight_variable([4, 4, N_FILTERS])
        b0 = bias_variable([4, 4, N_FILTERS])
        for 
    
    # 畳み込み層2
    with tf.name_scope('conv2') as scope:
        W_conv2 = weight_variable([3, 3, N_FILTERS, N_FILTERS])
        b_conv2 = bias_variable([N_FILTERS])
        h_conv2 = tf.nn.relu(conv2d(h_conv1, W_conv2) + b_conv2)
    
    # 畳み込み層3
    with tf.name_scope('conv3') as scope:
        W_conv3 = weight_variable([3, 3, N_FILTERS, N_FILTERS])
        b_conv3 = bias_variable([N_FILTERS])
        h_conv3 = tf.nn.relu(conv2d(h_conv2, W_conv3) + b_conv3)
    
    # 畳み込み層4
    with tf.name_scope('conv4') as scope:
        W_conv4 = weight_variable([3, 3, N_FILTERS, N_FILTERS])
        b_conv4 = bias_variable([N_FILTERS])
        h_conv4 = tf.nn.relu(conv2d(h_conv3, W_conv4) + b_conv4)
    
    # 畳み込み層5
    with tf.name_scope('conv5') as scope:
        W_conv5 = weight_variable([3, 3, N_FILTERS, N_FILTERS])
        b_conv5 = bias_variable([N_FILTERS])
        h_conv5 = tf.nn.relu(conv2d(h_conv4, W_conv5) + b_conv5)
    
    # 畳み込み層6
    with tf.name_scope('conv6') as scope:
        W_conv6 = weight_variable([3, 3, N_FILTERS, N_FILTERS])
        b_conv6 = bias_variable([N_FILTERS])
        h_conv6 = tf.nn.relu(conv2d(h_conv5, W_conv6) + b_conv6)
    
    # 畳み込み層7
    with tf.name_scope('conv7') as scope:
        W_conv7 = weight_variable([3, 3, N_FILTERS, N_FILTERS])
        b_conv7 = bias_variable([N_FILTERS])
        h_conv7 = tf.nn.relu(conv2d(h_conv6, W_conv7) + b_conv7)
    
    # 畳み込み層8
    with tf.name_scope('conv8') as scope:
        W_conv8 = weight_variable([3, 3, N_FILTERS, N_FILTERS])
        b_conv8 = bias_variable([N_FILTERS])
        h_conv8 = tf.nn.relu(conv2d(h_conv7, W_conv8) + b_conv8)
    
    # 畳み込み層9
    with tf.name_scope('conv9') as scope:
        W_conv9 = weight_variable([3, 3, N_FILTERS, N_FILTERS])
        b_conv9 = bias_variable([N_FILTERS])
        h_conv9 = tf.nn.relu(conv2d(h_conv8, W_conv9) + b_conv9)
    
    # 畳み込み層10
    with tf.name_scope('conv10') as scope:
        W_conv10 = weight_variable([3, 3, N_FILTERS, N_FILTERS])
        b_conv10 = bias_variable([N_FILTERS])
        h_conv10 = tf.nn.relu(conv2d(h_conv9, W_conv10) + b_conv10)
    
    # 畳み込み層11
    with tf.name_scope('conv11') as scope:
        W_conv11 = weight_variable([3, 3, N_FILTERS, N_FILTERS])
        b_conv11 = bias_variable([N_FILTERS])
        h_conv11 = tf.nn.relu(conv2d(h_conv10, W_conv11) + b_conv11)
    
    # 畳み込み層12
    with tf.name_scope('conv12') as scope:
        W_conv12 = weight_variable([3, 3, N_FILTERS, N_FILTERS])
        b_conv12 = bias_variable([N_FILTERS])
        h_conv12 = tf.nn.relu(conv2d(h_conv11, W_conv12) + b_conv12)
    
    # 畳み込み層13
    """with tf.name_scope('conv13') as scope:
        W_conv13 = weight_variable([3, 3, N_FILTERS, N_FILTERS])
        b_conv13 = bias_variable([N_FILTERS])
        h_conv13 = tf.nn.relu(conv2d(h_conv12, W_conv13) + b_conv13)
        
        # 畳み込み層14
        with tf.name_scope('conv14') as scope:
        W_conv14 = weight_variable([3, 3, N_FILTERS, N_FILTERS])
        b_conv14 = bias_variable([N_FILTERS])
        h_conv14 = tf.nn.relu(conv2d(h_conv13, W_conv14) + b_conv14)
        
        # 畳み込み層15
        with tf.name_scope('conv15') as scope:
        W_conv15 = weight_variable([3, 3, N_FILTERS, N_FILTERS])
        b_conv15 = bias_variable([N_FILTERS])
        h_conv15 = tf.nn.relu(conv2d(h_conv14, W_conv15) + b_conv15)
        
        # 畳み込み層16
        with tf.name_scope('conv16') as scope:
        W_conv16 = weight_variable([3, 3, N_FILTERS, N_FILTERS])
        b_conv16 = bias_variable([N_FILTERS])
        h_conv16 = tf.nn.relu(conv2d(h_conv15, W_conv16) + b_conv16)
        
        # 畳み込み層17
        with tf.name_scope('conv17') as scope:
        W_conv17 = weight_variable([3, 3, N_FILTERS, N_FILTERS])
        b_conv17 = bias_variable([N_FILTERS])
        h_conv17 = tf.nn.relu(conv2d(h_conv16, W_conv17) + b_conv17)"""
    
    # 全結合層1
    with tf.name_scope('fc1') as scope:
        W_fc1 = weight_variable([dc.IMAGE_WIDTH * dc.IMAGE_LENGTH * N_FILTERS, N_FC_UNITS])
        b_fc1 = bias_variable([N_FC_UNITS])
        h_pool3_flat = tf.reshape(h_conv12, [-1, dc.IMAGE_WIDTH * dc.IMAGE_LENGTH * N_FILTERS])
        h_fc1 = tf.nn.relu(tf.matmul(h_pool3_flat, W_fc1) + b_fc1)
        # dropout
        h_fc1_drop = tf.nn.dropout(h_fc1, keep_prob)
    
    # 全結合層2
    with tf.name_scope('fc2') as scope:
        W_fc2 = weight_variable([N_FC_UNITS, dc.SCORE_LENGTH])
        #b_fc2 = bias_variable([dc.SCORE_LENGTH])
        y = tf.matmul(h_fc1_drop, W_fc2)# + b_fc2
    
    return y