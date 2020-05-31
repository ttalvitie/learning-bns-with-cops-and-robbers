#pragma once

#include "bayesian_oracle.hpp"
#include "cpdag.hpp"
#include "graph.hpp"

Digraph pcAlgorithm(BayesianOracle& oracle) {
    int vertCount = oracle.vertCount();
    Graph skeleton = Graph::complete(vertCount);

    vector<pair<pair<int, int>, Bitset>> edgeSeparators;

    int i = 0;
    while(true) {
        for(int x = 0; x < vertCount; ++x) {
            skeleton.adjacentVerts(x).iterate([&](int y) {
                Bitset sup = skeleton.adjacentVerts(x).without(y);
                if(!sup.iterateSubsetsOfSizeWhile(i, [&](Bitset S) {
                    if(oracle.indTest(x, S, y)) {
                        edgeSeparators.emplace_back(make_pair(x, y), S);
                        return false;
                    } else {
                        return true;
                    }
                })) {
                    skeleton.delEdge(x, y);
                }
            });
        }

        ++i;

        int maxDeg = 0;
        for(int v = 0; v < vertCount; ++v) {
            maxDeg = max(maxDeg, skeleton.adjacentVerts(v).count());
        }
        if(maxDeg <= i) {
            break;
        }
    }

    return constructCPDAG(skeleton, edgeSeparators);
}
