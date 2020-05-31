#include "bayesian_oracle.hpp"
#include "bayesian_solve.hpp"
#include "digraph.hpp"
#include "graph.hpp"
#include "tree_decomposition.hpp"
#include "treewidth_solver.hpp"

mt19937 rng(random_device{}());

Clock statsPrintClock;
Clock totalRunClock;
double totalAlgoRunTime = 0.0;
map<int, int> runsByVertCount;
map<int, int> runsByTW;

void updateStats(int vertCount, int tw, double runTime) {
    totalAlgoRunTime += runTime;
    ++runsByVertCount[vertCount];
    ++runsByTW[tw];

    if(statsPrintClock.elapsedTime() >= 600.0) {
        statsPrintClock = Clock();
        cerr << "STATS: Run time: " << (int)totalRunClock.elapsedTime() << " s\n";
        cerr << "STATS: Efficiency: " << (int)(100.0 * (totalAlgoRunTime / totalRunClock.elapsedTime())) << "%\n";
        cerr << "STATS:\n";
        cerr << "STATS: Runs by vertex count:\n";
        for(pair<int, int> p : runsByVertCount) {
            cerr << "STATS:   " << p.first << " -> " << p.second << "\n";
        }
        cerr << "STATS:\n";
        cerr << "STATS: Runs by treewidth:\n";
        for(pair<int, int> p : runsByTW) {
            cerr << "STATS:   " << p.first << " -> " << p.second << "\n";
        }
        cerr << "STATS: ---------------------------\n";
    }
}

Graph moralizeDAG(const Digraph& dag) {
    Graph graph(dag.vertCount());
    for(int v = 0; v < dag.vertCount(); ++v) {
        dag.edgesIn(v).iterate([&](int x) {
            graph.addEdge(x, v);
        });
    }
    for(int v = 0; v < dag.vertCount(); ++v) {
        Bitset parents = dag.edgesIn(v);
        parents.iterate([&](int x) {
            parents.without(x).minus(graph.adjacentVerts(x)).iterate([&](int y) {
                graph.addEdge(x, y);
            });
        });
    }
    return graph;
}

Graph constructSkeleton(const Digraph& digraph) {
    Graph graph(digraph.vertCount());
    for(int v = 0; v < digraph.vertCount(); ++v) {
        digraph.neighbors(v).iterate([&](int x) {
            graph.addEdge(v, x);
        });
    }
    return graph;
}

// Returns false if the time limit was exceeded
bool runTest(const Digraph& dag, double timeLimit, TreewidthSolver& twSolver) {
    ScopedFailureContextPrint scopedFailureContextPrint(
        [&](std::ostream& out) {
            out << "DAG:\n";
            out << dag.vertCount() << '\n';
            for(int v = 0; v < dag.vertCount(); ++v) {
                Bitset edgesOut = dag.edgesOut(v);
                out << edgesOut.count();
                edgesOut.iterate([&](int x) {
                    out << ' ' << x;
                });
                out << '\n';
            }
        }
    );

    Digraph cpdag;
    vector<TreeDecomposition> treeDecompositions;
    int tw;
    BayesianOracle oracle(dag, timeLimit);
    try {
        tie(cpdag, treeDecompositions, tw) = reconstructBayesianNetwork(oracle);
    } catch(BayesianOracle::TimeLimitExceeded) {
        return false;
    }
    double runTime = oracle.elapsedTime();

    CHECK(oracle.maxQueriedSeparatorSize() <= tw + 1);

    Graph skeleton = constructSkeleton(cpdag);
    Graph correctSkeleton = constructSkeleton(dag);
    CHECK(skeleton == correctSkeleton);

    for(int a = 0; a < dag.vertCount(); ++a) {
        skeleton.adjacentVerts(a).iterate([&](int b) {
            CHECK(!dag.hasEdge(a, b) || cpdag.hasEdge(a, b));
            skeleton.adjacentVerts(a)
                .minus(skeleton.adjacentVerts(b))
                .without(b)
                .iterate([&](int c) {
                    bool dagV = dag.hasDirectedEdge(b, a) && dag.hasDirectedEdge(c, a);
                    bool cpdagV = cpdag.hasDirectedEdge(b, a) && cpdag.hasDirectedEdge(c, a);
                    CHECK(dagV == cpdagV);
                });
        });
    }

    Graph moralGraph = moralizeDAG(dag);
    checkTreeDecompositions(treeDecompositions, moralGraph, tw);

    int correctTW = twSolver.solve(moralGraph);
    CHECK(tw == correctTW);

    updateStats(dag.vertCount(), tw, runTime);

    return true;
}

void runTests(int vertCount, double timeLimit, TreewidthSolver& twSolver) {
    Digraph dag(vertCount);

    std::vector<std::pair<int, int>> unusedEdges;
    for(int a = 0; a < vertCount; ++a) {
        for(int b = a + 1; b < vertCount; ++b) {
            unusedEdges.emplace_back(a, b);
        }
    }

    while(true) {
        if(!runTest(dag, timeLimit, twSolver)) {
            break;
        }

        if(unusedEdges.empty()) {
            break;
        }

        std::swap(
            unusedEdges[uniform_int_distribution<int>(0, unusedEdges.size() - 1)(rng)],
            unusedEdges.back()
        );
        dag.addEdge(unusedEdges.back().first, unusedEdges.back().second);
        unusedEdges.pop_back();
    }
}

int main(int argc, char* argv[]) {
    if(argc != 4) {
        cerr << "Usage: ./bayesian_test <min vert count> <max vert count> <time limit/run>\n";
        CHECK(false);
    }

    int minVertCount = parseString<int>(argv[1]);
    int maxVertCount = parseString<int>(argv[2]);
    double timeLimit = parseString<double>(argv[3]);

    CHECK(minVertCount >= 0);
    CHECK(minVertCount <= maxVertCount);
    CHECK(maxVertCount <= Graph::MaxVertCount);
    CHECK(isfinite(timeLimit) && timeLimit > 0.0);

    TreewidthSolver twSolver;

    std::priority_queue<
        std::pair<double, int>,
        std::vector<std::pair<double, int>>,
        greater<std::pair<double, int>>
    > vertCountQueue;
    for(int n = minVertCount; n <= maxVertCount; ++n) {
        vertCountQueue.emplace(0.0, n);
    }

    while(true) {
        double totalTime = vertCountQueue.top().first;
        int vertCount = vertCountQueue.top().second;
        vertCountQueue.pop();

        Clock clock;
        runTests(vertCount, timeLimit, twSolver);
        totalTime += clock.elapsedTime();

        vertCountQueue.emplace(totalTime, vertCount);
    }

    return 0;
}
