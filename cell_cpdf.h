#ifndef _cell_cpdf_
#define _cell_cpdf_

#include <iostream>
#include "cpdf.h"
//#include "pdfmath.h"
//#include "heuristics_metrics.h"

using namespace std;

class heuristics_metrics;

extern heuristics_metrics hm;

class cell_cpdf : public cpdf
{
  public:
	//Gate name
	string gate;
	//Operation
	string op;
	//Cost
	float cost;
	
	
	//Thse values are read very often but calculated very little
	//Only calculate when changed (not often)
	void calc_read_often_values();
	//Cost-delay metric
	float cost_gate_delay;
	//Call this only when cost or delay is updated
	void calc_cost_gate_delay_metric();
	
	//Constructor
    cell_cpdf();
    
  private:
};
//#pragma message "cell_cpdf defined"


#endif
