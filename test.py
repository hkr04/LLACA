import LLACA
from utils import DATA_PATH

import argparse
import functools
from typing import Union, List

def parse_datasets(value: str) -> Union[str, List[str]]:
    if value == "all":
        return value
    elif ',' in value:
        return value.split(',')
    return value

parser = argparse.ArgumentParser()

# LLM config
parser.add_argument(
    "--model",
    type=str,
    required=True,
    help="LLM name"
)
parser.add_argument(
    "--api-key",
    type=str,
    default="sk-"
)
parser.add_argument(
    "--base-url",
    type=str,
    default="http://localhost:8080",
    help="Base URL for LLM calling"
)
parser.add_argument("--temperature", type=float, default=0.)
parser.add_argument("--top-p", type=float, default=0.)
parser.add_argument("--extra-body", type=dict, default=None)

# LLACA config
parser.add_argument(
    "--top-ratio",
    type=float,
    default=0.99
)
parser.add_argument(
    "--iter-cnt",
    type=int,
    default=1,
    help="Iteration count of sampling from LLM to segment texts"
)
parser.add_argument(
    "--datasets",
    type=parse_datasets,
    default=["pku", "kwdlc", "best"],
    help=f"Dataset names for testing. Available datasets in {DATA_PATH}/test/. Use `all` to test all."
)

args = parser.parse_args()

# Examples (same as data/test/demo_test.utf8)
zh_example = "武汉市长江大桥，是发展中国家中国的一座大桥。而武汉市长并不是江大桥。"
# 武汉市/长江大桥/，/是/发展中国家/中国/的/一座/大桥/。/而/武汉/市长/并/不是/江/大桥/。
# Meaning: The Yangtze River Bridge in Wuhan city, is a bridge of developing country China. But the mayor of Wuhan is not Jiang Daqiao.

ja_example = "すもももももももものうち"
# すもも/も/もも/も/もも/の/うち
# Plums/also/peaches/also/peaches/of/inside
# Meaning: Plums and peaches are both types of peaches.

th_example = "เขากำลังไปซื้อของที่ตลาด" 
# เขา/กำลัง/ไป/ซื้อ/ของ/ที่/ตลาด
# Meaning: (He/She)/is/going/to buy/goods/at/market.

examples = [zh_example, ja_example, th_example]

print("\nWarming up...")

LLACA.set_api_key(args.api_key)
LLACA.set_base_url(args.base_url)

results = LLACA.llm_cut(examples,
                        model=args.model,
                        temperature=args.temperature,
                        top_p=args.top_p,
                        extra_body=args.extra_body)

print(f"\nResults from {args.model}:")

for result in results:
    print("/".join(result))

print("\nReady for testing!")

cut = functools.partial(LLACA.llm_cut,
                        model=args.model,
                        temperature=args.temperature,
                        top_p=args.top_p,
                        extra_body=args.extra_body)

datasets = args.datasets

if datasets == "all":
    datasets = ["cityu", "msr", "pku", "kwdlc", "ud_ja", "ud_ko", "best", "ud_th", "as"]

if isinstance(datasets, str):
    datasets = [datasets]

print(f"Testing on {datasets}...")

# You could pass previous `timestamp` to resume

for dataset in datasets:
    LLACA.run(
        cut=cut, 
        text_path=f"{DATA_PATH}/test/{dataset}_test.utf8", 
        truth_path=f"{DATA_PATH}/gold/{dataset}_test_gold.utf8", 
        test_name=f"{dataset}_{args.model}", 
        top_ratio=args.top_ratio, 
        iter_cnt=args.iter_cnt,
        # timestamp=""
    )