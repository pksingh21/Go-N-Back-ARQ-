import os
import subprocess
import matplotlib as mp
import matplotlib.pyplot as plt

probability_range = [0, 100]

probability_percent = []
retransmission_percent = []
data_file = open("data.csv", "w")
data_file.write("Probability_percent,Retransmission_percent")

for i in range(probability_range[0],probability_range[1]):
    deci = round(i/100,2)
    command = f" ./server 12345 500 {deci} "
    print(f"{command}")
    check = subprocess.run(command, shell=True)
data_file.close()
# os.remove("./temp_output.txt")

# fig, ax = plt.subplots(1, 1)
# ax.plot(
#     probability_percent,
#     retransmission_percent,
# )
# ax.set_title("Graph")
# ax.set_xlabel("Error Probability %")
# ax.set_ylabel("Percent of Packets retransmitted")
# # "Plot of probability of noise vs ratio of retransmissions/packets sent",
# plt.show()
