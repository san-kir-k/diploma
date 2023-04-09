import matplotlib.pyplot as plt
from matplotlib.ticker import FixedLocator
import numpy as np

def forward(x):
    return x**(1/3)

def inverse(x):
    return x**3

fig, ax = plt.subplots()
order_naive = [16, 20, 24, 28]
time_stamp_naive = [107, 531, 1622, 30000]

order_ctx = [16, 20, 24, 28]
time_stamp_ctx = [4, 2, 933, 15750]

order_mt = [16, 20, 24, 28]
time_stamp_mt = [2, 1, 56, 3200]

line_naive, = ax.plot(order_naive, time_stamp_naive, linewidth=2.0, color='r')
line_ctx, = ax.plot(order_ctx, time_stamp_ctx, linewidth=2.0, color='b')
line_mt, = ax.plot(order_mt, time_stamp_mt, linewidth=2.0, color='g')

ax.legend([line_naive, line_ctx, line_mt], ["наивный", "однопоточный с контекстом", "многопоточный с контекстом"])
ax.set_yscale('function', functions=(forward, inverse))
ax.grid(True)
ax.yaxis.set_major_locator(FixedLocator(np.arange(0, 20_000, 1_00)**2))
ax.yaxis.set_major_locator(FixedLocator(np.arange(0, 20_000, 1_00)))
plt.yticks([5, 50, 100, 500, 1_000, 1_500, 3_000, 10_000, 15_000, 20_000])

ax.set(xlim=(12, 32), ylim=(0, 20_000))

plt.ylabel("Время, [мин]")
plt.xlabel("Порядок матрицы")

plt.xticks(order_mt)
plt.show()