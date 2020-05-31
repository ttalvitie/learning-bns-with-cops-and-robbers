#include "bayesian_oracle.hpp"
#include "bayesian_solve.hpp"
#include "file.hpp"
#include "pc_algorithm.hpp"

template <typename F>
void testAlgorithm(
    const Digraph& cpdag,
    const Data& data,
    double timeLimit,
    F algo
) {
    BayesianOracle oracle(data, timeLimit);
    Digraph learnedCPDAG;
    bool ok = true;
    try {
        learnedCPDAG = algo(oracle);
    } catch(BayesianOracle::TimeLimitExceeded) {
        cout << "  TIMEOUT\n";
        ok = false;
    }

    if(ok) {
        int shd = 0;
        for(int a = 0; a < oracle.vertCount(); ++a) {
            for(int b = a + 1; b < oracle.vertCount(); ++b) {
                if(
                    cpdag.hasEdge(a, b) != learnedCPDAG.hasEdge(a, b) ||
                    cpdag.hasEdge(b, a) != learnedCPDAG.hasEdge(b, a)
                ) {
                    ++shd;
                }
            }
        }
        cout << "  Structural Hamming distance: " << shd << '\n';

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

    Digraph cpdag = readBnRepositoryNet(argv[1]).second;

    Data data = readData(cin);
    CHECK((int)data.catCounts.size() == cpdag.vertCount());

    cout << "Our algorithm:\n";
    testAlgorithm(cpdag, data, timeLimit, [&](BayesianOracle& oracle) {
        return get<0>(reconstructBayesianNetwork(oracle));
    });

    cout << '\n';
    cout << "PC algorithm:\n";
    testAlgorithm(cpdag, data, timeLimit, [&](BayesianOracle& oracle) {
        return pcAlgorithm(oracle);
    });

    return 0;
}
