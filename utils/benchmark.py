import LLACA
from utils import SCORE_SCRIPT_PATH, TEMP_PATH, DATA_PATH

import os
import time
import subprocess
from tqdm import tqdm


def evaluate(gold_path, output_path) -> float:
    """Call Perl script to evaluate F1 score"""
    result = []
    f1_score = 0.0

    with open(gold_path, "r", encoding="utf-8") as f:
        total_lines = sum(1 for _ in f)

    process = subprocess.Popen(
        ["perl", SCORE_SCRIPT_PATH, gold_path, output_path],
        stdout=subprocess.PIPE, 
        stderr=subprocess.PIPE, 
        text=True,
        encoding="utf-8",
        errors="replace"
    )

    pbar = tqdm(total=total_lines)

    for stdout_line in iter(process.stdout.readline, ""):
        if stdout_line.startswith("--"):
            try:
                processed_line = int(stdout_line.strip().split("-")[-1])
                pbar.update(processed_line - pbar.n)
            except:
                continue
        elif stdout_line.startswith("==="):
            result.append(stdout_line)
        elif stdout_line.startswith("###"):
            f1_score = float(stdout_line.strip().split()[-1])
            print(f"F1 score\t {f1_score * 100:.1f}")

    process.stdout.close()
    process.wait()
    pbar.close()
    return f1_score

def benchmark(dataset_names):
    summary = []

    os.makedirs(TEMP_PATH, exist_ok=True)
    output_path = TEMP_PATH + "/output.txt"

    for name in dataset_names:
        dict_path = f"{DATA_PATH}/dict/{name}_dict.utf8"
        test_path = f"{DATA_PATH}/test/{name}_test.utf8"
        gold_path = f"{DATA_PATH}/gold/{name}_test_gold.utf8"

        print(f"\n--- Benchmarking dataset: {name} ---")

        t0 = time.time()
        ac = LLACA.Automaton(dict_path)
        t1 = time.time()
        build_time = t1 - t0

        t0 = time.time()
        LLACA.cutf(ac, test_path, output_path)
        t1 = time.time()
        cut_time = t1 - t0

        f1 = evaluate(gold_path, output_path)

        os.remove(output_path)

        del ac

        summary.append({
            "dataset": name,
            "F1": f1 * 100,
            "build_time": build_time,
            "cut_time": cut_time
        })

    print("\n====================== Benchmark Summary ======================")
    print(f"{'Dataset':<15} {'F1 Score (%)':<15} {'Build Time (s)':<15} {'Cut Time (s)':<15}")
    for item in summary:
        print(f"{item['dataset']:<15} {item['F1']:<15.1f} {item['build_time']:<15.3f} {item['cut_time']:<15.3f}")

if __name__ == "__main__":
    datasets = [
        # Traditional Chinese
        "as",
        "cityu",
        # Simplified Chinese
        "msr",
        "pku",
        # Japanese
        "kwdlc",
        "ud_ja",
        # Thai
        "best"
    ]

    benchmark(datasets)

    if os.path.exists(TEMP_PATH):
        os.removedirs(TEMP_PATH)
    
