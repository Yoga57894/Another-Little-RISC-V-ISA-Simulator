import os
import sys

def run_aliss(folder_name, pattern_list_file):
    with open(pattern_list_file, 'r') as file:
        patterns = [line.strip() for line in file.readlines()]

    if not os.path.exists(folder_name):
        print("No file" + folder_name + "found")
        return

    for pattern in patterns:
        aliss_command = f"./ALISS -e {folder_name}/{pattern}"
        run_code = os.system(aliss_command)
        if run_code != 0 : #check return value
            aliss_command = f"./ALISS -e {folder_name}/{pattern} -d > {folder_name}_debug/{pattern}.log" #-d is new option to dump log
            run_code = os.system(aliss_command)


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("python3 run_aliss.py folder_name pattern_list")
        sys.exit(1)

    folder_name = sys.argv[1]
    pattern_list_file = sys.argv[2]

    run_aliss(folder_name, pattern_list_file)
