# -*- coding: utf-8 -*-
# est_learn_xgb.py
# Katsuki Ohto

import sys
import copy
import numpy as np

from sklearn.metrics import confusion_matrix
#import matplotlib.pyplot as plt

import xgboost as xgb

import dc

N_PHASES = 16

if __name__ == "__main__":

    args = sys.argv
 
    # データのロードと設定
    shot_logs_train = dc.load_shot_log(args[1])
    loaded_data_num_train = len(shot_logs_train)
    print("number of loaded training shot-logs = %d" % loaded_data_num_train)
    
    shot_logs_test = dc.load_shot_log(args[2])
    loaded_data_num_test= len(shot_logs_test)
    print("number of loaded test shot-logs = %d" % loaded_data_num_test)
    
    # 場合分けのそれぞれのパターン数を求める
    phase_data_num_train = np.zeros(N_PHASES, dtype = np.int32)
    phase_data_num_test = np.zeros(N_PHASES, dtype = np.int32)
    for sl in shot_logs_train:
        phase_data_num_train[sl['turn']] += 1
    for sl in shot_logs_test:
        phase_data_num_test[sl['turn']] += 1

    shot_logs = shot_logs_train
    shot_logs.extend(shot_logs_test)

    phase_index_vector = [[] for ph in range(N_PHASES)]

    for i, sl in enumerate(shot_logs):
        phase_index_vector[sl['turn']].append(i)

    phase_data_num = phase_data_num_train + phase_data_num_test
    data_num = phase_data_num.sum()

    print(phase_data_num_train)
    print(phase_data_num_test)
    print(phase_data_num)
    
    board_vector = [np.empty((phase_data_num[ph], 2 + dc.N_STONES * 4), dtype = np.float32) for ph in range(N_PHASES)]
    score_vector = [np.zeros(phase_data_num[ph], dtype = np.float32) for ph in range(N_PHASES)]
    
    score_vector_sum = np.zeros(dc.SCORE_LENGTH, dtype = np.float32)

    for ph in range(N_PHASES):
        for i in range(phase_data_num[ph]):
            sl = shot_logs[phase_index_vector[ph][i]]
        
            board_vector[ph][i][0] = sl['end']
            board_vector[ph][i][1] = sl['rscore']
        
            for n in range(dc.N_STONES):
            
                st = sl['previous_stone'][n]
                bs = 2 + n * 4
            
                #board_vector[ph][i][bs + 0] = st[0]
                #board_vector[ph][i][bs + 1] = st[1]
                board_vector[ph][i][bs + 0] = dc.calc_r(dc.XY_TEE, st)
                board_vector[ph][i][bs + 1] = dc.calc_th(dc.XY_TEE, st)
                board_vector[ph][i][bs + 2] = dc.calc_r(dc.XY_THROW, st)
                board_vector[ph][i][bs + 3] = dc.calc_th(dc.XY_THROW, st)
        
            #sc = dc.count_score_a(sl['previous_stone'])
            sc = sl['escore']
        
            sc_index = dc.StoIDX(sc)
            score_vector[ph][i] = sc_index
            score_vector_sum[sc_index] += 1 # 事前解析

    print("number of used shot-logs = %d" % data_num)

    print("score distribution = ")
    print((score_vector_sum / data_num * 1000).astype(int))
    print(score_vector_sum)
    
    classifier = [xgb.XGBClassifier(max_depth = 8) for _ in range(N_PHASES)]
    
    for ph in range(N_PHASES):
        classifier[ph].fit(board_vector[ph][0 : phase_data_num_train[ph]], score_vector[ph][0 : phase_data_num_train[ph]])

        #print(classifier.predict(board_vector_all[train_num : data_num]))

        # best_matrix
        best = classifier[ph].predict(board_vector[ph][phase_data_num_train[ph] : phase_data_num[ph]])
        cm = confusion_matrix(score_vector[ph][phase_data_num_train[ph] : phase_data_num[ph]], best)
        cm = (cm / cm.sum().astype(float) * 1000).astype(int)
        print(cm)
        
        # proba_matrix
        prob = classifier[ph].predict_proba(board_vector[ph][phase_data_num_train[ph] : phase_data_num[ph]])
        mat = np.zeros((dc.SCORE_LENGTH, len(prob[0])))
        for i in range(len(prob)):
            #print(best[i])
            #print(prob[i])
            mat[score_vector[ph][i]] += prob[i]
        mat = (mat / mat.sum() * 1000).astype(int)
        print(mat)

        # tree
        #xgb.plot_tree(booster = classifier[ph], num_trees = 1)
        #plt.savefig("image_tree_" + str(ph) + "_" + str(1) + ".png")

        # f score
        #xgb.plot_importance(classifier[ph])
        #plt.savefig("image_fscore_" + str(ph) + ".png")

        print("phase %d accuracy = %f" % (ph, (np.sum(np.diag(cm)) / float(np.sum(cm)))))


