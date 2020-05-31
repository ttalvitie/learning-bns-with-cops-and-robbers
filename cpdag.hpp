#pragma once

#include "digraph.hpp"
#include "graph.hpp"

inline Digraph constructCPDAG(
    const Graph& skeleton,
    const vector<pair<pair<int, int>, Bitset>>& edgeSeparators
) {
    Digraph cpdag(skeleton.vertCount());
    for(int b = 0; b < cpdag.vertCount(); ++b) {
        skeleton.adjacentVerts(b).intersectWith(Bitset::range(b)).iterate([&](int a) {
            cpdag.addEdge(a, b);
            cpdag.addEdge(b, a);
        });
    }

    for(const auto& edgeSeparator : edgeSeparators) {
        int a = edgeSeparator.first.first;
        int b = edgeSeparator.first.second;
        Bitset X = edgeSeparator.second;
        skeleton.adjacentVerts(a)
            .intersectWith(skeleton.adjacentVerts(b))
            .minus(X)
            .iterate([&](int v) {
                cpdag.delEdge(v, a);
                cpdag.delEdge(v, b);
            });
    }

    while(true) {
        bool progress = false;

        // Meek rule 1
        for(int a = 0; a < cpdag.vertCount(); ++a) {
            cpdag.edgesOnlyOut(a).iterate([&](int b) {
                cpdag.bidirNeighbors(b).minus(cpdag.neighbors(a)).without(a)
                    .iterate([&](int c) {
                        cpdag.delEdge(c, b);
                        progress = true;
                    });
            });
        }

        // Meek rule 2
        for(int a = 0; a < cpdag.vertCount(); ++a) {
            cpdag.edgesOnlyOut(a).iterate([&](int b) {
                cpdag.edgesOnlyOut(b).intersectWith(cpdag.bidirNeighbors(a))
                    .iterate([&](int c) {
                        cpdag.delEdge(c, a);
                        progress = true;
                    });
            });
        }

        // Meek rule 3
        for(int a = 0; a < cpdag.vertCount(); ++a) {
            cpdag.bidirNeighbors(a).iterate([&](int b) {
                cpdag.bidirNeighbors(a).minus(cpdag.neighbors(b)).without(b)
                    .iterate([&](int c) {
                        cpdag.bidirNeighbors(a)
                            .intersectWith(cpdag.edgesOnlyOut(b))
                            .intersectWith(cpdag.edgesOnlyOut(c))
                            .iterate([&](int d) {
                                cpdag.delEdge(d, a);
                                progress = true;
                            });
                    });
            });
        }

        // Meek rule 4
        for(int a = 0; a < cpdag.vertCount(); ++a) {
            cpdag.bidirNeighbors(a).iterate([&](int b) {
                cpdag.bidirNeighbors(a).minus(cpdag.neighbors(b)).without(b)
                    .iterate([&](int c) {
                        cpdag.neighbors(a)
                            .intersectWith(cpdag.edgesOnlyIn(b))
                            .intersectWith(cpdag.edgesOnlyOut(c))
                            .iterate([&](int d) {
                                cpdag.delEdge(b, a);
                                progress = true;
                            });
                    });
            });
        }

        if(!progress) {
            break;
        }
    }

    return cpdag;
}
