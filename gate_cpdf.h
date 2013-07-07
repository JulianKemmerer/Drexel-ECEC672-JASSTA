#ifndef _gate_cpdf_
#define _gate_cpdf_

#include <iostream>
//#include "cell_cpdf.h"
#include "wire_cpdf.h"
#include <vector>
#include <string>
#include "pdfmath.h"
#include "heuristics_metrics.h"
using namespace std;

//cell_cpdf is an instance of the 'template' gate
//gate_cpdf is the actual gate in the circuit with connecting wires

//Mutual dependency
//#pragma message "wire_cpdf incomplete" 
class wire_cpdf;
class heuristics_metrics;
extern heuristics_metrics hm;

#include "cell_cpdf.h"

class gate_cpdf : public cell_cpdf
{
  public:
	//Gate is the same as a cell except for having connecting wires
	//All of our gates have multiple inputs and a single output
	vector<wire_cpdf*> outputs;
	vector<wire_cpdf*> inputs;
	
	//Arrival times are at the output of gates
	bool valid_output_arrival_times;
	cpdf max_output_arrival_time;
	cpdf min_output_arrival_time;
	
	//Gate name as given in the bench file
	//G10 = NAND(G1, G3) means gate G10 is a NAND gate
	string name;
    gate_cpdf();
    
    //Number of downstream nodes
    unsigned int downstream_gates;
    bool counted;
    int global_index;
    
    //Compute the criticalness
    float criticalness();
    float spread();
    
    void print();
    
  private:
};
//#pragma message "gate_cpdf defined"


#endif
