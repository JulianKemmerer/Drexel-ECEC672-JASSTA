#include <iostream>
#include "pdfmath.h"

using namespace std;

void test_math() 
{
    cpdf a;
    a.a[0] = 10;
    a.a[1] = 1;
    a.a[2] = 2;
    a.a[3] = 3;

    cpdf b;
    b.a[0] = 15;
    b.a[1] = 2;
    b.a[2] = 3;
    b.a[3] = 2;

    float sigmaA = sigma(a);
    float sigmaB = sigma(b);
    float _rho = rho(a, b, sigmaA, sigmaB);
    float _theta = theta(sigmaA, sigmaB, _rho);
    float x = calc_x(a, b, _theta);
    float _bigphi = bigphi(x);
    float _smallphi = smallphi(x);
    float _maxmean = maxmean(a, b, _bigphi, _theta, _smallphi);
    float _maxvariance = maxvariance(   a, 
                                    b, 
                                    sigmaA, 
                                    sigmaB, 
                                    _theta, 
                                    _bigphi, 
                                    _smallphi,
                                    _maxmean
                                );
    float _minmean = minmean(a, b, _bigphi, _theta, _smallphi);
    float _minvariance = minvariance(   a, 
                                    b, 
                                    sigmaA, 
                                    sigmaB, 
                                    _theta, 
                                    _bigphi, 
                                    _smallphi,
                                    _minmean
                                );
    cpdf maxg = max(a, b);
    cpdf ming = min(a, b);

    cout << "sigmaA : " << sigmaA << endl;
    cout << "sigmaB : " << sigmaB << endl;
    cout << "rho : " << _rho << endl;
    cout << "theta : " << _theta << endl;
    cout << "x : " << x << endl;
    cout << "bigphi : " << _bigphi << endl;
    cout << "smallphi : " << _smallphi << endl;
    cout << "maxmean : " << _maxmean << endl;
    cout << "maxvariance : " << _maxvariance << endl;
    cout << "minmean : " << _minmean << endl;
    cout << "minvariance : " << _minvariance << endl;
    cout << "maxg[0] : " << maxg.a[0] << endl;
    cout << "maxg[1] : " << maxg.a[1] << endl;
    cout << "maxg[2] : " << maxg.a[2] << endl;
    cout << "maxg[3] : " << maxg.a[3] << endl;
    cout << "ming[0] : " << ming.a[0] << endl;
    cout << "ming[1] : " << ming.a[1] << endl;
    cout << "ming[2] : " << ming.a[2] << endl;
    cout << "ming[3] : " << ming.a[3] << endl;
}

//Implementations of pdf math functions
cpdf max(cpdf & a, cpdf & b)
{	
    float sigmaA = sigma(a);
    float sigmaB = sigma(b);
    float _rho = rho(a, b, sigmaA, sigmaB);
    float _theta = theta(sigmaA, sigmaB, _rho);
    float x = calc_x(a, b, _theta);
    float _bigphi = bigphi(x);
    float _smallphi = smallphi(x);
    float mean = maxmean(a, b, _bigphi, _theta, _smallphi);
    float variance = maxvariance(   a, 
                                    b, 
                                    sigmaA, 
                                    sigmaB, 
                                    _theta, 
                                    _bigphi, 
                                    _smallphi,
                                    mean
                                );
    cpdf g;
    //cout << "a " << a << endl;
    //cout << "b " << b << endl;
    g.a[0] = mean;
    g.a[1] = a.a[1] * _bigphi + b.a[1] * (1.0 - _bigphi);
    g.a[2] = a.a[2] * _bigphi + b.a[2] * (1.0 - _bigphi);
    float tmp = variance - (g.a[1] * g.a[1]) - (g.a[2] * g.a[2]);
    if( tmp < 0 )
    {
		cout << "Error: max function cannot sqrt a negative value! " << tmp << endl;
		exit(-1);
	}
    g.a[3] = sqrt(tmp);
	//cout << "g " << g << endl;
	return g;
}

cpdf min(cpdf & a, cpdf & b)
{
    float sigmaA = sigma(a);
    float sigmaB = sigma(b);
    float _rho = rho(a, b, sigmaA, sigmaB);
    float _theta = theta(sigmaA, sigmaB, _rho);
    float x = calc_x(a, b, _theta);
    float _bigphi = bigphi(x);
    float _smallphi = smallphi(x);
    float mean = minmean(a, b, _bigphi, _theta, _smallphi);
    float variance = minvariance(   a, 
                                    b, 
                                    sigmaA, 
                                    sigmaB, 
                                    _theta, 
                                    _bigphi, 
                                    _smallphi,
                                    mean
                                );
    cpdf g;
    g.a[0] = mean;
    g.a[1] = a.a[1] * (1.0 - _bigphi) + b.a[1] * _bigphi;
    g.a[2] = a.a[2] * (1.0 - _bigphi) + b.a[2] * _bigphi;
    g.a[3] = sqrt(variance - (g.a[1] * g.a[1]) - (g.a[2] * g.a[2]));
	return g;
}

float sigma(cpdf & a) //std dev
{
    // sigma is 
    // the square root
    // of the sum
    // of the squares
    // of the normal form coeffs
    // from a[1] to a[n+1]
    return sqrt(
        (a.a[1] * a.a[1]) + 
        (a.a[2] * a.a[2]) + 
        (a.a[3] * a.a[3])
    );
}

float rho(cpdf &a, cpdf &b, float sigmaA, float sigmaB)
{
    // rho is
    // the sum of
    // a[i] * b[i]
    // from i = 1 to n
    // all divided by
    // sigmaA * sigmaB
    return 
        ((a.a[1] * b.a[1]) + (a.a[2] * b.a[2])) /
        (sigmaA * sigmaB);
}

float theta(float sigmaA, float sigmaB, float rho)
{
    // theta is
    // the square root of
    // sigmaA squared plus
    // sigmaB squared minus
    // 2 * sigmaA * sigmaB * rho
    return sqrt(
        sigmaA * sigmaA +
        sigmaB * sigmaB -
        2 * sigmaA * sigmaB * rho
    );
}
    
float calc_x(cpdf &a, cpdf &b, float theta)
{
    // x is
    // a[0] - b[0]
    // divided by
    // theta
    return (a.a[0] - b.a[0]) / theta;
}

float bigphi(float y)
{
    // bigphi is
    // the area under a unit normal distribution
    // from -infinity
    // to y
    /*
    if (y < 0) {
        return erfc(abs(y));
    } else {
        return erf(y);
    }
    */ 
    
    if (y < 0) 
    {
        return 0.5*erfc(abs(y)/sqrt(2));
    } 
    else
    {
        return 0.5 + (0.5*erf(y / sqrt(2)));
    }
}

float smallphi(float x)
{
    // smallphi is
    // 1 divided by the square root of 2 * PI [SMALLPHI_COEFF]
    // times
    // e to the
    // -x squared
    // divided by 2
    return SMALLPHI_COEFF * exp(-(x * x)/2);
}
    
float maxmean(cpdf &a, cpdf &b, float bigphi, float theta, float smallphi)
{
    // maxmean is the expected value E 
    // of the max
    // of two probability distributions
    return  
        a.a[0] * bigphi +
        b.a[0] * (1.0 - bigphi) +
        theta * smallphi;
}
    
float minmean(cpdf &a, cpdf &b, float bigphi, float theta, float smallphi)
{
    // minmean is the expected value E 
    // of the min
    // of two probability distributions
    return  
        a.a[0] * (1.0 - bigphi) +
        b.a[0] * bigphi -
        theta * smallphi;
}

float maxvariance(   
        cpdf & a, 
        cpdf & b, 
        float sigmaA, 
        float sigmaB, 
        float theta, 
        float bigphi, 
        float smallphi,
        float mean
    )
{
    // maxvariance is the variance of the resulting pdf
    // when two pdfs are maxed

    return
        (sigmaA * sigmaA + a.a[0] * a.a[0]) * bigphi +
        (sigmaB * sigmaB + b.a[0] * b.a[0]) * (1.0 - bigphi) +
        (a.a[0] + b.a[0]) * theta * smallphi -
        mean * mean;
}

float minvariance(   
        cpdf & a, 
        cpdf & b, 
        float sigmaA, 
        float sigmaB, 
        float theta, 
        float bigphi, 
        float smallphi,
        float mean
    )
{
    // minvariance is the variance of the resulting pdf
    // when two pdfs are minned

    return
        (sigmaA * sigmaA + a.a[0] * a.a[0]) * (1.0 - bigphi) +
        (sigmaB * sigmaB + b.a[0] * b.a[0]) * bigphi -
        (a.a[0] + b.a[0]) * theta * smallphi -
        mean * mean;
}

cpdf multi_max(vector<cpdf> & to_max)
{
	//Return val
	cpdf rv;
	
	//Vec size
	int vec_size = to_max.size();
	
	//Special case for 1 input
	if(vec_size == 1)
	{
		rv = to_max[0];
		//cout << "Multi max 1 a0: " << rv.a[0] << endl;
		return rv;
	}
	
	//Loop through to_max
	//cout << "to max 0 a0: " << to_max[0].a[0] << endl;
	//cout << "to max 1 a0: " << to_max[1].a[0] << endl;
	rv = max(to_max[0], to_max[1]);
	//cout << "Multi max 2 a0: " << rv.a[0]<< endl;
	for(int i = 2; i < vec_size; ++i)
	{
		rv = max(rv, to_max[i]);
		//cout << "Multi max 3 a0: " << rv.a[0]<< endl;
	}
	return rv;
}

cpdf multi_min(vector<cpdf> & to_min)
{
	//Return val
	cpdf rv;
	
	//Vec size
	int vec_size = to_min.size();
	
	//Special case for 1 input
	if(vec_size == 1)
	{
		rv = to_min[0];
		return rv;
	}
	
	//Loop through to_max
	rv = min(to_min[0], to_min[1]);
	for(int i = 2; i < vec_size; ++i)
	{
		rv = min(rv, to_min[i]);
	}
	return rv;
}

float variance(vector<float> values)
{
    float total = 0;

    float num_values = values.size();
    for (int i = 0; i < num_values; i++)
    {
        total += values[i];
    }

    float mean = total / (float)num_values;

    float variance = 0;
    for (int i = 0; i < num_values; ++i)
    {
        variance += (values[i] - mean) * (values[i] - mean) / (float)num_values;
    }

    return variance;
}
