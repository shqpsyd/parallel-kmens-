//refer to http://fastcpp.blogspot.com/2011/04/how-to-process-stl-vector-using-sse.html
#include <bits/stdc++.h>
#include <immintrin.h>
#include <x86intrin.h>
#include <iostream>
#define ROUND_DOWN(X,Y) ((X) & ~  ((Y) - 1))
using namespace std;
double distance(const vector<double> & v1, const vector<double> & v2) {
	const int N = v1.size();
	const double *p1 = N > 0 ? &v1.front() : NULL;
	const double *p2 = N > 0 ? &v2.front() : NULL;
	int i = 0;
	__m128d result = _mm_setzero_pd();
	//const __m128 two = _mm_setr_ps(2,2,2,2);
	//const __m128 two_ss = _mm_set_ss(2);
	__m128d temp = _mm_setzero_pd();
	for(; i < ROUND_DOWN(N,2); i += 2) {	
		temp = _mm_sub_pd(_mm_loadu_pd(p1 + i), _mm_loadu_pd(p2 + i));
		//temp = _mm_mul_ps(temp, temp);		
		result = _mm_add_pd(result, _mm_mul_pd(temp, temp));
	} 
	
	for(; i < N; i++) {
		temp = _mm_sub_sd(_mm_load_sd(p1 + i), _mm_load_sd(p2 + i));	
		//temp = _mm_mul_ss(temp,temp);
		result = _mm_add_sd(result,_mm_mul_sd(temp,temp));
	}
	result = _mm_hadd_pd(result,result);
	//result = _mm_hadd_pd(result,result);
	return _mm_cvtsd_f64(result);	
}
