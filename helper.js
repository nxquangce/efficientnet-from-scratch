const fs = require('fs')

fs.readFile('npy_weights.txt', 'utf8', (err, data) => {
    if (err) {
        console.error(err)
        return
    }

    var sData = JSON.parse(data);
    var oData = [[[[]]]];
    var content = "";

    sData.forEach((num, idxNum) => {
        num.forEach((row, idxRow) => {
            row.forEach((col, idxCol) => {
                col.forEach((channel, idxChannel) => {
                    if (!oData[idxNum]) oData[idxNum] = [];
                    if (!oData[idxNum][idxChannel]) oData[idxNum][idxChannel] = [];
                    if (!oData[idxNum][idxChannel][idxRow]) oData[idxNum][idxChannel][idxRow] = [];
                    // if (!oData[idxNum][idxChannel][idxRow][idxCol]) oData[idxNum][idxChannel][idxRow] = [];
                    oData[idxNum][idxChannel][idxRow][idxCol] = channel;
                });
            });
        });
    });

    oData.forEach((num, idxNum) => {
        num.forEach((channel, idxChannel) => {
            channel.forEach((row, idxRow) => {
                row.forEach((col, idxCol) => {
                    content += col;
                    if (idxCol != (row.length - 1)) content += ' ';
                })
                content += '\n';
            })
        })
    })

    fs.writeFile('sweights.txt', content, err => {
        if (err) {
            console.error(err)
            return
        }
        //file written successfully
    })
})