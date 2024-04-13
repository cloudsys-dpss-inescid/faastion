#include "Factorization.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_fACTORS 50

JNIEXPORT void JNICALL Java_Factorization_computeFactors
  (JNIEnv *, jobject){
	
	int num_factors[MAX_fACTORS];
	int count = 0;
	int number = 1000000;
    
	for (int i = 1; i <= number; i++) {
		if((number % i) == 0){
            num_factors[count++] = i;
		}
	}

	for(int i = 0;i < count;i++){
		printf("%d ",num_factors[i]);
	}
}
