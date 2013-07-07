#ifndef _fileio_
#define _fileio_

#include <iostream>
#include <vector>
#include <string>
#include <cstdlib> //Exit function?
#include <fstream>
#include <ostream>
#include <sstream>
#include "cell_cpdf.h"
#include "wire_cpdf.h"
#include "gate_cpdf.h"
#include "cell_library.h"
#include <map>
#include <algorithm>
#include "heuristics_metrics.h"
#include "pdfmath.h"

using namespace std;

extern map<string, vector<unsigned int> > gate_type_to_indices;
extern vector<string> gate_types;
extern map<string, float> gate_type_to_importance;
extern vector<unsigned int> input_indices;
extern vector<unsigned int> output_indices;
extern heuristics_metrics hm;
extern cell_library the_cell_library;

//Process the file and populate/link data structures
//Also pass globals to these functions - easier than managing externs
void readFiles(int argc, char *argv[], cell_library & cl,vector<wire_cpdf> & wcpdfs, vector<gate_cpdf> & gcpdfs );
ifstream & openFile(string filename);
void readCellLibraryFile(ifstream & inputFile, cell_library & cl);
vector<string> superSplit(string s, string delims);
vector<string> superSplitHelper(vector<string> & elems, char delim);
void readBenchFile(ifstream & inputFile,vector<gate_cpdf> & gcpdfs );
void readTimeFile(ifstream & inputFile,vector<wire_cpdf> & wcpdfs, vector<gate_cpdf> & gcpdfs );
//Extra function to do re-read of wire vector 
//(changed since Time file read complete)
void finalizeData(vector<wire_cpdf> & wcpdfs, vector<gate_cpdf> & gcpdfs);

//Get the best cell from a list of cells - only done at file read time
cell_cpdf * get_best_verion(vector<cell_cpdf>  & ccpdfs);

//Stack overflow split code
//http://stackoverflow.com/questions/236129/splitting-a-string-in-c
vector<string> &split(const string &s, char delim, vector<string> &elems);
vector<string> split(const string &s, char delim);

//Count downstream nodes
unsigned int count_downstream_gates(gate_cpdf * g);
unsigned int help_count_downstream_gates(gate_cpdf * g);
//Gate importance sorting
void sort_gate_types_by_importance();
bool gate_importance_compare(string g1, string g2);
float get_gate_importance(string type);

#endif
