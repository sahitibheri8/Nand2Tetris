#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

bool isSymbol(char c) {
    static string symbols = "{}()[].,;+-*/&|<>=~";
    return symbols.find(c) != string::npos;
}

bool isKeyword(const string& word) {
    static const vector<string> keywords = {
        "class","constructor","function","method","field","static","var",
        "int","char","boolean","void","true","false","null","this",
        "let","do","if","else","while","return"
    };

    return find(keywords.begin(), keywords.end(), word) != keywords.end();
}

string xmlEscape(const string& s) {
    if (s == "<") return "&lt;";
    if (s == ">") return "&gt;";
    if (s == "&") return "&amp;";
    return s;
}

string tokenType(const string& token) {
    if (isKeyword(token)) return "keyword";
    if (token.size() == 1 && isSymbol(token[0])) return "symbol";
    if (isdigit(token[0])) return "integerConstant";
    if (token[0] == '"') return "stringConstant";
    return "identifier";
}

void tokenizeFile(const string& filePath) {
    ifstream in(filePath);
    if (!in.is_open()) {
        cerr << "Error: Couldn't open file " << filePath << "\n";
        return;
    }

    string content((istreambuf_iterator<char>(in)), {});
    vector<string> tokens;

    string current;
    bool insideString = false;
    bool insideBlockComment = false;

    for (size_t i = 0; i < content.size(); i++) {
        char c = content[i];

        if (!insideBlockComment && i + 1 < content.size() &&
            content[i] == '/' && content[i + 1] == '*') {
            insideBlockComment = true;
            i++;
            continue;
        }

        if (insideBlockComment && i + 1 < content.size() &&
            content[i] == '*' && content[i + 1] == '/') {
            insideBlockComment = false;
            i++;
            continue;
        }

        if (insideBlockComment) continue;

        if (!insideString && i + 1 < content.size() &&
            content[i] == '/' && content[i + 1] == '/') {
            while (i < content.size() && content[i] != '\n') i++;
            continue;
        }

        if (insideString) {
            current += c;
            if (c == '"') {
                tokens.push_back(current);
                current.clear();
                insideString = false;
            }
            continue;
        }

        if (c == '"') {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
            current = "\"";
            insideString = true;
            continue;
        }

        if (isspace(c)) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
            continue;
        }

        if (isSymbol(c)) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
            tokens.push_back(string(1, c));
            continue;
        }

        current += c;
    }

    if (!current.empty()) {
        tokens.push_back(current);
    }

    fs::path p(filePath);
    string outputPath = (p.parent_path() / ("My_" + p.stem().string() + "T.xml")).string();

    ofstream out(outputPath);

    out << "<tokens>\n";
    for (const auto& token : tokens) {
        string type = tokenType(token);
        string value = token;

        if (type == "stringConstant") {
            value = value.substr(1, value.size() - 2);
        }

        out << "<" << type << "> " << xmlEscape(value) << " </" << type << ">\n";
    }
    out << "</tokens>\n";

    cout << "Generated: " << outputPath << "\n";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Usage: tokenizer <directory>\n";
        return 1;
    }

    string directory = argv[1];

    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.path().extension() == ".jack") {
            tokenizeFile(entry.path().string());
        }
    }

    return 0;
}
