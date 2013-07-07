//Julian Kemmerer
//Andrew Sauber
//ECEC 672 SSTA Project


#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <map>

#include "cpdf.h"
#include "heuristics_metrics.h"
#include "cell_cpdf.h"
#include "gate_cpdf.h"
#include "wire_cpdf.h"
#include "cell_library.h"
#include "gate_set.h"

#include "fileio.h"
#include "pdfmath.h"

using namespace std;

//Cell library - where all cell templates are stored
//See class for why it is a class
cell_library the_cell_library;

//List of wires - where all wire_cpdf objects are stored
vector<wire_cpdf> wire_cpdfs;

//List of gates
//gate_cpdfs are the actual gate in the circuit with connecting wires
vector<gate_cpdf> gate_cpdfs;
//Map of gate names (NAND, OR) to indicies within the gate_cpdfs vector
//Don't use pointers are they will change when making copies
map<string, vector<unsigned int> > gate_type_to_indices;

//List of inputs as indices into gate_cpdfs
vector<unsigned int> input_indices;
//List of outputs as indices into gate_cpdfs
vector<unsigned int> output_indices;

//Critical path to be populated at very end
vector<gate_cpdf*> critical_path;
gate_cpdf * critical_output;

//Object that represents the set of heurisitcs and metrics
heuristics_metrics hm;
//Number of local heurisitic passes
int num_local_heurisitic_passes = 3;

//Map of gate names to their importance
map<string, float> gate_type_to_importance;
//Gate types  - will be sorted in in order of importance
vector<string> gate_types; 

//Store current value of cost delay metric
float current_total_circuit_max_cost_arrival_time_metric = -1;

//Print the result at the end
void print_results();

int main(int argc, char *argv[]) 
{
	//Order of files should be
	//$ ./jassta <cell_library> <.bench> <.time>
	//Read the files - pass globals rather than extern a bunch
	readFiles(argc,argv,the_cell_library,wire_cpdfs,gate_cpdfs);
	
	//Do the heuristic processes
	hm.do_heuristics();
	
	//Read out the results
	print_results();
}

void print_results()
{	
	/*
	cout << "Gate Delays" << endl;
	//Loop through each output
	int num_outputs = outputs.size();
	for(int i = 0; i < num_outputs; ++i)
	{
		outputs[i]->print();
		cout << endl;
	}
	*/

	/*
	//Print the critical path
	//End node was added first so print backwards
	cout << "Critical Path:" << endl;
	int num_gates = critical_path.size();
	for(int i = num_gates -1 ; i >= 0; --i)
	{
		cout << critical_path[i]->name << "->";
	}
	cout << endl;
	if(critical_output != NULL)
	{
		cout << "Critical Path Delay: "	<< critical_output->max_output_arrival_time << endl;
	}
	else
	{
		cout << "No critical path delay found? Our bad, man." << endl;
	}
	* */
	
	//Compute cost
	float cost = 0;
	int num_gates = gate_cpdfs.size();
	for(int i = 0; i<num_gates; ++i)
	{
		//cout << gate_cpdfs[i].cost << endl;
		cost += gate_cpdfs[i].cost;
	}
	cout << "Cost: " << cost << endl;
	
	
	//Print metric at end too
	cout << "Final Cost-Delay Metric: " << hm.get_total_circuit_cost_arrival_time_metric(gate_cpdfs) << endl;
}










