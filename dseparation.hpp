#pragma once

#include "digraph.hpp"

// Returns true if a is d-separated from b given X
bool isDSeparated(const Digraph& dag, int a, Bitset X, int b) {
    if(dag.neighbors(a).contains(b)) {
        return false;
    }

    Bitset ancestorsX = Bitset::empty();
    Bitset ancestorQueueX = X;
    while(!ancestorQueueX.isEmpty()) {
        int v = ancestorQueueX.min();
        ancestorQueueX.del(v);
        ancestorsX.add(v);
        ancestorQueueX = ancestorQueueX.unionWith(dag.edgesIn(v)).minus(ancestorsX);
    }

    Bitset seenAdvs[Digraph::MaxVertCount];
    fill(seenAdvs, seenAdvs + dag.vertCount(), Bitset::empty());

    queue<pair<int, int>> advQueue;
    dag.neighbors(a).iterate([&](int v) {
        advQueue.push(make_pair(a, v));
        seenAdvs[a].add(v);
    });
    bool found = false;
    while(!found && !advQueue.empty()) {
        int x = advQueue.front().first;
        int y = advQueue.front().second;
        advQueue.pop();

        auto consider = [&](int z) {
            if(z == b) {
                found = true;
            }
            advQueue.push(make_pair(y, z));
            seenAdvs[y].add(z);
        };
        auto considerIn = [&]() {
            dag.edgesIn(y).without(a).without(x).minus(seenAdvs[y]).iterate(consider);
        };
        auto considerOut = [&]() {
            dag.edgesOut(y).without(a).without(x).minus(seenAdvs[y]).iterate(consider);
        };

        if(dag.edgesOut(x).contains(y)) {
            // x -> y -> z
            if(!X.contains(y)) {
                considerOut();
            }

            // x -> y <- z
            if(ancestorsX.contains(y)) {
                considerIn();
            }
        } else {
            if(!X.contains(y)) {
                // x <- y -> z
                considerOut();

                // x <- y <- z
                considerIn();
            }

        }
    }
    return !found;
}
