# -*- coding: utf-8 -*-
# est_learn_cnn.py
# Katsuki Ohto
# http://kivantium.hateblo.jp/entry/2015/11/18/233834

import sys
import time
import locale

import numpy as np

import tensorflow as tf

import dc
import image_log

#import model.linear as mdl
import model.cnn_estimator as mdl

N_PHASES = 16
N_TRAIN_BATCHES = 600
N_TEST_BATCHES = 4

if __name__ == "__main__":

    args = sys.argv
 
    # データのロードと設定
    path = args[1]
    mepoch = int(args[2])
    batch_size = int(args[3])
    mdl_file = args[4]

    """images = []
    labels = []
    escs = []
    for ph in range(N_PHASES):
        img, lbl = image_log.load_image_esc_np(path, ph, n)
        esc = np.argmax(lbl, axis = 1)
        images.append(img)
        labels.append(lbl)
        escs.append(esc)

    test_rate = 0.1
    data_num = [min(32784, len(images[ph])) for ph in range(N_PHASES)]
    test_num = [int(data_num[ph] * test_rate) for ph in range(N_PHASES)]
    train_num = [data_num[ph] - test_num[ph] for ph in range(N_PHASES)]"""
    
    # test-data loading
    img_test = []
    esc_test = []
    for ph in range(N_PHASES):
        imgs, lbls = image_log.load_image_esc_np(path, ph, N_TRAIN_BATCHES)
        for i in range(1, N_TEST_BATCHES, 1):
            img, lbl = image_log.load_image_esc_np(path, ph, N_TRAIN_BATCHES + i)
            imgs = np.r_[imgs, img]
            lbls = np.r_[lbls, lbl]
        
        #print(imgs)
        #print(lbls)
        
        img_test.append(imgs)
        esc_test.append(np.argmax(lbls, axis = 1))

    with tf.Graph().as_default():
        # 画像を入れる仮のTensor
        x_placeholder = tf.placeholder("float",
                                       shape = (None, dc.IMAGE_WIDTH, dc.IMAGE_LENGTH, dc.IMAGE_PLAINS))
        # ラベルを入れる仮のTensor
        labels_placeholder = tf.placeholder("float", shape = (None, dc.SCORE_LENGTH))
        
        # 正解を入れる仮のTensor
        escs_placeholder = tf.placeholder("int64", shape = (None))
        
        # dropout率を入れる仮のTensor
        keep_prob = tf.placeholder("float")
        
        # モデル作成
        y_op = [mdl.make(x_placeholder, keep_prob) for ph in range(N_PHASES)]
        # 正規化
        prob_op = [mdl.normalize(y_op[ph]) for ph in range(N_PHASES)]
        # loss計算
        loss_op = [mdl.calc_loss(prob_op[ph], labels_placeholder) for ph in range(N_PHASES)]
        # training
        train_op = [mdl.train(loss_op[ph], 0.0003) for ph in range(N_PHASES)]
        # accuracy
        test_op = [mdl.count_correct_num(y_op[ph], escs_placeholder) for ph in range(N_PHASES)]
        
        # 保存の準備
        saver = tf.train.Saver()
        # Sessionの作成
        sess = tf.Session()
        # 変数の初期化
        sess.run(tf.initialize_all_variables())
        # パラメータ読み込み
        if mdl_file != "none":
            saver.restore(sess, mdl_file)
        
        # TensorBoardで表示する値の設定
        #summary_op = tf.merge_all_summaries()
        #summary_writer = tf.train.SummaryWriter(FLAGS.train_dir, sess.graph_def)
        
        for e in range(mepoch):
            
            # training phase
            tstart = time.time()
            for ph in range(N_PHASES):
                # data loading
                index = np.random.randint(N_TRAIN_BATCHES)
                img, lbl = image_log.load_image_esc_np(path, ph, index)
                print("ph = %d, id = %d" % (ph, index))
                for i in range(0, len(img), batch_size):
                    sess.run(train_op[ph], feed_dict={
                             x_placeholder : img[i : (i + batch_size)],
                             labels_placeholder : lbl[i : (i + batch_size)],
                             keep_prob : 0.5})
            print("training %f sec" % (time.time() - tstart))

            if e % 16 != 0:
                continue

            # test phase
            tstart = time.time()
            """for ph in range(N_PHASES):
                correct_num = 0
                for i in range(0, train_num[ph], batch_size):
                    iend = min(i + batch_size, train_num[ph])
                    correct_num += sess.run(test_op[ph],
                                            feed_dict = {
                                            x_placeholder : images[ph][i : iend],
                                            escs_placeholder : escs[ph][i : iend],
                                            keep_prob : 1.0})
                print("[epoch %d] phase %d, train accuracy %g" % (e, ph, correct_num / float(train_num[ph])))"""
            for ph in range(N_PHASES):
                correct_num = 0
                for i in range(0, len(img_test[ph]), batch_size):
                    correct_num += sess.run(test_op[ph],
                                            feed_dict = {
                                            x_placeholder : img_test[ph][i : (i + batch_size)],
                                            escs_placeholder : esc_test[ph][i : (i + batch_size)],
                                            keep_prob : 1.0})
                print("[epoch %d] phase %d, test accuracy %g" % (e, ph, correct_num / float(len(img_test[ph]))))
            print("test %f sec" % (time.time() - tstart))
            
            save_path = saver.save(sess, "tf_estimator_cnn.ckpt")


