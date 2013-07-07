.PHONY : jassta clean

jassta : 
	g++-4.6 -g -std=c++0x -pthread main.cpp cell_cpdf.cpp cell_library.cpp cpdf.cpp fileio.cpp gate_cpdf.cpp gate_set.cpp heuristics_metrics.cpp pdfmath.cpp wire_cpdf.cpp -o jassta
	
clean :
	rm jassta
