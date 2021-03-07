import numpy as np

fname = input("Enter weight file name (*.npy): ")
numDimension = int(input("Enter number of dimension (default: 4): "))
isTransform = input(
    "  If dimension = 2, do you want transform it? (yes/no) (default: yes): ")

sData = np.load(fname)
oData = []
content = ""

if (numDimension == 2):
    for idxRow in range(len(sData)):
        row = sData[idxRow]
        for idxCol in range(len(row)):
            col = row[idxCol]
            if (idxRow >= len(oData)):
                oData.append([])
            oData[idxRow].append(col)
    if (isTransform == "no"):
        for row in oData:
            for idxCol in range(len(row)):
                col = row[idxCol]
                content += str(col)
                if (idxCol != (len(row) - 1)):
                    content += " "
            content += "\n"
    else:
        for idxCol in range(len(oData[0])):
            for idxRow in range(len(oData)):
                row = oData[idxRow]
                col = row[idxCol]
                content += str(col)
                if (idxRow != (len(oData) - 1)):
                    content += " "
            content += "\n"
else:
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
