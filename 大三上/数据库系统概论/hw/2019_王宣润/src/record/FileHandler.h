#ifndef DATABASE_RECORDMANAGER_H
#define DATABASE_RECORDMANAGER_H


#include "../filesystem/bufmanager/BufPageManager.h"
#include "../global.h"

#include "SingleFileHandler.h"

class FileHandler {
public:
    static FileHandler *instance();
    static void resetInstance();

    ~FileHandler();

    RETVAL createFile(const char *fileName, int recordSize);

    RETVAL destroyFile(const char *fileName);
    RETVAL renameFile(const char *fileNameA, const char *fileNameB);

    SingleFileHandler* openFile(const char* fileName);
    RETVAL closeFile();
    BufPageManager *bufPageManager;
private:
    static FileHandler *recordManager;
    FileManager *fileManager;

    SingleFileHandler currentFileHandle;

    FileHandler();

    FileHandler(const FileHandler &) { }

    FileHandler &operator=(const FileHandler &) { return *this; }

    RETVAL openFile(const char *fileName, SingleFileHandler &fileHandle);
    RETVAL closeFile(SingleFileHandler &fileHandle);
};


#endif //DATABASE_RECORDMANAGER_H
