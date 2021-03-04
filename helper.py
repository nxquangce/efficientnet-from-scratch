import numpy as np

fname = input("Enter weight file name (*.npy): ")

sData = np.load(fname)
oData = []
content = ""

for idxNum in range(len(sData)):
    num = sData[idxNum]
    for idxRow in range(len(num)):
        row = num[idxRow]
        for idxCol in range(len(row)):
            col = row[idxCol]
            for idxChannel in range(len(col)):
                channel = col[idxChannel]
                if (idxNum >= len(oData)):
                    oData.append([])
                if (idxChannel >= len(oData[idxNum])):
                    oData[idxNum].append([])
                if (idxRow >= len(oData[idxNum][idxChannel])):
                    oData[idxNum][idxChannel].append([])
                # if (!oData[idxNum][idxChannel][idxRow][idxCol]) oData[idxNum][idxChannel][idxRow] = []
                oData[idxNum][idxChannel][idxRow].append(channel)

for num in oData:
    for channel in num:
        for row in channel:
            for idxCol in range(len(row)):
                col = row[idxCol]
                content += str(col)
                if (idxCol != (len(row) - 1)):
                    content += " "
            content += "\n"

f = open("sweights.txt", "w+")
f.write(content)
f.close()