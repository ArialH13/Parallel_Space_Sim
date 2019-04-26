'''
# This import registers the 3D projection, but is otherwise unused.
from mpl_toolkits.mplot3d import Axes3D  # noqa: F401 unused import

import matplotlib.pyplot as plt
import numpy as np

# Fixing random state for reproducibility
np.random.seed(19680801)


def randrange(n, vmin, vmax):

    #Helper function to make an array of random numbers having shape (n, )
    #with each number distributed Uniform(vmin, vmax).

    return (vmax - vmin)*np.random.rand(n) + vmin

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

n = 100

# For each set of style and range settings, plot n random points in the box
# defined by x in [23, 32], y in [0, 100], z in [zlow, zhigh].
for c, m, zlow, zhigh in [('r', 'o', -50, -25), ('b', '^', -30, -5)]:
    xs = randrange(n, 23, 32)
    ys = randrange(n, 0, 100)
    zs = randrange(n, zlow, zhigh)
    ax.scatter(xs, ys, zs, c=c, marker=m)

ax.set_xlabel('X')
ax.set_ylabel('Y')
ax.set_zlabel('Z')

plt.show()
'''
# This import registers the 3D projection, but is otherwise unused.
from mpl_toolkits.mplot3d import Axes3D  # noqa: F401 unused import

import matplotlib.pyplot as plt
import numpy as np

import csv
import sys
from pylab import *

# Plots a 3d scatterplot
# Run with python 3dgraph.py <csv file>

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

# csv format: mass, x, y, z
# colors/markers: 
# asteroid: black 'k' & point "."
# planet: blue 'b' & circle "o"
# star: red 'r' & star "*"
with open(sys.argv[1]) as csv_file:
	csv_reader = csv.reader(csv_file, delimiter=',');
	line_count = 0;
	for row in csv_reader:
		name = row[0]
		mass = row[1]
		if name == "Asteroid":
			col = 'k'
			mark = '.'
		elif name == "Planet":
			col = 'b'
			mark = 'o'
		elif name == "Star":
			col = 'r'
			mark = '*'

		x, y, z = int(row[2]), int(row[3]), int(row[4])
		ax.scatter(x, y, z, c=col, marker=mark, label=name)

#code to make sure legend doesn't have repeating labels
handles,labels=ax.get_legend_handles_labels() #get existing legend item handles and labels
i=arange(len(labels)) #make an index for later
filter=array([]) #set up a filter (empty for now)
unique_labels=list(set(labels)) #find unique labels
for ul in unique_labels: #loop through unique labels
    filter=np.append(filter,[i[array(labels)==ul][0]]) #find the first instance of this label and add its index to the filter
handles=[handles[int(f)] for f in filter] #filter out legend items to keep only the first instance of each repeated label
labels=[labels[int(f)] for f in filter]
ax.legend(handles,labels,loc='upper left') #draw the legend with the filtered handles and labels lists

ax.set_xlabel('X')
ax.set_ylabel('Y')
ax.set_zlabel('Z')

plt.show()