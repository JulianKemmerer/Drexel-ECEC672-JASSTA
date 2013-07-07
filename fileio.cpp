#include <iostream>
#include "fileio.h"
#include "heuristics_metrics.h"

using namespace std;

//Implementations for fileio functions
//Do all the file reading
void readFiles(int argc, char *argv[], cell_library & cl,vector<wire_cpdf> & wcpdfs, vector<gate_cpdf> & gcpdfs)
{
	//Order of files should be
	//$ ./jassta <cell_library> <.bench> <.time>
	
	//Check that at least 4 arguments exist
	if(argc < 4)
	{
		cout << "Please supply files to read." << endl << "$ ./jassta <cell_library> <.bench> <.time>" << endl;
		exit(-1);
	} 
	else 
	{
		cout << "Starting file read." << endl;
		//Store file names
		string cell_library_filename = argv[1];
		string bench_filename = argv[2];
		string time_filename = argv[3];
		//cout << "Cell library: " << cell_library_filename << endl;
		cout << "Benchmark file: " << bench_filename << endl;
		cout << "Time file: " << time_filename << endl;
		
		//Run functions to read those files
		//Read cell library
		readCellLibraryFile( openFile(cell_library_filename), cl );
		//This will be inefficient
		//Gates/cells have ptrs to wires, and wires have ptrs to gates
		//We cannot populate both sets of ptrs, only one then forced
		//to go back and populate the other
		//DO NOT CHANGE THE ORDER OF THESE FEW LINES ************
		//Read gates from bench - will need to go back and populate
		readBenchFile( openFile(bench_filename), gcpdfs );
		//Will need to populate gate info too
		readTimeFile( openFile(time_filename), wcpdfs, gcpdfs );
		finalizeData(wcpdfs, gcpdfs);
		//*******************************************************
	}
	cout << "Completed reading input files." << endl;
}

//Open file return ifstream
ifstream & openFile(string filename)
{
	//Try to open the file
	ifstream * inputFile = new ifstream(filename.c_str());
	//Suceeded in opening?
	if ( !inputFile->is_open() ) 
	{
	  cout << "Could not open file: " << filename << endl;
	  exit(-1);
	}
	return *inputFile;
}

void readCellLibraryFile(ifstream & inputFile, cell_library & cl)
{
	//Temp variable to use to copy into vectors
	cell_cpdf * tmp;
	//Temp elements from splitting strings
	vector<string> elems;
	
	//Loop through each line in the file
	for(string line; getline(inputFile, line); ) 
	{
		//If the line starts with: GATE:
		if(line.find("GATE:") == 0)
		{
			//This line starts a new cell template
			//Allocate a new one
			tmp = new cell_cpdf();
			
			//These lines are split by space
			//Look like GATE: NAND1
			elems = superSplit(line," ");
			//Second element should be gate string
			tmp->gate = elems[1];
		}
		else if(line.find("OP:") == 0)
		{
			elems = superSplit(line," ");
			//Second element should be op string
			tmp->op = elems[1];
		}
		else if(line.find("COST:") == 0)
		{
			elems = superSplit(line," ");
			//Second element should be cost string
			tmp->cost = atof(elems[1].c_str());
		}
		else if(line.find("DELAY:") == 0)
		{
			//Last information for this gate is delay coeffs
			//a[4]; //Coeffs
			//DELAY: 7 1.4 2.8 4.1
			elems = superSplit(line," ");
			//Element index 1 is a[0] ... and so on
			tmp->a[0] = atof(elems[1].c_str());
			tmp->a[1] = atof(elems[2].c_str());
			tmp->a[2] = atof(elems[3].c_str());
			tmp->a[3] = atof(elems[4].c_str());
			
			//All info in, calc read often values
			tmp->calc_read_often_values();
			
			//cout << "Found " << tmp->gate << " cost_delay metric: " << tmp->cost_delay << endl;
			
			//Add tmp to appropriate vector in cell library
			if(tmp->op == "NAND")
			{
				cl.nands.push_back(*tmp);
			}
			else if(tmp->op == "AND")
			{
				cl.ands.push_back(*tmp);
			}
			else if(tmp->op == "NOR")
			{
				cl.nors.push_back(*tmp);
			}
			else if(tmp->op == "OR")
			{
				cl.ors.push_back(*tmp);
			}
			else if(tmp->op == "XOR")
			{
				cl.xors.push_back(*tmp);
			}
			else if(tmp->op == "XNOR")
			{
				cl.xnors.push_back(*tmp);
			}
			else if(tmp->op == "BUFF")
			{
				cl.buffs.push_back(*tmp);
			}
			else if(tmp->op == "NOT")
			{
				cl.nots.push_back(*tmp);
			}
			else
			{
				cout << "Unrecognized OP string: " << tmp->op << endl;
				exit(-1);
			}

			//Deallocate since copied by value above
			delete tmp;
		}
	}
	
	//Set the best versions after reading the entire file
	cl.best_nand = get_best_verion(cl.nands);
	cl.best_and = get_best_verion(cl.ands);
	cl.best_nor = get_best_verion(cl.nors);
	cl.best_or = get_best_verion(cl.ors);
	cl.best_xor = get_best_verion(cl.xors);
	cl.best_xnor = get_best_verion(cl.xnors);
	cl.best_buff = get_best_verion(cl.buffs);
	cl.best_not = get_best_verion(cl.nots);
	
	//Quick check for errors, eh
	if(cl.best_nand == NULL || cl.best_and == NULL || 
	cl.best_or == NULL || cl.best_nor == NULL || cl.best_xor == NULL
	|| cl.best_buff == NULL || cl.best_not == NULL)
	{
		cout << "Error: Unable to select and initial best gate." << endl;
		exit(-1);
	}
	
	cl.gate_type_to_cell_choices["NAND"] = &(cl.nands);
	cl.gate_type_to_cell_choices["AND"] = &(cl.ands);
	cl.gate_type_to_cell_choices["NOR"] = &(cl.nors);
	cl.gate_type_to_cell_choices["OR"] = &(cl.ors);
	cl.gate_type_to_cell_choices["XOR"] = &(cl.xors);
	cl.gate_type_to_cell_choices["XNOR"] = &(cl.xnors);
	cl.gate_type_to_cell_choices["BUFF"] = &(cl.buffs);
	cl.gate_type_to_cell_choices["NOT"] = &(cl.nots);
}


cell_cpdf * get_best_verion(vector<cell_cpdf>  & ccpdfs)
{
	//Loop through this lists of cells and find the best one
	//Loop through all the gates in this type of gate
	int list_size = ccpdfs.size();
	//Store max gate
	cell_cpdf * best_version = NULL;
	float best_cost_delay = -99999; //Want large cost_delay metric
	for(int i = 0; i < list_size; ++i)
	{
		//Best version uses gate_delay not arrival time delay
		if( ccpdfs[i].cost_gate_delay > best_cost_delay)
		{
			best_cost_delay = ccpdfs[i].cost_gate_delay;
			best_version = &(ccpdfs[i]);
		}
	}
    cout << "Gate init: found best version of " << best_version->op << " -> " << best_version->gate << endl;
	return best_version;
}


void readBenchFile(ifstream & inputFile,vector<gate_cpdf> & gcpdfs )
{
	//Temp variable to use to copy into vectors
	gate_cpdf * tmp_gcpdf;
	//Temp vector
	vector<string> tmp;
	int tmp_size;
	
	//Loop through each line in the file
	//Bench files will populate gate_cpdf objects
	for(string line; getline(inputFile, line); ) 
	{
		//If the line starts with
		if(line.find("INPUT(") == 0)
		{
			//Make a gate with no inputs
			tmp = superSplit(line,"()");
			//tmp[1] is name
			tmp_gcpdf = new gate_cpdf();
			tmp_gcpdf->name = tmp[1];
			//Copy tmp into vector by val
			gcpdfs.push_back(*tmp_gcpdf);
			//Then deallocate
			delete tmp_gcpdf;
		}
		else if(line.find("OUTPUT(") == 0)
		{
			//Not sure what to do here quite yet
		}
		else if(line.find("#") == 0)
		{
			//Comment line - do nothing
		}
		else if(line.find("G") == 0)
		{
			//Should be a "G10 = NAND(G1, G3)" style line
			//Allocate a gate_cpdf to be copied into vector
			tmp_gcpdf = new gate_cpdf();
			//Split
			tmp = superSplit(line, " =(,)");
			tmp_size = tmp.size();
			//Each line in tmp vector now contains G10,NAND,G1,G3
			//Parse this style element by element
			//All we can do now is store the name (output)
			//And the OP
			//Cannot map pointers to inputs or outputs
			//Elem 0, should be name
			tmp_gcpdf->name = tmp[0];
			//Elem 1 should be op
			tmp_gcpdf->op = tmp[1];	
			
			//Copy tmp into vector by val
			gcpdfs.push_back(*tmp_gcpdf);
			//Then deallocate
			delete tmp_gcpdf;
		}
		else
		{
			//Blank line - do nothin' yo
		}
	}
}

void readTimeFile(ifstream & inputFile,vector<wire_cpdf> & wcpdfs, vector<gate_cpdf> & gcpdfs )
{
	//Temp variable to use to copy into vectors
	wire_cpdf * tmp_wcpdf;
	//Temp vector
	vector<string> tmp;
	int tmp_size;
	//Temp index for indexing into vector
	int index;
	
	//Loop through each line in the file
	for(string line; getline(inputFile, line); ) 
	{
		//If the line starts with
		if(line.find("#") == 0)
		{
			//Comment line - do nothing
		}
		else if(line.find("G") == 0)
		{
			//Should be a "G16	G23	8.7	0.6	1.7	1.1" style line
			//Split the line on space and tab
			tmp = superSplit(line, " \t");
			tmp_size = tmp.size();
			
			//These next few lines are silly
			//Allocate a new wire_cpdf in the vector
			wcpdfs.push_back(wire_cpdf());
			//Get the index of the new item
			index = wcpdfs.size() - 1;
			//Immediately get the ptr to this object where it lives
			tmp_wcpdf = &(wcpdfs[index]);
			
			//Number of gates in the gates list
			int num_gates = gcpdfs.size();
			
			//This is the terribly slow process on matching wire<->gate
			//Only done once so ok...for now.
			//The first two elements are gate names
			for(int i = 0; i < 2; ++i)
			{
				//Find the gate_cpdf with this name: tmp[i]
				//Loop through the list of gates
				for(int j = 0; j < num_gates; ++j)
				{
					//If this gate name matches the name here
					if(gcpdfs[j].name == tmp[i])
					{
						//Make the gate have info about this wire and
						//make wire have info for this gate
						//If this is first gate
						if(i==0)
						{
							//Can't add pointer to tmp_wcpdf here
							//since that value could change as the
							//vector grows and moves and is relocated
							//across memory spaces
							//Cant do:
							//Add this wire as output of first list gate
							//gcpdfs[j].output = tmp_wcpdf;
							
							//Can do because 
							//gcpdfs is static at this point
							//Add this as start for this wire
							tmp_wcpdf->start_gate_index = j;
						}
						else if(i==1)
						{
							//Cant do
							//And as input to second listed gate
							//gcpdfs[j].inputs.push_back(tmp_wcpdf);
							
							//Can do because 
							//gcpdfs is static at this point
							//Add this as end for this wire
							tmp_wcpdf->end_gate_index =  j;
						}
					}
				}
			}
			
			//Elements 2-> end should be delay values
			//Element 2 is a[0] and so on...
			tmp_wcpdf->a[0] = atof(tmp[2].c_str());
			tmp_wcpdf->a[1] = atof(tmp[3].c_str());
			tmp_wcpdf->a[2] = atof(tmp[4].c_str());
			tmp_wcpdf->a[3] = atof(tmp[5].c_str());
		}
		else
		{
			//Blank line
		}
	}
}

//Function to loop and do the operations not able to be done
//in the readTimeFile function
//Ex.
//gcpdfs[j].inputs.push_back(tmp_wcpdf);
//gcpdfs[j].output = tmp_wcpdf;
void finalizeData(vector<wire_cpdf> & wcpdfs, vector<gate_cpdf> & gcpdfs)
{
	//Loop through all wires and link gates to them
	//Loop over all wires
	int num_wires = wcpdfs.size();
	int num_gates = gcpdfs.size();
	for(int i = 0; i < num_wires; ++i)
	{
		//For each wire loop across all the gates
		//And find the gate this wire is mentions
		for(int j = 0; j < num_gates; ++j)
		{	
			gcpdfs[j].global_index = j;
			//if this gate is the start of this wire
			if(  j == wcpdfs[i].start_gate_index )
			{
				//then this gate has this wire as an output
				gcpdfs[j].outputs.push_back( &(wcpdfs[i]) );
			}
			//if this gate is the end of this wire
			else if( j == wcpdfs[i].end_gate_index )
			{
				//The this gate has this wire an an input
				gcpdfs[j].inputs.push_back( &(wcpdfs[i]) );
			}
		}
	}

	//Final loop to store inputs and outputs, populate map, get downstream nodes
	//For each gate
	num_gates = gcpdfs.size();
	for(int i = 0; i < num_gates; i++)
	{
		if(gcpdfs[i].inputs.size() == 0)
		{
			//This is an input
			//Set default to having valid arrival times
			gcpdfs[i].valid_output_arrival_times = true;
			input_indices.push_back(i);
		}
        else
        {
            //Populate the map of gate type to index
            map<string, vector<unsigned int> >::iterator it;
            if( (it = gate_type_to_indices.find(gcpdfs[i].op)) == 
                    gate_type_to_indices.end())
            {
                //Did not find in map
                //Check if not empty
                if(gcpdfs[i].op != "")
                {			
					gate_type_to_indices[gcpdfs[i].op] = vector<unsigned int>();
					
					//Also push onto list of gate names as this is first time
					//Seeing the name
					gate_types.push_back(gcpdfs[i].op);
				}

            }
            //Now valid entry, push back index
            gate_type_to_indices[gcpdfs[i].op].push_back(i);
            
            if(gcpdfs[i].outputs.size() == 0)
            {
                //This is an primary output
                output_indices.push_back(i);
            }
            
            //Get the downstream gates
            gcpdfs[i].downstream_gates = count_downstream_gates(&(gcpdfs[i]));
            //cout << "number of downstream gates for " << gcpdfs[i].name << ": " << gcpdfs[i].downstream_gates << endl;
        }
	}
	
	//Sort gate types by importance
	sort_gate_types_by_importance();
}

unsigned int count_downstream_gates(gate_cpdf * g)
{
    //Clear all "counted" marks
	int num_gates = gate_cpdfs.size();
	for(int i = 0; i < num_gates; i++)
	{
        gate_cpdfs[i].counted = false;
    }

    //Find number of downstream gates
	return help_count_downstream_gates(g) - 1;
}

unsigned int help_count_downstream_gates(gate_cpdf * g)
{
    if (g->counted)
    {
        return 0;
    }

    int num_outputs = g->outputs.size();

    if (num_outputs == 0) 
    {
        // we are at an output
        g->counted = true;
        return 1;
    }

    int count = 0;

    for(int i = 0; i < num_outputs; i++)
    {
        count += help_count_downstream_gates(&(gate_cpdfs[g->outputs[i]->end_gate_index]));
    }

    g->counted = true;

    return count + 1;
}

bool gate_importance_compare(string g1, string g2)
{
	//Look up importance map to sort
	return (gate_type_to_importance[g1] < gate_type_to_importance[g2]);
}

void sort_gate_types_by_importance()
{
	//Populate the importance map
	//Loop over gate types
	int num_gate_types = gate_types.size();
	for(int i = 0; i < num_gate_types; ++i)
	{
		gate_type_to_importance[gate_types[i]] = get_gate_importance(gate_types[i]);
	}
	
	//Do sort using custom function
	sort(gate_types.begin(), gate_types.end(), gate_importance_compare);

}

float get_gate_importance(string type)
{
    int num_components = gate_type_to_indices[type].size();

    vector<cell_cpdf> * gate_choices = 
        the_cell_library.gate_type_to_cell_choices[type];

    //Aggregate the costs and delays
    vector<float> costs;
    vector<float> delays; 

    vector<cell_cpdf>::iterator it;
    for(it = gate_choices->begin(); it != gate_choices->end(); ++it)
    {
        costs.push_back(it->cost);
        delays.push_back(hm.delay_metric(&(*it)));
    }

    //Compute the variance of prices
    float cost_variance = variance(costs); 
    //Compute the variance of delays
    float delay_variance = variance(delays);

	//Importance is # of components * var of costs * var of delays
	return num_components * cost_variance * delay_variance;
}

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}
std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    return split(s, delim, elems);
}

vector<string> superSplit(string s, string delims)
{
	//Start with vector of just one element
	vector<string> current;
	current.push_back(s);
	
	//For each delim in delims
	int delims_size = delims.length();
	for(int i =0; i < delims_size; ++i)
	{
		//Split the entire current vector on single delim
		current = superSplitHelper(current, delims[i]);
	}

	return current;
}

vector<string> superSplitHelper(vector<string> & elems, char delim)
{
	//Take every string in the vector and split
	vector< vector<string> > results;
	//Return val
	vector<string> rv;
	
	int elems_size = elems.size();
	for(int i = 0; i < elems_size; ++i)
	{
		results.push_back( split(elems[i],delim));
	}
	
	int results_size = results.size();
	int inner_results_size;
	//Make the 2d vector into one long 1d vector
	//For each inner vector inside results
	for(int i = 0 ; i< results_size; ++i)
	{
		inner_results_size = results[i].size();
		//For each string inside the inner vector
		for(int j = 0; j < inner_results_size; ++j)
		{
			//Append this string to rv
			rv.push_back( (results[i])[j] );
		}
	}
	
	return rv;
}

