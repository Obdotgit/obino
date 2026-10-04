import os
import platform
import subprocess
import sys

def set_persistent_env_var(name, value):
    current_os = platform.system()
    print(f"Setting persistent variable [{name}] to: {value}")
    if current_os == "Windows":
        try:
            subprocess.run(["setx", name, value], check=True, stdout=subprocess.DEVNULL)
            print("Successfully updated Windows user environment.")
            print("👉 NOTE: Restart your terminal or IDE for the changes to take effect.")
        except subprocess.CalledProcessError as e:
            print(f"Error setting variable on Windows: {e}", file=sys.stderr)
    elif current_os in ["Linux", "Darwin"]:
        shell = os.environ.get("SHELL", "")
        if "zsh" in shell:
            rc_file = os.path.expanduser("~/.zshrc")
        else:
            rc_file = os.path.expanduser("~/.bashrc")
        export_line = f'export {name}="{value}"\n'
        already_exists = False
        if os.path.exists(rc_file):
            with open(rc_file, "r") as f:
                if export_line in f.readlines():
                    already_exists = True
        if not already_exists:
            with open(rc_file, "a") as f:
                f.write(f"\n{export_line}")
            print(f"Successfully appended variable to {rc_file}")
            print(f"👉 NOTE: Run 'source {rc_file}' or open a new terminal window to apply changes.")
        else:
            print(f"Variable is already present in {rc_file} (skipped duplicate entry).")
    else:
        print(f"Unsupported operating system: {current_os}", file=sys.stderr)

    os.environ[name] = value

def run_command(command):
    print(f"Running: {' '.join(command)}")
    subprocess.run(command, check=True)

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    target_path = os.path.join(script_dir, "src", "std")
    set_persistent_env_var("OBN_STD_PATH", target_path)

    current_os = platform.system()
    if current_os == "Windows":
        run_command(["cmake.exe", "-S", ".", "-B", "build", "-A", "x64"])
        run_command(["cmake.exe", "--build", "build", "--config", "Release"])
    elif current_os in ["Linux", "Darwin"]:
        run_command(["cmake", "-S", ".", "-B", "build", "-DCMAKE_BUILD_TYPE=Release"])
        run_command(["cmake", "--build", "build"])
    else:
        print(f"Unsupported operating system: {current_os}", file=sys.stderr)
        raise SystemExit(1)


if __name__ == "__main__":
    main()