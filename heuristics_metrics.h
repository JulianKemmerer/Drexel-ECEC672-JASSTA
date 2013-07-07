#ifndef _heuristic_metrics_
#define _heuristic_metrics_

//#include "gate_cpdf.h"
#include "wire_cpdf.h"
#include "cell_library.h"
//For threading
#include <thread>
#include <future>
#include "gate_set.h"
#include <map>
#include <algorithm>
#include <math.h>
#include <stdio.h>

//Mutual dependency
//class cell_cpdf;
class cell_library;
class wire_cpdf;

//Defined in main
extern vector<unsigned int> input_indices;
extern vector<unsigned int> output_indices;
extern vector<gate_cpdf*> critical_path;
extern gate_cpdf * critical_output;
extern cell_library the_cell_library;
extern vector<gate_cpdf> gate_cpdfs;
extern int num_local_heurisitic_passes;
extern vector<string> gate_types; 
extern map<string, vector<unsigned int> > gate_type_to_indices;
//Mutual dep
class heuristics_metrics;
class gate_cpdf;
extern heuristics_metrics hm;
extern float current_total_circuit_max_cost_arrival_time_metric;

class heuristics_metrics
{
  public:
	heuristics_metrics();
	
	float file_read_fp_compare_epsilon; //Value in constructor
	
	//The whole sha-bang to run after the first run fo ssta
	void do_heuristics();
  
	//The cost gate delay metric - NOT FOR USE WITH ARRIVAL TIMES
	float cost_gate_delay_metric(cell_cpdf * ccpdf);
	//Use this with arrival times
	float cost_gate_arrival_time_metric(float cost, cpdf * arrival_time);
	
	//The delay metric
	float delay_metric(cpdf * cpdf);

	//Run the full ssta on using the main global variables
	void run_full_thread_safe_ssta(vector<gate_cpdf> & gcpdfs);
	//Function to recursively propogate arrival times
	void propogate_arrival_times(vector<gate_cpdf> & gcpdfs, gate_cpdf * gate);
	//Calc the arrive time at the end of a wire given a starting gate
	void calc_wire_arrival_times(gate_cpdf * from_gate, wire_cpdf * wire);
	//Calc the  arrival time at the end of a gate given the input wires
	void calc_gate_arrival_times(vector<wire_cpdf*> & inputs, gate_cpdf * gate);
	//Return critical path
	void find_main_critical_path();
	//Find the slowest wire at a specific gate
	wire_cpdf * find_slow_wire(gate_cpdf * gate);
	
	//Mak initial gate choices
    void init_gates(vector<gate_cpdf> & gcpdfs);
    //Apply a cell template to a gate
    void apply_cell(cell_cpdf * ccpdf, gate_cpdf & gcpdf);
    
    //Heurisitics
    void do_global_heuristics();
	//Do local heurisitics
	void do_local_heuristics(int num_passes);
	//Do final set inversion
	void do_set_inversion_minimization();
	//All gates ever in critical path
	gate_set super_set;
	
	//Find optimate gate type
	cell_cpdf find_optimal_cell_choice(string gate_type, vector<unsigned int> & gate_indices);
	//Find just the cheapest metric sustaining/improving gate
	cell_cpdf find_cheapest_working_cell_choice(string gate_type, vector<unsigned int> & gate_indices, float current_circuit_max_arrival_time_metric);
	//Get just the whole circuit arrival time
	float get_total_circuit_max_arrival_time_metric(vector<gate_cpdf> & gcpdfs);
	
	//Get the max end delay (lowest metric) for the circuit
	float get_total_circuit_cost_arrival_time_metric(vector<gate_cpdf> & gcpdfs);
	
	//Store sets of gates for heuristic use
	//Working and next sets, working for global and local, next for local
	gate_set working_set;
	gate_set next_set;
	//Add to set and also to super set
	void add_to_heuristic_sets(gate_set * gset, gate_cpdf *  gate);
	
	//Find the index of gate in a list of pointers
	unsigned int find_gate_index_in_global(gate_cpdf * gate);
	
	//Sorting of gate sets
	void sort_gate_set_by_criticalness(gate_set & gs);
	//bool gate_criticalness_compare(gate_cpdf * g1, gate_cpdf * g2);
	
	//Total circuit cost
	float total_circuit_cost(vector<gate_cpdf> & gcpdfs);
};

#endif
