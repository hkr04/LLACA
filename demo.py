import LLACA
from utils import benchmark

import argparse
import functools

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

ac = LLACA.Automaton()

for result in results:
    for word in result:
        ac.insert(word)

ac.build()

print(f"\nResults from LLACA:")

for example in examples:
    print(LLACA.cut(ac, example, delim="/"))