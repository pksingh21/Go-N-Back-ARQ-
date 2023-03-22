import pandas as pd
import re
from matplotlib import pyplot as plt
plt.rcParams["figure.figsize"] = [7.00, 3.50]
plt.rcParams["figure.autolayout"] = True
columns = ["Probability_percent", "Retransmission_percent"]
df = pd.read_csv("data.csv", usecols=columns)
li = df['Probability_percent'].tolist()
newLi = []
newLi2 = []
for i in range(len(li)):
    stringToConvert = li[i]
    pattern = r"[-+]?\d*\.\d+|\d+"
    numbers = re.findall(pattern, stringToConvert)
    newLi.append(float(numbers[0]))
    newLi2.append(float(numbers[1]))
print("newLi", newLi)
plt.xlabel("probability of packet loss")
plt.ylabel("Number of packets retransmitted")
plt.plot(newLi2, newLi)
plt.show()
