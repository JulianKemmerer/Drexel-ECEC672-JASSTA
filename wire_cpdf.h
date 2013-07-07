#ifndef _wire_cpdf_
#define _wire_cpdf_

#include <iostream>
#include "cpdf.h"
#include "gate_cpdf.h"
#include <vector>

using namespace std;

//Mutual dependency
//#pragma message "gate_cpdf incomplete" 
class gate_cpdf;

extern vector<gate_cpdf> gate_cpdfs;

class wire_cpdf : public cpdf
{
  public:
	//Start gate
	int start_gate_index;
	//End gate
	int end_gate_index;
	//Arrival times are at the end of wires 
	bool valid_output_arrival_times;
	cpdf max_output_arrival_time;
	cpdf min_output_arrival_time;
	
	//Constructor
    wire_cpdf();
    
    void print();
    
  private:
};
//#pragma message "wire_cpdf defined"

#endif
