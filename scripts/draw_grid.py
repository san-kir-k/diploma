import matplotlib as mlib
import matplotlib.pyplot as plt
import numpy as np

def plot_colored_grid(data, colors=['black', 'white'], bounds=[0, 0.5, 1], grid=True, labels=False, frame=True):
    # create discrete colormap
    cmap = mlib.colors.ListedColormap(colors)
    norm = mlib.colors.BoundaryNorm(bounds, cmap.N)

    # enable or disable frame
    plt.figure(frameon=frame)

    # show grid
    if grid:
        plt.grid(axis='both', color='k', linewidth=2)
        plt.xticks(np.arange(0.5, data.shape[1], 1))  # correct grid sizes
        plt.yticks(np.arange(0.5, data.shape[0], 1))

    # disable labels
    if not labels:
        plt.tick_params(bottom=False, top=False, left=False, right=False, labelbottom=False, labelleft=False)
    # plot data matrix
    plt.imshow(data, cmap=cmap, norm=norm)

    # display main axis
    plt.show()

data = np.array([
    [0, 0, 0, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 1, 1, 1, 1],
    [0, 0, 1, 1, 0, 0, 1, 1],
    [0, 0, 1, 1, 1, 1, 0, 0],
    [0, 1, 0, 1, 0, 1, 0, 1],
    [0, 1, 0, 1, 1, 0, 1, 0],
    [0, 1, 1, 0, 0, 1, 1, 0],
    [0, 1, 1, 0, 1, 0, 0, 1]
])

plot_colored_grid(data)