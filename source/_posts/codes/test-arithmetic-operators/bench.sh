#!/bin/bash
# export MODE=${MODE:=CPP11}

if [ "${MODE}" = "CPP11" ] ; then
    export CXXFLAGS='-std=c++11 -DCPP11 -O3 -pedantic -Wall'
else
    export CXXFLAGS='-O3 -pedantic -Wall'
fi

run_test() {
    exe=test-arithmetic-operators-left$1
    echo
    make ${exe}
    # g++ $CXXFLAGS -DMETHOD_LEFT=$1 -o ${exe} test-arithmetic-operators.cpp
    echo "Running Method Left $1--------------------------------------------------"
    ./${exe}
}

run_test 1
run_test 2
run_test 3
run_test 4
run_test 5
# run_test 6
# run_test 7
# run_test 8
# run_test 9
