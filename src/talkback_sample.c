/*
 * talkback.c
 *
 *  Talkback interface to the olvm
 *  Created on: Apr 13, 2017
 *      Author: uri
 */

#if EMBEDDED_VM
#include <stdio.h>

void* OL_tb_start();
void OL_tb_stop(void* oltb);
int OL_tb_eval(void* oltb, char* program, char* out, int size);


int main(int argc, char** argv)
{
	void* oltb = OL_tb_start();

	char output[1024];
	printf("%d: ", OL_tb_eval(oltb, "(fold * 1 (iota 99 1 1))", output, sizeof(output)));
	printf("%s\n", output);

	OL_tb_stop(oltb);
	return 0;
}

#endif
