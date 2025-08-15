#!/usr/bin/env python3
import argparse
from pathlib import Path
import sys
import subprocess

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
    args = parser.parse_args()

    if args.command == "list":
        if args.config == "all":
            print("config = debug")
            binaries = list_baltazar_binaries("debug")
            
            for p in binaries:
                print(p.name)

            print("config = release")
            binaries = list_baltazar_binaries("release")
            
            for p in binaries:
                print(p.name)

        else:
            print("config = "+args.config)
            binaries = list_baltazar_binaries(args.config)
            
            for p in binaries:
                print(p.name)

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
            print("running command: "+cmd)
            subprocess.run(cmd, shell=True)

            cmd = BUILD_COMMANDS_MAP["release"]
            print("running command: "+cmd)
            subprocess.run(cmd, shell=True)
        else:
            cmd = BUILD_COMMANDS_MAP[args.config]

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


        # TODO: write run all tests.



if __name__ == "__main__":
    main()

