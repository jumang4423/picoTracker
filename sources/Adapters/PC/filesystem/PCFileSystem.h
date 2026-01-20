#ifndef _PC_FILESYSTEM_H_
#define _PC_FILESYSTEM_H_

#include "System/FileSystem/FileSystem.h"
#include <vector>
#include <string>
#include <filesystem>
#include <cstdio>

class PCFile : public I_File {
public:
    PCFile(FILE* f);
    virtual ~PCFile();
    virtual int Read(void* ptr, int size) override;
    virtual int GetC() override;
    virtual int Write(const void* ptr, int size, int nmemb) override;
    virtual void Seek(long offset, int whence) override;
    virtual long Tell() override;
    virtual int Error() override;
    virtual bool Sync() override;
    virtual void Dispose() override;
protected:
    virtual bool Close() override;
private:
    FILE* file_;
};

struct PCFileEntry {
    std::string name;
    PicoFileType type;
    uint64_t size;
};

class PCFileSystem : public FileSystem {
public:
    PCFileSystem();
    virtual ~PCFileSystem();

    virtual FileHandle Open(const char* name, const char* mode) override;
    virtual bool chdir(const char* path) override;
    virtual void list(etl::ivector<int>* fileIndexes, const char* filter, bool subDirOnly) override;
    virtual void getFileName(int index, char* name, int length) override;
    virtual PicoFileType getFileType(int index) override;
    virtual bool isParentRoot() override;
    virtual bool isCurrentRoot() override;
    virtual bool DeleteFile(const char* name) override;
    virtual bool DeleteDir(const char* name) override;
    virtual bool exists(const char* path) override;
    virtual bool makeDir(const char* path, bool pFlag = false) override;
    virtual uint64_t getFileSize(int index) override;
    virtual bool CopyFile(const char* src, const char* dest) override;

private:
   std::filesystem::path currentDir_;
   std::vector<PCFileEntry> fileList_;
   void refreshFileList();
};

#endif
