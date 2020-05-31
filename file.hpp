#pragma once

#include "data.hpp"
#include "digraph.hpp"

Digraph readDigraph(ifstream& fp, int vertCount) {
    Digraph digraph(vertCount);

    int edgeCount;
    fp >> edgeCount;
    CHECK(edgeCount >= 0);

    for(int edgeIdx = 0; edgeIdx < edgeCount; ++edgeIdx) {
        int a, b;
        fp >> a >> b;
        CHECK(a >= 0 && a < vertCount);
        CHECK(b >= 0 && b < vertCount);
        CHECK(a != b);
        CHECK(!digraph.hasEdge(a, b));
        digraph.addEdge(a, b);
    }

    return digraph;
}

// Returns pair (DAG, CPDAG)
pair<Digraph, Digraph> readBnRepositoryNet(string filename) {
    ifstream fp;
    fp.exceptions(fp.failbit | fp.badbit | fp.eofbit);
    fp.open(filename);

    int vertCount;
    fp >> vertCount;
    CHECK(
        vertCount >= 0 &&
        vertCount <= Graph::MaxVertCount &&
        vertCount <= Digraph::MaxVertCount
    );

    Digraph dag = readDigraph(fp, vertCount);
    Digraph cpdag = readDigraph(fp, vertCount);

    return {move(dag), move(cpdag)};
}

Data readData(istream& in) {
    int varCount, pointCount;
    in >> varCount >> pointCount;
    CHECK(in.good());
    CHECK(varCount > 0);
    CHECK(pointCount > 0);

    Data data;
    data.catCounts.resize(varCount);
    for(int v = 0; v < varCount; ++v) {
        in >> data.catCounts[v];
        CHECK(in.good());
        CHECK(data.catCounts[v] >= 2);
    }

    data.points.resize(pointCount);
    for(int i = 0; i < pointCount; ++i) {
        data.points[i].resize(varCount);
        for(int v = 0; v < varCount; ++v) {
            in >> data.points[i][v];
            CHECK(in.good());
            CHECK(data.points[i][v] >= 0 && data.points[i][v] < data.catCounts[v]);
        }
    }
    
    return data;
}
