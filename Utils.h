#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <iostream>

using namespace std;

namespace Utils {
    inline vector<string> split(const string& s, char delimiter) {
        vector<string> tokens;
        string token;
        istringstream tokenStream(s);
        while (getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    inline string trim(const string& str) {
        size_t first = str.find_first_not_of(" \t\r\n");
        if (string::npos == first) {
            return "";
        }
        size_t last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, (last - first + 1));
    }

    inline void printHeader(const string& title) {
        cout << "\n==================================================\n";
        cout << "  " << title << "\n";
        cout << "==================================================\n";
    }

    inline void printFooter() {
        cout << "==================================================\n";
    }
}

#endif // UTILS_H