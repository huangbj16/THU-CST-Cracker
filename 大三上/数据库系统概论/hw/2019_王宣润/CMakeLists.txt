cmake_minimum_required(VERSION 3.3)
project(database)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -DYYERROR_VERBOSE")

set(SOURCE_FILES
        src/global.h
        src/filesystem/bufmanager/BufPageManager.h
        src/filesystem/bufmanager/FindReplace.h
        src/filesystem/fileio/FileManager.h
        src/filesystem/fileio/FileTable.h
        src/filesystem/utils/compare.h
        src/filesystem/utils/MyBitMap.cpp
        src/filesystem/utils/MyBitMap.h
        src/filesystem/utils/MyHashMap.h
        src/filesystem/utils/MyLinkList.h
        src/filesystem/utils/pagedef.h
        src/record/FileHandler.cpp
        src/record/FileHandler.h
        src/record/SingleFileHandler.cpp
        src/record/SingleFileHandler.h
        src/record/RecordID.cpp
        src/record/RecordID.h
        src/record/Record.cpp
        src/record/Record.h
        src/record/PageInfo.cpp
        src/record/PageInfo.h
        src/record/FirstPageInfo.cpp
        src/record/FirstPageInfo.h
        src/record/FileScan.cpp
        src/record/FileScan.h
        src/record/utils.h
        src/system/SystemManager.cpp
        src/system/SystemManager.h
        src/system/Printer.h
        src/system/Printer.cpp
        src/system/RecordDescriptor.h
        src/system/RecordDescriptor.cpp
        src/system/DBHandle.h
        src/system/DBHandle.cpp
        src/parser/lex.yy.cpp
        src/parser/Tree.cpp
        src/parser/Tree.h
        src/parser/y.tab.cpp
        src/parser/y.tab.hpp
        src/system/RecordDescriptor.cpp
        src/system/RecordDescriptor.h
        src/index/IndexHandler.cpp
        src/index/IndexHandler.h
        src/index/BNode.cpp
        src/index/BNode.h
        src/index/SingleIndexHandler.cpp
        src/index/SingleIndexHandler.h
        src/error/Error.cpp
        src/error/Error.h
)

SET_SOURCE_FILES_PROPERTIES(src/parser/lex.yy.cpp PROPERTIES LANGUAGE CXX )

add_executable(database ${SOURCE_FILES} src/main.cpp)
target_link_libraries(database)