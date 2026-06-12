#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

bool isJackSymbol(char ch) {
    static string sym = "{}()[].,;+-*/&|<>=~";
    return sym.find(ch) != string::npos;
}

bool isJackKeyword(const string& tok) {
    static const vector<string> kw = {
        "class","constructor","function","method","field","static","var",
        "int","char","boolean","void","true","false","null","this",
        "let","do","if","else","while","return"
    };
    return find(kw.begin(), kw.end(), tok) != kw.end();
}

string convertXML(const string& s) {
    if (s == "<") return "&lt;";
    if (s == ">") return "&gt;";
    if (s == "&") return "&amp;";
    return s;
}

string getTokenType(const string& tok) {
    if (isJackKeyword(tok)) return "keyword";
    if (tok.size() == 1 && isJackSymbol(tok[0])) return "symbol";
    if (isdigit(tok[0])) return "integerConstant";
    if (!tok.empty() && tok[0] == '"') return "stringConstant";
    return "identifier";
}

void processJackFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Couldn't open file: " << filename << endl;
        return;
    }

    string code((istreambuf_iterator<char>(file)), {});
    vector<string> tokenList;

    string temp;
    bool inString = false;
    bool inBlockComment = false;

    for (size_t i = 0; i < code.size(); i++) {
        char ch = code[i];

        if (!inBlockComment && i + 1 < code.size() && ch == '/' && code[i + 1] == '*') {
            inBlockComment = true;
            i++;
            continue;
        }
        if (inBlockComment && i + 1 < code.size() && ch == '*' && code[i + 1] == '/') {
            inBlockComment = false;
            i++;
            continue;
        }
        if (inBlockComment) continue;

        if (!inString && i + 1 < code.size() && ch == '/' && code[i + 1] == '/') {
            while (i < code.size() && code[i] != '\n') i++;
            continue;
        }

        if (inString) {
            temp.push_back(ch);
            if (ch == '"') {
                tokenList.push_back(temp);
                temp.clear();
                inString = false;
            }
            continue;
        }
        if (ch == '"') {
            if (!temp.empty()) {
                tokenList.push_back(temp);
                temp.clear();
            }
            temp = "\"";
            inString = true;
            continue;
        }

        if (isspace(ch)) {
            if (!temp.empty()) {
                tokenList.push_back(temp);
                temp.clear();
            }
            continue;
        }

        if (isJackSymbol(ch)) {
            if (!temp.empty()) {
                tokenList.push_back(temp);
                temp.clear();
            }
            tokenList.push_back(string(1, ch));
            continue;
        }

        temp.push_back(ch);
    }

    if (!temp.empty())
        tokenList.push_back(temp);

    fs::path p(filename);
    string output = (p.parent_path() / ("My_" + p.stem().string() + "T.xml")).string();

    ofstream xml(output);
    xml << "<tokens>\n";

    for (auto& tok : tokenList) {
        string type = getTokenType(tok);
        string val = tok;

        if (type == "stringConstant")
            val = val.substr(1, val.size() - 2);

        xml << "<" << type << "> "
            << convertXML(val)
            << " </" << type << ">\n";
    }

    xml << "</tokens>\n";

    cout << " Token file created: " << output << endl;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Usage: ./tokenizer <folder>\n";
        return 1;
    }

    string folder = argv[1];

    for (auto& entry : fs::directory_iterator(folder)) {
        if (entry.path().extension() == ".jack") {
            processJackFile(entry.path().string());
        }
    }

    return 0;
}
