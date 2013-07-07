#include <iostream>
#include "gate_set.h"

using namespace std;

gate_set::gate_set()
{
}

void gate_set::insert(gate_cpdf * g)
{
	//Loop through the whole set, if not found, put in
	int size = gates.size();
	for(int i = 0; i < size; ++i)
	{
		if(gates[i] == g)
		{
			return;
		}
	}
	gates.push_back(g);
}
gate_set gate_set::gs_union( gate_set * gs)
{
	//Really cheap any slow - sorry world
	//Create a new gate set and insert values from both
	gate_set rv;
	int self_size = gates.size();
	for(int i = 0; i < self_size; ++i)
	{
		rv.insert(gates[i]);
	}
	int param_size = gs->gates.size();
	for(int i = 0; i < param_size; ++i)
	{
		rv.insert(gs->gates[i]);
	}
	
	return rv;
}
int gate_set::get_index(gate_cpdf * g)
{
	//Loop through and find gate
	int self_size = gates.size();
	for(int i = 0; i < self_size; ++i)
	{
		if(gates[i] == g)
		{
			return i;
		}
	}
	return -1;
}

gate_set gate_set::intersect( gate_set * gs)
{
	//Loop through self
	//If self exists in other then it is part of intersection
	//Really slow too.
	gate_set rv;
	int self_size = gates.size();
	for(int i = 0; i < self_size; ++i)
	{
		//Found it in other
		if(gs->get_index(gates[i]) > -1)
		{
			rv.insert(gates[i]);
		}
	}
	return rv;
}
gate_set gate_set::difference( gate_set * gs)
{
	//Loop through self
	//If self does not exist in other then it is part of difference
	//Really slow too.
	gate_set rv;
	int self_size = gates.size();
	for(int i = 0; i < self_size; ++i)
	{
		//Not found in other
		if(gs->get_index(gates[i]) == -1)
		{
			rv.insert(gates[i]);
		}
	}
	return rv;
}
