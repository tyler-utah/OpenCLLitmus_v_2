import subprocess
from scipy.stats import chisquare
import numpy as np
import pandas as pd
import time

iterations_per = raw_input("Enter number of iterations: ")
mp_param_string = raw_input("Enter MP param string output by opt_param.py: ")
sb_param_string = raw_input("Enter SB param string output by opt_param.py: ")
lb_param_string = raw_input("Enter LB param string output by opt_param.py: ")

batch_name = raw_input("Enter batch name: ")
             
mp_param_string = mp_param_string.replace("(","")
mp_param_string = mp_param_string.replace(")","")
mp_param_string = mp_param_string.replace(" ","")

mp_params_input = mp_param_string.split(",")

sb_param_string = sb_param_string.replace("(","")
sb_param_string = sb_param_string.replace(")","")
sb_param_string = sb_param_string.replace(" ","")

sb_params_input = sb_param_string.split(",")

lb_param_string = lb_param_string.replace("(","")
lb_param_string = lb_param_string.replace(")","")
lb_param_string = lb_param_string.replace(" ","")

lb_params_input = lb_param_string.split(",")


start_time = time.time()

cumulative = [0,0,0,0]

barrier_switch = [0, 1]
shuffle_switch = [0, 1]

stress_switch = [0,1]
# stress_iterations = range(512)
stress_iterations = 512
stress_pattern = [0, 1, 2, 3]

prestress_switch = [0,1]
# prestress_iterations = range(512)
prestress_iterations = 100 # change to 128, make sure is not fuzzing
prestress_pattern = [0, 1, 2, 3]

x_y_stride = [1 << i for i in range(9)]

patch_size = [1 << i for i in range(10)]

conc_stressed = range(17)[1:]

conc_split = [0, 1] # sequential, round robin

results_hist = []
barrier_hist = []
shuffle_hist = []
stress_hist = []
stress_pat_hist = []
stress_it_hist = []
prestress_hist = []
prestress_pat_hist = []
prestress_it_hist = []
stride_hist = []
batch_id_hist = []
sample_id_hist = []
iteration_id_hist = []
patch_size_hist = []
conc_stress_hist = []
conc_split_hist = []
lit_test_hist = []

cmd = './build/bin/litmus_exe -i ' + iterations_per
# results = [0, 0, 0, 0]

barrier_val = [mp_params_input[0], sb_params_input[0], lb_params_input[0]]
mp_barrier_tag = " -D BARRIER=" + str(barrier_val[0])
sb_barrier_tag = " -D BARRIER=" + str(barrier_val[1])
lb_barrier_tag = " -D BARRIER=" + str(barrier_val[2])


shuffle_val = [mp_params_input[3], sb_params_input[3], lb_params_input[3]]
mp_shuffle_tag = " -D ID_SHUFFLE=" + str(shuffle_val[0])
sb_shuffle_tag = " -D ID_SHUFFLE=" + str(shuffle_val[1])
lb_shuffle_tag = " -D ID_SHUFFLE=" + str(shuffle_val[2])



stress_val = [mp_params_input[4], sb_params_input[4], lb_params_input[4]]
mp_stress_tag  = " -D MEM_STRESS=" + str(stress_val[0])
sb_stress_tag  = " -D MEM_STRESS=" + str(stress_val[1])
lb_stress_tag  = " -D MEM_STRESS=" + str(stress_val[2])


    # stress_it_val = np.random.choice(stress_iterations)
stress_it_val = stress_iterations
stress_iterations_tag = " -D STRESS_ITERATIONS=" + str(stress_it_val)

stress_pat_val = [mp_params_input[9], sb_params_input[9], lb_params_input[9]]
mp_stress_pattern_tag = " -D STRESS_PATTERN=" + str(stress_pat_val[0])
sb_stress_pattern_tag = " -D STRESS_PATTERN=" + str(stress_pat_val[1])
lb_stress_pattern_tag = " -D STRESS_PATTERN=" + str(stress_pat_val[2])


prestress_val = [mp_params_input[6], sb_params_input[6], lb_params_input[6]]
mp_prestress_tag = " -D PRE_STRESS=" + str(prestress_val[0])
sb_prestress_tag = " -D PRE_STRESS=" + str(prestress_val[1])
lb_prestress_tag = " -D PRE_STRESS=" + str(prestress_val[2])

    # prestress_it_val = np.random.choice(prestress_iterations)
prestress_it_val = prestress_iterations
prestress_iterations_tag = " -D PRE_STRESS_ITERATIONS=" + str(prestress_it_val)

prestress_pat_val = [mp_params_input[8], sb_params_input[8], lb_params_input[8]]
mp_prestress_pattern_tag = " -D PRE_STRESS_PATTERN=" + str(prestress_pat_val[0])
sb_prestress_pattern_tag = " -D PRE_STRESS_PATTERN=" + str(prestress_pat_val[1])
lb_prestress_pattern_tag = " -D PRE_STRESS_PATTERN=" + str(prestress_pat_val[2])

x_y_stride_val = [mp_params_input[10], sb_params_input[10], lb_params_input[10]]
mp_x_y_stride_tag = " -D X_Y_STRIDE=" + str(x_y_stride_val[0])
sb_x_y_stride_tag = " -D X_Y_STRIDE=" + str(x_y_stride_val[1])
lb_x_y_stride_tag = " -D X_Y_STRIDE=" + str(x_y_stride_val[2])

patch_size_val = [mp_params_input[5], sb_params_input[5], lb_params_input[5]]
mp_patch_size_tag = " -D PATCH_SIZE=" + str(patch_size_val[0])
sb_patch_size_tag = " -D PATCH_SIZE=" + str(patch_size_val[1])
lb_patch_size_tag = " -D PATCH_SIZE=" + str(patch_size_val[2])

conc_stress_val = [mp_params_input[2], sb_params_input[2], lb_params_input[2]]
mp_conc_stress_tag = " -D CONC_STRESS=" + str(conc_stress_val[0])
sb_conc_stress_tag = " -D CONC_STRESS=" + str(conc_stress_val[1])
lb_conc_stress_tag = " -D CONC_STRESS=" + str(conc_stress_val[2])

conc_split_val = [mp_params_input[3], sb_params_input[3], lb_params_input[3]]
mp_conc_split_tag = " -D CONC_SPLIT=" + str(conc_split_val[0])
sb_conc_split_tag = " -D CONC_SPLIT=" + str(conc_split_val[1])
lb_conc_split_tag = " -D CONC_SPLIT=" + str(conc_split_val[2])

mp_flag = mp_barrier_tag + mp_shuffle_tag + mp_stress_tag + stress_iterations_tag + mp_stress_pattern_tag + mp_prestress_tag + prestress_iterations_tag + mp_prestress_pattern_tag + mp_x_y_stride_tag + mp_patch_size_tag + mp_conc_stress_tag + mp_conc_split_tag

sb_flag = sb_barrier_tag + sb_shuffle_tag + sb_stress_tag + stress_iterations_tag + sb_stress_pattern_tag + sb_prestress_tag + prestress_iterations_tag + sb_prestress_pattern_tag + sb_x_y_stride_tag + sb_patch_size_tag + sb_conc_stress_tag + sb_conc_split_tag

lb_flag = lb_barrier_tag + lb_shuffle_tag + lb_stress_tag + stress_iterations_tag + lb_stress_pattern_tag + lb_prestress_tag + prestress_iterations_tag + lb_prestress_pattern_tag + lb_x_y_stride_tag + lb_patch_size_tag + lb_conc_stress_tag + lb_conc_split_tag


    
        ##if sum(cumulative) != 0:
    ##    p_val = chisquare([x + 1 for x in results], [y + 1 for y in cumulative])[1]
    ##    old_cumulative = list(cumulative)
    ##    for j in range(len(cumulative)):
    ##        cumulative[j] += results[j]
    ##    if p_val > 0.5:
    ##        print("Likely match. P-value: " + str(p_val))
    ##        print(results)
    ##        print(old_cumulative)
    ##        break
    ##    if p_val < 0.05 or iterations >= 16000:
    ##        print("Not a match. P-value: " + str(p_val))
    ##        print(results)
    ##        print(old_cumulative)
    ##        break
    ##else:
    ##    for k in range(len(cumulative)):
    ##        cumulative[k] += results[k]
    # print(flag)
    # print(results)
   ## iterations = sum(cumulative)

for test in ['MP','SB','LB']:
    cmd = './build/bin/litmus_exe -i ' + iterations_per
    ind = 0
    if (test == 'MP'):
        cmd += mp_flag + ' ../OpenCL_tests/interwg_base/' + test
        ind = 0
        print(cmd)
    if (test == 'SB'):
        cmd += sb_flag + ' ../OpenCL_tests/interwg_base/' + test
        ind = 1
        print(cmd)
    if (test == 'LB'):
        cmd += lb_flag + ' ../OpenCL_tests/interwg_base/' + test            
        ind = 2
        print(cmd)
    output = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)

    i = 0
    for row in output.stdout:
        if '&&' in row and ':' in row:
            key, val = row.split(':')
            results_hist.append(int(val.strip()))
        else:
            continue
        barrier_hist.append(barrier_val[ind])
        shuffle_hist.append(shuffle_val[ind])
        stress_hist.append(stress_val[ind])
        stress_it_hist.append(stress_it_val)
        stress_pat_hist.append(stress_pat_val[ind])
        prestress_hist.append(prestress_val[ind])
        prestress_it_hist.append(prestress_it_val)
        prestress_pat_hist.append(prestress_pat_val[ind])
        stride_hist.append(x_y_stride_val[ind])
        batch_id_hist.append(batch_name)
        sample_id_hist.append(0)
        iteration_id_hist.append(i)
        patch_size_hist.append(patch_size_val[ind])
        conc_stress_hist.append(conc_stress_val[ind])
        conc_split_hist.append(conc_split_val[ind])
        lit_test_hist.append(test)
        i += 1

data = {'BARRIER' : barrier_hist, 'ID_SHUFFLE' : shuffle_hist, 'MEM_STRESS' : stress_hist, 'STRESS_ITERATIONS' : stress_it_hist, 'STRESS_PATTERN' : stress_pat_hist, 'PRE_STRESS' : prestress_hist, 'PRE_STRESS_ITERATIONS' : prestress_it_hist, 'PRE_STRESS_PATTERN' : prestress_pat_hist, 'X_Y_STRIDE' : stride_hist, 'BATCH_NAME' : batch_id_hist, 'SAMPLE_ID' : sample_id_hist, 'ITERATION_NUMBER' : iteration_id_hist, 'RESULT' : results_hist, 'PATCH_SIZE' : patch_size_hist, 'CONC_STRESS' : conc_stress_hist, 'LITMUS_TEST' : lit_test_hist, 'CONC_SPLIT' : conc_split_hist}

df = pd.DataFrame(data)
df.to_csv(batch_name + "_" + "HETEROSKED" + "_RESULTS.csv", index=False)

print("--- %s seconds ---" % (time.time() - start_time))
