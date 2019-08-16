#include "stockindexinfo.h"
#include <cmath>
#include <exception>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <boost/utility/string_ref.hpp>
#include <iostream>

/**
 * 从新浪财经当中获取的数据解析为StockIndexBatchInfo对象（单条）
 */
void StockIndexBatchInfo::decodeFromStrForSina(QString &str) {
    // 解析逻辑
    QString realCode = str.mid(11, 8);
    // 将sina当中获取的格式转换为目标格式：000001.SZ
    if(realCode.contains("sz")) {
        realCode = realCode.mid(2, 6);
        realCode.append(".SZ");
    }
    else {
        realCode = realCode.mid(2, 6);
        realCode.append(".SH");
    }
    if(realCode == ts_code || ts_code.size() == 0) {   // 只有是同当前股票编码相同的才能够进入
        int mainStartIndex = str.indexOf("\"");
        int mainEndIndex = str.lastIndexOf("\"");
        QStringRef mainPart = str.midRef(mainStartIndex, mainEndIndex - mainStartIndex - 1);
        QVector<QStringRef> divRest = mainPart.split(",");
        if(divRest.size() > 31) {
            SingleIndexInfo tempInfo;
            QStringRef temp = divRest[0]; // 当前股票名称
            ts_name = temp.toString();

            today_open = divRest[1].toFloat(); //　今日开盘价
            pre_close = divRest[2].toFloat(); //　昨日收盘价
            for(int i = 3;i < divRest.size() && i < 30;i++) {
                tempInfo.mainContent[i - 3] = divRest[i].toDouble();
            }
            // 当前时间
            QString dateTimeStr = divRest[30].toString();
            dateTimeStr.append(" ").append(divRest[31]);
            tempInfo.time = QDateTime::fromString(dateTimeStr, "yyyy-MM-dd HH:mm:ss");
            info_list.push_back(tempInfo);
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
 * %ts_code,ts_name,today_open,pre_close:mainContent[0~26],time; \
 * mainContent[0~26],time;
 */
std::string StockIndexBatchInfo::encodeToStr() noexcept {
    std::string rst("%");
    rst.append(this->ts_code.toStdString()).append(ts_name.toStdString());
    rst.append(std::to_string(pre_close)).append(",");
    rst.append(std::to_string(today_open)).append(":");
    for(size_t i = 0;i < this->info_list.size();i++) {
        for(int j = 0;j < 27;j++) {
            rst.append(std::to_string(info_list[i].mainContent[j]));
            rst.append(",");
        }
        rst.append(info_list[i].time.toString("yyyy-MM-dd HH:mm:ss").toStdString());
        rst.append(";");
    }

    if(this->info_list.size() == 0) {
        rst.append(";");
    }
    return rst;
}

/**
 * 将字符串转换为对象
 * 字符串格式如下：
 * %ts_code,ts_name,today_open,pre_close:mainContent[0~26],time; \
 * mainContent[0~26],time;
 */
bool StockIndexBatchInfo::decodeFromStr(const std::string& input) noexcept {
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

    std::string headPart = input.substr(firstIndex + 1, infoStartIndex - firstIndex - 1);
    std::vector<std::string> headSplitInfo;
    boost::split(headSplitInfo, headPart, boost::is_any_of(","), boost::token_compress_on);
    ts_code = QString::fromStdString(headSplitInfo[0]);
    ts_name = QString::fromStdString(headSplitInfo[1]);
    today_open = QString::fromStdString(headSplitInfo[2]).toFloat();
    pre_close = QString::fromStdString(headSplitInfo[3]).toFloat();

    //std::cout << ts_code.toStdString() << std::endl;

    // 将字符串当中的头部去掉，只留主体部分
    // TODO -- 可以在这个地方替换成boost::string_ref试下
    boost::string_ref inputRef(input);
    boost::string_ref mainPart = inputRef.substr(infoStartIndex + 1, std::string::npos);

    // 分割子串，获取分项信息
    std::vector<std::string> v;
    boost::split(v, mainPart, boost::is_any_of(";"), boost::token_compress_on);
    for(auto temp : v) {
        SingleIndexInfo info;
        std::vector<std::string> mainContentV;
        boost::split(mainContentV, temp, boost::is_any_of(","), boost::token_compress_on);
        // 除最后一项为时间之外，其余项都是按顺序复制到SingleIndexInfo.mainContent当中即可
        for(size_t i = 0;i < mainContentV.size() - 1;i++) {
            info.mainContent[i] = std::stod(mainContentV[i]);
        }
        info.time = QDateTime::fromString(
                    QString::fromStdString(mainContentV[mainContentV.size() - 1]),
                    "yyyy-MM-dd HH:mm:ss");

        this->info_list.push_back(info);
    }
    return true;
}

/**
 * 将本对象编码为字符串并且追加到输入参数input当中
 * 格式如下；
 * %ts_code,ts_name,today_open,pre_close:mainContent[0~26],time; \
 * mainContent[0~26],time;
 *
 * FIXME -- 可能存在如下情况：先前的内容是在开盘之前获取的，所以今日开盘和昨日收盘是相对于 \
 * 昨天来说的
 */
std::string& StockIndexBatchInfo::appendEncodeToStr(std::string& origin) {
    if(origin.size() == 0) {
        origin.append("%");
        origin.append(ts_code.toStdString()).append(ts_name.toStdString());
        origin.append(std::to_string(pre_close)).append(",");
        origin.append(std::to_string(today_open)).append(":");
    }
    else {
        if(origin.at(0) != '%') {
            throw new std::runtime_error("输入字符串格式不匹配！");;
        }

        size_t mainStartIndex = origin.find_first_of(':');
        if(mainStartIndex == std::string::npos) {
            throw new std::runtime_error("输入字符串格式不匹配！");
        }
        std::string ts_code = origin.substr(0, mainStartIndex + 1);
        if(this->ts_code.toStdString().compare(ts_code) != 0) {
            throw new std::runtime_error("输入参数同本对象编码不一致！");
        }
    }

    for(SingleIndexInfo tempInfo : info_list) {
        for(int j = 0;j < 27;j++) {
            origin.append(std::to_string(tempInfo.mainContent[j]));
            origin.append(",");
        }
        origin.append(tempInfo.time.toString("yyyy-MM-dd HH:mm:ss").toStdString());
        origin.append(";");
    }

    return origin;
}

/**
 * 类静态方法
 * 从新浪当中获取到的原始字符串信息，拼接到StockIndexBatchInfo的字符串表示当中
 * StockIndexBatchInfo字符串格式如下；
 * %ts_code,ts_name,today_open,pre_close:mainContent[0~26],time; \
 * mainContent[0~26],time;
 * ts_code格式类似于：000001.SZ
 *
 * FIXME -- 可能存在如下情况：先前的内容是在开盘之前获取的，所以今日开盘和昨日收盘是相对于 \
 * 昨天来说的
 */
std::string& StockIndexBatchInfo::appendEncodeUseSina(std::string& origin, std::string& sinaStr) {
    std::string realCode = sinaStr.substr(11, 8);

    // 将sina当中获取的格式转换为目标格式：000001.SZ
    if(realCode.find("sz") == std::string::npos) {
        realCode = std::string(realCode.substr(2, 6));
        realCode.append(".SH");
    }
    else {
        realCode = std::string(realCode.substr(2, 6));
        realCode.append(".SZ");
    }

    // 每次都从最新的信息当中获取头部，就能够避免昨日开盘等信息获取的是旧的的问题
    std::string preHead;
    std::string ts_code;
    bool originHasInfo = false;
    preHead.append("%");
    preHead.append(realCode).append(",");
    // 事前校验
    if(origin.size() > 0) {
        if(origin.at(0) != '%') {
            throw new std::runtime_error("输入字符串格式不匹配！");
        }

        size_t mainStartIndex = origin.find_first_of(':');
        if(mainStartIndex == std::string::npos) {
            throw new std::runtime_error("输入字符串格式不匹配！");
        }
        mainStartIndex = origin.find_first_of(',');
        ts_code = origin.substr(1, mainStartIndex - 1);
        if(realCode.compare(ts_code) != 0) {
            throw new std::runtime_error("代转换字符串同目标字符串所代表的股票不同！");
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
        if(v.size() > 31) {
            preHead.append(std::string(v[0].begin(), v[0].end())).append(",");
            preHead.append(std::string(v[1].begin(), v[1].end())).append(",");
            preHead.append(std::string(v[2].begin(), v[2].end())).append(":");
            if(!originHasInfo) {
                origin = std::move(preHead);
            }
            else {
                // 替换掉原先的头部，用preHead
                size_t mainBeginIndex = origin.find_first_of(':');
                origin.replace(0, mainBeginIndex + 1, preHead);
            }

            //　主体内容
            for(size_t i = 3;i < 30;i++) {
                origin.append(v[i].begin(), v[i].end()).append(",");
            }

            // 当前时间
            std::string dateStr(v[30].begin(), v[30].end());
            std::string timeStr(v[31].begin(), v[31].end());
            origin.append(dateStr).append(" ").append(timeStr).append(";");
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
 * %ts_code,ts_name,today_open,pre_close:mainContent[0~26],time; \
 * mainContent[0~26],time;
 * ts_code格式类似于：000001.SZ
 *
 * FIXME -- 可能存在如下情况：先前的内容是在开盘之前获取的，所以今日开盘和昨日收盘是相对于 \
 * 昨天来说的
 */
void StockIndexBatchInfo::mergeTwoEncodeStr(std::string& in1, std::string& in2) {
    if(in1.size() == 0 || in2.size() == 0) {
        return;
    }

    // TODO -- 校验，两个输入参数的合法性以及是否是同一支股票

    size_t mainStartIndex = in2.find_first_of(':');
    //std::cout << "before:" << std::endl;
    //std::cout << in1 << std::endl;
    in1.append(in2.substr(mainStartIndex + 1));
    //std::cout << "after:" << std::endl;
    //std::cout << in1 << std::endl;
}
