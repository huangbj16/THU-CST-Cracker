#include <fstream>
#include "system/SystemManager.h"

using namespace std;

struct yy_buffer_state;
extern int yyparse();
extern void yy_switch_to_buffer(yy_buffer_state *);
extern yy_buffer_state *yy_scan_string(const char *);

vector<string> &split(const string &s, char delim, vector<string> &elems) {
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


vector<std::string> split(const string &s, char delim) {
    vector<string> elems;
    split(s, delim, elems);
    return elems;
}


void runSQL(const char *sql) {
    delete Tree::tree;
    Tree::tree = nullptr;
    yy_switch_to_buffer(yy_scan_string(sql));
    yyparse();
}

#define bufsize 10000
char charbuf[bufsize + 1];

int main(int argc, char** argv) {
    string buffer;
    string message;

    if (argc == 2){
        for (int i = 1; i < argc; ++i){
            FILE *fin = fopen(argv[i], "r");
            if (!fin){
                cerr << "[ERROR] Can not open file <" << argv[i] << ">." << endl;
                return 0;
            }
            cerr << "[INFO] Start running file <" << argv[i] << ">." << endl;
            while (fgets(charbuf, bufsize, fin)){
                if (strlen(charbuf) == 0) continue;
                charbuf[strlen(charbuf) - 1] = 0;
                message = string(charbuf);
                if (!message.empty()){
                    buffer += message;
                    if (message[message.size() - 1] == ';'){
                        runSQL(buffer.c_str());
                        buffer.clear();
                    }
                }
            }
            if (buffer.size() > 0)
                runSQL(buffer.c_str());
            fclose(fin);
        }
        return 0;
    }

    printf("\n>> ");
    while(getline(cin, message)) {
        if(!message.empty()) {
            if(message[message.size()-1] == ';') {
                buffer += message;
                runSQL(buffer.c_str());
                buffer.clear();
                printf("\n>> ");
            }
            else
                buffer += message;
        }
    }
    SystemManager::resetInstance();
    return 0;
}