#include <stdio.h>

extern "C" int plus(int m, int n);

int plus(int m, int n){
	int result = m + m;
	
	/* TODO: generates a Span here... */
	
	printf("C++ called!! result=%d", result);
	fflush(stdout);
	
    return m + n;
}
