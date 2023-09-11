import os,sys
if len(sys.argv)>3:
    if sys.argv[1] in ["--clang", "-c"] and sys.argv[2] in ["--shared", "-s"]:os.system(f"clang140 -fPIC -shared -o {sys.argv[3]}.so {sys.argv[3]}.c -target armv7a-linux-androideabi19 -s")
    elif sys.argv[1] in ["--clang", "-c"] and sys.argv[2] in ["--binary", "-b"]:os.system(f"clang140 -o {sys.argv[3]} {sys.argv[3]}.c {sys.argv[4]} {sys.argv[5]} {sys.argv[6]} -target armv7a-linux-androideabi19 -s")
    elif sys.argv[1] in ["--clang++", "-cpp"] and sys.argv[2] in ["--shared", "-s"]:os.system(f"clang140++ -fPIC -shared -o {sys.argv[3]}.so {sys.argv[3]}.cpp -target armv7a-linux-androideabi19 -static-libstdc++ -s")
    elif sys.argv[1] in ["--clang++", "-cpp"] and sys.argv[2] in ["--binary", "-b"]:os.system(f"clang140++ -o {sys.argv[3]} {sys.argv[3]}.cpp {sys.argv[4]} {sys.argv[5]} {sys.argv[6]} -target armv7a-linux-androideabi19 -static-libstdc++ -s")
    else:print("Invalid command-line argument. Use '--shared' or '--binary'.")
elif len(sys.argv)>1 and (sys.argv[1]=="--help" or sys.argv[1]=="-h"):print("Clang/Clang++ NDK Compilation Script\n\n** https://github.com/Solaree **\n\nRequirements: You must have added Android NDK Toolchain to your PATH variable\n\nUsage:\n['--clang', '-c']: Compilation of '.c' file to file with arch 'armv7a-linux-androideabi' and API 19\n['--clang++', '-cpp']: Compilation of '.cpp' file to file with arch 'armv7a-linux-androideabi' and API 19\n['--shared', '-s']: Compilation to '.so' shared library\n['--binary', '-b']: Compilation to binary executable.")
else:print("Clang/Clang++ NDK Compilation Script. Type '--help' or '-h' for help.")