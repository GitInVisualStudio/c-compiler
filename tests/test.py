import subprocess
import sys
import os

def progressbar (iteration, total, prefix='', suffix='', decimals=1, length=100, fill='=', printEnd="\r"):
    percent = ("{0:." + str(decimals) + "f}").format(100 * (iteration / float(total)))
    filledLength = int(length * iteration // total)
    bar = fill * filledLength + ">" + '-' * (length - filledLength - 1)
    print(f'\r{prefix} |{bar}| {percent}% {suffix}', end=printEnd)
    if iteration == total: 
        print()

if len(sys.argv) != 2:
    print("Invalid amount of arguments given: \n python3 test.py {PATH_TO_YOUR_COMPILER}")
    exit(-1)

tests = []
compiler_path = sys.argv[1]

def get_tests(path):
    global tests
    if os.path.isdir(path):
        for sub in os.listdir(path):
            get_tests(path + "/" + sub)
        return
    if path.endswith(".c"):
        tests.append(path)

def execute(args) -> subprocess.Popen:
    p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    p.wait()
    return p

def maketests(path: str, fail: bool):
    
    p = execute((compiler_path, path, "../build/output.s"))
    valid = p.returncode == 0

    if not fail:
        execute(("gcc", "-o", "../build/out", "../build/output.s"))
        compiler_value = execute(("../build/out"))
        execute(("gcc", "-o", "../build/out", path))
        verify_value = execute(("../build/out"))
        if verify_value.returncode != compiler_value.returncode:
            print("Error with test: " + path)
            print("Returncode was not correct!")
            exit()

    if valid == fail:
        print("Error with test: " + path)
        exit()

get_tests("./tests")

for i in range(len(tests)):
    fail = "invalid" in tests[i]
    maketests(tests[i], fail)
    progressbar(i + 1, len(tests), prefix="Progress:", suffix=f"({i + 1}/{len(tests)}) Complete")
    

print("All tests passed!")