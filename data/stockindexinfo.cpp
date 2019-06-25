#include "stockindexinfo.h"
#include <cmath>
#include <exception>
#include <boost/algorithm/string.hpp>

/**
 * 从新浪财经当中获取的数据解析为StockIndexBatchInfo对象（单条）
 */
void StockIndexBatchInfo::decodeFromStrForSina(QString &str) {
    // 解析逻辑
    SingleIndexInfo tempIndexInfo;
    QString realCode = str.mid(11, 8);
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

/**
 * 从新浪财经当中批量获取的数据解析为StockIndexBatchInfo对象
 */
void StockIndexBatchInfo::batchDecodeForSina(QString& input) {
    QStringList divRest = input.split(";");
    auto iteBegin = divRest.begin(), iteEnd = divRest.end();
    while(iteBegin != iteEnd) {
        decodeFromStrForSina(*iteBegin);
        ++iteBegin;
    }
}

/**
 * 将对象转换为字符串，便于存储在Redis当中
 * 格式如下：
 * %ts_code:trade_date,currPrice;trade_date,currPrice;
 */
std::string StockIndexBatchInfo::encodeToStr() noexcept {
    std::string rst("%");
    rst.append(this->ts_code.toStdString()).append(":");
    for(int i = 0;i < this->infoList.size();i++) {
        rst.append(this->infoList[i].time.toString().toStdString());
        rst.append(",");
        rst.append(std::to_string(this->infoList[i].price));
        rst.append(";");
    }

    if(this->infoList.size() == 0) {
        rst.append(";");
    }
    return rst;
}

/**
 * 将字符串转换为对象
 * 字符串格式如下：
 * %ts_code:trade_date,currPrice;trade_date,currPrice;
 */
bool StockIndexBatchInfo::decodeFromStr(std::string& input) noexcept {
    if(input.size() == 0) {
        return false;
    }

    size_t firstIndex, infoStartIndex;
    firstIndex = input.find_first_of('%');
    infoStartIndex = input.find_first_of(':');
    // 格式不符合规范
    if(firstIndex != 0 || infoStartIndex > input.size() - 1
            || infoStartIndex == std::string::npos) {
        return false;
    }

    std::string ts_code = input.substr(firstIndex + 1, infoStartIndex - firstIndex);
    this->ts_code = QString::fromStdString(ts_code);

    // 将字符串当中的%ts_code:去掉，只留主体部分
    input.erase(0, infoStartIndex - firstIndex + 1);

    // 分割子串，获取分项信息
    std::vector<std::string> v;
    boost::split(v, input, boost::is_any_of(";"), boost::token_compress_on);
    for(auto temp : v) {
        std::string timeStr = temp.substr(0, temp.find_first_of(',') + 1);
        QDateTime tempTime = QDateTime::fromString(QString::fromStdString(timeStr));

        std::string priceStr = temp.substr(temp.find_first_of(',') + 1);
        float price = QString::fromStdString(priceStr).toFloat();

        SingleIndexInfo info;
        info.time = tempTime;
        info.price = price;
        this->infoList.push_back(info);
    }
    return true;
}

/**
 * 将本对象编码为字符串并且追加到输入参数input当中
 * 格式如下；
 * %ts_code:trade_date,currPrice;trade_date,currPrice;
 */
std::string& StockIndexBatchInfo::appendEncodeToStr(std::string& origin) {
    if(origin.size() == 0) {
        origin.append("%").append(ts_code.toStdString()).append(":");
    }
    else {
        if(origin.at(0) != '%') {
            throw new std::exception("输入字符串格式不匹配！");
        }

        size_t mainStartIndex = origin.find_first_of(':');
        if(mainStartIndex == std::string::npos) {
            throw new std::exception("输入字符串格式不匹配！");
        }
        std::string ts_code = origin.substr(0, mainStartIndex + 1);
        if(this->ts_code.toStdString().compare(ts_code) != 0) {
            throw new std::exception("输入参数同本对象编码不一致！");
        }
    }

    for(SingleIndexInfo tempInfo : infoList) {
        origin.append(tempInfo.time.toString().toStdString()).append(",");
        origin.append(std::to_string(tempInfo.price));
        origin.append(";");
    }

    return origin;
}

/**
 * 类静态方法
 * 从新浪当中获取到的原始字符串信息，拼接到StockIndexBatchInfo的字符串表示当中
 * StockIndexBatchInfo字符串格式如下；
 * %ts_code:trade_date,currPrice;trade_date,currPrice;
 * ts_code格式类似于：000001.SZ
 */
std::string& StockIndexBatchInfo::appendEncodeUseSina(std::string& origin, std::string& sinaStr) {
    std::string realCode = sinaStr.substr(11, 8);

    // 将sina当中获取的格式转换为目标格式：000001.SZ
    if(realCode.find("sz") == std::string::npos) {
        realCode = std::string(realCode.substr(2, 6));
        realCode.append(".SZ");
    }
    else {
        realCode = std::string(realCode.substr(2, 6));
        realCode.append(".SH");
    }

    std::string ts_code;
    bool originHasInfo = false;
    // 事前校验
    if(origin.size() == 0) {
        origin.append("%").append(realCode).append(":");
    }
    else {
        if(origin.at(0) != '%') {
            throw new std::exception("输入字符串格式不匹配！");
        }

        size_t mainStartIndex = origin.find_first_of(':');
        if(mainStartIndex == std::string::npos) {
            throw new std::exception("输入字符串格式不匹配！");
        }
        ts_code = origin.substr(1, mainStartIndex - 1);
        if(realCode.compare(ts_code) != 0) {
            throw new std::exception("代转换字符串同目标字符串所代表的股票不同！");
        }
        originHasInfo = true;
    }

    if(!originHasInfo || realCode == ts_code) {   // 只有是同当前股票编码相同的才能够进入
        size_t mainStartIndex = sinaStr.find_first_of('\"');
        size_t mainEndIndex = sinaStr.find_last_of('\"');
        std::string mainPart = sinaStr.substr(mainStartIndex, mainEndIndex - mainStartIndex - 1);

        // 分割子串，获取分项信息
        std::vector<boost::iterator_range<std::string::iterator>> v;
        boost::split(v, mainPart, boost::is_any_of(","), boost::token_compress_on);
        // 提取当日价格(位于第三位上)
        if(v.size() > 4) {
            // 当前时间价格
            float currPrice = QString::fromStdString(std::string(v[3].begin(), v[3].end())).toFloat();
            std::string priceStr(v[3].begin(), v[3].end());

            // 上一个时间点价格
            if(origin.at(origin.size() - 1) == ';') {
                size_t lastPriStartIdx = origin.find_last_of(',');
                std::string lastPriceStr = origin.substr(lastPriStartIdx + 1,
                                                         origin.size() - lastPriStartIdx - 2);
                float prePrice = QString::fromStdString(lastPriceStr).toFloat();
                if(abs(currPrice - prePrice) > 0.01f) {
                    // 如果价格相同，就不把新价格追加到字符串当中了
                    return origin;
                }
            }

            // 当前时间
            std::string dateStr(v[30].begin(), v[30].end());
            std::string timeStr(v[31].begin(), v[31].end());
            origin.append(dateStr).append(" ").append(timeStr).append(",");
            origin.append(priceStr).append(";");
        }
    }

    return origin;
}

/**
 * 类静态方法
 * 将两个自定义的格式字符串合并为一个
 * 第一、二个参数为两个待合并的字符串，将第二个字符串的内容追加到第一个字符串当中
 * 需要注意的是：调用者必须保证第二个字符串的分时信息晚于第一个字符串的，该方法不校验该内容
 * StockIndexBatchInfo字符串格式如下；
 * %ts_code:trade_date,currPrice;trade_date,currPrice;
 * ts_code格式类似于：000001.SZ
 */
void StockIndexBatchInfo::mergeTwoEncodeStr(std::string& in1, std::string& in2) {
    if(in1.size() == 0 || in2.size() == 0) {
        return;
    }

    // TODO -- 校验，两个输入参数的合法性以及是否是同一支股票

    size_t mainStartIndex = in2.find_first_of(':');
    in1.append(in2.substr(mainStartIndex + 1));
}
