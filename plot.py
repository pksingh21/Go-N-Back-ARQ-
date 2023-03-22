import pandas as pd
import re
from matplotlib import pyplot as plt
# plt.rcParams["figure.figsize"] = [7.00, 3.50]
# plt.rcParams["figure.autolayout"] = True
# columns = ["Probability_percent", "Retransmission_percent"]
# df = pd.read_csv("data.csv", usecols=columns)
# li = df['Probability_percent'].tolist()
newLi = [101, 201, 301, 401,501, 601, 701, 801, 901]
newLi2 = [72, 140, 182, 248, 304, 360, 416, 472, 528]
# newLi = [10,20,30,40,50,60]
# newLi2 = [182,110,72,46,42,32]
# for i in range(len(li)):
#     stringToConvert = li[i]
#     pattern = r"[-+]?\d*\.\d+|\d+"
#     numbers = re.findall(pattern, stringToConvert)
#     newLi.append(float(numbers[0]))
#     newLi2.append(float(numbers[1]))
print("newLi", newLi)
plt.xlabel("number of transmitted packets")
plt.ylabel("size of data file in bytes")
# plt.xlabel("number of transmitted packets")
# plt.ylabel("packet size")
plt.plot(newLi2, newLi)
plt.show()
