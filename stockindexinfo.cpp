#include "stockindexinfo.h"
#include <cmath>

void StockIndexBatchInfo::decodeFromStr(QString &str) {
    // 解析逻辑
    SingleIndexInfo tempIndexInfo;
    QString ts_code = str.mid(11, 8);
    QString realCode = str.mid(3, 6);
    if(realCode == ts_code) {   // 只有是同当前股票编码相同的才能够进入
        int mainStartIndex = str.indexOf("\"");
        int mainEndIndex = str.lastIndexOf("\"");
        QStringRef mainPart = str.midRef(mainStartIndex, mainEndIndex - mainStartIndex - 1);
        QVector<QStringRef> divRest = mainPart.split(",");
        // 提取当日价格(位于第三位上)
        if(divRest.size() > 4) {
            QStringRef currPrice = divRest[3];
            tempIndexInfo.price = currPrice.toFloat();

            // 当前时间
            QString dateTimeStr = divRest[30].toString();
            dateTimeStr.append(" ").append(divRest[31]);
            tempIndexInfo.time = QDateTime::fromString(dateTimeStr);
            // 查看下是否同最后获取到的价格相同，如果相同的话，就不存了（存了也没用）
            if(infoList.size() > 0 &&
                    abs((infoList[infoList.size() - 1].price - tempIndexInfo.price)) > 0.01f) {
                infoList.push_back(tempIndexInfo);
            }
        }
    }
}
