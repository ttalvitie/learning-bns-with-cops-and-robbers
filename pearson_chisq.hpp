#pragma once

#include "data.hpp"

#include <boost/math/distributions/chi_squared.hpp>

// Returns true if a is independent of b given X according to Pearson's
// chi-squared test applied to given data.
bool pearsonChiSquaredIndTest(const Data& data, int a, Bitset X, int b) {
    CHECK(a >= 0 && a <= (int)data.catCounts.size());
    CHECK(b >= 0 && b <= (int)data.catCounts.size());
    CHECK(a != b);
    CHECK(X.isSubsetOf(Bitset::range((int)data.catCounts.size())));
    CHECK(!X.contains(a));
    CHECK(!X.contains(b));

    vector<int> ord(data.points.size());
    for(int i = 0; i < (int)ord.size(); ++i) {
        ord[i] = i;
    }

    vector<int> splits;
    splits.push_back(0);
    if(!ord.empty()) {
        splits.push_back((int)ord.size());
    }

    vector<int> newSplits;

    double freedom = 1.0;
    vector<vector<int>> bins;

    X.iterate([&](int v) {
        freedom *= data.catCounts[v];

        if((int)bins.size() < data.catCounts[v]) {
            bins.resize(data.catCounts[v]);
        }

        newSplits.clear();
        newSplits.push_back(0);
        for(int s = 0; s < (int)splits.size() - 1; ++s) {
            int x = splits[s];
            int y = splits[s + 1];

            if(y - x == 1) {
                newSplits.push_back(y);
            } else {
                for(int c = 0; c < data.catCounts[v]; ++c) {
                    bins[c].clear();
                }
                for(int i = x; i < y; ++i) {
                    bins[data.points[ord[i]][v]].push_back(ord[i]);
                }
                int i = x;
                for(int c = 0; c < data.catCounts[v]; ++c) {
                    for(int p : bins[c]) {
                        ord[i++] = p;
                    }
                    if(i != newSplits.back()) {
                        newSplits.push_back(i);
                    }
                }
            }
        }
        swap(splits, newSplits);
    });

    int aCatCount = data.catCounts[a];
    int bCatCount = data.catCounts[b];
    vector<double> freqs(aCatCount * bCatCount);
    vector<double> aFreqs(aCatCount);
    vector<double> bFreqs(bCatCount);

    freedom *= (double)aCatCount - 1.0;
    freedom *= (double)bCatCount - 1.0;

    double chisq = 0.0;
    for(int s = 0; s < (int)splits.size() - 1; ++s) {
        fill(freqs.begin(), freqs.end(), 0.0);
        fill(aFreqs.begin(), aFreqs.end(), 0.0);
        fill(bFreqs.begin(), bFreqs.end(), 0.0);

        int x = splits[s];
        int y = splits[s + 1];
        double N = (double)(y - x);
        double unit = 1.0 / N;

        for(int i = x; i < y; ++i) {
            int aVal = data.points[ord[i]][a];
            int bVal = data.points[ord[i]][b];
            freqs[bVal * aCatCount + aVal] += unit;
            aFreqs[aVal] += unit;
            bFreqs[bVal] += unit;
        }

        double term = 0.0;
        for(int aVal = 0; aVal < aCatCount; ++aVal) {
            for(int bVal = 0; bVal < bCatCount; ++bVal) {
                double expected = aFreqs[aVal] * bFreqs[bVal];
                if(expected > 0.0) {
                    double diff = freqs[bVal * aCatCount + aVal] - expected;
                    term += diff * diff / expected;
                }
            }
        }
        term *= N;
        chisq += term;
    }

    boost::math::chi_squared_distribution<> dist(freedom);
    double crit = boost::math::quantile(dist, 0.95);
    return chisq < crit;
}
