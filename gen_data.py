#!/usr/bin/env python3

import sys
import subprocess

filename = sys.argv[1]
count = int(sys.argv[2])
out = subprocess.check_output([
    "Rscript",
    "-e",
    "library(bnlearn); \
        bn <- read.bif(\"" + filename + "\"); \
        str(rbn(bn, " + str(count) + "), vec.len=1000000000, list.len=1000000000, nchar.max=1000000000)"
], encoding="UTF-8", stderr=subprocess.DEVNULL)
out = out.split("\n")
out = [line.strip() for line in out if line.strip() != ""]

var_count = len(out[1:])

header = out[0].split()
assert header == [
    "'data.frame':",
    str(count),
    "obs.",
    "of",
    str(var_count),
    "variables:",
]

cat_counts = []
data = []

for line in out[1:]:
    i = line.index("$")
    i = line.index(":", i + 1)
    assert line[i : i + 12] == ": Factor w/ "
    i += 12

    line = line[i:].split()
    assert len(line) == 3 + count
    assert line[1] == "levels" and line[2].endswith(":")

    cat_count = int(line[0])
    assert cat_count >= 2
    cat_counts.append(cat_count)

    data.append([])
    for i in range(count):
        val = int(line[3 + i]) - 1
        assert val >= 0 and val < cat_count
        data[-1].append(val)

assert len(cat_counts) == var_count
assert len(data) == var_count

print("{} {}".format(var_count, count))
print(" ".join(str(cat_counts[v]) for v in range(var_count)))
for i in range(count):
    print(" ".join(str(data[v][i]) for v in range(var_count)))
