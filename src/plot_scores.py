from math import *
import numpy as np
import matplotlib.pyplot as plt

step = 0.1
r = 14
bins = [[] for _ in range(ceil(r / step))]
for line in open('../data/entropy_scores.txt').readlines():
    x, y = map(float, line.split())
    bins[round(x / step)].append(y)
mask = np.array([len(arr) > 10 for arr in bins])
xs = np.arange(0, r, step)[mask]
bins = [bins[i] for i in range(len(mask)) if mask[i]]
means = np.array([np.mean(arr) for arr in bins])
approx = np.sqrt(1 + xs)

plt.xlabel('entropy')
plt.ylabel('average score')
plt.plot(xs, means)
plt.plot(xs, approx)
plt.show()
