#pragma execution_character_set("utf-8")
#ifndef COMPRESS_H
#define COMPRESS_H

#include <string>
#include <exception>
#include <vector>
#include <zlib.h>

namespace zlib {

    class ZLibException: public std::exception {
        std::string reason;

    public:
        ZLibException(std::string) noexcept;

        char const* what() const {
            return reason.data();
        }
    };

    class ZLibCompress {

        z_stream compressStrm;
        z_stream decompressStrm;

        int compressStatus, decompressStatus;
        int compressRet, decompressRet;

        std::vector<unsigned char> compressWithZLibPri(unsigned char*, size_t, int);

    public:

        enum {
            START_COMPRESS, COMPRESSING, END_COMPRESS,
            START_DECOMPRESS, DECOMPRESSING, END_DECOMPRESS
        };

        ZLibCompress() noexcept;

        ~ZLibCompress() noexcept;

        ZLibCompress(ZLibCompress&&) noexcept;

        void startCompress() noexcept;

        ZLibCompress(const ZLibCompress&) = delete;

        ZLibCompress& operator=(const ZLibCompress&) = delete;

        ZLibCompress& operator=(ZLibCompress&&) noexcept;

        std::vector<unsigned char> endCompress(unsigned char*, size_t) noexcept;

        std::vector<unsigned char> endCompress(std::string&) noexcept;

        std::vector<unsigned char> compressDataWithZlib(std::string&);

        std::vector<unsigned char> compressDataWithZlib(unsigned char*, size_t);

        inline int getCompressStatus() noexcept;

        void startDecompress() noexcept;

        std::vector<unsigned char> endDecompress(unsigned char*, size_t) noexcept;

        std::vector<unsigned char> decompressDataWithZlib(std::string&);

        std::vector<unsigned char> decompressDataWithZlib(unsigned char*, size_t);

        inline int getDecompressStatus() noexcept;
    };
}

#endif // COMPRESS_H
