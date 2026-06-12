#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

class JackLexer {
private:
    vector<string> tokenStream;

    const vector<string> keywords = {
        "class","constructor","function","method","field","static","var",
        "int","char","boolean","void","true","false","null","this",
        "let","do","if","else","while","return"
    };

    const string symbolSet = "{}()[].,;+-*/&|<>=~";

    bool isKeyword(const string& s) {
        return find(keywords.begin(), keywords.end(), s) != keywords.end();
    }

    bool isSymbol(char c) {
        return symbolSet.find(c) != string::npos;
    }

    bool isInteger(const string& s) {
        if (!all_of(s.begin(), s.end(), ::isdigit)) return false;
        int val = stoi(s);
        return val >= 0 && val <= 32767;
    }

    bool isStringLiteral(const string& s) {
        return s.size() >= 2 && s.front() == '"' && s.back() == '"';
    }

    vector<string> stripComments(const string& file) {
        ifstream fin(file);
        vector<string> code;
        string line;
        bool block = false;

        while (getline(fin, line)) {
            string cur;
            for (size_t i = 0; i < line.size(); i++) {
                if (!block && i + 1 < line.size() && line[i] == '/' && line[i + 1] == '/') break;
                if (!block && i + 1 < line.size() && line[i] == '/' && line[i + 1] == '*') {
                    block = true;
                    i++;
                    continue;
                }
                if (block && i + 1 < line.size() && line[i] == '*' && line[i + 1] == '/') {
                    block = false;
                    i++;
                    continue;
                }

                if (!block) cur.push_back(line[i]);
            }
            if (!cur.empty()) code.push_back(cur);
        }
        return code;
    }

    vector<string> tokenizeLine(const string& line) {
        vector<string> out;
        string temp;
        bool stringMode = false;

        for (char c : line) {
            if (c == '"') {
                if (stringMode) {
                    temp.push_back(c);
                    out.push_back(temp);
                    temp.clear();
                    stringMode = false;
                } else {
                    if (!temp.empty()) out.push_back(temp), temp.clear();
                    temp.push_back(c);
                    stringMode = true;
                }
            }
            else if (stringMode) temp.push_back(c);
            else if (isspace(c)) {
                if (!temp.empty()) out.push_back(temp), temp.clear();
            }
            else if (isSymbol(c)) {
                if (!temp.empty()) out.push_back(temp), temp.clear();
                out.push_back(string(1, c));
            }
            else temp.push_back(c);
        }
        if (!temp.empty()) out.push_back(temp);
        return out;
    }

public:
    string processFile(const string& file) {
        vector<string> cleaned = stripComments(file);
        string outname = file.substr(0, file.find_last_of('.')) + "_tokens.xml";

        ofstream out(outname);
        out << "<tokens>\n";

        for (const string& line : cleaned) {
            for (const string& tok : tokenizeLine(line)) {

                tokenStream.push_back(tok);

                if (isKeyword(tok))
                    out << "<keyword> " << tok << " </keyword>\n";

                else if (tok.size() == 1 && isSymbol(tok[0])) {
                    string v;
                    if (tok == "<") v = "&lt;";
                    else if (tok == ">") v = "&gt;";
                    else if (tok == "&") v = "&amp;";
                    else v = tok;
                    out << "<symbol> " << v << " </symbol>\n";
                }
                else if (isInteger(tok))
                    out << "<integerConstant> " << tok << " </integerConstant>\n";

                else if (isStringLiteral(tok))
                    out << "<stringConstant> " << tok.substr(1, tok.size() - 2) << " </stringConstant>\n";

                else
                    out << "<identifier> " << tok << " </identifier>\n";
            }
        }

        out << "</tokens>\n";
        return outname;
    }
};

int main(int argc, char* argv[]) {

    if (argc != 2) {
        cout << "Usage: tokenizer <file.or.directory>\n";
        return 0;
    }

    string target = argv[1];
    vector<string> inputFiles;

    if (fs::is_directory(target)) {
        for (auto& it : fs::directory_iterator(target)) {
            if (it.path().extension() == ".jack")
                inputFiles.push_back(it.path().string());
        }
    } else {
        inputFiles.push_back(target);
    }

    for (auto& f : inputFiles) {
        JackLexer jl;
        string out = jl.processFile(f);
        cout << "Processed: " << f << " -> " << out << endl;
    }
}
