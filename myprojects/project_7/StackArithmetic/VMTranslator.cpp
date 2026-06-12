#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
using namespace std;
namespace fs = std::filesystem;

// -------- Helpers --------
string trim(string str)
{
    size_t start = str.find_first_not_of(" \t\r\n");
    size_t end = str.find_last_not_of(" \t\r\n");
    return (start == string::npos) ? "" : str.substr(start, end - start + 1);
}

string segmentBase(const string &seg)
{
    if (seg == "local")
        return "LCL";
    if (seg == "argument")
        return "ARG";
    if (seg == "this")
        return "THIS";
    if (seg == "that")
        return "THAT";
    return "";
}

string getBaseName(const string &path)
{
    size_t slash = path.find_last_of("/\\");
    size_t dot = path.find_last_of('.');
    size_t start = (slash == string::npos) ? 0 : slash + 1;
    size_t end = (dot == string::npos) ? path.size() : dot;
    return path.substr(start, end - start);
}

string getDirName(const string &path)
{
    size_t slash = path.find_last_of("/\\");
    if (slash == string::npos)
        return path;
    return path.substr(slash + 1);
}

// -------- Code Writers --------
string writeArithmetic(const string &cmd)
{
    static int label = 0;
    ostringstream out;

    if (cmd == "add" || cmd == "sub" || cmd == "and" || cmd == "or")
    {
        out << "@SP\nAM=M-1\nD=M\nA=A-1\n";
        if (cmd == "add")
            out << "M=M+D\n";
        if (cmd == "sub")
            out << "M=M-D\n";
        if (cmd == "and")
            out << "M=M&D\n";
        if (cmd == "or")
            out << "M=M|D\n";
    }
    else if (cmd == "neg" || cmd == "not")
    {
        out << "@SP\nA=M-1\n";
        if (cmd == "neg")
            out << "M=-M\n";
        if (cmd == "not")
            out << "M=!M\n";
    }
    else if (cmd == "eq" || cmd == "gt" || cmd == "lt")
    {
        string TRUE = "TRUE_" + to_string(label);
        string END = "END_" + to_string(label++);
        out << "@SP\nAM=M-1\nD=M\nA=A-1\nD=M-D\n";
        out << "@" << TRUE << "\n";
        if (cmd == "eq")
            out << "D;JEQ\n";
        if (cmd == "gt")
            out << "D;JGT\n";
        if (cmd == "lt")
            out << "D;JLT\n";
        out << "@SP\nA=M-1\nM=0\n@" << END << "\n0;JMP\n";
        out << "(" << TRUE << ")\n@SP\nA=M-1\nM=-1\n(" << END << ")\n";
    }
    return out.str();
}

string writePushPop(const string &cmd, const string &segment, int index, const string &filename)
{
    ostringstream out;

    if (cmd == "push")
    {
        if (segment == "constant")
            out << "@" << index << "\nD=A\n";
        else if (segment == "temp")
            out << "@" << (5 + index) << "\nD=M\n";
        else if (segment == "pointer")
            out << "@" << (index == 0 ? "THIS" : "THAT") << "\nD=M\n";
        else if (segment == "static")
            out << "@" << filename << "." << index << "\nD=M\n";
        else
            out << "@" << segmentBase(segment) << "\nD=M\n@" << index << "\nA=A+D\nD=M\n";
        out << "@SP\nA=M\nM=D\n@SP\nM=M+1\n";
    }
    else if (cmd == "pop")
    {
        if (segment == "static")
            out << "@SP\nAM=M-1\nD=M\n@" << filename << "." << index << "\nM=D\n";
        else if (segment == "temp")
            out << "@SP\nAM=M-1\nD=M\n@" << (5 + index) << "\nM=D\n";
        else if (segment == "pointer")
            out << "@SP\nAM=M-1\nD=M\n@" << (index == 0 ? "THIS" : "THAT") << "\nM=D\n";
        else
        {
            out << "@" << segmentBase(segment) << "\nD=M\n@" << index << "\nD=A+D\n@R13\nM=D\n";
            out << "@SP\nAM=M-1\nD=M\n@R13\nA=M\nM=D\n";
        }
    }
    return out.str();
}

string makeScopedLabel(const string &functionName, const string &label)
{
    if (functionName.empty())
        return label;
    return functionName + "$" + label;
}

string writeLabel(const string &functionName, const string &label)
{
    return "(" + makeScopedLabel(functionName, label) + ")\n";
}

string writeGoto(const string &functionName, const string &label)
{
    return "@" + makeScopedLabel(functionName, label) + "\n0;JMP\n";
}

string writeIf(const string &functionName, const string &label)
{
    return "@SP\nAM=M-1\nD=M\n@" + makeScopedLabel(functionName, label) + "\nD;JNE\n";
}

string writeFunction(const string &fname, int nVars)
{
    ostringstream out;
    out << "(" << fname << ")\n";
    for (int i = 0; i < nVars; i++)
        out << "@0\nD=A\n@SP\nA=M\nM=D\n@SP\nM=M+1\n";
    return out.str();
}

string writeCall(const string &fname, int nArgs)
{
    static int callId = 0;
    string ret = "RET_" + to_string(callId++);
    ostringstream out;
    out << "@" << ret << "\nD=A\n@SP\nA=M\nM=D\n@SP\nM=M+1\n";
    for (string s : {"LCL", "ARG", "THIS", "THAT"})
    {
        out << "@" << s << "\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n";
    }
    out << "@SP\nD=M\n@" << (nArgs + 5) << "\nD=D-A\n@ARG\nM=D\n";
    out << "@SP\nD=M\n@LCL\nM=D\n";
    out << "@" << fname << "\n0;JMP\n(" << ret << ")\n";
    return out.str();
}

string writeReturn()
{
    ostringstream out;
    // FRAME = LCL => store in R13
    out << "@LCL\nD=M\n@R13\nM=D\n";
    // RET = *(FRAME-5) => store in R14
    out << "@5\nA=D-A\nD=M\n@R14\nM=D\n";
    // *ARG = pop()
    out << "@SP\nAM=M-1\nD=M\n@ARG\nA=M\nM=D\n";
    // SP = ARG+1
    out << "@ARG\nD=M+1\n@SP\nM=D\n";
    // THAT = *(FRAME-1)
    out << "@R13\nD=M\n@1\nA=D-A\nD=M\n@THAT\nM=D\n";
    // THIS = *(FRAME-2)
    out << "@R13\nD=M\n@2\nA=D-A\nD=M\n@THIS\nM=D\n";
    // ARG = *(FRAME-3)
    out << "@R13\nD=M\n@3\nA=D-A\nD=M\n@ARG\nM=D\n";
    // LCL = *(FRAME-4)
    out << "@R13\nD=M\n@4\nA=D-A\nD=M\n@LCL\nM=D\n";
    // goto RET
    out << "@R14\nA=M\n0;JMP\n";
    return out.str();
}

string writeInit()
{
    ostringstream out;
    out << "@256\nD=A\n@SP\nM=D\n";
    out << writeCall("Sys.init", 0);
    return out.str();
}

// -------- File translation --------
void translateFile(const fs::path &filepath, ostream &outStream, const string &filename, string &currentFunction)
{
    ifstream fin(filepath);
    if (!fin.is_open())
    {
        cerr << "Warning: could not open " << filepath << "\n";
        return;
    }

    string rawLine;
    while (getline(fin, rawLine))
    {
        // remove comments
        size_t commentPos = rawLine.find("//");
        string line = (commentPos == string::npos) ? rawLine : rawLine.substr(0, commentPos);
        line = trim(line);
        if (line.empty())
            continue;

        stringstream ss(line);
        string cmd;
        ss >> cmd;
        if (cmd.empty())
            continue;

        if (cmd == "push" || cmd == "pop")
        {
            string arg1;
            int idx;
            if (!(ss >> arg1 >> idx))
            {
                cerr << "Malformed " << cmd << " in " << filepath << ": '" << line << "'\n";
                continue;
            }
            outStream << writePushPop(cmd, arg1, idx, filename);
        }
        else if (cmd == "label")
        {
            string label;
            ss >> label;
            outStream << writeLabel(currentFunction, label);
        }
        else if (cmd == "goto")
        {
            string label;
            ss >> label;
            outStream << writeGoto(currentFunction, label);
        }
        else if (cmd == "if-goto")
        {
            string label;
            ss >> label;
            outStream << writeIf(currentFunction, label);
        }
        else if (cmd == "function")
        {
            string fname;
            int nVars;
            ss >> fname >> nVars;
            currentFunction = fname; // update current function for label scoping
            outStream << writeFunction(fname, nVars);
        }
        else if (cmd == "call")
        {
            string fname;
            int nArgs;
            ss >> fname >> nArgs;
            outStream << writeCall(fname, nArgs);
        }
        else if (cmd == "return")
        {
            outStream << writeReturn();
        }
        else
        {
            // arithmetic / logical ops
            outStream << writeArithmetic(cmd);
        }
    }

    fin.close();
}

// -------- Main Driver --------
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cerr << "Usage: " << argv[0] << " <input.vm | inputDirectory>\n";
        return 1;
    }

    fs::path inputPath(argv[1]);

    // Determine output file and list of vm files
    vector<fs::path> vmFiles;
    string outFilename; // path for output .asm
    if (fs::is_directory(inputPath))
    {
        // collect .vm files
        for (auto &p : fs::directory_iterator(inputPath))
        {
            if (!p.is_regular_file())
                continue;
            if (p.path().extension() == ".vm")
                vmFiles.push_back(p.path());
        }
        if (vmFiles.empty())
        {
            cerr << "No .vm files found in directory " << inputPath << "\n";
            return 1;
        }
        sort(vmFiles.begin(), vmFiles.end()); // deterministic order
        outFilename = inputPath.string();
        // if path ends with slash, getDirName handles it
        outFilename = (fs::path(outFilename).filename()).string() + ".asm";
    }
    else if (fs::is_regular_file(inputPath) && inputPath.extension() == ".vm")
    {
        vmFiles.push_back(inputPath);
        outFilename = inputPath.stem().string() + ".asm";
    }
    else
    {
        cerr << "Input must be a .vm file or a directory containing .vm files\n";
        return 1;
    }

    ofstream fout(outFilename);
    if (!fout.is_open())
    {
        cerr << "Error opening output file " << outFilename << "\n";
        return 1;
    }

    // Emit bootstrap once
    if (fs::is_directory(inputPath))
    {
        fout << writeInit();
    }

    // Translate each file, passing its base filename for static symbols
    string currentFunction = "";
    for (const auto &vmf : vmFiles)
    {
        string fname = getBaseName(vmf.string());
        translateFile(vmf, fout, fname, currentFunction);
    }

    fout.close();
    cout << "Created " << outFilename << " from input " << inputPath << "\n";
    return 0;
}