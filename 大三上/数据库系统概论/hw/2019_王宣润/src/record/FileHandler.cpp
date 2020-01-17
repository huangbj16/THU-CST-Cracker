#include <stdlib.h>

#include "FileHandler.h"

FileHandler *FileHandler::recordManager = nullptr;

FileHandler *FileHandler::instance() {
    if (recordManager == nullptr) {
        recordManager = new FileHandler();
    }
    return recordManager;
}

FileHandler::~FileHandler() {
    // When will the singleton be released ?
    // Is it necessary to delete the followings ?
    //bufPageManager->close();
    closeFile();
}

RETVAL FileHandler::createFile(const char *fileName, int recordSize) {
    if (recordSize > PAGE_SIZE)
        return RETVAL_ERR;
    if(currentFileHandle.isOpen()) {
        RETVAL rc = closeFile(currentFileHandle);
        if (rc != RETVAL_OK) {
            cerr << "Close File Failed!" << endl;
            exit(1);
        }
    }

    int fileID;
    RETURNIF(fileManager->createFile(fileName));
    RETURNIF(fileManager->openFile(fileName, fileID));
    FirstPageInfo firstPageHandle(fileID, bufPageManager);
    firstPageHandle.init(recordSize);
    bufPageManager->close();
    RETURNIF(fileManager->closeFile(fileID));

    return RETVAL_OK;
}

RETVAL FileHandler::destroyFile(const char *fileName) {
    RETVAL rc = fileManager->destroyFile(fileName);
    return RETVAL_OK;
}

RETVAL FileHandler::renameFile(const char *fileNameA, const char *fileNameB){
    if(currentFileHandle.isOpen()) {
        RETVAL rc = closeFile(currentFileHandle);
        if (rc != RETVAL_OK) {
            cerr << "Close File Failed!" << endl;
            exit(1);
        }
    }
    RETVAL rc = fileManager->renameFile(fileNameA, fileNameB);
    if (rc != RETVAL_OK)
        return RETVAL_ERR;
    return RETVAL_OK;
    // int fileID;
    // RETURNIF(fileManager->openFile(fileNameB, fileID));
    // FirstPageInfo firstPageHandle(fileID, bufPageManager);
    // firstPageHandle.init(recordSize);
    // bufPageManager->close();
    // RETURNIF(fileManager->closeFile(fileID));
}

RETVAL FileHandler::openFile(const char *fileName, SingleFileHandler &fileHandle) {

    int fileID;
    RETURNIF(fileManager->openFile(fileName, fileID));
    currentFileHandle = SingleFileHandler(fileID, bufPageManager);
    return RETVAL_OK;
}

RETVAL FileHandler::closeFile(SingleFileHandler &fileHandle) {
    bufPageManager->close();
    RETURNIF(fileManager->closeFile(fileHandle.getFileID()));
    fileHandle.release();
    return RETVAL_OK;
}

SingleFileHandler *FileHandler::openFile(const char *fileName) {
    if(currentFileHandle.isOpen()) {
        RETVAL rc = closeFile(currentFileHandle);
        if (rc != RETVAL_OK) {
            cerr << "Close File Failed!" << endl;
            exit(1);
        }
    }

    RETVAL rc = openFile(fileName, currentFileHandle);
    if (rc != RETVAL_OK) {
        cerr << "Open File Failed!" << endl;
        exit(1);
    }
    return &currentFileHandle;
}

RETVAL FileHandler::closeFile() {
    if(currentFileHandle.isOpen())
    {
        RETURNIF(closeFile(currentFileHandle));
    }
    return RETVAL_OK;
}

FileHandler::FileHandler() {
    fileManager = new FileManager();
    bufPageManager = new BufPageManager(fileManager);
}

void FileHandler::resetInstance() {
    delete recordManager;
    recordManager = new FileHandler();
}
