import sys
import os
from token_reader import TokenReader
from symbol_table import SymbolTable
from error_reporter import ErrorReporter
from vm_writer import VMWriter
from compilation_engine import CompilationEngine

def vm_output_path(token_path):
    base = os.path.basename(token_path)
    name = base[:-8]
    return os.path.join(os.path.dirname(token_path), name + '.vm')

def compile_one(path):
    reporter = ErrorReporter()
    tr = TokenReader(path, reporter)
    out = vm_output_path(path)
    vmw = VMWriter(out)
    sym = SymbolTable()
    engine = CompilationEngine(tr, vmw, sym, reporter, path)
    engine.compile_class()
    vmw.save()
    reporter.show()
    print(f"[✓] Generated: {out}")
    return len(reporter.errors), len(reporter.warnings)

def main():
    if(len(sys.argv) < 2):
        print("Usage: python Compiler.py <source_file>")
        sys.exit(2)

    input_path = sys.argv[1]

    fileNames = []

    if os.path.isdir(input_path):
        for filename in os.listdir(input_path):
            if filename.endswith("_myT.xml"):
                fileNames.append(os.path.join(input_path, filename))
    

    elif os.path.isfile(input_path):
        if input_path.endswith("_myT.xml"):
            fileNames.append(input_path)
        else:
            print("Error: Input file must be tokenized XML file with '_myT.xml' suffix.")
            sys.exit(2)
    
    else:
        print("Error: Input path is neither a file nor a directory.")
        sys.exit(2)
    
    if not fileNames:
        print("No token files found (expected *_myT.xml)")
        sys.exit(2)
    total_errors = 0
    total_warnings = 0
    for f in fileNames:
        print(f"[INFO] Compiling: {f}")
        e,w = compile_one(f)
        total_errors += e
        total_warnings += w
    print('[SUMMARY]')
    print(f' Files processed: {len(fileNames)}')
    print(f' Total errors: {total_errors}')
    print(f' Total warnings: {total_warnings}')
    
if __name__ == "__main__":
    main()
    