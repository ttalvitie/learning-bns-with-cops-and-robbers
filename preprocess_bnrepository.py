#!/usr/bin/env python3

import os
import subprocess

for filename in os.listdir("bnrepository"):
    if not filename.endswith(".bif.gz"):
        continue
    name = filename[:-7]

    out = subprocess.check_output([
        "Rscript",
        "-e",
        "library(bnlearn); \
            bn <- read.bif(\"bnrepository/" + name + ".bif.gz\"); \
            nodes(bn); \
            arcs(bn); \
            arcs(cpdag(bn))"
    ], encoding="UTF-8", stderr=subprocess.DEVNULL)
    out = out.split("\n")
    out = [line.strip().split() for line in out if line.strip() != ""]

    nodes = []

    i = 0
    while True:
        line = out[i]
        i += 1
        if line == ["from", "to"]:
            break
        
        assert line[0].startswith("[") and line[0].endswith("]")
        nodes += line[1:]
    
    for node in nodes:
        assert len(node) >= 3 and node.startswith("\"") and node.endswith("\"")
    nodes = [node[1:-1] for node in nodes]

    dag = []
    while True:
        line = out[i]
        i += 1
        if line == ["from", "to"]:
            break

        assert line[0].startswith("[") and line[0].endswith("]")
        assert line[1].startswith("\"") and line[1].endswith("\"")
        assert line[2].startswith("\"") and line[2].endswith("\"")

        src = line[1][1:-1]
        dest = line[2][1:-1]
        assert src in nodes and dest in nodes

        dag.append((nodes.index(src), nodes.index(dest)))

    nodes_in_dag = set([node for edge in dag for node in edge])
    assert nodes_in_dag.issubset(set(range(0, len(nodes))))

    cpdag = []
    while i != len(out):
        line = out[i]
        i += 1

        assert line[0].startswith("[") and line[0].endswith("]")
        assert line[1].startswith("\"") and line[1].endswith("\"")
        assert line[2].startswith("\"") and line[2].endswith("\"")

        src = line[1][1:-1]
        dest = line[2][1:-1]
        assert src in nodes and dest in nodes

        cpdag.append((nodes.index(src), nodes.index(dest)))

    with open("bnrepository_nets/" + name + ".net", "w") as fp:
        fp.write("{}\n".format(len(nodes)))
        fp.write("{}\n".format(len(dag)))
        for (a, b) in dag:
            fp.write("{} {}\n".format(a, b))
        fp.write("{}\n".format(len(cpdag)))
        for (a, b) in cpdag:
            fp.write("{} {}\n".format(a, b))
