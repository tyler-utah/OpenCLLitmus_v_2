import subprocess
from scipy.stats import chisquare
import numpy as np
import pandas as pd
import time 
iterations_per = raw_input("Enter number of iterations per sample: ")
test = raw_input("Enter litmus test: ")
iterations = raw_input("Number of samples: ")
batch_name = raw_input("Batch name: ")

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

for j in range(int(iterations)):
    cmd = './build/bin/litmus_exe -i ' + iterations_per
    # results = [0, 0, 0, 0]

    barrier_val = np.random.choice(barrier_switch, p=[0.1, 0.9])
    barrier_tag = " -D BARRIER=" + str(barrier_val)

    shuffle_val = np.random.choice(shuffle_switch, p=[0.1, 0.9])
    shuffle_tag = " -D ID_SHUFFLE=" + str(shuffle_val)

    stress_val = np.random.choice(stress_switch, p=[0.1, 0.9])
    stress_tag  = " -D MEM_STRESS=" + str(stress_val)

    # stress_it_val = np.random.choice(stress_iterations)
    stress_it_val = stress_iterations
    stress_iterations_tag = " -D STRESS_ITERATIONS=" + str(stress_it_val)

    stress_pat_val = np.random.choice(stress_pattern)
    stress_pattern_tag = " -D STRESS_PATTERN=" + str(stress_pat_val)

    prestress_val = np.random.choice(prestress_switch)
    prestress_tag = " -D PRE_STRESS=" + str(prestress_val)

    # prestress_it_val = np.random.choice(prestress_iterations)
    prestress_it_val = prestress_iterations
    prestress_iterations_tag = " -D PRE_STRESS_ITERATIONS=" + str(prestress_it_val)

    prestress_pat_val = np.random.choice(prestress_pattern)
    prestress_pattern_tag = " -D PRE_STRESS_PATTERN=" + str(prestress_pat_val)

    x_y_stride_val = np.random.choice(x_y_stride)
    x_y_stride_tag = " -D X_Y_STRIDE=" + str(x_y_stride_val)

    patch_size_val = np.random.choice(patch_size)
    patch_size_tag = " -D PATCH_SIZE=" + str(patch_size_val)

    conc_stress_val = np.random.choice(conc_stressed)
    conc_stress_tag = " -D CONC_STRESS=" + str(conc_stress_val)

    conc_split_val = np.random.choice(conc_split)
    conc_split_tag = " -D CONC_SPLIT=" + str(conc_split_val)
    
    flag = barrier_tag + shuffle_tag + stress_tag + stress_iterations_tag + stress_pattern_tag + prestress_tag + prestress_iterations_tag + prestress_pattern_tag + x_y_stride_tag + patch_size_tag + conc_stress_tag + conc_split_tag
    
    cmd += flag + ' ../OpenCL_tests/interwg_base/' + test
    output = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True) 
    
    i = 0
    for row in output.stdout:
        if '&&' in row and ':' in row:
            key, val = row.split(':')
            results_hist.append(int(val.strip()))
        else:
            continue
        barrier_hist.append(barrier_val)
        shuffle_hist.append(shuffle_val)
        stress_hist.append(stress_val)
        stress_it_hist.append(stress_it_val)
        stress_pat_hist.append(stress_pat_val)
        prestress_hist.append(prestress_val)
        prestress_it_hist.append(prestress_it_val)
        prestress_pat_hist.append(prestress_pat_val)
        stride_hist.append(x_y_stride_val)
        batch_id_hist.append(batch_name)
        sample_id_hist.append(j)
        iteration_id_hist.append(i)
        patch_size_hist.append(patch_size_val)
        conc_stress_hist.append(conc_stress_val)
        conc_split_hist.append(conc_split_val)
        i += 1
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

data = {'BARRIER' : barrier_hist, 'ID_SHUFFLE' : shuffle_hist, 'MEM_STRESS' : stress_hist, 'STRESS_ITERATIONS' : stress_it_hist, 'STRESS_PATTERN' : stress_pat_hist, 'PRE_STRESS' : prestress_hist, 'PRE_STRESS_ITERATIONS' : prestress_it_hist, 'PRE_STRESS_PATTERN' : prestress_pat_hist, 'X_Y_STRIDE' : stride_hist, 'BATCH_NAME' : batch_id_hist, 'SAMPLE_ID' : sample_id_hist, 'ITERATION_NUMBER' : iteration_id_hist, 'RESULT' : results_hist, 'PATCH_SIZE' : patch_size_hist, 'CONC_STRESS' : conc_stress_hist, 'CONC_SPLIT' : conc_split_hist}

df = pd.DataFrame(data)
df.to_csv(batch_name + "_" + test + "_RESULTS.csv", index=False)

print("--- %s seconds ---" % (time.time() - start_time))
