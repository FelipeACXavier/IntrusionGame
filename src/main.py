import os
import math
import json
import pygame
import argparse
import settings

import numpy as np
import scipy.stats as st
import matplotlib.pyplot as plt

from sys import exit
from game import Game
from pathlib import Path
from game_statistics import GameStats

from matplotlib.backends.backend_pdf import PdfPages

parser = argparse.ArgumentParser(
                    prog='IntrusionGame',
                    description="""Simulates a specified intrusion game based on the \"Physical Intrusion
                                   Games—Optimizing Surveillance by Simulation and Game Theory\" paper""",
                    epilog='For research purpose only, see: https://github.com/FelipeACXavier/IntrusionGame')

parser.add_argument('-c', dest="config", help="Specify the simulation config", type=str)
parser.add_argument('-f', dest="data", help="Run only analysis from given file", type=str)
parser.add_argument('-d', dest="directory", help="Run all configs in directory", type=str)
parser.add_argument('-i', dest="iterations", help="Overwrite config iterations", type=int)
parser.add_argument('-r', dest="runs", default=1, help="How many runs to average over", type=int)
parser.add_argument('-e', dest="observed", help="Expected Z-test mean", type=float)
parser.add_argument('--confidence', dest="confidence", default=0.75, help="Expected Z-test mean", type=float)
parser.add_argument('--fps', dest="fps", help="Overwrite config fps", type=int)
parser.add_argument('--cycles', dest="cycles", help="Overwrite config cycles per frame", type=int)
parser.add_argument("--hidden", dest="hidden", action="store_true")

def run_simulation(config_file, runs=None, fps=None, cycles=None, hidden=False):
    print("Running with file: {}".format(config_file))

    test_type = str()
    iterations = int()
    with open(config_file, 'r') as config:
        data = json.load(config)
        test_type = data["test_type"]
        iterations = data["iterations"] if runs == None else runs

        settings.FPS = data["fps"] if fps == None else fps
        settings.DEBUG = data["log_debug"]
        settings.DAY_LENGTH = data["day_duration"] * 60
        settings.CYCLES_PER_FRAME = data["cycles_per_frame"] if cycles == None else cycles

    # Always init the library before the simulation starts
    pygame.init()

    stats = GameStats(test_type, iterations)
    game = Game(config_file, stats, hidden)
    game_stopped = False
    for i in range(iterations):
        game.run()

        stats.update_result(game.result(), i)

        if game.is_stopped():
            game_stopped = True
            break

        game.reset(i + 1, config_file, stats)

    stats.done()
    stats.save("results/{}".format(os.path.basename(config_file)))

    # Dont forget to deinitialise the library
    pygame.quit()

    mean = stats.p_value() if test_type == "p-test" else stats.q_value()
    samples = stats.p_samples if test_type == "p-test" else stats.q_samples
    return mean, samples, iterations, game_stopped

if __name__ == "__main__":
    args = parser.parse_args()

    var = 0
    mean = 0
    means = list()
    observed = float()

    path = str()
    runs = args.runs

    # =========================================================
    # Only perform statistics tests
    if args.data != None:
        if not args.observed:
            print("No observed value provided")
            exit()

        observed = args.observed
        path = Path(args.data).stem
        with open(args.data, "r") as f:
            for i, line in enumerate(f.read().splitlines()):
                if i == 0:
                    mean = float(line)
                elif i == 1:
                    var = float(line)
                else:
                    means.append(float(line))
    # =========================================================
    # Run single level
    elif args.config != None:
        with open(args.config, 'r') as config:
            data = json.load(config)
            observed = data["observed_mean"]

        for i in range(runs):
            mean_value, samples, iter, stopped = run_simulation(args.config, args.iterations, args.fps, args.cycles, args.hidden)
            if runs == 1:
                means.extend(samples)
            else:
                means.append(mean_value)

            mean = sum(means) / len(means)
            var = sum((j - mean) ** 2 for j in means) / len(means)

            print("Finished {} out of {} runs".format(i + 1, runs))
            print("Mean={:.6f} Variance={:.6f}".format(mean, var))
            print("=========================================")

            if stopped:
                break

        path = Path(args.config).stem + "_{}_{}".format(args.iterations if args.iterations else iter, args.runs)
        with open("results/" + path + ".txt", "w+") as f:
            f.write(str(mean) + "\n")
            f.write(str(var) + "\n")
            for v in means:
                f.write(str(v) + "\n")

    # =========================================================
    # Run all levels in directory
    elif args.directory != None:
        for filename in os.listdir(args.directory):
            config_file = os.path.join(args.directory, filename)
            # Checking if it is a file
            if not os.path.isfile(config_file):
                continue

            run_simulation(config_file, args.iterations, args.fps, args.cycles, args.hidden)
    else:
        print("No configuration file provided")

    if len(means) > 0:
        dev = math.sqrt(var)
        pdf = PdfPages('results/{}_graph.pdf'.format(path))

        fig_hist = plt.figure(1, figsize=(10,10))
        fig_means = plt.figure(2, figsize=(10,10))
        axis_hist = fig_hist.add_subplot(111)
        axis_means = fig_means.add_subplot(111)

        n, bins, patches = axis_hist.hist(x=means, bins=10, color='#0504aa', alpha=0.7, rwidth=0.85)

        maxfreq = n.max()
        axis_hist.set_ylim(ymax=np.ceil(maxfreq / 10) * 10 if maxfreq % 10 else maxfreq + 10)

        axis_hist.grid(axis='y', alpha=0.75)
        axis_hist.set_xlabel('Value')
        axis_hist.set_ylabel('Frequency')
        axis_hist.set_title('Histogram for Q test')

        axis_hist.axvline(x=mean, color='r', linestyle='-')
        axis_hist.axvline(x=mean + dev, color='g', linestyle='-')
        axis_hist.axvline(x=mean - dev, color='g', linestyle='-')

        # Z-test
        dev = math.sqrt(var)
        confidence = args.confidence

        prob = 1 - ((1 - confidence) / 2)
        p_value = st.norm.ppf(prob)
        den = dev / math.sqrt(len(means))

        Z = 1000 if den == 0 else abs( (observed - mean) / den )

        print("=========================================")
        print("Observed mean={:.4f}".format(observed))
        print("Expected mean={:.4f} variance={:.4f}".format(mean, var))
        print("Z={:.4f} P-value={:.4f} {}".format(Z, p_value, "Passed" if Z < p_value else "Failed"))
        print("=========================================")

        x_axis = np.linspace(0, len(means), len(means))
        axis_means.plot(x_axis, means)

        plt.show()

        pdf.savefig(figure=fig_hist)
        pdf.savefig(figure=fig_means)
        pdf.close()

    exit()