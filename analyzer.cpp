#include <bits/stdc++.h>
using namespace std;

void stripString(string &s) {
    if (s.length() == 0) return;
    if (s.length() >= 2) {
        if (s[0] == '/' && s[1] == '/') {
            s = "";
            return;
        }
    }

    int i;
    for (i = 0; i < s.length(); ++i) {
        if (s[i] != ' ' && s[i] != '\t') break;
    }
    s = s.substr(i, s.length() - i);

    bool comment = false;

    if (s.length() > 2) {
        if (s.substr(0, 3) == "/**") {
            s = "";
            return;
        }
    }

    if (s[0] == '*') {
        s = "";
        return;
    }

    for (i = s.length() - 1; i >= 0; --i) {
        if (s[i] =='/') {
            if (i > 0) {
                if (s[i-1] == '/') {
                    comment = true;
                    break;
                }
            }
        }
    }

    if (comment) s = s.substr(0, i-1);
    return;
}

bool isInt(string &s) {
    bool ans = true;
    for (int i = 0; i < s.length(); ++i) {
        if (s[i] < '0' || s[i] > '9') {
            ans = false;
            break;
        }
    }
    return ans;
}

int main() {

    vector<string> filenames = {"./ArrayTest/Main.jack", "./ExpressionLessSquare/Main.jack", "./ExpressionLessSquare/Square.jack", "./ExpressionLessSquare/SquareGame.jack", "./Square/Main.jack", "./Square/Square.jack", "./Square/SquareGame.jack"};
    vector<string> outfilenames = {"./ArrayTest/myMainT.xml", "./ExpressionLessSquare/myMainT.xml", "./ExpressionLessSquare/mySquareT.xml", "./ExpressionLessSquare/mySquareGameT.xml", "./Square/myMainT.xml", "./Square/mySquareT.xml", "./Square/mySquareGameT.xml"};

    set<string> keywordSet = {
        "class", "constructor", "function", "method",
        "field", "static", "var",
        "int", "char", "boolean", "void",
        "true", "false", "null", "this",
        "let", "do", "if", "else", "while", "return"
    };

    set<char> symbolSet = {
        '{', '}', '(', ')', '[', ']',
        '.', ',', ';', '+', '-', '*',
        '/', '&', '|', '<', '>', '=', '~', '"'
    };

    map<char, std::string> xmlEscapeMap = {
        {'<', "&lt;"},
        {'>', "&gt;"},
        {'"', "&quot;"},
        {'&', "&amp;"}
    };

    for (int i = 0; i < filenames.size(); ++i) {
        ifstream file(filenames[i]);
        ofstream out(outfilenames[i]);

        string s; 
        out << "<tokens>\n";
        while (getline(file , s)) {
            stripString(s);
            if (s.length() == 0) continue;

            string token;
            bool stringConstMode = false;
            for (size_t i = 0; i < s.length();) {
                while (i < s.length() && s[i] != '\t' && symbolSet.find(s[i]) == symbolSet.end() && (s[i] != ' ' || stringConstMode)) {
                    token.push_back(s[i]);
                    ++i;
                }
                if (token.size() == 0) {
                    if (symbolSet.find(s[i]) != symbolSet.end()){
                        if (s[i] == '"') stringConstMode = !stringConstMode;
                        else if (s[i] == '<' || s[i] == '>') {
                            out << "<symbol> ";
                            out << xmlEscapeMap[s[i]];
                            out << " </symbol>\n";
                        }
                        else if (s[i] == '&') {
                            out << "<symbol> ";
                            out << xmlEscapeMap[s[i]];
                            out << " </symbol>\n";
                        }
                        else {
                            out << "<symbol> ";
                            out << s[i];
                            out << " </symbol>\n";
                        }
                    }
                    ++i;
                }
                else {
                    if (stringConstMode) {
                        out << "<stringConstant> ";
                        out << token;
                        out << " </stringConstant>\n";
                    }
                    else if (isInt(token)) {
                        out << "<integerConstant> ";
                        out << token;
                        out << " </integerConstant>\n";
                    }
                    else if (keywordSet.find(token) != keywordSet.end()) {
                        out << "<keyword> ";
                        out << token;
                        out << " </keyword>\n";
                    }
                    else {
                        out << "<identifier> ";
                        out << token;
                        out << " </identifier>\n";
                    }
                    token = "";
                }
            }   
        }
        out << "</tokens>";
    }
}