#pragma once

#include "graph.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <signal.h>

class TreewidthSolver {
public:
    // Forks to create solver subprocess
    TreewidthSolver() {
        int inPipeFd[2];
        int outPipeFd[2];
        CHECK(!pipe(inPipeFd));
        CHECK(!pipe(outPipeFd));

        pid_t pid = fork();
        CHECK(pid != -1);
        if(pid == 0) {
            close(outPipeFd[1]);
            close(inPipeFd[0]);

            dup2(outPipeFd[0], STDIN_FILENO);
            dup2(inPipeFd[1], STDOUT_FILENO);
            dup2(open("/dev/null", O_WRONLY), STDERR_FILENO);

            prctl(PR_SET_PDEATHSIG, SIGTERM);

            execlp(
                "java",
                "java",
                "-classpath", "tamaki2017",
                "-Xmx1g", "-Xss10m",
                "tw.exact.MainDecomposer",
                (char*)nullptr
            );
            exit(1);
        }

        CHECK(!close(outPipeFd[0]));
        CHECK(!close(inPipeFd[1]));

        childPid_ = pid;
        input_ = fdopen(outPipeFd[1], "w");
        CHECK(input_ != nullptr);
        output_ = fdopen(inPipeFd[0], "r");
        CHECK(output_ != nullptr);
    }

    ~TreewidthSolver() {
        CHECK(!fclose(input_));
        CHECK(!fclose(output_));
        kill(childPid_, SIGKILL);
        int status;
        CHECK(waitpid(childPid_, &status, 0) != -1);
    }

    TreewidthSolver(const TreewidthSolver&) = delete;
    TreewidthSolver(TreewidthSolver&&) = delete;
    TreewidthSolver& operator=(const TreewidthSolver&) = delete;
    TreewidthSolver& operator=(TreewidthSolver&&) = delete;

    int solve(const Graph& graph) {
        if(graph.vertCount() == 0) {
            return 0;
        }

        int edgeCount = 0;
        for(int v = 0; v < graph.vertCount(); ++v) {
            edgeCount += graph.adjacentVerts(v).minus(Bitset::range(v)).count();
        }

        CHECK(fprintf(input_, "p tw %d %d\n", graph.vertCount(), edgeCount) >= 0);
        for(int v = 0; v < graph.vertCount(); ++v) {
            graph.adjacentVerts(v).minus(Bitset::range(v)).iterate([&](int x) {
                CHECK(fprintf(input_, "%d %d\n", v + 1, x + 1) >= 0);
            });
        }
        CHECK(!fflush(input_));

        char tmp[3];
        CHECK(fscanf(output_, "%1s", tmp) == 1);
        CHECK(!strcmp(tmp, "s"));

        CHECK(fscanf(output_, "%2s", tmp) == 1);
        CHECK(!strcmp(tmp, "td"));

        int bagCount;
        CHECK(fscanf(output_, "%d", &bagCount) == 1);
        CHECK(bagCount >= 0);

        int tw;
        CHECK(fscanf(output_, "%d", &tw) == 1);
        --tw;
        CHECK(tw >= 0 && tw <= graph.vertCount() - 1);

        int vertCount;
        CHECK(fscanf(output_, "%d", &vertCount) == 1);
        CHECK(vertCount == graph.vertCount());

        for(int bagIdx = 0; bagIdx < bagCount; ++bagIdx) {
            CHECK(fscanf(output_, "%1s", tmp) == 1);
            CHECK(!strcmp(tmp, "b"));

            while(true) {
                int c = fgetc(output_);
                CHECK(c != EOF);
                if(c == '\n') {
                    break;
                }
            }
        }

        for(int linkIdx = 1; linkIdx < bagCount; ++linkIdx) {
            int a, b;
            CHECK(fscanf(output_, "%d %d", &a, &b) == 2);
            CHECK(a >= 1 && a <= bagCount);
            CHECK(b >= 1 && b <= bagCount);
        }

        return tw;
    }

private:
    pid_t childPid_;
    FILE* input_;
    FILE* output_;

    friend TreewidthSolver createTreewidthSolver();
};
