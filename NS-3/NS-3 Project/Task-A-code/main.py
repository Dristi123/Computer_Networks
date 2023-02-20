# This is a sample Python script.

# Press Shift+F10 to execute it or replace it with your code.
# Press Double Shift to search everywhere for classes, files, tool windows, actions, and settings.


import matplotlib.pyplot as plt

input_file = "time.txt"
x = []
y = []
with open(input_file) as fp:
    for line in fp:
        arr = line.strip().split()
        x.append(arr[0])
        y.append(float(arr[1]))
# plt.set_xticklables(['0','0.00001','0.0001','0.001','0.01'])
plt.plot(x, y)
# plt.xticks(x)
#plt.plot(x1, y1, label="TcpWestwood")
plt.xlabel('Time(ms)')
plt.ylabel('Throughput(Mbps)')
plt.title('Real time throughput')
plt.show()
