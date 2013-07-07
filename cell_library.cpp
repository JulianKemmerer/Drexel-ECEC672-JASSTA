#include <iostream>
#include "cell_library.h"

using namespace std;

cell_library::cell_library()
{
}

cell_cpdf * cell_library::get_best_version(string op)
{
	//This values are set at the file read in time
	if(op == "NAND")
	{
		return best_nand;
	}
	else if(op == "AND")
	{
		return best_and;
	}
	else if(op == "NOR")
	{
		return best_nor;
	}
	else if(op == "OR")
	{
		return best_or;
	}
	else if(op == "XOR")
	{
		return best_xor;
	}
	else if(op == "XNOR")
	{
		return best_xnor;
	}
	else if(op == "BUFF")
	{
		return best_buff;
	}
	else if(op == "NOT")
	{
		return best_not;
	}
	else
	{
		return NULL;
	}	
}
