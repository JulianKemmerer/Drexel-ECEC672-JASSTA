#ifndef _cpdf_
#define _cpdf_

#include <iostream>
#include <cmath>
#include "pdfmath.h"
using namespace std;

#define PRINT_DELIM ' '

//Canonical prob density function class

class cpdf
{
  public:
	//Coeffs
	float a[4]; 
	//Constuctor
    cpdf();
    
    //Plus overload
	cpdf operator + (cpdf&);
	
	//Print overload
	friend ostream& operator<< (ostream& out, cpdf& c);
	
	//Std dev - only calculate once not each time
	float stddev;
	//Calc std dev
	void calc_stddev();
    
  private:
	
	
};




#endif
