#ifndef _cell_library_
#define _cell_library_

#include <iostream>
#include <vector>
#include "cell_cpdf.h"
#include "gate_cpdf.h"
#include <stdlib.h>
#include <map>
using namespace std;

//This is a class for purely being a nice way to store things
//And it has a few useful methods that are part of the heurisitic

class cell_library
{
  public:
    cell_library();
    
    //These vectors will be constant (safe with ptrs)
    //once all files are read in
    vector<cell_cpdf> nands;
    vector<cell_cpdf> ands;
    vector<cell_cpdf> nors;
    vector<cell_cpdf> ors;
    vector<cell_cpdf> xors;
    vector<cell_cpdf> xnors;
    vector<cell_cpdf> buffs;
    vector<cell_cpdf> nots;
    //Have extra map for convenience
    map<string, vector<cell_cpdf> * > gate_type_to_cell_choices;
    
    //Store the 'best' version
    cell_cpdf * best_nand;
    cell_cpdf * best_and;
    cell_cpdf * best_nor;
    cell_cpdf * best_or;
    cell_cpdf * best_xor;
    cell_cpdf * best_xnor;
    cell_cpdf * best_buff;
    cell_cpdf * best_not;
    
    //Find the 'best' gate of a certain type
    cell_cpdf * get_best_version(string op);
    
  private:
};


#endif
