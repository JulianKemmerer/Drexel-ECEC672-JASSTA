#include <iostream>
#include <stdio.h>
#include "heuristics_metrics.h"

using namespace std;

heuristics_metrics::heuristics_metrics()
{
	//This is the smallest noticable difference in floats read in from file
	//They only go to one decimal place so make diff to be 0.1
	file_read_fp_compare_epsilon = 0.1;
}

void heuristics_metrics::do_heuristics()
{
	//Select cells from the library and apply them to the list gates
	//Do a simple init choosing all gates of the same operation
	//to be the 'best' version
	init_gates(gate_cpdfs);
	
	//Begin the first full ssta circuit propogation
	run_full_thread_safe_ssta(gate_cpdfs);
	
	//Find the end critical path
	find_main_critical_path();
	
	//Print initial result
	float initial_cost_delay_metric = get_total_circuit_cost_arrival_time_metric(gate_cpdfs);
	cpdf initial_cpdf = (cpdf)critical_output->max_output_arrival_time;
	float initial_a0 = critical_output->max_output_arrival_time.a[0];
	float initial_cost = total_circuit_cost(gate_cpdfs);
	cout << "No heuristic circuit cost-delay metric: " << initial_cost_delay_metric << endl;
	cout << "No heuristic critical path delay: " << initial_cpdf << endl;
	cout << "No heuristic critical path a0: " << initial_a0 << endl;
	cout << "No heuristic circuit cost: " << initial_cost << endl;
	
	//Do global heuristics
	do_global_heuristics();
	
	//Do local heurisitics
	do_local_heuristics(num_local_heurisitic_passes);
	
	//Find the end critical path
	find_main_critical_path();
	
	float pre_inversion_cost_delay_metric = get_total_circuit_cost_arrival_time_metric(gate_cpdfs);
	float pre_inversion_a0 = critical_output->max_output_arrival_time.a[0];
	float pre_inversion_cost = total_circuit_cost(gate_cpdfs);
	
	//Do set inversion
	do_set_inversion_minimization();
		
	//Find the end critical path
	find_main_critical_path();
	
	
	cout << "Basic heuristic circuit cost-delay metric: " << initial_cost_delay_metric << endl;
	cout << "No heuristic critical path delay: " << initial_cpdf << endl;
	cout << "No heuristic critical path a0: " << initial_a0 << endl;
	cout << "No heuristic circuit cost: " << initial_cost << endl;
	cout << "Pre inversion cost-delay metric: " << pre_inversion_cost_delay_metric << endl;
	cout << "Pre inversion critical path a0: " << pre_inversion_a0 << endl;
	cout << "Pre inversion circuit cost: " << pre_inversion_cost << endl;
}

void heuristics_metrics::do_set_inversion_minimization()
{
	//Construct gate_set representing the global set of gates (universe set)
	gate_set universe_set;
	int num_gates = gate_cpdfs.size();
	for(int i =0; i< num_gates; ++i)
	{
		universe_set.insert(&(gate_cpdfs[i]));
	}
	
	//Get inversion of super set
	gate_set inverted_set = universe_set.difference(&(super_set));
	
	int num_non_critical = inverted_set.gates.size();
	cout << num_non_critical << " gates have never been on a critical path:" << endl;
	//Print those gates
	for(int i = 0; i < num_non_critical; ++i)
	{
		//cout << inverted_set.gates[i]->global_index << ":" << inverted_set.gates[i]->name << ":" << inverted_set.gates[i]->op << " ";
	}
	//cout << endl;
	
	//For each of those gates find the
	//Tmp vector of single index
	vector<unsigned int> single_gate_vector;
	for(int i = 0; i < num_non_critical; ++i)
	{
		//Do not try to optimize input
		if(inverted_set.gates[i]->op.length() == 0)
		{
			continue;
		}
		
		cout << "In set inversion minimization, looking at gate index: " << inverted_set.gates[i]->global_index << ":" << inverted_set.gates[i]->name << ":" << inverted_set.gates[i]->op << endl;
		float total_circuit_max_arrival_time = get_total_circuit_max_arrival_time_metric(gate_cpdfs);
		cout << "Swapped gate must have same or lower cost than " << inverted_set.gates[i]->cost << " and circuit delay metric same or lower than: " << total_circuit_max_arrival_time << endl;
		
		//Find the optimate gate selection
		//Just swapping one gate
		single_gate_vector.clear();
		single_gate_vector.push_back(inverted_set.gates[i]->global_index);
		cell_cpdf optimal = find_cheapest_working_cell_choice(inverted_set.gates[i]->op, single_gate_vector, total_circuit_max_arrival_time);
        cout << "Cheapest working local choice for single " << optimal.op << " gate, index " << inverted_set.gates[i]->global_index << ", to be " << optimal.gate << " at cost " << optimal.cost << endl;
		
		
		
		//Swap that into global gates
		apply_cell(&optimal, *(inverted_set.gates[i]));
			
		//Re run incremental sta starting at the changed gate
		//Use the global gate list
		propogate_arrival_times(gate_cpdfs, &(gate_cpdfs[inverted_set.gates[i]->global_index]));
		cout << "Set inversion intermediate cost-delay metric: " << get_total_circuit_cost_arrival_time_metric(gate_cpdfs) << endl;
		cout << "Inversion progress:	" << ((float)i / (float)num_non_critical)*100 << endl;
	}
}

float heuristics_metrics::get_total_circuit_max_arrival_time_metric(vector<gate_cpdf> & gcpdfs)
{
	//For the total circuit we want the largest arrival time metric
	
	//Loop through the outputs of the circuit and find the output with the largest arrival time
	int num_outputs = output_indices.size();
	float max_arrival_time_metric = -99999;
	float tmp;
	for(int i = 0; i< num_outputs; ++i)
	{
		tmp = delay_metric(&(gcpdfs[output_indices[i]].max_output_arrival_time));
		
		if( tmp > max_arrival_time_metric)
		{
			max_arrival_time_metric = tmp;
		}
	}
	return max_arrival_time_metric;
}

void heuristics_metrics::do_global_heuristics()
{
	cout << "Starting global heuristics." << endl;
	
	//Have sorted list of gate type by importance
	//Loop over that list, from highest to lowest importance
	int num_types = gate_types.size();
	for(int i = num_types-1; i >=0; --i)
	{
		cout << "Global swapping gate type: " << gate_types[i] << endl;
		//For each gate type
		//Find the optimate gate selection
		cell_cpdf optimal = find_optimal_cell_choice(gate_types[i], gate_type_to_indices[gate_types[i]]);
        cout << "Optimal global choice for " << optimal.op << " to be " << optimal.gate << endl;
		//Apply optimal to set of gates of this type
		//by looping through the list of gates under this type
		int num_gates = gate_type_to_indices[gate_types[i]].size();
		cout << "Globally applying " << optimal.op << "->" << optimal.gate << " for " << num_gates << " gates" << endl;
		for(int j = 0; j< num_gates; ++j)
		{
			//Apply the optimal gate to gate in the main global set
			//i is gate type iterator,j is gate index interator
			apply_cell(&optimal, gate_cpdfs[ (gate_type_to_indices[gate_types[i]])[j] ]);
		}
		
		//Re run sta after this application of templates
		//Run the full ssta on using the main global variables
		run_full_thread_safe_ssta(gate_cpdfs);
		cout << "Global intermediate cost-delay metric: " << get_total_circuit_cost_arrival_time_metric(gate_cpdfs) << " after swapping gate " << optimal.op << "->" << optimal.gate <<  endl;	
		
		//Refind critical path using global variables
		find_main_critical_path();
		
		//Add the current main critical path to the working set and super_set
		int num_gates_critical_path = critical_path.size();
		for(int k = 0; k < num_gates_critical_path; ++k)
		{
			add_to_heuristic_sets(&working_set, critical_path[k]);
		}
	}
	cout << "Global heuristics complete with cost-delay metric of " << get_total_circuit_cost_arrival_time_metric(gate_cpdfs) << endl;
}

void propogate_invalid_output_arrival_times(gate_cpdf * gate)
{
	//Mark gate as invalid arrival times
	gate->valid_output_arrival_times = false;
	
	//Travese connected wires and gates marking them as invalid
	//arrival times from gate on out
	//Loop across each output wire of the gate and mark as invalid
	//Then recurse on that gate
	int num_output_wires = gate->outputs.size();
	for(int i = 0; i< num_output_wires; ++i)
	{
		//Each wire should be marked invalid
		gate->outputs[i]->valid_output_arrival_times = false;
		//Recurse on the gate connected at end of wire
		//Use global list as this is not in threads
		propogate_invalid_output_arrival_times( &(gate_cpdfs[gate->outputs[i]->end_gate_index]) );
	}
}

//Run the full ssta on using the main global variables
void heuristics_metrics::run_full_thread_safe_ssta(vector<gate_cpdf> & gcpdfs)
{
	//Make sure all gates are set to not valid output arrival times
	//before running
	int num_gates = gcpdfs.size();
	for(int i = 0; i < num_gates; ++i)
	{
		gcpdfs[i].valid_output_arrival_times = false;
		//cout << "SSTA A0: " << gcpdfs[i].a[0] << endl;
	}	
	
	//Loop through the inputs of copy
	int num_inputs = input_indices.size();
	for(int i = 0; i < num_inputs; ++i)
	{
		//Propogate for each input
		propogate_arrival_times(gcpdfs, &(gcpdfs[input_indices[i]]));
	}
}

void heuristics_metrics::add_to_heuristic_sets(gate_set * gset, gate_cpdf *  gate)
{
	gset->insert(gate);
	//Add to super set
	super_set.insert(gate);
} 

float apply_swaps_get_total_circuit_cost_arrival_time_metric(vector<gate_cpdf> gcpdfs_copy, 
	vector<unsigned int> & gate_indices, cell_cpdf swap_cell)
{
	//Apply swap_cell to gates indicates by indices
	int num_to_swap = gate_indices.size();
	for(int i = 0; i < num_to_swap; ++i)
	{
		hm.apply_cell(&swap_cell,gcpdfs_copy[gate_indices[i]]);
	}
	
	//Re run incremental or full sta using copied gcpdfs
	if(num_to_swap ==1)
	{
		//Re run incremental sta
		//This is done through propogate arrival times
		//It uses pointers for traversal so is gcpdfs vector dependent
		//That is, it will use a copy if we start it in a copy
		hm.propogate_arrival_times(gcpdfs_copy, &(gcpdfs_copy[gate_indices[0]]));
	}
	else
	{
		//Re run full sta
		hm.run_full_thread_safe_ssta(gcpdfs_copy);
	}
	gcpdfs_copy[0].cost = 999999;
	
	//cout << "Thread reporting potential intermediate cost-delay metric: " << hm.get_total_circuit_cost_arrival_time_metric(gcpdfs_copy) << " using " << swap_cell.gate << endl;	
	//Get total circuit cost delay
	return hm.get_total_circuit_cost_arrival_time_metric(gcpdfs_copy);
}

float apply_swaps_get_total_circuit_max_arrival_time_metric(vector<gate_cpdf> gcpdfs_copy, 
	vector<unsigned int> & gate_indices, cell_cpdf swap_cell)
{
	//Apply swap_cell to gates indicates by indices
	int num_to_swap = gate_indices.size();
	for(int i = 0; i < num_to_swap; ++i)
	{
		hm.apply_cell(&swap_cell,gcpdfs_copy[gate_indices[i]]);
	}
	
	//Re run incremental or full sta using copied gcpdfs
	if(num_to_swap ==1)
	{
		//Re run incremental sta
		//This is done through propogate arrival times
		//It uses pointers for traversal so is gcpdfs vector dependent
		//That is, it will use a copy if we start it in a copy
		hm.propogate_arrival_times(gcpdfs_copy, &(gcpdfs_copy[gate_indices[0]]));
	}
	else
	{
		//Re run full sta
		hm.run_full_thread_safe_ssta(gcpdfs_copy);
	}
	
	float tmp = hm.get_total_circuit_max_arrival_time_metric(gcpdfs_copy);
	//cout << "Inversion minimization thread reporting potential intermediate delay metric: " << tmp << " using " << swap_cell.gate << " at cost " << swap_cell.cost << endl;	
	//Get total circuit cost delay
	
	return tmp;
}

float heuristics_metrics::total_circuit_cost(vector<gate_cpdf> & gcpdfs)
{
	//Loop through all gates and sum cost
	float cost = 0;
	int num_gates = gcpdfs.size();
	for(int i = 0; i < num_gates; i++)
	{
		cost += gcpdfs[i].cost;
	}
	return cost;
}

float heuristics_metrics::get_total_circuit_cost_arrival_time_metric(vector<gate_cpdf> & gcpdfs)
{
	//For the total circuit we want the worst cost_arrival_time_metric which means we want the least value
	
	//Loop through the outputs of the circuit and find the output with the worst (slowest->lowest) cost arrival time metric
	int num_outputs = output_indices.size();
	float min_cost_gate_arrival_time = 99999;
	float tmp;
	float cost = total_circuit_cost(gcpdfs);
	for(int i = 0; i< num_outputs; ++i)
	{
		//cout << "checking: " << gcpdfs[output_indices[i]].max_output_arrival_time << endl;
		tmp = cost_gate_arrival_time_metric(cost, &(gcpdfs[output_indices[i]].max_output_arrival_time));
		if( tmp < min_cost_gate_arrival_time)
		{
			min_cost_gate_arrival_time = tmp;
		}
	}
	//cout << "min_cost_gate_arrival_time_metric " << min_cost_gate_arrival_time << endl;
	return min_cost_gate_arrival_time;
}


cell_cpdf heuristics_metrics::find_optimal_cell_choice(string gate_type, vector<unsigned int> & gate_indices)
{
	//Launch threads to find the 'best' cell choice for the entire circuit
	//Vector of return values
	vector< future<float> > fut_rets;
	
	//Collect the results of each thread, modify global (main) gate set
	float max_cost_arrival_time_delay = -9999;
	float tmp;
	int optimal_choice_index = -1;
	
	//Loop over all choices for this gate type
	//cout << gate_type << endl;
	//cout << "gate type len: " << gate_type.length() << endl;
	int num_choices = the_cell_library.gate_type_to_cell_choices[gate_type]->size();
	for(int i = 0; i < num_choices ; i++)
	{
		//cout << "i " << i << endl;
		//Launch threads to try each type
		fut_rets.push_back(	
			async(
				launch::async,
				apply_swaps_get_total_circuit_cost_arrival_time_metric,
				gate_cpdfs, //Copy by value the global gates into the thread
				gate_indices,
				(*(the_cell_library.gate_type_to_cell_choices[gate_type]))[i]
				)
						);
						
		float tmp =fut_rets[fut_rets.size()-1].get();
		if(tmp > max_cost_arrival_time_delay)
		{
			max_cost_arrival_time_delay = tmp;
			optimal_choice_index = i;
		}			
						
	}

    
	
	/*
	for(int i = 0; i < num_choices ; i++)
	{
		tmp = fut_rets[i].get();
		if(tmp > max_cost_arrival_time_delay)
		{
			max_cost_arrival_time_delay = tmp;
			optimal_choice_index = i;
		}
	}
	cout << "			TEST COST: " << gate_cpdfs[0].cost << endl;
	* */
	
	
	//Return the optimal choice
	//cout << "size: " << the_cell_library.gate_type_to_cell_choices[gate_type]->size() << endl;
	//cout << "op choice" << optimal_choice_index << endl;
	
	cell_cpdf rv = (*(the_cell_library.gate_type_to_cell_choices[gate_type]))[optimal_choice_index];
	return rv;
}

cell_cpdf heuristics_metrics::find_cheapest_working_cell_choice(string gate_type, vector<unsigned int> & gate_indices, float current_circuit_max_arrival_time_metric)
{
	//Launch threads to find the cheapest cell choice that does not increase circuit delay (ok to decrease)
	//Vector of return values
	vector< future<float> > fut_rets;
	
	//Collect the results of each thread, modify global (main) gate set
	float min_cost = 9999;
	float min_cost_index = -1;
	float tmp;
	int optimal_choice_index = -1;
	
	
	//Loop over all choices for this gate type
	int num_choices = the_cell_library.gate_type_to_cell_choices[gate_type]->size();
	for(int i = 0; i < num_choices ; i++)
	{	
		//DO non thread propoagate as test
		propogate_arrival_times(gate_cpdfs, &(gate_cpdfs[gate_indices[0]]));
		//Print arrival time
		
		//Launch threads to try each type
		fut_rets.push_back(	
			async(
				launch::async,
				apply_swaps_get_total_circuit_max_arrival_time_metric,
				gate_cpdfs, //Copy by value the global gates into the thread
				gate_indices,
				(*(the_cell_library.gate_type_to_cell_choices[gate_type]))[i]
				)
						);	
						
		//First check that this choice results in a less then or equal arrive time
		tmp = fut_rets[i].get();
		
		//if(tmp - current_circuit_max_arrival_time_metric <= file_read_fp_compare_epsilon)
		if(tmp <= current_circuit_max_arrival_time_metric)
		{
			//If it does then check if it is the new min cost
			if( ((*(the_cell_library.gate_type_to_cell_choices[gate_type]))[i]).cost < min_cost )
			{
				min_cost = ((*(the_cell_library.gate_type_to_cell_choices[gate_type]))[i]).cost;
				min_cost_index = i;
			}
		}			
						
	}

	/*
	for(int i = 0; i < num_choices ; i++)
	{
		//First check that this choice results in a less then or equal arrive time
		tmp = fut_rets[i].get();
		
		if(tmp - current_circuit_max_arrival_time_metric <= file_read_fp_compare_epsilon)
		{
			//If it does then check if it is the new min cost
			if( ((*(the_cell_library.gate_type_to_cell_choices[gate_type]))[i]).cost < min_cost )
			{
				min_cost = ((*(the_cell_library.gate_type_to_cell_choices[gate_type]))[i]).cost;
				min_cost_index = i;
			}
		}
	}
	*/
	
	//No new min cost index might be found, go with old index
	if(min_cost_index ==-1)
	{
		cout << "No new minimum cost could be found without sacrificing arrival time" << endl;
		cout << "This likely a floating point error, tried to fix" << endl;
		exit(-1);
	}
		
	cell_cpdf rv = (*(the_cell_library.gate_type_to_cell_choices[gate_type]))[min_cost_index];
	return rv;
}

void heuristics_metrics::do_local_heuristics(int num_passes)
{
	cout << "Starting local heuristics" << endl;
	//to pass to finding cell algorithm
	unsigned int index;
	//Tmp vector to only hold one element since only switching one
	//gate at a time
	vector<unsigned int> single_gate_vector;
	
	//Do the follow for num_passes times
	int pass = 1;
	for(pass = 1; pass <=num_passes; ++pass)
	{
		cout << "Starting local pass: " << pass << endl;
		//We have a working_set from global
		//Sort it by importance
		sort_gate_set_by_criticalness(working_set);
		cout << "Working set size: " << working_set.gates.size() << endl;
		//Most important item will be last
		//For each gate in the working set, start from last
		int num_working_set = working_set.gates.size();
		for(int i = num_working_set -1; i >=0; --i)
		{
			//Find the index of the gate in question
			//index = find_gate_index_in_global(working_set.gates[i]));
			index = working_set.gates[i]->global_index;
			
			single_gate_vector.clear();
			single_gate_vector.push_back(index);
			
			//Working set might include and input gate (empty op)
			//Dont do this for them
			if(working_set.gates[i]->op.length() == 0)
			{
				cout << "In pass " << pass << ", working set gate index: " << index << ":" << working_set.gates[i]->name << " is an input, skipping" << endl;
				continue;
			}
			
			cout << "In pass " << pass << ", looking at working set gate index: " << index << ":" << working_set.gates[i]->name << ":" << working_set.gates[i]->op << endl;
			
			//Find the optimal choice
			cell_cpdf optimal_cell = find_optimal_cell_choice(working_set.gates[i]->op,single_gate_vector);
			
			//Swap that into global gates
			apply_cell(&optimal_cell, *(working_set.gates[i]));
			
			//Re run incremental sta starting at the changed gate
			//Use the global gate list
			propogate_arrival_times(gate_cpdfs, &(gate_cpdfs[index]));
			
			cout << "Optimal local choice for single " << optimal_cell.op << " gate, index " << index << ", to be " << optimal_cell.gate << endl;
			//cout << "Local intermediate cost-delay metric: " << get_total_circuit_cost_arrival_time_metric(gate_cpdfs) << endl;	
			//cout << "Local intermediate delay metric: " << get_total_circuit_max_arrival_time_metric(gate_cpdfs) << endl;
			
			//Get critical path using global list
			find_main_critical_path();
			
			//Add those critical path gates to the next_set and super set
			int num_gates_critical_path = critical_path.size();
			int next_set_size = next_set.gates.size();
			cout << "Next working set has " << next_set_size << " gates before update" << endl;
			cout << "Next working set global gate indices: " << endl;
			for(int j = 0; j < next_set_size; ++j)
			{
				cout << next_set.gates[j]->global_index << ":" << next_set.gates[j]->name << " ";
			}
			cout << endl;
			//Print current working set and critical path
			cout << "Inserting " << num_gates_critical_path << " critical path gates into next_set" << endl;
			cout << "Current critical path global gate indices being inserted: " << endl;
			for(int j = 0; j < num_gates_critical_path; ++j)
			{
				cout << critical_path[j]->global_index << ":" << critical_path[j]->name << " ";
				add_to_heuristic_sets(&next_set, critical_path[j]);
			}
			cout << endl;
			cout << "Next working set now has " << next_set.gates.size() << " gates" << endl;
		}
		
		//Check that the next_set is not exactly the same as working_set
		bool same_set = true;
		num_working_set = working_set.gates.size();
		if(num_working_set == next_set.gates.size())
		{
			//Check each element
			for(int i =0; i< num_working_set; ++i)
			{
				bool found = false;
				for(int j = 0; j < num_working_set; ++j)
				{
					if(working_set.gates[i] == next_set.gates[j])
					{
						found = true;
						break;
					}
				}
				if(found == false)
				{
					//Did not find gate in on set that is in other, continue to next pass
					same_set = false;
					break;
				}
			}
		}
		else
		{
			same_set = false;
		}
		if(same_set)
		{
			cout << "At the end of this pass, the working set / next set has converged. No additonal passes needed." << endl;
			cout << "Ending pass " << pass << endl;
			break;
		}
		
		//Once finished working_set
		//Swap the next set into the working set
		working_set = next_set;
		//Clear the next_set
		next_set.gates.clear();
		//Go for another round.
		cout << "Ending pass " << pass << endl;
	}
	cout << "Local heuristics complete." << endl;
}

bool gate_criticalness_compare(gate_cpdf * g1, gate_cpdf * g2)
{
	return (g1->criticalness() < g2->criticalness());
}

void heuristics_metrics::sort_gate_set_by_criticalness(gate_set & gs)
{
	sort(gs.gates.begin(),gs.gates.end(), gate_criticalness_compare);
}

unsigned int heuristics_metrics::find_gate_index_in_global(gate_cpdf * gate)
{
	//Search through the gates library to find
	int num_gates = gate_cpdfs.size();
	for(int j = 0; j < num_gates; ++j)
	{
		if(gate == &(gate_cpdfs[j]))
		{
			return j;
		}
	}
	cout << "AHH! Couldn't find a specific gate in the global gate cpdfs!" << endl;
	return 0;
}

//The cost delay metric we want this to be a large value for a 'best' gate
float heuristics_metrics::cost_gate_delay_metric(cell_cpdf * ccpdf)
{
	//Currently cost delay metric is
	// 1/ (cost*delay) as we want cost and delay to be small
	
	//Multiply to get more 'understandable' floating point numbers
	//return 100000000.0 * (1.0/(ccpdf->cost * delay_metric(ccpdf)));
	return (1.0/(ccpdf->cost * delay_metric(ccpdf)));
	//return (1.0/(delay_metric(ccpdf)));
	//return (1.0/(ccpdf->cost));
}

//For use with arrival times
float heuristics_metrics::cost_gate_arrival_time_metric(float cost, cpdf * arrival_time)
{
	//Pass to cost_gate_delay_metric
	cell_cpdf tmp;
	tmp.a[0] =  arrival_time->a[0];
	tmp.a[1] =  arrival_time->a[1];
	tmp.a[2] =  arrival_time->a[2];
	tmp.a[3] =  arrival_time->a[3];
	tmp.calc_stddev();
	tmp.cost = cost;
	return cost_gate_delay_metric(&tmp);
}


//The delay metric
float heuristics_metrics::delay_metric(cpdf * cpdf)
{
	if((cpdf->a[0]) < 0) cout << "Error! a[0] < 0" << endl;
	if((cpdf->stddev) < 0)
	{
		cpdf->calc_stddev();
		if((cpdf->stddev) < 0)
		{
			cout << "Error! stddev is still < 0 after trying to fix." << cpdf->stddev << endl;
			exit(-1);
		}
	} 
	//Delay is define as a[0] + stdev
	return cpdf->a[0] + cpdf->stddev;
	//return cpdf->a[0];
}

void heuristics_metrics::propogate_arrival_times(vector<gate_cpdf> & gcpdfs, gate_cpdf * gate)
{
	//Dont bother checking for gate already calculated
	//That should valid_output_arrival_times should not happen unless
	//the inputs are all satisified as checked below
	
	//First check that all arrival times at inputs are calculated
	//Loop through inputs
	int num_inputs = gate->inputs.size();
	bool all_inputs_calculated = true;
	for(int i = 0; i < num_inputs; ++i)
	{
		if(gate->inputs[i]->valid_output_arrival_times == false)
		{
			all_inputs_calculated = false;
			break;
		}
	}
	
	//Inputs calculated?
	if(!all_inputs_calculated)
	{
		//not all values ready - return
		return;
	}
	
	//cout << "Gate: " << gate->name << " all inputs calculated." << endl;
	
	//All values are ready. Calculate the gate arrival times.
	//Do the add and max operations to get delay at gate output
	//cout << "pre gate arrival time a0: " << gate->max_output_arrival_time.a[0] << endl;
	calc_gate_arrival_times(gate->inputs, gate); //Happens here
	//cout << "post gate arrival time a0: " << gate->max_output_arrival_time.a[0] << endl;
	
	//Then Loop through each output
	//Loop through each output wire of this gate
	int num_outputs = gate->outputs.size();
	
	//If this gate has no outputs it should be a primary output
	if(num_outputs == 0)
	{
		//Reached a primary output
		return;
	}
	
	for(int i = 0; i < num_outputs; ++i)
	{
		//At each output wire calculate the wire arrival time
		//I.e. Calc the cpdf at the end of that wire
		calc_wire_arrival_times(gate, gate->outputs[i]);
		
		//Once that is calculated
		//Recurse on that gate at the end of that wire
		propogate_arrival_times(gcpdfs, &(gcpdfs[gate->outputs[i]->end_gate_index]));
	}
}

void heuristics_metrics::calc_wire_arrival_times(gate_cpdf * from_gate, wire_cpdf * wire)
{
	//Take the arrive times at the gate and add it to the wire
	//cout << "from_gate->max_output_arrival_time a0 " << from_gate->max_output_arrival_time.a[0] << endl;
	//cout << "(*wire) a0 " << (*wire).a[0] << endl;
	wire->max_output_arrival_time = from_gate->max_output_arrival_time + (*wire);
	wire->min_output_arrival_time = from_gate->min_output_arrival_time + (*wire);
	wire->valid_output_arrival_times = true;
}

void heuristics_metrics::calc_gate_arrival_times(vector<wire_cpdf*> & inputs, gate_cpdf * gate)
{
	//First compute the additon of all the inputs + the gate delay
	//these are the values to be maxed,min'ed together
	vector<cpdf> to_max;
	vector<cpdf> to_min;
	char c;
	
	//Loop through the inputs
	int num_inputs = inputs.size();
	
	if(num_inputs == 0)
	{
		//This is input gate
		return;
	}
	
	for(int i = 0; i < num_inputs; ++i)
	{
		//Add the result to the to_max,to_min vectors
		//cout << "gate name: " << gate->name << endl;
		//cout << "inputs[i]->max_output_arrival_time a0: " << inputs[i]->max_output_arrival_time.a[0] << endl;
		//cout << "inputs[i]->valid_output_arrival_times " << inputs[i]->valid_output_arrival_times << endl;
		//cout <<"*gate a0: " << (*gate).a[0] << endl;
		cpdf tmpmax = (inputs[i]->max_output_arrival_time) + *gate;
		to_max.push_back( tmpmax);
		to_min.push_back( (inputs[i]->min_output_arrival_time) + *gate);
	}
	
	//Now max and min all of the values together
	//These functions erase the vectors passed in
	cpdf max = multi_max(to_max);
	cpdf min = multi_min(to_min);
	
	//Now have max and min arival times at gate
	//cout << "Pre Mid ssta max arrival time a0: " << gate->max_output_arrival_time.a[0] << endl;
	gate->max_output_arrival_time = max;
	//cout << "Post Mid ssta max arrival time a0: " << gate->max_output_arrival_time.a[0] << endl;
	
	gate->min_output_arrival_time = min;
	gate->valid_output_arrival_times = true;	
}

void heuristics_metrics::find_main_critical_path()
{
	//Reset the current values
	critical_output = NULL;
	critical_path.clear();
	
	//Find the largest a[0] value at the outputs
	int slow_output_index = -1;
	float largest_delay = -1;
	
	//Loop through each output
	int num_outputs = output_indices.size();
	for(int i = 0; i < num_outputs; ++i)
	{
		//cout << "Delay metric value: " << delay_metric(&(gate_cpdfs[output_indices[i]].max_output_arrival_time)) << " gate=" << gate_cpdfs[output_indices[i]].name << endl;
		if(delay_metric(&(gate_cpdfs[output_indices[i]].max_output_arrival_time)) >largest_delay)
		{
			largest_delay = delay_metric(&(gate_cpdfs[output_indices[i]].max_output_arrival_time));
			slow_output_index = i;
		}
		else
		{
			if(gate_cpdfs[output_indices[i]].valid_output_arrival_times == false)
			{
				cout << "Error: trying to find critical path when output " << gate_cpdfs[output_indices[i]].name  << " does not have valid output arrival time values." << endl;
				exit(-1);
			}
		}
	}
	
	//Found something
	if( (slow_output_index != -1) )
	{
		//Put this gate on the slowest path
		critical_output = &(gate_cpdfs[output_indices[slow_output_index]]);
		critical_path.push_back(&(gate_cpdfs[output_indices[slow_output_index]]));
		
		gate_cpdf * gate = &(gate_cpdfs[output_indices[slow_output_index]]);
		
		//Start at the output
		wire_cpdf * slow_wire = find_slow_wire(gate);
		
		while(slow_wire != NULL)
		{
			//keep going backwards
			gate = &(gate_cpdfs[slow_wire->start_gate_index]);
			//Put gate on critical path
			critical_path.push_back(gate);
			slow_wire = find_slow_wire(gate);
		}
		//Done
	}
	else
	{
		cout << "Could not find a critical output!" << endl;
		exit(-1);
	}
	
	if(critical_output == NULL)
	{
		cout << "Could not find a critical output - null!" << endl;
		exit(-1);
	}
}

wire_cpdf * heuristics_metrics::find_slow_wire(gate_cpdf * gate)
{
	//Loop over all input wires and find the slowest
	//Find the largest a[0] value at the inputs
	int slow_input_index = -1;
	float largest_delay = -1;
	
	//Loop through each input
	int num_inputs = gate->inputs.size();
	for(int i = 0; i < num_inputs; ++i)
	{
		if(gate->inputs[i]->max_output_arrival_time.a[0] > largest_delay)
		{
			largest_delay = gate->inputs[i]->max_output_arrival_time.a[0];
			slow_input_index = i;
		}
	}
	
	//Found something
	if( (slow_input_index != -1) && (largest_delay != -1) )
	{
		return gate->inputs[slow_input_index];
	}
	else
	{
		return NULL;
	}
}

void heuristics_metrics::apply_cell(cell_cpdf * ccpdf, gate_cpdf & gcpdf)
{	
	//Copy vals
	//Probably an easier way to do this with inheritance?
	gcpdf.gate = ccpdf->gate;
	//cout << "cost: " << ccpdf->cost << endl;
	gcpdf.op = ccpdf->op;
	gcpdf.cost = ccpdf->cost;
	gcpdf.a[0] = ccpdf->a[0];
	gcpdf.a[1] = ccpdf->a[1];
	gcpdf.a[2] = ccpdf->a[2];
	gcpdf.a[3] = ccpdf->a[3];
	
	//Now that values have change recalc read often vals
	gcpdf.calc_read_often_values();
		
	//Set this gate as not having valid output arrival times
	//Since we just changed it's values
	gcpdf.valid_output_arrival_times = false;
	//All gates down stream need to be invalidated too
	propogate_invalid_output_arrival_times(&gcpdf);
}

void heuristics_metrics::init_gates(vector<gate_cpdf> & gcpdfs)
{
	//Loop through the gates and initially select the 'best' gate
	int num_gates = gcpdfs.size();
	cell_cpdf * tmp = NULL;
	for(int i = 0; i < num_gates; ++i)
	{
		tmp = the_cell_library.get_best_version(gcpdfs[i].op);
		//If best gate is found (wont be found if input or output gate)
		if(tmp != NULL)
		{
			apply_cell(tmp,gcpdfs[i]);
		}
	}
}
