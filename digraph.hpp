#pragma once

#include "bitset.hpp"

class Digraph {
public:
    static constexpr int MaxVertCount = Bitset::BitCount;
    static_assert(MaxVertCount <= Bitset::BitCount);

    Digraph() : Digraph(0) {}
    Digraph(int vertCount) : vertCount_(vertCount) {
        CHECK(vertCount >= 0 && vertCount <= MaxVertCount);
        fill(edgesIn_, edgesIn_ + vertCount_, Bitset::empty());
        fill(edgesOut_, edgesOut_ + vertCount_, Bitset::empty());
    }

    int vertCount() const {
        return vertCount_;
    }
    Bitset edgesIn(int v) const {
        CHECK(v >= 0 && v < vertCount_);
        return edgesIn_[v];
    }
    Bitset edgesOut(int v) const {
        CHECK(v >= 0 && v < vertCount_);
        return edgesOut_[v];
    }
    Bitset edgesOnlyIn(int v) const {
        CHECK(v >= 0 && v < vertCount_);
        return edgesIn_[v].minus(edgesOut_[v]);
    }
    Bitset edgesOnlyOut(int v) const {
        CHECK(v >= 0 && v < vertCount_);
        return edgesOut_[v].minus(edgesIn_[v]);
    }
    Bitset neighbors(int v) const {
        CHECK(v >= 0 && v < vertCount_);
        return edgesIn_[v].unionWith(edgesOut_[v]);
    }
    Bitset bidirNeighbors(int v) const {
        CHECK(v >= 0 && v < vertCount_);
        return edgesIn_[v].intersectWith(edgesOut_[v]);
    }

    void addEdge(int a, int b) {
        CHECK(a >= 0 && a < vertCount_);
        CHECK(b >= 0 && b < vertCount_);
        CHECK(a != b);
        edgesOut_[a].add(b);
        edgesIn_[b].add(a);
    }
    void delEdge(int a, int b) {
        CHECK(a >= 0 && a < vertCount_);
        CHECK(b >= 0 && b < vertCount_);
        CHECK(a != b);
        edgesOut_[a].del(b);
        edgesIn_[b].del(a);
    }

    bool hasEdge(int a, int b) const {
        CHECK(a >= 0 && a < vertCount_);
        CHECK(b >= 0 && b < vertCount_);
        CHECK(a != b);
        return edgesOut_[a].contains(b);
    }
    bool hasDirectedEdge(int a, int b) const {
        CHECK(a >= 0 && a < vertCount_);
        CHECK(b >= 0 && b < vertCount_);
        CHECK(a != b);
        return edgesOut_[a].contains(b) && !edgesIn_[a].contains(b);
    }

    bool operator==(const Digraph& other) const {
        if(vertCount_ != other.vertCount_) {
            return false;
        }
        for(int v = 0; v < vertCount_; ++v) {
            if(edgesOut_[v] != other.edgesOut_[v]) {
                return false;
            }
        }
        return true;
    }
    bool operator!=(const Digraph& other) const {
        return !(*this == other);
    }

private:
    int vertCount_;
    Bitset edgesIn_[MaxVertCount];
    Bitset edgesOut_[MaxVertCount];
};
