CXX ?= g++
CFLAGS := -std=c++14 -Wall -march=native -O3
LDFLAGS :=
HEADERS := $(shell find . -name '*.hpp')
TAMAKI2017_SRCS := $(shell find tamaki2017/tw/exact -name '*.java')
TAMAKI2017_CLASSES := $(TAMAKI2017_SRCS:%.java=%.class)

.PHONY: all clean

all: bayesian_test bnrepository_test bnrepository_data_test

bayesian_test: bayesian_test.cpp $(HEADERS) $(TAMAKI2017_CLASSES)
	$(CXX) $(CFLAGS) $< -o $@ $(LDFLAGS)

bnrepository_test: bnrepository_test.cpp $(HEADERS) $(TAMAKI2017_CLASSES)
	$(CXX) $(CFLAGS) $< -o $@ $(LDFLAGS)

bnrepository_data_test: bnrepository_data_test.cpp $(HEADERS) $(TAMAKI2017_CLASSES)
	$(CXX) $(CFLAGS) $< -o $@ $(LDFLAGS)

tamaki2017/tw/exact/%.class: tamaki2017/tw/exact/%.java
	javac -classpath tamaki2017 $<

clean:
	rm -f bayesian_test bnrepository_test bnrepository_data_test $(TAMAKI2017_CLASSES)
