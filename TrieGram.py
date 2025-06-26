from automaton import Automaton

import regex as re
import numpy as np
import os
from openai import OpenAI
from tqdm import tqdm

# Find all positions of integers and decimals in the text
def find_number(text):
    pattern = r"[-+]?\d+((\.|\．|\:|\：|\-|\－|\/)\d+)*"

    matches = re.finditer(pattern, text)
    
    positions = [(match.start(), match.end()) for match in matches]

    return positions

# Find all positions of English words in the text
def find_alpha(text):
    pattern = r"[a-zA-Z]+(['-][a-zA-Z]+)*"
    
    matches = re.finditer(pattern, text)
    
    positions = [(match.start(), match.end()) for match in matches]

    return positions

# Check if the character is a symbol
def is_symbol(char):
    return re.match(r'\p{P}|\p{S}', char) is not None

# Get the probability DAG
def get_DAG(ac: Automaton, text: str, unigram=False, patterns=None):
    # initiate
    n = len(text)
    min_prob = -ac.get_node(0)["log_prefix_sum"]
    edges = [[(i - 1, min_prob if not is_symbol(text[i]) else min_prob / 2)] for i in range(n)]

    # number
    number_pos = find_number(text)
    for p in number_pos:
        edges[p[1] - 1].append((p[0] - 1, min_prob / 4))

    # alpha
    alpha_pos = find_alpha(text)
    for p in alpha_pos:
        edges[p[1] - 1].append((p[0] - 1, min_prob / 2))

    # other patterns
    # TODO: add configurable patterns
    if patterns:
        pass

    # dict
    ac.reset()
    pre = -1
    for cur in range(n):
        node_id = ac.trans_string(text[cur])["id"]
        borders = ac.get_borders(node_id)
        for border in borders:
            if border["end"] == 0:
                continue
            pre_node = ac.get_node(border["pre"])
            len_border = border["length"]
            if len_border == 0:
                break
            log_cnt_pre = pre_node["log_prefix_sum"] if not unigram else -min_prob
            log_cnt_border = border["log_end"]
            prob = log_cnt_border - log_cnt_pre
            edges[cur].append((cur - len_border, prob))

    return edges
    
def cut(ac: Automaton, text: str, delim=None, unigram=False, return_prob=False):
    edges = get_DAG(ac, text, unigram=unigram)
    min_prob = -ac.get_node(0)["log_prefix_sum"]
    n = len(text)
    paths = {}

    # viterbi
    paths[-1] = (-1, 0)
    for cur in range(n):
        max_prob = float('-inf')
        for edge in edges[cur]:
            tpre, prob = edge
            if paths[tpre][1] + prob > max_prob:
                max_prob = paths[tpre][1] + prob
                pre = tpre
        paths[cur] = (pre, max_prob)
    cuts = []
    cur = n - 1
    while cur != -1:
        pre = paths[cur][0]
        cuts.append(text[pre + 1: cur + 1])
        cur = pre
    cuts.reverse()

    if delim:
        cuts = delim.join(cuts)

    return (cuts, max_prob) if return_prob else cuts

def cut_cpp(ac: Automaton, text: str, delim=None):
    words = ac.cut(text)
    
    if delim:
        words = delim.join(words)

    return words

def cutf(ac: Automaton, input_path: str, output_path: str, delim=" ", unigram=False):
    with open(input_path, "r", encoding="utf-8") as f:
        lines = f.readlines()
        f.close()

    res = ""

    for line in lines:
        res += cut(ac, line, delim=delim, unigram=unigram)
        # res += cut_cpp(ac, line, delim=delim)

    dir_path = os.path.dirname(output_path)

    if not os.path.exists(dir_path):
        os.makedirs(dir_path)

    with open(output_path, "w", encoding="utf-8") as f:
        f.write(res)

def max_cut(ac: Automaton, text: str, delim=None, reverse=False):
    ac.reset()
    n = len(text)

    if reverse:
        pre = [i - 1 for i in range(n)]
        for cur in range(n):
            node_id = ac.trans_string(text[cur])["id"]
            borders = ac.get_borders(node_id)
            for border in borders:
                len_border = border["length"]
                if len_border == 0:
                    break
                pre[cur] = min(pre[cur], cur - len_border)
        cuts = []
        cur = n - 1
        while cur != -1:
            cuts.append(text[pre[cur] + 1: cur + 1])
            cur = pre[cur]
        cuts.reverse()
    else:
        nxt = [i + 1 for i in range(n)]
        for cur in range(n):
            node_id = ac.trans_string(text[cur])["id"]
            borders = ac.get_borders(node_id)
            for border in borders:
                len_border = border["length"]
                if len_border == 0:
                    break
                nxt[cur - len_border + 1] = max(nxt[cur - len_border + 1], cur + 1)
        cuts = []
        cur = 0
        while cur != n:
            cuts.append(text[cur: nxt[cur]])
            cur = nxt[cur]

    if delim:
        cuts = delim.join(cuts)

    return cuts

DEFAULT_SYSTEM_PROMPT_EN = "Segment the text into words with spaces. Every punctuation should be a single part. DO NOT output other content!"
DEFAULT_SYSTEM_PROMPT_ZH = "请对下列文本进行分词，词之间以空格隔开，除了分词结果之外不需要多余回答"

client = OpenAI(
    api_key="sk"
)

def set_api_key(api_key: str):
    client.api_key = api_key

def set_base_url(base_url: str):
    client.base_url = base_url

def llm_cut(
    texts, 
    model, 
    system_prompt=DEFAULT_SYSTEM_PROMPT_ZH,
    temperature=0.,
    top_p=0.,
    extra_body="None"
):
    if isinstance(texts, str):
        texts = [texts]
    
    cut_list = []

    for text in tqdm(texts, unit='text'):
        # TODO: add retry to handle errors
        try:
            completion = client.chat.completions.create(
                model=model,
                messages=[
                    {"role": "system", "content": system_prompt},
                    {"role": "user", "content": text},
                ],
                temperature=temperature,
                top_p=top_p,
                extra_body=extra_body
            )
            response = completion.choices[0].message.content
        except Exception as e:
            response = str(e)

        cut_list.append(response.split())

    return cut_list

def get_ppl(ac: Automaton, file_path: str):
    with open(file_path, "r", encoding="utf-8") as f:
        lines = f.readlines()
        f.close()

    ppl_ac_uni = 0
    ppl_ac = 0
    for line in lines:
        _, logp_ac_uni = cut(ac, line.strip(), unigram=True, return_prob=True)
        _, logp_ac = cut(ac, line.strip(), return_prob=True)
        ppl_ac_uni += logp_ac_uni / len(line)
        ppl_ac += logp_ac / len(line)
        
    ppl_ac_uni = np.exp2(-ppl_ac_uni / len(lines))
    ppl_ac = np.exp2(-ppl_ac / len(lines))

    return {
        "ac-unigram": ppl_ac_uni,
        "ac-gram": ppl_ac
    }

