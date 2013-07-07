#include <iostream>
#include "cpdf.h"

using namespace std;

cpdf::cpdf()
{
	//Init
	a[0] = 0;
	a[1] = 0;
	a[2] = 0;
	a[3] = 0;
	stddev = 0;	
}
//Plus overload
cpdf cpdf::operator+ (cpdf & right)
{
	//Same letters from lectures
	cpdf * c = this;
	cpdf * d = &right;
	//Return val
	cpdf a;
	
	//Add params 0 through 2
	a.a[0] = c->a[0] + d->a[0];
	a.a[1] = c->a[1] + d->a[1];
	a.a[2] = c->a[2] + d->a[2];
	
	//Some maths for param 3
	a.a[3] = sqrt( (c->a[3]*c->a[3]) + (d->a[3]*d->a[3]) );
	
	//All done!
	return a;
}

//Print overload
ostream& operator<<(ostream& out, cpdf& c)
{
	out << c.a[0] << PRINT_DELIM << c.a[1] << PRINT_DELIM << 
		c.a[2] << PRINT_DELIM << c.a[3];
	return out;
}

void cpdf::calc_stddev()
{
	//use external function
	stddev = sigma(*this);
}
