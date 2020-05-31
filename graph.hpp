#pragma once

#include "bitset.hpp"

class Graph {
public:
    static constexpr int MaxVertCount = Bitset::BitCount;
    static_assert(MaxVertCount <= Bitset::BitCount);

    Graph() : Graph(0) {}
    Graph(int vertCount) : vertCount_(vertCount) {
        CHECK(vertCount >= 0 && vertCount <= MaxVertCount);
        fill(adjacentVerts_, adjacentVerts_ + vertCount_, Bitset::empty());
    }
    static Graph complete(int vertCount) {
        CHECK(vertCount >= 0 && vertCount <= MaxVertCount);
        Bitset allVerts = Bitset::range(vertCount);
        Graph ret = Graph(Uninitialized());
        ret.vertCount_ = vertCount;
        for(int v = 0; v < vertCount; ++v) {
            ret.adjacentVerts_[v] = allVerts.without(v);
        }
        return ret;
    }

    int vertCount() const {
        return vertCount_;
    }
    Bitset adjacentVerts(int v) const {
        CHECK(v >= 0 && v < vertCount_);
        return adjacentVerts_[v];
    }

    void addEdge(int a, int b) {
        CHECK(a >= 0 && a < vertCount_);
        CHECK(b >= 0 && b < vertCount_);
        CHECK(a != b);
        adjacentVerts_[a].add(b);
        adjacentVerts_[b].add(a);
    }
    void delEdge(int a, int b) {
        CHECK(a >= 0 && a < vertCount_);
        CHECK(b >= 0 && b < vertCount_);
        CHECK(a != b);
        adjacentVerts_[a].del(b);
        adjacentVerts_[b].del(a);
    }

    bool hasEdge(int a, int b) const {
        CHECK(a >= 0 && a < vertCount_);
        CHECK(b >= 0 && b < vertCount_);
        CHECK(a != b);
        return adjacentVerts_[a].contains(b);
    }

    bool operator==(const Graph& other) const {
        if(vertCount_ != other.vertCount_) {
            return false;
        }
        for(int v = 0; v < vertCount_; ++v) {
            if(adjacentVerts_[v] != other.adjacentVerts_[v]) {
                return false;
            }
        }
        return true;
    }
    bool operator!=(const Graph& other) const {
        return !(*this == other);
    }

private:
    struct Uninitialized {};
    Graph(Uninitialized) {}

    int vertCount_;
    Bitset adjacentVerts_[MaxVertCount];
};
