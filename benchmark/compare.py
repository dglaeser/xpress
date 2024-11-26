# SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.a.glaeser@gmail.com>
# SPDX-License-Identifier: MIT

import os
import sys
import time
import subprocess
import argparse


def compile(exe: str) -> float:
    print(f"Compiling {exe}")
    t1 = time.time()
    subprocess.run(["make", exe], check=True)
    t2 = time.time()
    delta = t2 - t1
    print(f"Compilation took {delta:.2f} seconds")
    return delta


def binary_size(exe: str) -> float:
    return os.path.getsize(exe)


def run(exe: str) -> float:
    print(f"Running {exe}")
    output = subprocess.run([f"./{exe}"], check=True, capture_output=True, text=True).stdout
    print(output)
    return float(output.split("average runtime:")[1].split("\n")[0])


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-e", "--executables", required=True, nargs="*", help="The benchmark executables to run")
    parser.add_argument("-n", "--names", required=True, nargs="*", help="The displayed names of the benchmarks")
    parser.add_argument("-c", "--clean", required=False, default=True, help="If set to true, recompilation is forced and compile-time info is displayed")
    args = vars(parser.parse_args())

    exes = args["executables"]
    names = args["names"]
    if len(exes) != len(names):
        sys.stderr.write("Number of executables and names don't match")
        sys.exit(1)

    if args["clean"]:
        subprocess.run(["make", "clean"], check=True)

    compile_times = [compile(e) for e in exes]
    runtimes = [run(e) for e in exes]

    if args["clean"]:
        print("Compile time ratios:")
        print(f"\n".join(f" -- {names[i]}/{names[0]}: {compile_times[i]/compile_times[0]:.2f}" for i in range(1, len(names))))
    print("Binary size ratios:")
    print(f"\n".join(f" -- {names[i]}/{names[0]}: {binary_size(exes[i])/binary_size(exes[0]):.2f}" for i in range(1, len(names))))
    print("Runtime ratios:")
    print(f"\n".join(f" -- {names[i]}/{names[0]}: {runtimes[i]/runtimes[0]:.2f}" for i in range(1, len(names))))
