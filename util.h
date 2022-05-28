//
// Created by lostrong on 10/31/19.
//

#ifndef RADARPUSHIMPL_UTIL_H
#define RADARPUSHIMPL_UTIL_H
#include <iostream>
#include <string>
#include <fstream>
using namespace std;
bool exists_test(const std::string &name) {
    std::ifstream f(name.c_str());
    if (f.good()) {
        f.close();
        return true;
    }
    else {
        f.close();
        return false;
    }
}

void assert_file_exist(string desc, string name) {

    if (!exists_test(name)) {
        cerr << desc << " " << name << " not find " << endl;
        exit(1);
    }
}
#endif //RADARPUSHIMPL_UTIL_H
