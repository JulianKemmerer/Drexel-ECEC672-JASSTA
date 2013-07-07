#include <iostream>
#include "gate_cpdf.h"

using namespace std;

gate_cpdf::gate_cpdf():cell_cpdf()
{
	valid_output_arrival_times = false;
	downstream_gates = 0;
    counted = false;
    global_index = -1;
}

void gate_cpdf::print()
{
	cout << "Gate: " << name << " " << a[0] << " "
		<< a[1] << " "
		<< a[2] << " "
		<< a[3] << " ";
}

float gate_cpdf::criticalness()
{
	//(1/spread) * #down stream nodes * cost * delay
	return (1.0/spread()) * downstream_gates * cost * hm.delay_metric(this);
}

float gate_cpdf::spread()
{
	//Variance of the delays at the gate input
	vector<float> delays;
	
	//Loop over the inputs and collect the delays
	int num_inputs = inputs.size();
	for(int i = 0; i< num_inputs; i++)
	{
		delays.push_back( hm.delay_metric(&(inputs[i]->max_output_arrival_time)));
	}
	return variance(delays);
}
