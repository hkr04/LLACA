from collections import Counter
import numpy as np

def mix_stop_punc(text):
    stop_punctuation = {
        '，', '。', '！', '？', '、',
        '；', '：', '“', '”', '（', '）', '…', '《', '》',
        '「', '」', '『', '』', '【', '】',
        ',', '.', ';', ':', '!', '?', '"', 
        '(', ')', '<', '>', '{', '}', '[', ']'
    }
    
    stop = None
    all_stop = True

    for char in text:
        if char in stop_punctuation:
            if not stop:
                stop = char
            elif stop != char or stop != '…':
                return True
        else:
            all_stop = False
    return stop and not all_stop

def build_dict(
    input_path, 
    output_path, 
    filter_punc=True, 
    show_count=True, 
    sa=None, 
    top_ratio=0.99,
    sample_count=1,
    batch_size=None
):
    if isinstance(input_path, str):
        with open(input_path, 'r', encoding='utf-8') as f:
            text = f.read()
    if isinstance(input_path, list):
        text = ""
        for filepath in input_path:
            with open(filepath, 'r', encoding='utf-8') as f:
                text = text + "\n" + f.read()
    
    # Split to words (exclude those mixed with stop punctuations)
    if sa is None:
        words = text.split()
    else:
        words = []
        result = text.split('\n')
        n = len(result)
        for _ in range(sample_count):
            permutation = np.random.permutation(n)
            shuffled_result = [result[permutation[i]] for i in range(n)]
            batch_start = 0
            if batch_size is None:
                batch_size = int(np.sqrt(len(result)))
            while batch_start < n:
                batch_end = min(n, batch_start + batch_size)
                batch_result = shuffled_result[batch_start:batch_end]
                batch_words = [_result.split(' ') for _result in batch_result]
                batch_words = [word for sentence in batch_words for word in sentence if not mix_stop_punc(word)]
                batch_word_counts = Counter(batch_words)
                sorted_words = sorted(batch_word_counts.items(), key=lambda x: sa.get_mutual_information(x[0]), reverse=True)
                top_words = sorted_words[:int(len(sorted_words) * top_ratio)]
                for top_word, top_count in top_words:
                    words.extend([top_word for _ in range(top_count)])
                batch_start = batch_end
        
    if filter_punc:
        words = [word for word in words if not mix_stop_punc(word)]
    
    word_counts = Counter(words).items()

    # Filter with PMI (Point-wise Mutual Information)
    if sa != None:
        word_counts = sorted(word_counts, key=lambda x: sa.get_mutual_information(x[0]), reverse=True)[:int(len(word_counts) * top_ratio)]
    
    # Write words to new file
    with open(output_path, 'w', encoding='utf-8') as f:
        # Each line with a word and its count (e.g. hello 100)
        if show_count:
            for word, cnt in word_counts:
                f.write(f"{word} {cnt}\n")
        # Only one world each line (e.g. hello)
        else:
            for word, _ in word_counts:
                f.write(f"{word}\n")