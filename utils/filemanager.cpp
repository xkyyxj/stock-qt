#include "utils/filemanager.h"

FileManager::FileManager(std::string& work_dir) noexcept {
    this->work_dir = work_dir;
}

void FileManager::setFilePath(std::string& file_path) noexcept {
    this->file_path = file_path;
}

void FileManager::writeFileInfo(char content[], long length, bool append) noexcept {
    std::string final_path(work_dir);
    if(!file_path.empty()) {
        final_path.append("/").append(file_path);
    }
    else {
        return;
    }

    std::ofstream out(final_path, append ? std::ios::app : std::ios::out);
    out.write(content, length);
}
