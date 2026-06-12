// VMTranslator.cpp
// Simple Project 7 VM translator (single .vm file -> .asm)
// Supports arithmetic and memory access commands.

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_set>

using namespace std;

enum class CommandType
{
    C_ARITHMETIC,
    C_PUSH,
    C_POP,
    C_UNKNOWN
};

struct Command
{
    CommandType type = CommandType::C_UNKNOWN;
    string arg1;  // for arithmetic: the command; for push/pop: segment
    int arg2 = 0; // for push/pop: index
    bool valid = false;
};

// Utility: trim and remove comments
static string strip(const string &line)
{
    string s = line;
    // remove comments
    auto pos = s.find("//");
    if (pos != string::npos)
        s = s.substr(0, pos);
    // trim spaces
    size_t start = 0;
    while (start < s.size() && isspace((unsigned char)s[start]))
        ++start;
    size_t end = s.size();
    while (end > start && isspace((unsigned char)s[end - 1]))
        --end;
    return s.substr(start, end - start);
}

// Parser: reads VM lines and produces Command
class Parser
{
    ifstream in;

public:
    Parser(const string &filename) { in.open(filename); }
    bool good() const { return in.is_open(); }

    bool hasMoreCommands()
    {
        while (in && in.peek() != EOF)
        {
            // peek next non-empty/non-comment line
            streampos pos = in.tellg();
            string line;
            getline(in, line);
            string s = strip(line);
            if (!s.empty())
            {
                // rewind to start of this line
                in.clear();
                in.seekg(pos);
                return true;
            }
        }
        return false;
    }

    // Read next command; returns false if none
    bool advance(Command &cmd)
    {
        string line;
        while (getline(in, line))
        {
            string s = strip(line);
            if (s.empty())
                continue;
            // parse tokens
            stringstream ss(s);
            string tok;
            vector<string> toks;
            while (ss >> tok)
                toks.push_back(tok);
            if (toks.empty())
                continue;

            // classify
            if (toks[0] == "push" && toks.size() == 3)
            {
                cmd.type = CommandType::C_PUSH;
                cmd.arg1 = toks[1];
                cmd.arg2 = stoi(toks[2]);
                cmd.valid = true;
                return true;
            }
            else if (toks[0] == "pop" && toks.size() == 3)
            {
                cmd.type = CommandType::C_POP;
                cmd.arg1 = toks[1];
                cmd.arg2 = stoi(toks[2]);
                cmd.valid = true;
                return true;
            }
            else
            {
                // arithmetic or logical or unknown (single token)
                static const unordered_set<string> arith = {
                    "add", "sub", "neg", "eq", "gt", "lt", "and", "or", "not"};
                if (toks.size() == 1 && arith.count(toks[0]))
                {
                    cmd.type = CommandType::C_ARITHMETIC;
                    cmd.arg1 = toks[0];
                    cmd.valid = true;
                    return true;
                }
            }
            // else unknown - skip
        }
        return false;
    }
};

// CodeWriter: emits Hack assembly for commands
class CodeWriter
{
    ofstream out;
    string basename; // for static variables
    int labelCounter = 0;

    string makeLabel(const string &prefix)
    {
        return prefix + to_string(labelCounter++);
    }

    void writeLine(const string &s) { out << s << '\n'; }

    // push D register value onto stack
    void pushDToStack()
    {
        writeLine("@SP");
        writeLine("A=M");
        writeLine("M=D");
        writeLine("@SP");
        writeLine("M=M+1");
    }

    // pop top of stack to D
    void popStackToD()
    {
        writeLine("@SP");
        writeLine("AM=M-1");
        writeLine("D=M");
    }

public:
    CodeWriter(const string &outfilename, const string &inbasename)
        : basename(inbasename)
    {
        out.open(outfilename);
    }

    bool good() const { return out.is_open(); }

    // Write arithmetic command
    void writeArithmetic(const string &cmd)
    {
        if (cmd == "add" || cmd == "sub" || cmd == "and" || cmd == "or")
        {
            // binary ops: pop y -> D, pop x -> M, compute, push result
            popStackToD(); // D = y
            writeLine("@SP");
            writeLine("A=M-1"); // A points to x
            if (cmd == "add")
                writeLine("M=M+D");
            if (cmd == "sub")
                writeLine("M=M-D");
            if (cmd == "and")
                writeLine("M=M&D");
            if (cmd == "or")
                writeLine("M=M|D");
        }
        else if (cmd == "neg" || cmd == "not")
        {
            // unary ops on top of stack
            writeLine("@SP");
            writeLine("A=M-1");
            if (cmd == "neg")
                writeLine("M=-M");
            else
                writeLine("M=!M");
        }
        else if (cmd == "eq" || cmd == "gt" || cmd == "lt")
        {
            // comparison: pop y -> D ; x in M (SP-1)
            popStackToD(); // D = y
            writeLine("@SP");
            writeLine("A=M-1"); // A -> x
            writeLine("D=M-D"); // D = x - y
            string labelTrue = makeLabel("TRUE");
            string labelEnd = makeLabel("END");
            writeLine("@" + labelTrue);
            if (cmd == "eq")
                writeLine("D;JEQ");
            if (cmd == "gt")
                writeLine("D;JGT");
            if (cmd == "lt")
                writeLine("D;JLT");
            // false:
            writeLine("@SP");
            writeLine("A=M-1");
            writeLine("M=0");
            writeLine("@" + labelEnd);
            writeLine("0;JMP");
            // true:
            writeLine("(" + labelTrue + ")");
            writeLine("@SP");
            writeLine("A=M-1");
            writeLine("M=-1"); // true = -1
            // end:
            writeLine("(" + labelEnd + ")");
        }
        else
        {
            // unknown arithmetic (shouldn't happen)
        }
    }

    // Write push command
    void writePush(const string &segment, int index)
    {
        if (segment == "constant")
        {
            writeLine("@" + to_string(index));
            writeLine("D=A");
            pushDToStack();
            return;
        }

        if (segment == "local" || segment == "argument" || segment == "this" || segment == "that")
        {
            string base;
            if (segment == "local")
                base = "LCL";
            if (segment == "argument")
                base = "ARG";
            if (segment == "this")
                base = "THIS";
            if (segment == "that")
                base = "THAT";
            writeLine("@" + to_string(index));
            writeLine("D=A");
            writeLine("@" + base);
            writeLine("A=M+D");
            writeLine("D=M");
            pushDToStack();
            return;
        }

        if (segment == "temp")
        {
            // temp base is 5
            writeLine("@" + to_string(5 + index));
            writeLine("D=M");
            pushDToStack();
            return;
        }

        if (segment == "pointer")
        {
            // pointer 0 -> THIS (3); pointer 1 -> THAT (4) actually in Hack: THIS=3? In Nand2Tetris, pointer 0 -> THIS, pointer1 -> THAT (addresses are in registers THIS/THAT)
            // Implementation: pointer 0 -> THIS, pointer 1 -> THAT
            if (index == 0)
                writeLine("@THIS");
            else
                writeLine("@THAT");
            writeLine("D=M");
            pushDToStack();
            return;
        }

        if (segment == "static")
        {
            string name = basename + "." + to_string(index);
            writeLine("@" + name);
            writeLine("D=M");
            pushDToStack();
            return;
        }

        // unknown
    }

    // Write pop command
    void writePop(const string &segment, int index)
    {
        if (segment == "constant")
        {
            // pop constant is invalid; ignore or no-op
            return;
        }

        if (segment == "local" || segment == "argument" || segment == "this" || segment == "that")
        {
            string base;
            if (segment == "local")
                base = "LCL";
            if (segment == "argument")
                base = "ARG";
            if (segment == "this")
                base = "THIS";
            if (segment == "that")
                base = "THAT";
            // compute target = base + index into R13
            writeLine("@" + to_string(index));
            writeLine("D=A");
            writeLine("@" + base);
            writeLine("D=M+D");
            writeLine("@R13");
            writeLine("M=D");
            // pop stack to D
            popStackToD();
            // store D into *R13
            writeLine("@R13");
            writeLine("A=M");
            writeLine("M=D");
            return;
        }

        if (segment == "temp")
        {
            popStackToD();
            writeLine("@" + to_string(5 + index));
            writeLine("M=D");
            return;
        }

        if (segment == "pointer")
        {
            popStackToD();
            if (index == 0)
                writeLine("@THIS");
            else
                writeLine("@THAT");
            writeLine("M=D");
            return;
        }

        if (segment == "static")
        {
            popStackToD();
            string name = basename + "." + to_string(index);
            writeLine("@" + name);
            writeLine("M=D");
            return;
        }

        // unknown
    }
};

static string getBaseName(const string &path)
{
    // return filename without directories and without .vm
    string s = path;
    // remove directories
    size_t pos = s.find_last_of("/\\");
    if (pos != string::npos)
        s = s.substr(pos + 1);
    // remove extension
    if (s.size() >= 3 && s.substr(s.size() - 3) == ".vm")
        s = s.substr(0, s.size() - 3);
    return s;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        cerr << "Usage: VMTranslator InputFile.vm\n";
        return 1;
    }
    string infile = argv[1];
    if (infile.size() < 4 || infile.substr(infile.size() - 3) != ".vm")
    {
        cerr << "Input must be a .vm file\n";
        return 1;
    }

    string basename = getBaseName(infile);
    string outfile = infile.substr(0, infile.size() - 3) + ".asm";

    Parser parser(infile);
    if (!parser.good())
    {
        cerr << "Failed to open " << infile << "\n";
        return 1;
    }
    CodeWriter writer(outfile, basename);
    if (!writer.good())
    {
        cerr << "Failed to open output " << outfile << "\n";
        return 1;
    }

    Command cmd;
    while (parser.advance(cmd))
    {
        if (!cmd.valid)
            continue;
        if (cmd.type == CommandType::C_ARITHMETIC)
        {
            writer.writeArithmetic(cmd.arg1);
        }
        else if (cmd.type == CommandType::C_PUSH)
        {
            writer.writePush(cmd.arg1, cmd.arg2);
        }
        else if (cmd.type == CommandType::C_POP)
        {
            writer.writePop(cmd.arg1, cmd.arg2);
        }
        // reset cmd
        cmd = Command();
    }

    cout << "Wrote " << outfile << "\n";
    return 0;
}