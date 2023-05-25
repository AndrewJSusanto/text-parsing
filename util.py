from subprocess import Popen, PIPE
from threading import Timer

def get_code_reason(code):
    if code == 0:
        return "Success"
    elif code in code_lookup_table:
        return code_lookup_table[code]
    else:
        return ""

code_lookup_table = {
    -6: "Double or invalid free",
    -9: "Program killed due to timeout",
    -11: "Segfault",
    51: "Invalid argumentds to unit test",
    50: "Test index is out of bounds",
    30: "Program tried to exit early",
    255: "Test not defined",
    88: "Not enough space to allocate output buffer"
}

def timeoutProcess(process, verbose = False):
    if verbose:
        print("Test timed out. Killing...")
    process.kill()

def run_program(program_with_args, stdin = None, verbose = False, timeout = 5):
    close_stdin = False
    if isinstance(stdin, str):
        close_stdin = True
        stdin = open(stdin, "r")
    if isinstance(program_with_args, str):
        program_with_args = [program_with_args]
    
    process = Popen(program_with_args, stdout=PIPE, stderr=PIPE, stdin=stdin)
    timer = Timer(timeout, lambda process: timeoutProcess(process, verbose), [process])
    output = None
    err = None
    try:
        timer.start()
        (output, err) = process.communicate()
    finally:
        timer.cancel()
    exit_code = process.wait()
    if close_stdin:
        stdin.close()

    return(exit_code, err.decode("utf-8", errors="ignore") if err else None, output.decode("utf-8", errors="ignore").splitlines(True) if output is not None else None)