import numpy as np
import matplotlib.pyplot as plt
import matplotlib
import csv

# labels = ["Summation", "Dot"+'\n'+"Product", "Encrypt", "Integrity Key"+'\n'+"Generation", "Tag Gen", "Verification", "Refresh"+'\n'+"Codewords",
#           "Rotate"+'\n'+"Columns", "One Column"+'\n'+"Recovery", "Two Column"+'\n'+"Recovery"]

# means = [[1465.866667,7668.066667,831.56,75.88,2,285.76,6024.36,1092.5,1877.225,5151.625],
#         [1461.266667,11158.46667,1157.6,79.24,4.08,279.08,9489.84,1384.904762,2419.357143,8372.52381],
#         [1260.4,18492,1825,79.56,6.28,283.32,16826.12,1796.747368,3179.284211,16211.2],
#         [1085.666667,22441.73333,2097.56,74.68,6.16,284.84,20778.52,1840.628205,3371.846154,20542.24359]]

# std = [[43.88925457,73.84004592,41.95342656,6.359507318,0,19.91080109,84.88368512,36.74862635,173.8021993,344.0794994],
#       [29.72605078,162.9644425,42.38513891,11.22229923,1.998332638,3.882868356,90.80275326,28.73167055,112.5227305,659.0476604],
#       [32.19316698,146.486957,72.37287706,9.517877915,2.746512941,4.819405219,125.7124099,57.14408225,210.5592686,1182.107635],
#       [70.29089218,231.5526681,66.41352774,7.063521313,1.818424226,16.50929031,152.5702352,53.27095868,149.3382874,1458.21013]]

# old
# setup_labels = ["Integrity"+"\n"+"Key Gen.", "Tag Gen.", "Verification"]
# setup_means = [[75.88,2,285.76],
#                 [79.24,4.08,279.08],
#                 [79.56,6.28,283.32],
#                 [74.68,6.16,284.84]]
# setup_std = [[6.359507318,0,19.91080109],
#                 [11.22229923,1.998332638,3.882868356],
#                 [9.517877915,2.746512941,4.819405219],
#                 [7.063521313,1.818424226,16.50929031]]

rec_ver_labels = ["Rotate Columns", "One Col. Recovery", "Two Col. Recovery"]
rec_ver_means = [[1092.5,1877.225,5151.625],#,285.76],
                [1384.904762,2419.357143,8372.52381],#,279.08],
                [1796.747368,3179.284211,16211.2],#283.32],
                [1840.628205,3371.846154,20542.24359]]#,284.84]]
rec_ver_std = [[36.74862635,173.8021993,344.0794994],#,19.91080109],
                [28.73167055,112.5227305,659.0476604],#,3.882868356],
                [57.14408225,210.5592686,1182.107635],#,4.819405219],
                [53.27095868,149.3382874,1458.21013]]#,16.50929031]]

comp_labels = ["Refresh Codewords", "Summation", "Dot Product"]
comp_means = [[6024.36,1465.866667,7668.066667],
                [9489.84,1461.266667,11158.46667],
                [16826.12,1260.4,18492],
                [20778.52,1085.666667,22441.73333]]
comp_std = [[84.88368512,43.88925457,73.84004592],
            [90.80275326,29.72605078,162.9644425],
            [125.7124099,32.19316698,146.486957],
            [152.5702352,70.29089218,231.5526681]]

large_labels = ["Refresh"+"\n"+"Codewords", "Dot Product", "Two Column"+"\n"+"Recovery"]
large_means = [[6024.36,7668.066667,5151.625],
               [9489.84,11158.46667,8372.52381],
               [16826.12,18492,16211.2],
               [20778.52,22441.73333,20542.24359]]
large_std = [[84.88368512,73.84004592,344.0794994],
             [90.80275326,162.9644425,659.0476604],
             [125.7124099,146.486957,1182.107635],
             [152.5702352,231.5526681,1458.21013]]

# small_labels = ["Integrity"+"\n"+"Key Gen.", "Tag Gen.", "Verify","Rotate"+"\n"+"Columns", "One Col."+"\n"+"Recovery","Summation"]

# small_means = [[75.88,2,285.76,1092.5,1877.225,1465.866667],
#                [79.24,4.08,279.08,1384.904762,2419.357143,1461.266667],
#                [79.56,6.28,283.32,1796.747368,3179.284211,1260.4],
#                [74.68,6.16,284.84,1840.628205,3371.846154,1085.666667]]

# small_std = [[6.359507318,0,19.91080109,36.74862635,173.8021993,43.88925457],
#              [11.22229923,1.998332638,3.882868356,28.73167055,112.5227305,29.72605078],
#              [9.517877915,2.746512941,4.819405219,57.14408225,210.5592686,32.19316698],
#              [7.063521313,1.818424226,16.50929031,53.27095868,149.3382874,70.29089218]]

small_labels = ["Verify","Rotate"+"\n"+"Columns", "One Col."+"\n"+"Recovery","Summation"]

# ## simple verify
# setup_labels = ["Verification"]
# setup_means = [[710.8],[933.95],[1505],[1718]]
# setup_std = [[87.13],[8.25],[53],[84.11]]

small_means = [[710.8,1092.5,1877.225,1465.866667],
               [933.95,1384.904762,2419.357143,1461.266667],
               [1505,1796.747368,3179.284211,1260.4],
               [1718,1840.628205,3371.846154,1085.666667]]

small_std = [[87.13,36.74862635,173.8021993,43.88925457],
             [8.25,28.73167055,112.5227305,29.72605078],
             [53,57.14408225,210.5592686,32.19316698],
             [84.11,53.27095868,149.3382874,70.29089218]]

for i in range(0, len(rec_ver_std)):
    for j in range(0, len(rec_ver_means[0])):
        rec_ver_means[i][j] = rec_ver_means[i][j]/1000
        rec_ver_std[i][j] = rec_ver_std[i][j]/1000
       
# for i in range(0, len(setup_means)):
#     for j in range(0, len(setup_means[0])):
#         setup_means[i][j] = setup_means[i][j]/1000
#         setup_std[i][j] = setup_std[i][j]/1000
#         print(setup_means[i][j])
       
for i in range(0, len(comp_means)):
    for j in range(0, len(comp_means[0])):
        comp_means[i][j] = comp_means[i][j]/1000
        comp_std[i][j] = comp_std[i][j]/1000
       
for i in range(0, len(small_means)):
    for j in range(0, len(small_means[0])):
        small_means[i][j] = small_means[i][j]/1000
        small_std[i][j] = small_std[i][j]/1000
       
for i in range(0, len(large_means)):
    for j in range(0, len(large_means[0])):
        large_means[i][j] = large_means[i][j]/1000
        large_std[i][j] = large_std[i][j]/1000

matplotlib.rcParams.update({'font.size':22})

scale = 10

# fig, (ax, ax2, ax1) = plt.subplots(3,1,figsize=(scale,.66*scale))
fig, (ax, ax1) = plt.subplots(2,1,figsize=(1.5*scale,scale))

fig.tight_layout()

# ax = fig.add_axes([0,0,2,1])
w = 0.02
spread = 0.1
start = 1
ind_5 = start-1.5*w
ind_7 = start-.5*w
ind_11 = start+.5*w
ind_13 = start+1.5*w


ind = [[ind_5,spread+ind_5,2*spread+ind_5,3*spread+ind_5],#, 4*spread+ind_5, 5*spread+ind_5],
       [ind_7,spread+ind_7,2*spread+ind_7,3*spread+ind_7],#, 4*spread+ind_7, 5*spread+ind_7],
       [ind_11,spread+ind_11,2*spread+ind_11,3*spread+ind_11],#, 4*spread+ind_11, 5*spread+ind_11],
       [ind_13,spread+ind_13,2*spread+ind_13,3*spread+ind_13]]#, 4*spread+ind_13, 5*spread+ind_13]]

setup_labels = small_labels
setup_means = small_means
setup_std = small_std

comp_labels = large_labels
comp_means = large_means
comp_std = large_std

ax.bar(ind[0],setup_means[0],width=w,yerr=setup_std[0],color='r',label='n=5, m=12')
ax.bar(ind[1],setup_means[1],width=w,yerr=setup_std[1],color='b',label='n=7, m=9')
ax.bar(ind[2],setup_means[2],width=w,yerr=setup_std[2],color='g',label='n=11, m=5')
ax.bar(ind[3],setup_means[3],width=w,yerr=setup_std[3],color='y',label='n=13, m=4')

# print(len([start, start+spread, start+spread*2, start+spread*3, start+spread*4, start+spread*5, start+spread*6]))
# print(len(setup_labels))
ax.set_xticks([start, start+spread, start+spread*2, start+spread*3])#, start+spread*4, start+spread*5])#, start+spread*6])
ax.set_xticklabels(setup_labels)

w = 0.015
spread = 0.1
start = 1
ind_5 = start-1.5*w
ind_7 = start-.5*w
ind_11 = start+.5*w
ind_13 = start+1.5*w


ind = [[ind_5,spread+ind_5,2*spread+ind_5,3*spread+ind_5, 4*spread+ind_5, 5*spread+ind_5],
       [ind_7,spread+ind_7,2*spread+ind_7,3*spread+ind_7, 4*spread+ind_7, 5*spread+ind_7],
       [ind_11,spread+ind_11,2*spread+ind_11,3*spread+ind_11, 4*spread+ind_11, 5*spread+ind_11],
       [ind_13,spread+ind_13,2*spread+ind_13,3*spread+ind_13, 4*spread+ind_13, 5*spread+ind_13]]

ax1.bar(ind[0][0:3],comp_means[0],width=w,yerr=comp_std[0],color='r',label='n=5, m=12')
ax1.bar(ind[1][0:3],comp_means[1],width=w,yerr=comp_std[1],color='b',label='n=7, m=9')
ax1.bar(ind[2][0:3],comp_means[2],width=w,yerr=comp_std[2],color='g',label='n=11, m=5')
ax1.bar(ind[3][0:3],comp_means[3],width=w,yerr=comp_std[3],color='y',label='n=13, m=4')

ax1.set_xticks([start, start+spread, start+spread*2])
ax1.set_xticklabels(comp_labels)

# ax2.bar(ind[0][0:3],rec_ver_means[0],width=w,yerr=rec_ver_std[0],color='r',label='n=5, m=12')
# ax2.bar(ind[1][0:3],rec_ver_means[1],width=w,yerr=rec_ver_std[1],color='b',label='n=7, m=9')
# ax2.bar(ind[2][0:3],rec_ver_means[2],width=w,yerr=rec_ver_std[2],color='g',label='n=11, m=5')
# ax2.bar(ind[3][0:3],rec_ver_means[3],width=w,yerr=rec_ver_std[3],color='y',label='n=13, m=4')

# ax2.set_xticks([start, start+spread, start+spread*2])
# ax2.set_xticklabels(rec_ver_labels)

ax.legend(loc="upper center", ncol=4, bbox_to_anchor=(.5, 1.15), shadow=True)

ax.set_ylabel('Seconds')
ax1.set_ylabel('Seconds')
# ax2.set_ylabel('Seconds')

# plt.show()
plt.savefig("plots-newverif.pdf")