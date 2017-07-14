# -*- coding: utf-8 -*-
# linear.py
# Katsuki Ohto
# https://www.tensorflow.org/versions/0.6.0/tutorials/mnist/pros/index.html より

import numpy as np
import tensorflow as tf

# 単純な線形モデル
in_size = 784
out_size = 17

def make(x_placeholder, keep_prob):
    # モデルを作成する関数
    # 学習させるパラメータ
    with tf.name_scope('linear') as scope:
        W = tf.Variable(tf.zeros([in_size, out_size]))
        b = tf.Variable(tf.zeros([out_size]))

    # softmax
    with tf.name_scope('softmax') as scope:
        y = tf.nn.softmax(tf.matmul(x_placeholder, W) + b)

    return y

def calc_loss(y_soft, labels):
    cross_entropy = -tf.reduce_sum(labels * tf.log(y_soft))
    return cross_entropy

def train(loss, learning_rate = 0.01):
    train_step = tf.train.GradientDescentOptimizer(learning_rate).minimize(loss)
    return train_step

def calc_accuracy(y_soft, labels):
    correct_prediction = tf.equal(tf.argmax(y_soft, 1), tf.argmax(labels, 1))
    accuracy = tf.reduce_mean(tf.cast(correct_prediction, "float"))
    return accuracy