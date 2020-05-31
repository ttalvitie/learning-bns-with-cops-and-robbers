#pragma once

#include "data.hpp"
#include "dseparation.hpp"
#include "pearson_chisq.hpp"

class BayesianOracle {
public:
    struct TimeLimitExceeded {};

    // Does not check that dag is indeed a DAG
    BayesianOracle(const Digraph& dag, double timeLimit)
        : graphical_(true),
          vertCount_(dag.vertCount()),
          dag_(dag),
          data_(*(const Data*)nullptr),
          timeLimit_(timeLimit),
          indTestCountSinceLastTimeCheck_(0),
          queriesBySeparatorSize_(1)
    {}

    BayesianOracle(const Data& data, double timeLimit)
        : graphical_(false),
          vertCount_(data.catCounts.size()),
          dag_(*(const Digraph*)nullptr),
          data_(data),
          timeLimit_(timeLimit),
          indTestCountSinceLastTimeCheck_(0),
          queriesBySeparatorSize_(1)
    {
        CHECK(!data.points.empty());
        for(const vector<int>& point : data.points) {
            CHECK((int)point.size() == vertCount_);
        }
    }

    BayesianOracle(const BayesianOracle&) = delete;
    BayesianOracle(BayesianOracle&&) = delete;
    BayesianOracle& operator=(const BayesianOracle&) = delete;
    BayesianOracle& operator=(BayesianOracle&&) = delete;

    bool graphical() const {
        return graphical_;
    }

    int vertCount() const {
        return vertCount_;
    }

    // Returns true if a is independent of b given X
    bool indTest(int a, Bitset X, int b) {
        CHECK(a >= 0 && a <= vertCount_);
        CHECK(b >= 0 && b <= vertCount_);
        CHECK(a != b);
        CHECK(X.isSubsetOf(Bitset::range(vertCount_)));
        CHECK(!X.contains(a));
        CHECK(!X.contains(b));

        if(a > b) {
            swap(a, b);
        }

        int sepSize = X.count();
        if(sepSize > (int)queriesBySeparatorSize_.size() - 1) {
            queriesBySeparatorSize_.resize(sepSize + 1);
        }

        ++indTestCountSinceLastTimeCheck_;
        if(indTestCountSinceLastTimeCheck_ >= (graphical_ ? 1000 : 10)) {
            indTestCountSinceLastTimeCheck_ = 0;
            if(clock_.elapsedTime() > timeLimit_) {
                throw TimeLimitExceeded();
            }
        }

        unordered_map<Query, bool>::iterator iter;
        bool inserted;
        tie(iter, inserted) =
            queriesBySeparatorSize_[sepSize].insert({{{a, b}, X}, false});

        if(inserted) {
            if(graphical_) {
                iter->second = isDSeparated(dag_, a, X, b);
            } else {
                iter->second = pearsonChiSquaredIndTest(data_, a, X, b);
            }
        }
        return iter->second;
    }

    int maxQueriedSeparatorSize() const {
        return (int)queriesBySeparatorSize_.size() - 1;
    }
    vector<uint64_t> queryCountBySeparatorSize() const {
        vector<uint64_t> ret;
        for(const auto& queries : queriesBySeparatorSize_) {
            ret.push_back(queries.size());
        }
        return ret;
    }
    double elapsedTime() const {
        return clock_.elapsedTime();
    }

private:
    bool graphical_;
    int vertCount_;

    const Digraph& dag_;
    const Data& data_;

    Clock clock_;
    double timeLimit_;
    int indTestCountSinceLastTimeCheck_;

    typedef pair<pair<int, int>, Bitset> Query;
    vector<unordered_map<Query, bool>> queriesBySeparatorSize_;
};
