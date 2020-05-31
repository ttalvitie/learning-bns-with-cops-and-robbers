#pragma once

#include "bayesian_oracle.hpp"
#include "cpdag.hpp"
#include "digraph.hpp"
#include "tree_decomposition.hpp"

class BayesianNetworkTreeDecompositionSolver {
public:
    BayesianNetworkTreeDecompositionSolver(
        BayesianOracle& oracle,
        Bitset verts,
        int tw
    )
        : oracle_(oracle),
          verts_(verts),
          tw_(tw)
    {
        result_ = run_();
    }
    
    bool result() {
        return result_;
    }

    TreeDecomposition takeTreeDecomposition() {
        TreeDecomposition ret;
        swap(ret, treeDecomposition_);
        return ret;
    }

private:
    BayesianOracle& oracle_;
    Bitset verts_;
    int tw_;
    bool result_;
    unordered_map<pair<Bitset, Bitset>, bool> preSolveMem_;
    unordered_map<pair<Bitset, int>, Bitset> extractComponentMem_;
    TreeDecomposition treeDecomposition_;

    bool run_() {
        CHECK(tw_ >= 1);
        if(verts_.count() <= 1) {
            treeDecomposition_.emplace_back();
            treeDecomposition_.back().verts = verts_;
            treeDecomposition_.back().child1 = -1;
            treeDecomposition_.back().child2 = -1;
            return true;
        }

        int initialCop = verts_.min();
        if(!preSolve_(Bitset::singleton(initialCop), verts_.without(initialCop))) {
            return false;
        }

        int root = preSolveConstruct_(Bitset::singleton(initialCop), verts_.without(initialCop));
        CHECK(root == 0);

        return true;
    }

    bool preSolveImpl_(Bitset cops, Bitset robbers) {
        if(robbers.isEmpty()) {
            return true;
        }

        Bitset newRobbers = extractComponent_(cops, robbers.min());

        if(!oracle_.graphical()) {
            newRobbers = newRobbers.intersectWith(robbers);
        }

        Bitset newCops = cops;
        cops.iterate([&](int c) {
            if(newRobbers.iterateWhile([&](int r) {
                return oracle_.indTest(c, newCops.without(c), r);
            })) {
                newCops.del(c);
            }
        });

        if(newCops.count() == tw_ + 1) {
            return false;
        }

        if(!solve_(newCops, newRobbers)) {
            return false;
        }

        return preSolve_(cops, robbers.minus(newRobbers));
    }
    bool preSolve_(Bitset cops, Bitset robbers) {
        std::unordered_map<pair<Bitset, Bitset>, bool>::iterator iter;
        bool inserted;
        tie(iter, inserted) = preSolveMem_.emplace(make_pair(cops, robbers), false);
        if(inserted) {
            iter->second = preSolveImpl_(cops, robbers);
        }
        return iter->second;
    }
    int preSolveConstruct_(Bitset cops, Bitset robbers) {
        int nodeIdx = treeDecomposition_.size();
        treeDecomposition_.emplace_back();
        treeDecomposition_[nodeIdx].verts = cops;
        treeDecomposition_[nodeIdx].child1 = -1;
        treeDecomposition_[nodeIdx].child2 = -1;

        if(robbers.isEmpty()) {
            return nodeIdx;
        }

        auto iter = extractComponentMem_.find(make_pair(cops, robbers.min()));
        CHECK(iter != extractComponentMem_.end());
        Bitset newRobbers = iter->second;

        if(!oracle_.graphical()) {
            newRobbers = newRobbers.intersectWith(robbers);
        }

        Bitset newCops = cops;
        cops.iterate([&](int c) {
            if(newRobbers.iterateWhile([&](int r) {
                return oracle_.indTest(c, newCops.without(c), r);
            })) {
                newCops.del(c);
            }
        });

        CHECK(newCops.count() <= tw_);

        int child = solveConstruct_(newCops, newRobbers);
        treeDecomposition_[nodeIdx].child1 = child;
        if(newRobbers != robbers) {
            CHECK(newRobbers.isSubsetOf(robbers));
            child = preSolveConstruct_(cops, robbers.minus(newRobbers));
            treeDecomposition_[nodeIdx].child2 = child;
        }
        return nodeIdx;
    }

    bool solve_(Bitset cops, Bitset robbers) {
        return !robbers.iterateWhile([&](int a) {
            if(preSolve_(cops.with(a), robbers.without(a))) {
                return false;
            }
            return true;
        });
    }
    int solveConstruct_(Bitset cops, Bitset robbers) {
        int ret = -1;
        CHECK(!robbers.iterateWhile([&](int a) {
            auto iter = preSolveMem_.find(make_pair(cops.with(a), robbers.without(a)));
            CHECK(iter != preSolveMem_.end());
            if(iter->second) {
                ret = preSolveConstruct_(cops.with(a), robbers.without(a));
                return false;
            }
            return true;
        }));
        CHECK(ret != -1);
        return ret;
    }

    Bitset extractComponentImpl_(Bitset cops, int r0) {
        Bitset robbers = Bitset::singleton(r0);
        Bitset robberQueue = Bitset::singleton(r0);
        while(!robberQueue.isEmpty()) {
            int r1 = robberQueue.min();
            robberQueue.del(r1);
            verts_.minus(cops.unionWith(robbers)).iterate([&](int r) {
                if(!oracle_.indTest(r, cops, r1)) {
                    robbers.add(r);
                    robberQueue.add(r);
                }
            });
        }
        return robbers;
    }
    Bitset extractComponent_(Bitset cops, int r0) {
        std::unordered_map<pair<Bitset, int>, Bitset>::iterator iter;
        bool inserted;
        tie(iter, inserted) = extractComponentMem_.emplace(make_pair(cops, r0), Bitset::empty());
        if(inserted) {
            iter->second = extractComponentImpl_(cops, r0);
        }
        return iter->second;
    }
};

// Returns (tree decomposition, treewidth)
inline pair<TreeDecomposition, int> reconstructConnectedBayesianNetworkTreeDecomposition(
    BayesianOracle& oracle,
    Bitset verts
) {
    CHECK(!verts.isEmpty());
    if(verts.count() == 1) {
        TreeDecomposition treeDecomposition;
        treeDecomposition.emplace_back();
        treeDecomposition[0].verts = Bitset::singleton(verts.min());
        treeDecomposition[0].child1 = -1;
        treeDecomposition[0].child2 = -1;
        return {move(treeDecomposition), 0};
    }

    int tw = 1;
    while(true) {
        BayesianNetworkTreeDecompositionSolver solver(oracle, verts, tw);
        if(solver.result()) {
            return {solver.takeTreeDecomposition(), tw};
        }
        ++tw;
    }
}

// Returns (tree decompositions, treewidth)
inline pair<vector<TreeDecomposition>, int> reconstructBayesianNetworkTreeDecomposition(
    BayesianOracle& oracle
) {
    int vertCount = oracle.vertCount();

    vector<Bitset> comps;
    for(int v = 0; v < vertCount; ++v) {
        int found = -1;
        int compIdx = 0;
        while(compIdx < (int)comps.size()) {
            if(!comps[compIdx].iterateWhile([&](int x) {
                return oracle.indTest(v, Bitset::empty(), x);
            })) {
                if(found == -1) {
                    comps[compIdx].add(v);
                    found = compIdx;
                    ++compIdx;
                } else {
                    swap(comps[compIdx], comps.back());
                    comps[found] = comps[found].unionWith(comps.back());
                    comps.pop_back();
                }
            } else {
                ++compIdx;
            }
        }
        if(found == -1) {
            comps.push_back(Bitset::singleton(v));
        }
    }

    vector<TreeDecomposition> treeDecompositions;

    int tw = 0;
    for(Bitset comp : comps) {
        int compTW;
        TreeDecomposition treeDecomposition;
        tie(treeDecomposition, compTW) = reconstructConnectedBayesianNetworkTreeDecomposition(oracle, comp);
        tw = max(tw, compTW);
        treeDecompositions.push_back(move(treeDecomposition));
    }

    return {move(treeDecompositions), tw};
}

// Returns (skeleton, removed edge separators, tree decompositions, treewidth)
inline tuple<
    Graph,
    vector<pair<pair<int, int>, Bitset>>,
    vector<TreeDecomposition>,
    int
> reconstructBayesianNetworkSkeleton(
    BayesianOracle& oracle
) {
    vector<TreeDecomposition> treeDecompositions;
    int tw;
    tie(treeDecompositions, tw) = reconstructBayesianNetworkTreeDecomposition(oracle);

    vector<Bitset> bags;
    for(const TreeDecomposition& treeDecomposition : treeDecompositions) {
        for(const TreeDecompositionNode& node : treeDecomposition) {
            bags.push_back(node.verts);
        }
    }
    int bagIdx = 0;
    while(bagIdx < (int)bags.size()) {
        bool found = false;
        for(int i = 0; i < (int)bags.size(); ++i) {
            if(i != bagIdx && bags[bagIdx].isSubsetOf(bags[i])) {
                found = true;
                break;
            }
        }
        if(found) {
            swap(bags[bagIdx], bags.back());
            bags.pop_back();
        } else {
            ++bagIdx;
        }
    }

    Graph skeleton(oracle.vertCount());
    for(Bitset bag : bags) {
        bag.iterate([&](int b) {
            bag.intersectWith(Bitset::range(b)).minus(skeleton.adjacentVerts(b)).iterate([&](int a) {
                skeleton.addEdge(a, b);
            });
        });
    }

    vector<pair<pair<int, int>, Bitset>> edgeSeparators;
    for(int b = 0; b < oracle.vertCount(); ++b) {
        skeleton.adjacentVerts(b).intersectWith(Bitset::range(b)).iterate([&](int a) {
            for(Bitset bag : bags) {
                if(!bag.contains(a) && !bag.contains(b)) {
                    continue;
                }
                Bitset supset = bag.without(a).without(b);
                if(supset.isEmpty()) {
                    continue;
                }
                if(!supset.iterateSubsetsWhile([&](Bitset X) {
                    if(oracle.indTest(a, X, b)) {
                        edgeSeparators.emplace_back(make_pair(a, b), X);
                        return false;
                    } else {
                        return true;
                    }
                    return !oracle.indTest(a, X, b);
                })) {
                    skeleton.delEdge(a, b);
                    break;
                }
            }
        });
    }

    return {move(skeleton), move(edgeSeparators), move(treeDecompositions), tw};
}

// Returns (CPDAG, tree decompositions, treewidth)
inline tuple<
    Digraph,
    vector<TreeDecomposition>,
    int
> reconstructBayesianNetwork(
    BayesianOracle& oracle
) {
    Graph skeleton;
    vector<pair<pair<int, int>, Bitset>> edgeSeparators;
    vector<TreeDecomposition> treeDecompositions;
    int tw;
    tie(skeleton, edgeSeparators, treeDecompositions, tw) = reconstructBayesianNetworkSkeleton(oracle);

    Digraph cpdag = constructCPDAG(skeleton, edgeSeparators);

    return {move(cpdag), move(treeDecompositions), tw};
}
