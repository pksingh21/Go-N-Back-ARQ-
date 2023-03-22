import os
import re
import subprocess
import matplotlib as mp
import matplotlib.pyplot as plt

probability_range = [0, 100]

probability_percent = []
retransmission_percent = []
data_file = open("data.csv", "w")
data_file.write("Probability_percent,Retransmission_percent")
data = []

for i in range(probability_range[0], probability_range[1]):
    commandForClient = f" ./client 10.10.75.15 12345 500 3 > output.txt"
    print(f"{commandForClient}")
    check2 = subprocess.run(commandForClient, shell=True)
    with open('output.txt', 'r') as f:
        # seek to the end of the file
        f.seek(0, 2)

    # find the start of the last line
        pos = f.tell()
        while pos > 0:
            pos -= 1
            f.seek(pos, 0)
            if f.read(1) == '\n':
                break

    # read the last line
        last_line = f.readline().strip()
        match = re.search(r'\d+(?:,\d+)*(?:\.\d+)?', last_line)
        if match:
            number = float(match.group().replace(',', ''))
            print(number, "number of retransmissions")
            data.append(number)
        else:
            print("No number found")
    data_file.write(f"\n {number}  \t {round(i/100,2)}")
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
