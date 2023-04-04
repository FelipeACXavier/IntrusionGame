import os
import re
import math
import argparse

import numpy as np
import matplotlib.pyplot as plt

from sys import exit
from pathlib import Path
from matplotlib.backends.backend_pdf import PdfPages


parser = argparse.ArgumentParser(
                    prog='IntrusionGame',
                    description="""Simulates a specified intrusion game based on the \"Physical Intrusion
                                   Games—Optimizing Surveillance by Simulation and Game Theory\" paper""",
                    epilog='For research purpose only, see: https://github.com/FelipeACXavier/IntrusionGame')

parser.add_argument('-l', dest="level", default=0, help="Specify the level", type=str)
parser.add_argument('-t', dest="title", default=None, help="Title, i.e. parameter used", type=str)
parser.add_argument('-d', dest="dir", default=None, help="Folder where data is located", type=str)

args = parser.parse_args()
if args.level not in ["11", "12", "2", "3"]:
    print("Invalid level selected")
    exit()

if not args.title:
  print("No title given")
  exit()

if not args.dir:
  print("No folder given")
  exit()

level = args.level
subtitle = args.title
data_folder = args.dir
name = "level_" + str(level)

def read_data(file):
  file_data = {"mean" : 0.0, "var": 0.0, "data": [], "name": 0}
  print("Reading file: " + file)

  # Get second result because first is the level
  base = Path(file).stem
  file_data["name"] = re.findall(r'\d+\.?\d*', base)[-1]

  with open(file, "r") as f:
    for i, line in enumerate(f.read().splitlines()):
      if i == 0:
        file_data["mean"] = float(line)
      elif i == 1:
        file_data["var"] = float(line)
      else:
        file_data["data"].append(float(line))

  return file_data

def get_histogram(hist_data, axis):
  # Plot histogram
  n, bins, patches = axis.hist(x=hist_data, bins=10, color='#0504aa', alpha=0.7, rwidth=0.85)
  maxfreq = n.max()
  axis.set_ylim(ymax=np.ceil(maxfreq / 10) * 10 if maxfreq % 10 else maxfreq + 10)

  m = sum(hist_data) / len(hist_data)
  v = sum([(x - m) ** 2 for x in hist_data]) / len(hist_data)
  d = math.sqrt(v)

  axis.grid(axis='y', alpha=0.75)
  axis.set_xlabel('Value')
  axis.set_ylabel('Frequency')
  axis.set_title('Histogram')

  axis.axvline(x=m, color='r', linestyle='-')
  axis.axvline(x=m + d, color='g', linestyle='-')
  axis.axvline(x=m - d, color='g', linestyle='-')

  axis.legend(["Mean: {:.2f}".format(m), "Deviation: {:.2f}".format(d)])

def get_plot(x, y, axis, x_label):
  axis.plot(x, y)

  axis.grid(alpha=0.5)
  axis.set_xlabel(x_label)
  axis.set_ylabel('Q value')
  axis.set_title('{} vs Q value'.format(x_label))

# =====================================================================
# Get data from all files
data = []

for file in os.listdir(data_folder):
  if file.endswith(".txt") and name in file:
    data.append(read_data(data_folder + file))

data = sorted(data, key=lambda d : d["name"])
# =====================================================================
# Generate graphs
x = []
y = []
full_data = []

mean = 0.0
variance = 0.0

pdf_name = data_folder + subtitle.replace(" ", "_") + str(level) + ".pdf"
pdf = PdfPages(pdf_name)

for i, d in enumerate(data):
  fig = plt.figure(i, figsize=(10, 10))
  axis = fig.add_subplot(111)

  plt.suptitle("Level {} - {}={}".format(level, subtitle, d["name"]), fontsize=24)
  get_histogram(d["data"], axis)

  pdf.savefig(figure=fig)

  x.append(float(d["name"]))
  y.append(d["mean"])
  full_data.extend(d["data"])

# This plots the complete histogram and change for each parameter
if len(data) > 1:
  fig = plt.figure(10, figsize=(20, 10))
  plt.suptitle("Level {} - {}".format(level, subtitle), fontsize=32)

  axis_hist = fig.add_subplot(121)
  axis_line = fig.add_subplot(122)

  # Plot histogram
  get_histogram(full_data, axis_hist)

  # Plot how the parameter affects the mean
  get_plot(x, y, axis_line, subtitle)

  pdf.savefig(figure=fig)

pdf.close()