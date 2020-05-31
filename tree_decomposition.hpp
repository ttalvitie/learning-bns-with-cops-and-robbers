#pragma once

#include "graph.hpp"

struct TreeDecompositionNode {
    Bitset verts;

    // Missing child is marked with -1
    int child1;
    int child2;
};
typedef std::vector<TreeDecompositionNode> TreeDecomposition;

namespace tree_decomposition_check_ {

Bitset subtreeVertsUnion(const TreeDecomposition& treeDecomposition, int nodeIdx) {
    CHECK(nodeIdx >= 0 && nodeIdx < (int)treeDecomposition.size());
    const TreeDecompositionNode& node = treeDecomposition[nodeIdx];
    CHECK(!node.verts.isEmpty());
    Bitset ret = node.verts;
    if(node.child1 != -1) {
        CHECK(node.child1 > nodeIdx);
        ret = ret.unionWith(subtreeVertsUnion(treeDecomposition, node.child1));
    }
    if(node.child2 != -1) {
        CHECK(node.child2 > nodeIdx);
        ret = ret.unionWith(subtreeVertsUnion(treeDecomposition, node.child2));
    }
    return ret;
}

void checkRunningIntersection(
    const TreeDecomposition& treeDecomposition,
    Bitset& vertsSeen,
    Bitset parentVerts,
    int nodeIdx
) {
    CHECK(nodeIdx >= 0 && nodeIdx < (int)treeDecomposition.size());
    const TreeDecompositionNode& node = treeDecomposition[nodeIdx];
    Bitset verts = node.verts;
    CHECK(verts.intersectWith(vertsSeen.minus(parentVerts)).isEmpty());
    vertsSeen = vertsSeen.unionWith(verts);

    if(node.child1 != -1) {
        checkRunningIntersection(treeDecomposition, vertsSeen, verts, node.child1);
    }
    if(node.child2 != -1) {
        checkRunningIntersection(treeDecomposition, vertsSeen, verts, node.child2);
    }
}

}

void checkTreeDecompositions(
    const vector<TreeDecomposition>& treeDecompositions,
    const Graph& graph,
    int tw
) {
    using namespace tree_decomposition_check_;

    Bitset vertsSeen = Bitset::empty();
    for(const TreeDecomposition& treeDecomposition : treeDecompositions) {
        Bitset verts = subtreeVertsUnion(treeDecomposition, 0);
        CHECK(verts.intersectWith(vertsSeen).isEmpty());
        vertsSeen = vertsSeen.unionWith(verts);
    }
    CHECK(vertsSeen == Bitset::range(graph.vertCount()));

    vector<Bitset> adjacentVertsSupset(graph.vertCount(), Bitset::empty());
    for(const TreeDecomposition& treeDecomposition : treeDecompositions) {
        vector<bool> hasParent(treeDecomposition.size(), false);
        hasParent[0] = true;
        for(int nodeIdx = 0; nodeIdx < (int)treeDecomposition.size(); ++nodeIdx) {
            CHECK(hasParent[nodeIdx]);
            Bitset verts = treeDecomposition[nodeIdx].verts;
            CHECK(verts.count() <= tw + 1);
            verts.iterate([&](int v) {
                adjacentVertsSupset[v] = adjacentVertsSupset[v].unionWith(verts.without(v));
            });
            if(treeDecomposition[nodeIdx].child1 != -1) {
                hasParent[treeDecomposition[nodeIdx].child1] = true;
            }
            if(treeDecomposition[nodeIdx].child2 != -1) {
                hasParent[treeDecomposition[nodeIdx].child2] = true;
            }
        }
    }

    for(int v = 0; v < graph.vertCount(); ++v) {
        CHECK(graph.adjacentVerts(v).isSubsetOf(adjacentVertsSupset[v]));
    }

    for(const TreeDecomposition& treeDecomposition : treeDecompositions) {
        Bitset vertsSeen = Bitset::empty();
        Bitset parentVerts = Bitset::empty();
        checkRunningIntersection(treeDecomposition, vertsSeen, parentVerts, 0);
    }
}
