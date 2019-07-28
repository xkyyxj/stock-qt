#ifndef FILEMANAGE_H
#define FILEMANAGE_H

#include <string>
#include <fstream>

class FileManager {
    std::string work_dir;
    std::string file_path;
public:
    FileManager(std::string&) noexcept;

    void setFilePath(std::string&) noexcept;
    void writeFileInfo(char content[], long length, bool) noexcept;
};

#endif // FILEMANAGE_H
