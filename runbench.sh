#!/bin/sh

./jassta benchmarks/taskin/cell_library.time benchmarks/taskin/Combinational/c17.bench benchmarks/taskin/Combinational/c17.time > c17.basic &

./jassta benchmarks/taskin/cell_library.time benchmarks/taskin/Combinational/c432.bench benchmarks/taskin/Combinational/c432.time > c432.basic &

./jassta benchmarks/taskin/cell_library.time benchmarks/taskin/Combinational/c880.bench benchmarks/taskin/Combinational/c880.time > c880.basic &

./jassta benchmarks/taskin/cell_library.time benchmarks/taskin/Combinational/c1355.bench benchmarks/taskin/Combinational/c1355.time > c1355.basic &

./jassta benchmarks/taskin/cell_library.time benchmarks/taskin/Combinational/c1908.bench benchmarks/taskin/Combinational/c1908.time > c1908.basic &

./jassta benchmarks/taskin/cell_library.time benchmarks/taskin/Combinational/c2670.bench benchmarks/taskin/Combinational/c2670.time > c2670basic &

./jassta benchmarks/taskin/cell_library.time benchmarks/taskin/Combinational/c3540.bench benchmarks/taskin/Combinational/c3540.time > c3540.basic &

./jassta benchmarks/taskin/cell_library.time benchmarks/taskin/Combinational/c5315.bench benchmarks/taskin/Combinational/c5315.time > c5315.basic &

./jassta benchmarks/taskin/cell_library.time benchmarks/taskin/Combinational/c7552.bench benchmarks/taskin/Combinational/c7552.time > c7552.basic &
