import matplotlib . pyplot as plt

# Нарисовать простейший график:
plt.plot([1, 2, 3], [1, 2, 3], 'o', color='red')
plt.plot([2, 3, 4], [3, 2, 1], '--' ,color='green')
plt.plot([2, 3, 4], [1, 3, 2], '-.' ,color='blue')
plt.show()