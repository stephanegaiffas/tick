#!/usr/bin/env python3

from tick.simulation import SimuHawkesSumExpKernels, SimuHawkesMulti
from tick.optim.model import ModelHawkesFixedSumExpKernLeastSq

import argparse

parser = argparse.ArgumentParser()
parser.add_argument('-t', '--threads', help="Specify the number of threads", type=int)
parser.add_argument('-n', '--number',  help="Specify the N number", type=int)
args = parser.parse_args()

threads = 1
if args.threads:
    threads = args.threads
number  = 50000
if args.number:
    number = args.number

period_length = 300
baselines = [[0.3, 0.5, 0.6, 0.4, 0.2, 0],
             [0.8, 0.5, 0.2, 0.3, 0.3, 0.4]]
n_baselines = len(baselines[0])
decays = [.5, 2., 6.]
adjacency = [[[0, .1, .4], [.2, 0., .2]],
             [[0, 0, 0], [.6, .3, 0]]]

# simulation
hawkes = SimuHawkesSumExpKernels(baseline=baselines,
                                 period_length=period_length,
                                 decays=decays, adjacency=adjacency,
                                 seed=2093, verbose=False)
hawkes.end_time = number
hawkes.adjust_spectral_radius(0.5)

multi = SimuHawkesMulti(hawkes, n_simulations=4)
multi.simulate()

# estimation
model = ModelHawkesFixedSumExpKernLeastSq(
    decays, n_baselines=n_baselines, 
    period_length=period_length, n_threads=threads)

model.fit(multi.timestamps)
model._model.compute_weights()


