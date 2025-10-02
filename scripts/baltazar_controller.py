#!/usr/bin/env python3
# Add the directory where *this* file actually resides
import sys
from pathlib import Path
import argparse
import subprocess
from typing import List
from scripts.baltazar_update_targets import collect_targets

BUILD_FOLDERS_MAP = {
        "debug": Path("build-debug") / "bin",
        "release": Path("build-release") / "bin",
} 

CONFIGURE_COMMANDS_MAP = {
    "debug": "cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ .",
    "release": "cmake -B build-release -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ .",
}

BUILD_COMMANDS_MAP = {
    "debug": "cmake --build build-debug",
    "release": "cmake --build build-release",
}

def read_lines_to_list(file_path: str) -> List[str]:
    """
    Read a text file and return a list where each element is one line (without the newline character).
    Blank lines are preserved as empty strings.
    """
    path = Path(file_path)
    if not path.exists():
        raise FileNotFoundError(f"No such file: {file_path}")

    with path.open("r", encoding="utf-8") as f:
        return [line.rstrip("\n") for line in f]

def list_baltazar_binaries(config: str):
    # Map config -> build dir
    build_dir = BUILD_FOLDERS_MAP[config]

    if not build_dir.is_dir():
        print(f"Error: directory not found: {build_dir}", file=sys.stderr)
        sys.exit(1)

    # List files starting with "baltazar_"
    matched = sorted(p for p in build_dir.iterdir() if p.is_file() and p.name.startswith("baltazar_"))

    if not matched:
        # Not necessarily error; just report nothing found
        print(f"(no matching files in {build_dir})")
        sys.exit(1)

    return matched

def main():
    parser = argparse.ArgumentParser(description="List Baltazar binaries for a given config.")
    parser.add_argument("command", choices=["configure", "build", "run", "list", "test"], help="Select which action to perform.")
    parser.add_argument("--target", help="Select which specific target to build or run.")
    parser.add_argument(
        "--config",
        choices=["debug", "release", "all"],
        default="debug",
        help="Select which build to perform action on (default: debug).",
    )
    parser.add_argument("--print-log", action="store_true", help="Turn on log prints in configure step.")
    parser.add_argument("--verbose", action="store_true", help="Turn on verbose output.")
    args = parser.parse_args()

    if args.command == "list":
        all_targets = collect_targets(".")

        for target in all_targets:
            print(target)

    if args.command == "configure":
        if args.config == "all":

            cmd = CONFIGURE_COMMANDS_MAP["debug"]
            if args.print_log:
                cmd = cmd + ' -DCMAKE_CXX_FLAGS="$CMAKE_CXX_FLAGS -DDEBUGLOG=1"'
            else:
                cmd = cmd + ' -DCMAKE_CXX_FLAGS="$CMAKE_CXX_FLAGS -UDEBUGLOG"'
            print("running command: "+cmd)
            subprocess.run(cmd, shell=True)

            cmd = CONFIGURE_COMMANDS_MAP["release"]
            if args.print_log:
                cmd = cmd + ' -DCMAKE_CXX_FLAGS="$CMAKE_CXX_FLAGS -DDEBUGLOG=1"'
            else:
                cmd = cmd + ' -DCMAKE_CXX_FLAGS="$CMAKE_CXX_FLAGS -UDEBUGLOG"'
            print("running command: "+cmd)
            subprocess.run(cmd, shell=True)
        else:
            cmd = CONFIGURE_COMMANDS_MAP[args.config]
            if args.print_log:
                cmd = cmd + ' -DCMAKE_CXX_FLAGS="$CMAKE_CXX_FLAGS -DDEBUGLOG=1"'
            else:
                cmd = cmd + ' -DCMAKE_CXX_FLAGS="$CMAKE_CXX_FLAGS -UDEBUGLOG"'
            print("running command: "+cmd)
            subprocess.run(cmd, shell=True)

    if args.command == "build":
        if args.config == "all":
            if args.target:
                print("WARNING: using taget for config=all is redundant.")

            cmd = BUILD_COMMANDS_MAP["debug"]
            if (args.verbose):
                cmd += " --verbose"
            print("running command: "+cmd)
            subprocess.run(cmd, shell=True)

            cmd = BUILD_COMMANDS_MAP["release"]
            if (args.verbose):
                cmd += " --verbose"
            print("running command: "+cmd)
            subprocess.run(cmd, shell=True)
        else:
            cmd = BUILD_COMMANDS_MAP[args.config]
            if (args.verbose):
                cmd += " --verbose"

            if args.target:
                cmd = cmd + " --target " + args.target

            print("running command: "+cmd)
            subprocess.run(cmd, shell=True)
            

    if args.command == "run":
        if args.config == "all":
            print("ERROR: It is not allowed to run both configs at the same time!")
            sys.exit(1)
        else:
            binaries = list_baltazar_binaries(args.config)

            if not args.target:
                print("ERROR: target not specified!")
                sys.exit(1)

            binary_exists = False
            for bin in binaries:
                if bin.name == args.target:
                    subprocess.run("./"+str(bin), shell=True)
                    binary_exists = True
                    break
            
            if not binary_exists:
                print("ERROR: binary doesn't exist!")
                sys.exit(1)


    if args.command == "test":
        binaries = list_baltazar_binaries("debug") 

        filtered = [s for s in binaries if str(s).endswith("_test")]

        if args.target:
            binary_exists = False
            for bin in filtered:
                if bin.name == args.target:
                    subprocess.run("./"+str(bin), shell=True)
                    binary_exists = True
                    break
            
            if not binary_exists:
                print("ERROR: binary doesn't exist!")
                sys.exit(1)
        else:
            all_successfull = True 
            for bin in filtered:
                result = subprocess.run("./"+str(bin), shell=True)
                if result.returncode != 0 and all_successfull:
                    all_successfull = False


            if not all_successfull:
                print("ERROR: Some tests failed!")

                


if __name__ == "__main__":
    main()

