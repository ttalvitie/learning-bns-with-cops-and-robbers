#include "bayesian_oracle.hpp"
#include "bayesian_solve.hpp"
#include "file.hpp"
#include "pc_algorithm.hpp"

template <typename F>
static void testAlgorithm(
    const Digraph& dag,
    const Digraph& cpdag,
    double timeLimit,
    F algo
) {
    BayesianOracle oracle(dag, timeLimit);
    Digraph learnedCPDAG;
    bool ok = true;
    try {
        learnedCPDAG = algo(oracle);
    } catch(BayesianOracle::TimeLimitExceeded) {
        cout << "  TIMEOUT\n";
        ok = false;
    }

    if(ok) {
        CHECK(learnedCPDAG == cpdag);

        cout << "  Queries by separator size:\n";
        const vector<uint64_t>& qc = oracle.queryCountBySeparatorSize();
        for(int i = 0; i < (int)qc.size(); ++i) {
            cout << "    " << i << ": " << qc[i] << '\n';
        }
    }
}

int main(int argc, char* argv[]) {
    if(argc != 3) {
        cerr << "Usage: ./bnrepository_test <filename> <time limit>\n";
        CHECK(false);
    }

    double timeLimit = parseString<double>(argv[2]);
    CHECK(isfinite(timeLimit) && timeLimit > 0.0);

    Digraph dag, cpdag;
    tie(dag, cpdag) = readBnRepositoryNet(argv[1]);

    cout << "Our algorithm:\n";
    testAlgorithm(dag, cpdag, timeLimit, [&](BayesianOracle& oracle) {
        return get<0>(reconstructBayesianNetwork(oracle));
    });

    cout << '\n';
    cout << "PC algorithm:\n";
    testAlgorithm(dag, cpdag, timeLimit, [&](BayesianOracle& oracle) {
        return pcAlgorithm(oracle);
    });

    return 0;
}
