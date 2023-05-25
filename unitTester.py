#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os
import sys
import glob
import difflib
from subprocess import Popen, PIPE
from threading import Timer
from util import *

MAX_TEST_RUN_TIME = 3
def timeoutProcess(process, verbose = False):
    if verbose:
        print("Test timed out. Killing...")
    process.kill()

def run_unit_test(program, test_num, verbose = False):
    exit_code, err, output = run_program([program, test_num], verbose=verbose, timeout=MAX_TEST_RUN_TIME)
    if exit_code == 0:
        if verbose:
            print("Test passed")
        return True
    else:
        if verbose:
            if exit_code != 0:
                print(f"Unit test failed with code: {exit_code} ({get_code_reason(exit_code)})")
            if err is not None:
                print("Error occured during execution:", err)
        return False


def run_unit_tests(program, maxPoints, verbose = 0):
    if verbose > 1:
        print(f"Running unit tests")
    if not os.path.isfile(program):
        if verbose > 0:
            print(f"Unit test program does not exist. Check and fix and deduct 10 special handling points")
        return 0
    exit_code, err, output = run_program([program, "-tests"], verbose=verbose, timeout=MAX_TEST_RUN_TIME)
    if exit_code != 0:
        print("Failed to retrieve test names. Check and fix and deduct 10 special handling points")
        return 0
    tests = [line.split()[1] for line in output]
    totalTests = 0
    testsPassed = 0
    for testNum in range(len(tests)):
        # Find the corresponding out file
        if verbose > 1:
            print(f"Running test: [{testNum:2d}] {tests[testNum]}")
        totalTests += 1
        result = run_unit_test(program, str(testNum), verbose = verbose > 1)
        if result:
            testsPassed += 1
            if verbose == 1:
                print("+", end="", flush=True)
        else:
            if verbose == 1:
                print("-", end="", flush=True)
            
    if verbose == 1:
        print("\n", end="")
    pointsAchieved = round(maxPoints * (testsPassed / totalTests))
    if verbose > 0:
        print(f"Passed {testsPassed} out of {totalTests} unit tests, resulting in {pointsAchieved} points.")
    return pointsAchieved
                
    

if __name__ == '__main__':
    unitTestName = sys.argv[1]
    maxPoints = int(sys.argv[2])
    verbose = int(sys.argv[3]) if len(sys.argv) >=4 else 0
    finalScore = run_unit_tests(unitTestName, maxPoints, verbose=verbose)
    print(finalScore)
