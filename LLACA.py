from suffix_array import SuffixArray
from TrieGram import *
from utils import *

from collections import Counter
import numpy as np
import regex as re
import os
from tqdm import tqdm
import subprocess
import datetime
import time


def test(input_path, truth_path, log_folder, text_path=None, \
         test_name="UNKNOWN", timestamp=None, \
         method=0, delim=" "):
    if not os.path.exists(input_path):
        raise FileNotFoundError(f"{input_path} does not exist!")
    
    if not os.path.exists(truth_path):
        raise FileNotFoundError(f"{truth_path} does not exist!")
    
    if not timestamp:
        timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")

    log_path = f"{log_folder}/{test_name}-{timestamp}.log"

    start_time = time.time()
    
    if method == 0:
        output_path = input_path
    elif method == 1:
        if not os.path.exists(text_path):
            raise FileNotFoundError(f"{text_path} does not exist!")
        
        test_name += "-ac"
        output_path = f"{RESULT_PATH}/seg/ac/{test_name}-{timestamp}.utf8"

        if os.path.exists(output_path):
            return
        
        ensure_directory_exists(output_path)

        ac = Automaton(input_path)
        with open(text_path, "r", encoding="utf-8") as f:
            lines = f.readlines()
            f.close()
        with open(output_path, "w", encoding="utf-8") as f:
            for line in lines:
                f.write(cut(ac, line.strip(), delim=delim) + "\n")
            f.close()
        del ac
    elif method == 2:
        if not os.path.exists(text_path):
            raise FileNotFoundError(f"{text_path} does not exist!")

        test_name += "-uni"
        output_path = f"{RESULT_PATH}/seg/uni/{test_name}-{timestamp}.utf8"

        if os.path.exists(output_path):
            return
        
        ensure_directory_exists(output_path)

        try:
            import jieba
        except Exception as e:
            raise ImportError("Please install `jieba` for testing") from e

        jieba.initialize(dictionary=input_path)
        jieba.re_han_default = re.compile(
            "([\p{L}\p{N}\p{M}+#&\\._%-]+)", 
            re.U
        )
        with open(text_path, "r", encoding="utf-8") as f:
            lines = f.readlines()
            f.close()
        with open(output_path, "w", encoding="utf-8") as f:
            for line in lines:
                f.write(delim.join(jieba.cut(line.strip(), HMM=False)) + "\n")
            f.close()
    elif method == 3:
        if not os.path.exists(text_path):
            raise FileNotFoundError(f"{text_path} does not exist!")
        
        test_name += "-fmm"
        output_path = f"{RESULT_PATH}/seg/fmm/{test_name}-{timestamp}.utf8"

        if os.path.exists(output_path):
            return
        
        ensure_directory_exists(output_path)

        ac = Automaton(input_path)
        with open(text_path, "r", encoding="utf-8") as f:
            lines = f.readlines()
            f.close()
        with open(output_path, "w", encoding="utf-8") as f:
            for line in lines:
                f.write(max_cut(ac, line.strip(), delim) + "\n")
            f.close()
        del ac
    elif method == 4:
        if not os.path.exists(text_path):
            raise FileNotFoundError(f"{text_path} does not exist!")

        test_name += "-bmm"
        output_path = f"{RESULT_PATH}/seg/bmm/{test_name}-{timestamp}.utf8"

        if os.path.exists(output_path):
            return
        
        ensure_directory_exists(output_path)

        ac = Automaton(input_path)
        with open(text_path, "r", encoding="utf-8") as f:
            lines = f.readlines()
            f.close()
        with open(output_path, "w", encoding="utf-8") as f:
            for line in lines:
                f.write(max_cut(ac, line.strip(), delim, True) + "\n")
            f.close()
        del ac

    end_time = time.time()

    with open(log_path, "a", encoding="utf-8") as f:
        f.write(f"Time\t{end_time-start_time:.2f}s\n")
    
        
    total_lines = sum(1 for _ in open(truth_path))
    
    process = subprocess.Popen(["perl", SCORE_SCRIPT_PATH, truth_path, output_path], 
                               stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

    pbar = tqdm(total=total_lines, desc="Evaluating lines")

    result = []
    for stdout_line in iter(process.stdout.readline, ""):
        if stdout_line.startswith("--"):
            try:
                processed_line = int(stdout_line.strip().split("-")[-1])
                pbar.update(processed_line - pbar.n)
            except Exception as e:
                pass
        elif stdout_line.startswith("==="):
            result.append(stdout_line)
        elif stdout_line.startswith("###"): # Line of final result
            f1 = float(stdout_line.strip().split()[-1])
            print(f"Test name\t {test_name}")
            print(f"F1 score\t {f1*100:.1f}")
            print(f"Time\t {end_time - start_time:.2f}s\n")
            with open(log_path, "a", encoding="utf-8") as f:
                f.write(f"Test name\t {test_name}\n")
                f.write(f"F1 score\t {f1*100:.1f}\n")
                f.write(f"Time\t {end_time - start_time:.2f}s\n\n\n")
                f.close()

    process.stdout.close()
    process.wait()
    pbar.close()

    with open(output_path, "a", encoding="utf-8") as f:
        f.writelines(result)
        f.close()

def run(cut, text_path, top_ratio=0.99, iter_cnt=1, batch_size=None, truth_path=None, test_name="UNKOWN", train_path=None, timestamp=None):
    data = []

    with open(text_path, 'r', encoding='utf-8') as f:
        data.extend(f.readlines())
        f.close()

    test_size = len(data)
    
    if train_path:
        with open(train_path, 'r', encoding='utf-8') as f:
            data.extend(f.readlines())
            f.close()

    n = len(data)

    if not n:
        raise ValueError(f"{text_path} is empty!")

    if not batch_size:
        batch_size = int(np.sqrt(n))

    if not timestamp:
        timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")

    log_folder = f"{RESULT_PATH}/log/{test_name}-{top_ratio}-{timestamp}"
    if not os.path.exists(log_folder):
        os.makedirs(log_folder)

    sa = SuffixArray(text_path)

    new_dict = Counter()
    for iter_idx in range(1, iter_cnt + 1):
        output_path = f"{RESULT_PATH}/seg/LLM/{test_name}-{top_ratio}-{iter_idx}-{timestamp}.utf8"
        dict_path = f"{RESULT_PATH}/dict/{test_name}-{top_ratio}-{iter_idx}_dict-{timestamp}.utf8"
        log_path = f"{log_folder}/{test_name}-{top_ratio}-{iter_idx}-{timestamp}.log"

        ensure_directory_exists(output_path)
        ensure_directory_exists(dict_path)
        ensure_directory_exists(log_path)

        if os.path.exists(output_path):
            print(f"{output_path} already existed")
            word_counts = dict()

            if not os.path.exists(dict_path):
                build_dict(output_path, dict_path, sa=SuffixArray(text_path))

            with open(dict_path, "r", encoding="utf-8") as f:
                for line in f.readlines():
                    try:
                        word, count = line.strip().split()
                        count = int(count)
                        word_counts[word] = count
                    except Exception as e:
                        pass
                
            print(f"Loaded {len(word_counts)} words from {dict_path}")
            new_dict = Counter(word_counts)
        else:
            print(f"{n} lines in progress")
            permutation = np.random.permutation(n)
            shuffled_data = [data[permutation[i]] for i in range(n)]
            result = ["" for i in range(n)]
            batch_start = 0
            start_time = time.time()

            while batch_start < n:
                batch_end = min(n, batch_start + batch_size)
                batch = shuffled_data[batch_start:batch_end]
                words = cut(batch)

                for i in range(batch_start, batch_end):
                    result[permutation[i]] = " ".join(words[i - batch_start]) + "\n"

                words = [word for sentence in words for word in sentence if not mix_stop_punc(word)]
                word_counts = Counter(words)
                sorted_words = sorted(word_counts.items(), key=lambda x: sa.get_mutual_information(x[0]), reverse=True)
                top_words = sorted_words[:int(len(sorted_words) * top_ratio)]
                new_dict.update(Counter(dict(top_words)))
                batch_start = batch_end
                print(f"Processed {batch_start/n:.2f}")

            end_time = time.time()
            total_time = end_time - start_time

            with open(output_path, "w", encoding="utf-8") as f:
                f.writelines(result[:test_size])
                f.close()

            with open(log_path, "a", encoding="utf-8") as f:
                f.write(f"Time\t{total_time/3600:.2f}h\n")
                f.write(f"Word count\t{len(new_dict)}\n\n")
                f.close()

            # 假设不会在写入完成之后、测试完成之前中断
            if truth_path:
                test(input_path=output_path, 
                    truth_path=truth_path, 
                    text_path=text_path, 
                    log_folder=log_folder,
                    test_name=f"{test_name}-{top_ratio}-{iter_idx}", 
                    timestamp=timestamp,
                    method=0)
            with open(dict_path, "w", encoding="utf-8") as f:
                for word, count in new_dict.items():
                    f.write(f"{word} {count}\n") 
                f.close()

        if truth_path: # 
            for i in range(1, 5):
                test(input_path=dict_path, 
                    truth_path=truth_path, 
                    text_path=text_path, 
                    log_folder=log_folder,
                    test_name=f"{test_name}-{top_ratio}-{iter_idx}", 
                    timestamp=timestamp, 
                    method=i)

    del sa