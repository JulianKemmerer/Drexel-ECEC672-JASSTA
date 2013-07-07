#include <iostream>
#include "cell_cpdf.h"
#include "heuristics_metrics.h"

using namespace std;

cell_cpdf::cell_cpdf(): cpdf()
{
	cost = 0;
	cost_gate_delay = -1;
	stddev = -1;
}

void cell_cpdf::calc_cost_gate_delay_metric()
{
	//Use the external defintions of the metric
	//Want cost delay metric to be large (better)
	cost_gate_delay = hm.cost_gate_delay_metric(this);
}

void cell_cpdf::calc_read_often_values()
{
	//cost delay depends on stddev so do that first
	calc_stddev();
	calc_cost_gate_delay_metric();
}
