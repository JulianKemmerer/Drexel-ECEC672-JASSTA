#include <iostream>
#include "wire_cpdf.h"

using namespace std;

wire_cpdf::wire_cpdf() : cpdf()
{
	valid_output_arrival_times = false;
	start_gate_index = -1;
	end_gate_index = -1;
}

void wire_cpdf::print()
{
	cout << "Wire: " << gate_cpdfs[start_gate_index].name << " "
		<< gate_cpdfs[end_gate_index].name << " " << a[0] << " "
		<< a[1] << " "
		<< a[2] << " "
		<< a[3] << " ";
}
