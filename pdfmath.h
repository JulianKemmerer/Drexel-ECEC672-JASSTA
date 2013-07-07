#ifndef _pdfmath_
#define _pdfmath_

#define _USE_MATH_DEFINES
#include <iostream>
#include <cmath>
#include "cpdf.h"
#include <vector>
using namespace std;

//Mutual dependency
//#pragma message "cell_cpdf incomplete"
//class cell_cpdf;
//#pragma message "gate_cpdf incomplete" 
//class gate_cpdf;
//#pragma message "wire_cpdf incomplete" 
//class wire_cpdf;

class cpdf;

static float SMALLPHI_COEFF = 1 / sqrt(2 * M_PI);

// protypes for pdf math
cpdf max(cpdf & a, cpdf & b);
cpdf min(cpdf & a, cpdf & b);
float sigma(cpdf & a);
float rho(cpdf &a, cpdf &b, float sigmaA, float sigmaB);
float theta(float sigmaA, float sigmaB, float rho);
float calc_x(cpdf &a, cpdf &b, float theta);
float bigphi(float y);
float smallphi(float x);
float maxmean(cpdf &a, cpdf &b, float bigphi, float theta, float smallphi);
float minmean(cpdf &a, cpdf &b, float bigphi, float theta, float smallphi);
float maxvariance(   
        cpdf & a, 
        cpdf & b, 
        float sigmaA, 
        float sigmaB, 
        float theta, 
        float bigphi, 
        float smallphi,
        float mean
    );
float minvariance(   
        cpdf & a, 
        cpdf & b, 
        float sigmaA, 
        float sigmaB, 
        float theta, 
        float bigphi, 
        float smallphi,
        float mean
    );
cpdf multi_max(vector<cpdf> & to_max);
cpdf multi_min(vector<cpdf> & to_min);

//float variance
float variance(vector<float> values);

// prototypes for test functions
void test_math();

#endif
