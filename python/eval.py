# -*- coding: utf-8 -*-
# eval.py
# Katsuki Ohto
# http://kivantium.hateblo.jp/entry/2015/11/18/233834

import sys

import numpy as np
import tensorflow as tf

import dc
import image_log

import model.cnn as mdl

#def get_estimation_vector(img):
#    return model.eval(feed_dict =)

N_PHASES = 16

if __name__ == '__main__':
    shot_logs = dc.load_shot_log(sys.argv[1])
    model_path = sys.argv[2]
    
    data_num = len(shot_logs)
    
    images = np.empty((data_num, dc.IMAGE_WIDTH, dc.IMAGE_LENGTH, dc.IMAGE_PLAINS), dtype = np.float32)
    phases = []
    scores = []

    for i, sl in enumerate(shot_logs):
        bd = dc.shot_log_to_board(sl)
        images[i] = dc.board_to_image(bd)
        #phases.append(dc.to_turn_color(bd.turn))
        phases.append(bd.turn)
        scores.append(sl['escore'])

    x_placeholder = tf.placeholder("float", shape = (None, dc.IMAGE_WIDTH, dc.IMAGE_LENGTH, dc.IMAGE_PLAINS))
    labels_placeholder = tf.placeholder("float", shape = (None, dc.SCORE_LENGTH))
    keep_prob = tf.placeholder("float")
    
    y_soft = [mdl.make(x_placeholder, keep_prob) for ph in range(N_PHASES)]
    prob_op = [mdl.normalize(y_soft[ph]) for ph in range(N_PHASES)]

    sess = tf.InteractiveSession()
    
    saver = tf.train.Saver()
    sess.run(tf.initialize_all_variables())
    saver.restore(sess, model_path)
    
    for i, img in enumerate(images) :
        prob = sess.run(prob_op[phases[i]], feed_dict = {x_placeholder : [img],
                                     keep_prob : 1.0 })[0]
        print("end %d turn %d" % (((i / 16) + 1), ((i % 16) + 1)))
        print(scores[i])
        print([int(p * 100) for p in prob])

