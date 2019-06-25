#pragma execution_character_set("utf-8")
#include "zlibcompress.h"
#include <fstream>
#include <iostream>
#include <vector>

using namespace zlib;

ZLibException::ZLibException(std::string val) noexcept {
    reason = val;
}

void ZLibCompress::startCompress() noexcept {
    if(compressStatus == COMPRESSING || compressStatus == START_COMPRESS) {
        deflateEnd(&compressStrm);
    }
    compressStrm.zalloc = nullptr;
    compressStrm.zfree = nullptr;
    compressStrm.opaque = nullptr;
    deflateInit(&compressStrm, Z_DEFAULT_COMPRESSION);

    compressStatus = START_COMPRESS;
}

std::vector<unsigned char> ZLibCompress::compressWithZLibPri(unsigned char* input, size_t size, int status) {
    compressStatus = COMPRESSING;

    int ret = 0;
    std::vector<unsigned char> finalResult;
    unsigned char output[102400];

    compressStrm.avail_in = size;
    compressStrm.next_in = input;

    do {
        compressStrm.avail_out = 102400;
        compressStrm.next_out = output;
        ret = deflate(&compressStrm, status);
        if(ret == Z_STREAM_ERROR) {
            throw new ZLibException("输入流错误！");
        }
        // 将输出追加到vector当中
        finalResult.assign(output, output + 102400 - compressStrm.avail_out);
    } while (compressStrm.avail_out == 0);

    if(ret != Z_STREAM_ERROR) {
        return finalResult;
    }

    return std::vector<unsigned char>();
}

std::vector<unsigned char> ZLibCompress::compressDataWithZlib(std::string& val) {
    char* temp = const_cast<char*>(val.data());
    unsigned char* input = reinterpret_cast<unsigned char*>(temp);
    return compressDataWithZlib(input, val.size());
}

std::vector<unsigned char> ZLibCompress::compressDataWithZlib(unsigned char* input, size_t size) {
    if(compressStatus != COMPRESSING && compressStatus != START_COMPRESS) {
        throw new ZLibException("状态错误：没有处于压缩进程当中");
    }

    return compressWithZLibPri(input, size, Z_NO_FLUSH);
}

std::vector<unsigned char> ZLibCompress::endCompress(unsigned char* input, size_t size) noexcept {
    // 调用compressDataWithZlib,输出压缩尾
    std::vector<unsigned char>&& temp = compressWithZLibPri(input, size, Z_FINISH);

    deflateEnd(&compressStrm);
    compressStatus = END_COMPRESS;

    return std::move(temp);
}

std::vector<unsigned char> ZLibCompress::endCompress(std::string& input) noexcept {
    // 调用compressDataWithZlib,输出压缩尾
    char* signedCharArray = const_cast<char*>(input.data());
    unsigned char* tempCharArray = reinterpret_cast<unsigned char*>(signedCharArray);
    std::vector<unsigned char>&& temp = compressWithZLibPri(tempCharArray, input.size(), Z_FINISH);

    deflateEnd(&compressStrm);
    compressStatus = END_COMPRESS;

    return std::move(temp);
}

inline int ZLibCompress::getCompressStatus() noexcept {
    return compressStatus;
}

void ZLibCompress::startDecompress() noexcept {
    if(decompressStatus == COMPRESSING || decompressStatus == START_COMPRESS) {
        inflateEnd(&decompressStrm);
    }
    decompressStrm.zalloc = nullptr;
    decompressStrm.zfree = nullptr;
    decompressStrm.opaque = nullptr;
    inflateInit(&decompressStrm);

    decompressStatus = START_DECOMPRESS;
}

std::vector<unsigned char> ZLibCompress::endDecompress(unsigned char* input, size_t size) noexcept {
    // 调用compressDataWithZlib,输出压缩尾
    std::vector<unsigned char>&& temp = decompressDataWithZlib(input, size);

    inflateEnd(&decompressStrm);
    decompressStatus = END_DECOMPRESS;

    return std::move(temp);
}

std::vector<unsigned char> ZLibCompress::decompressDataWithZlib(std::string& val) {
    char* temp = const_cast<char*>(val.data());
    unsigned char* input = reinterpret_cast<unsigned char*>(temp);
    return decompressDataWithZlib(input, val.size());
}

std::vector<unsigned char> ZLibCompress::decompressDataWithZlib(unsigned char* input, size_t size) {
    if(decompressStatus != DECOMPRESSING && decompressStatus != START_DECOMPRESS) {
        throw new ZLibException("状态错误：没有处于解压缩进程当中");
    }

    int ret = 0;
    unsigned char output[102400];
    std::vector<unsigned char> finalResult;

    decompressStrm.avail_in = size;
    decompressStrm.next_in = input;

    do {
        decompressStrm.avail_out = 102400;
        decompressStrm.next_out = output;
        ret = inflate(&decompressStrm, Z_NO_FLUSH);
        if(ret == Z_STREAM_ERROR) {
            break;
        }
        // 将输出追加到vector当中
        finalResult.assign(output, output + 102400 - decompressStrm.avail_out);
    } while (decompressStrm.avail_out == 0);

    if(ret != Z_STREAM_ERROR) {
        return finalResult;
    }

    return std::vector<unsigned char>();
}

inline int ZLibCompress::getDecompressStatus() noexcept {
    return decompressStatus;
}

ZLibCompress::ZLibCompress() noexcept {
    compressStatus = END_COMPRESS;
    decompressStatus = END_DECOMPRESS;
}

ZLibCompress::ZLibCompress(ZLibCompress&& origin) noexcept {
    compressStatus = origin.compressStatus;
    decompressStatus = origin.decompressStatus;
    origin.compressStatus = END_COMPRESS;
    origin.decompressStatus = END_DECOMPRESS;

    compressStrm = origin.compressStrm;
    decompressStrm = origin.decompressStrm;
}

ZLibCompress& ZLibCompress::operator=(ZLibCompress&& origin) noexcept {
    if(this == &origin)
        return *this;

    compressStatus = origin.compressStatus;
    decompressStatus = origin.decompressStatus;
    origin.compressStatus = END_COMPRESS;
    origin.decompressStatus = END_DECOMPRESS;

    compressStrm = origin.compressStrm;
    decompressStrm = origin.decompressStrm;
}

// 清理资源，意味着该类对象是不是不应该拷贝构造？
ZLibCompress::~ZLibCompress() noexcept {
    if(compressStatus == START_COMPRESS || compressStatus == COMPRESSING) {
        deflateEnd(&compressStrm);
    }

    if(decompressStatus == START_DECOMPRESS || decompressStatus == DECOMPRESSING) {
        inflateEnd(&decompressStrm);
    }
}
