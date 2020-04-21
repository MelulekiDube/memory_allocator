#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

typedef struct temp{
	char *val;
	int d;
}temp;

typedef temp* temp_ptr;

void* function(){
	temp_ptr tp = malloc(5);
	tp->d  =1;
	return tp;
}

int main(){
	void* tp = function();
	
	printf("temp_ptr value is   %p\n", tp);
	
	printf("temp_ptr+1 value is %p\n", (tp+1));
	
	//printf("*temp_ptr+1 is: %i\n", (*(tp+1)));
	
	return 0;
}