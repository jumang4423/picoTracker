#include "PCFileSystem.h"
#include "System/Process/SysMutex.h"
#include <algorithm>
#include <cstring>
#include <iostream>

// PCFile Implementation

PCFile::PCFile(FILE* f) : file_(f) {}

PCFile::~PCFile() {
    if (file_) fclose(file_);
}

int PCFile::Read(void* ptr, int size) {
    return fread(ptr, 1, size, file_);
}

int PCFile::GetC() {
    return fgetc(file_);
}

int PCFile::Write(const void* ptr, int size, int nmemb) {
    return fwrite(ptr, size, nmemb, file_);
}

void PCFile::Seek(long offset, int whence) {
    fseek(file_, offset, whence);
}

long PCFile::Tell() {
    return ftell(file_);
}

int PCFile::Error() {
    return ferror(file_);
}

bool PCFile::Sync() {
    return fflush(file_) == 0;
}

void PCFile::Dispose() {
    delete this;
}

bool PCFile::Close() {
    if (file_) {
        fclose(file_);
        file_ = nullptr;
        return true;
    }
    return false;
}

// PCFileSystem Implementation

PCFileSystem::PCFileSystem() {
    std::cout << "PCFileSystem Constructor: CWD=" << std::filesystem::current_path() << std::endl;
    // Start at current working directory
    currentDir_ = std::filesystem::current_path();
}

PCFileSystem::~PCFileSystem() {}

FileHandle PCFileSystem::Open(const char* name, const char* mode) {
    if (!name) return FileHandle(nullptr);
    std::string fullPath;
    // Check if absolute path
    if (name[0] == '/') {
        std::filesystem::path p = std::filesystem::current_path() / (name + 1); // Skip leading /
        fullPath = p.string();
    } else {
        fullPath = (currentDir_ / name).string();
    }
    std::cout << "PCFileSystem Open: " << fullPath << std::endl;
    
    // For read modes, ensure file exists? fopen handles it.
    FILE* f = fopen(fullPath.c_str(), mode);
    if (f) {
        return FileHandle(new PCFile(f));
    }
    return FileHandle(nullptr);
}

bool PCFileSystem::chdir(const char* path) {
    std::filesystem::path newPath;
    if (path[0] == '/') {
         newPath = std::filesystem::current_path() / (path + 1);
    } else {
         newPath = currentDir_ / path;
    }

    /*
    if (strcmp(path, "..") == 0) {
        currentDir_ = currentDir_.parent_path();
        return true;
    }
    */
    
    if (std::filesystem::exists(newPath) && std::filesystem::is_directory(newPath)) {
        currentDir_ = std::filesystem::canonical(newPath);
        return true;
    }
    return false;
}

void PCFileSystem::list(etl::ivector<int>* fileIndexes, const char* filter, bool subDirOnly) {
    fileList_.clear();
    fileIndexes->clear();
    
    int index = 0;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(currentDir_)) {
            PCFileEntry pcEntry;
            pcEntry.name = entry.path().filename().string();
            
            bool isDir = entry.is_directory();
            pcEntry.type = isDir ? PFT_DIR : PFT_FILE;
            pcEntry.size = isDir ? 0 : entry.file_size();
            
            // Filter
            bool match = true;
            if (filter && strlen(filter) > 0) {
                 // Simple substring matching or extension check? 
                 // The app usually passes extension like ".wav"
                 if (!isDir) {
                     if (pcEntry.name.find(filter) == std::string::npos) {
                         match = false;
                     }
                 }
            }
            
            if (subDirOnly && !isDir) match = false;
            
            if (match) {
                fileList_.push_back(pcEntry);
                fileIndexes->push_back(index);
                index++;
                if (index >= MAX_FILE_INDEX_SIZE) break;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "List error: " << e.what() << std::endl;
    }
}

void PCFileSystem::getFileName(int index, char* name, int length) {
    if (index >= 0 && index < fileList_.size()) {
        strncpy(name, fileList_[index].name.c_str(), length);
        name[length - 1] = 0; // Ensure null termination
    } else {
        name[0] = 0;
    }
}

PicoFileType PCFileSystem::getFileType(int index) {
    if (index >= 0 && index < fileList_.size()) {
        return fileList_[index].type;
    }
    return PFT_UNKNOWN;
}

bool PCFileSystem::isParentRoot() {
    // Check if parent is root? 
    // Simply check if currentDir has a parent
    return currentDir_.has_parent_path(); 
}

bool PCFileSystem::isCurrentRoot() {
    // Assumption: we run from the "Root"
    return currentDir_ == std::filesystem::current_path();
}

bool PCFileSystem::DeleteFile(const char* name) {
    std::filesystem::path p = currentDir_ / name;
    return std::filesystem::remove(p);
}

bool PCFileSystem::DeleteDir(const char* name) {
    std::filesystem::path p = currentDir_ / name;
    return std::filesystem::remove_all(p) > 0;
}

bool PCFileSystem::exists(const char* path) {
    if (!path) return false;
    std::filesystem::path p;
    if (path[0] == '/') {
         p = std::filesystem::current_path() / (path + 1);
    } else {
         p = currentDir_ / path;
    }
    bool res = std::filesystem::exists(p);
    std::cout << "PCFileSystem exists(" << path << ") -> " << res << " [" << p << "]" << std::endl;
    return res;
}

bool PCFileSystem::makeDir(const char* path, bool pFlag) {
    if (!path) return false;
    std::filesystem::path p;
    if (path[0] == '/') {
         p = std::filesystem::current_path() / (path + 1);
    } else {
         p = currentDir_ / path;
    }
    std::cout << "PCFileSystem makeDir(" << path << ") [" << p << "] calling create_directories..." << std::endl;
    try {
        bool result = std::filesystem::create_directories(p);
        std::cout << "PCFileSystem makeDir success. Result=" << result << std::endl;
        return result;
    } catch (const std::exception& e) {
        std::cout << "PCFileSystem makeDir EXCEPTION: " << e.what() << std::endl;
        return false;
    }
}

uint64_t PCFileSystem::getFileSize(int index) {
    if (index >= 0 && index < fileList_.size()) {
        return fileList_[index].size;
    }
    return 0;
}

bool PCFileSystem::CopyFile(const char* src, const char* dest) {
    try {
        std::filesystem::path s = currentDir_ / src;
        std::filesystem::path d = currentDir_ / dest;
        std::filesystem::copy_file(s, d, std::filesystem::copy_options::overwrite_existing);
        return true;
    } catch (...) {
        return false;
    }
}
