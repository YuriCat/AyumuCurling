# -*- coding: utf-8 -*-
# score_learn.py
# Katsuki Ohto

import sys
import numpy as np

import tensorflow as tf

import dc
import image_log

#import model.linear as mdl
import model.cnn as mdl

in_size = dc.IMAGE_SIZE
out_size = dc.SCORE_LENGTH

if __name__ == "__main__":

    args = sys.argv
 
    # shot-log を読みこみ
    data_num = int(args[1])
    mepoch = int(args[2])
    batch_size = int(args[3])

    # training data, test dataを作る
    
    image_vector_all = np.empty((data_num, in_size), dtype = np.float32)
    score_vector_all = np.zeros((data_num, out_size), dtype = np.float32)
    
    score_vector_sum = np.zeros(out_size, dtype = np.float32)

    for i in range(data_num):
        bd = dc.locate_in_play_area_b(8, 8)
        image = dc.board_to_image(bd)
        image_vector_all[i] = image.ravel()
        
        sc_index = dc.StoIDX(dc.count_score(bd))
        score_vector_all[i][sc_index] = 1
        score_vector_sum[sc_index] += 1 # 事前解析

    print("number of used images = %d" % data_num)

    test_rate = 0.2
    test_num = int(data_num * test_rate)
    train_num = data_num - test_num

    print("score distribution = ")
    print((score_vector_sum / data_num * 1000).astype(int))
    
    with tf.Graph().as_default():
        # 画像を入れる仮のTensor
        x_placeholder = tf.placeholder("float", shape = (None, in_size))
        # ラベルを入れる仮のTensor
        labels_placeholder = tf.placeholder("float", shape = (None, out_size))
        # dropout率を入れる仮のTensor
        keep_prob = tf.placeholder("float")
        
        # モデル作成
        y_soft = [mdl.make(x_placeholder, keep_prob) for ph in range(N_PHASES)]
        # loss計算
        loss_value = [mdl.calc_loss(y_soft[ph], labels_placeholder) for ph in range(N_PHASES)]
        # training()を呼び出して訓練
        train_op = [mdl.train(loss_value[ph], 0.0005) for ph in range(N_PHASES)]
        # 精度の計算
        acc_op = [mdl.calc_accuracy(y_soft[ph], labels_placeholder) for ph in range(N_PHASES)]
        
        # 保存の準備
        saver = tf.train.Saver()
        # Sessionの作成
        sess = tf.Session()
        # 変数の初期化
        sess.run(tf.initialize_all_variables())
        # TensorBoardで表示する値の設定
        #summary_op = tf.merge_all_summaries()
        #summary_writer = tf.train.SummaryWriter(FLAGS.train_dir, sess.graph_def)
        
        for e in range(mepoch):
            
            # training phase
            for ph in range(N_PHASES):
                for bi in range((train_num[ph] - 1) // batch_size + 1):
                    
                    # make batch
                    tmp_batch_size = min(batch_size, train_num[ph] - bi * batch_size)
                    
                    istart = bi * batch_size
                    iend = min((bi + 1) * batch_size, train_num[ph])
                    
                    # トレーニング
                    # feed_dictでplaceholderに入れるデータを指定する
                    sess.run(train_op[ph], feed_dict={
                             x_placeholder : image_vector[ph][istart : iend],
                             labels_placeholder : score_vector[ph][istart : iend],
                             keep_prob : 0.5})
        
            # test phase
            # 1 step終わるたびに精度を計算する
            for ph in range(N_PHASES):
                acc_sum = 0
                for i in range(train_num[ph], phase_data_num[ph], 1):
                    acc_sum += sess.run(acc_op[ph],
                                        feed_dict = {
                                        x_placeholder : image_vector[ph][i : (i + 1)],
                                        labels_placeholder : score_vector[ph][i : (i + 1)],
                                        keep_prob : 1.0})
                print("[epoch %d] phase %d, test accuracy %g" % (e, ph, acc_sum / test_num[ph]))
            
            # 1 step終わるたびにTensorBoardに表示する値を追加する
            """summary_str = sess.run(summary_op, feed_dict={
                x_placeholder : image_vector_all[],
                labels_placeholder: score_vector_all,
                keep_prob: 1.0})
                summary_writer.add_summary(summary_str, step)"""
            
                    save_path = saver.save(sess, "ev_model_phase.ckpt")

np.set_printoptions(threshold = 100000)
    
    if True:
        sess.run(tf.initialize_all_variables()) # 初期化処理の実行
        for e in range(mepoch):
            
            # training phase
            for bi in range((train_num - 1) // batch_size + 1):
                
                # make batch
                tmp_batch_size = min(batch_size, train_num - bi * batch_size)
                
                istart = bi * batch_size
                iend = min((bi + 1) * batch_size, train_num)
                
                image_vector = image_vector_all[istart : iend]
                score_vector = score_vector_all[istart : iend]
                
                #print(image_vector)
                #print(score_vector)
                #print(len(image_vector))
                #print(len(score_vector))
                
                #print(y.dtype)
            
                # トレーニング
                mdl.train_step.run(feed_dict = {mdl.x: image_vector, mdl.y_: score_vector, mdl.keep_prob: 0.5})


            # test phase
            #print(sess.run(W))
            #print(sess.run(mdl.b_fc2))
            
            train_success = 0
            test_success = 0
            train_pred_distribution = np.zeros(out_size)
            test_pred_distribution = np.zeros(out_size)
            
            for i in range(0, train_num):
                train_success += int(sess.run(mdl.correct_prediction, feed_dict = {mdl.x: image_vector_all[i : (i + 1)], mdl.y_: score_vector_all[i : (i + 1)], mdl.keep_prob: 1.0})[0])
            for i in range(train_num, data_num):
                ok = sess.run(mdl.correct_prediction, feed_dict = {mdl.x: image_vector_all[i : (i + 1)], mdl.y_: score_vector_all[i : (i + 1)], mdl.keep_prob: 1.0})[0]
                #print(sess.run(tf.matmul(x, W) + b, feed_dict = {x: image_vector_test[i : (i + 1)], y_: score_vector_test[i : (i + 1)]}))
                #print(score_vector_all[i : (i + 1)])
                #print(sess.run(tf.argmax(score_vector_all[i : (i + 1)], 1)))
                #print(ok)
                test_success += int(ok)
            
            
            # 正解率の表示
            print("epoch %d" % e)
            print("training accuracy %d / %d (%f pct)" % (train_success, train_num, train_success * 100 / float(train_num)))
            print("test     accuracy %d / %d (%f pct)" % (test_success, test_num, test_success * 100 / float(test_num)))

    np.set_printoptions(threshold=100000)
    #print(sess.run(W))
    #print(sess.run(b))


