import os
import platform
import subprocess
from pathlib import Path
import shutil


def is_executable(file_path):
    """Check if a file is executable."""
    if platform.system() == "Windows":
        return file_path.suffix.lower() == ".exe" and file_path.is_file()
    else:
        return os.access(file_path, os.X_OK) and file_path.is_file()


def run_file(file_path, use_wine=False):
    print(f"Running: {file_path}")
    try:
        if use_wine:
            result = subprocess.run(["wine", str(file_path)], cwd=file_path.parent, check=False)
        else:
            result = subprocess.run([str(file_path)], cwd=file_path.parent, check=False)
        if result.returncode == 0:
            print("Test passed!")
        else:
            print(f"Test failed with exit code: {result.returncode}")
        return result.returncode
    except Exception as e:
        print(f"Failed to run {file_path}: {e}")
        return -1  # Indicate failure due to exception


def main(test_dir):
    system = platform.system()
    test_dir = Path(test_dir)

    if not test_dir.is_dir():
        print(f"Error: Directory '{test_dir}' does not exist.")
        return

    use_wine = False
    if system != "Windows":
        # Check if wine is available on non-Windows systems
        use_wine = shutil.which("wine") is not None

    failures = []
    for file_path in test_dir.glob("*_test*"):
        # ... your existing logic ...
        ret_code = None
        if system == "Windows":
            if file_path.suffix.lower() == ".exe":
                ret_code = run_file(file_path)
        else:
            if is_executable(file_path):
                ret_code = run_file(file_path)
            elif file_path.suffix.lower() == ".exe" and use_wine:
                ret_code = run_file(file_path, use_wine=True)

        if ret_code is not None and ret_code != 0:
            failures.append((file_path.name, ret_code))

    if failures:
        print("\nSummary of failed tests:")
        for test_name, code in failures:
            print(f"- {test_name} failed with exit code {code}")
        exit(1)  # Optionally exit with failure status
    else:
        print("\nAll tests passed successfully!")


if __name__ == "__main__":
    # Change this to your test directory
    test_directory = "./cmake-build-debug/bin"
    main(test_directory)
