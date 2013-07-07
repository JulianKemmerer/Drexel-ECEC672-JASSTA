#ifndef _gate_set_
#define _gate_set_

#include <iostream>
//#include "gate_cpdf.h"
#include <vector>
using namespace std;

//Class to do set operations with gates
class gate_cpdf;

class gate_set
{
  public:
    gate_set();
    
    void insert(gate_cpdf * g);
    gate_set gs_union( gate_set * gs); //union keyword used
    gate_set intersect( gate_set * gs);
    gate_set difference( gate_set * gs);
    int get_index(gate_cpdf * g);
    vector<gate_cpdf * > gates;
};


#endif
