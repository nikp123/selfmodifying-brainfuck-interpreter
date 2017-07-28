#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#ifdef __unix__
	#include <termios.h>
	#include <unistd.h>
#endif

#define u_char unsigned char
#define u_long unsigned long

u_char *mem;
u_long mem_siz = 64000, exec_siz = 0, memptr = 0;

void printHelp(char *executableName)
{
	printf("Usage:\n");
	printf("\t%s filename --option value\n", executableName);
	printf("Options:\n");
	printf("\t--mem (number in bytes)\n");
	printf("\t--help (displays this page)\n");
	return;
}

int loadFile(char *filename)
{
	// load the file and check for errors
	FILE *file = fopen(filename, "rb");
	if(file == NULL)
	{
		fprintf(stderr, "Error loading input file!\n");
		return 1;
	}
	
	// get the file size so we can allocate our executable buffer
	fseek(file, 0L, SEEK_END);	// jumps to the end of the file
	exec_siz = ftell(file);		// gets the file pointer
	memptr = exec_siz;
	fseek(file, 0L, SEEK_SET);	// jumps back to the begining of the file
	
	// check for errors
	if(mem_siz <= exec_siz)
	{
		fprintf(stderr, "Memory must be larger than the executable!\n");
		return 1;
	}
	
	// allocating instruction data buffer
	mem = (u_char*)malloc(sizeof(u_char)*(mem_siz+1));
	if(mem == NULL)
	{
		fprintf(stderr, "Memory error!\n");
		return 1;
	}
	
	// clear memory for safety
	memset(mem, 0x00, mem_siz);
	
	// reading instruction buffer
	if(fread(mem, sizeof(char), exec_siz, file) != exec_siz)
	{
		fprintf(stderr, "Something has gone terribly wrong!\n");
		return 1;
	}
	
	// free the file
	fclose(file);
	
	return 0;
}

int interpret(void)
{	
	#ifdef DEBUG
		// print greeting message
		printf("Successfully started Brainfuck interpreter with %lu bytes of working memory.\n", mem_siz);
	#endif
	
	emergency_jump:
	// actual interpreter code here
	for(u_long i = 0; i < exec_siz; i++)
	{
		switch(mem[i])
		{	
			// simulate an overflow/underflow effect
			case '>':
				memptr++;
				break;
			case '<':
				memptr--;
				break;
			
			// Increments and decrements
			case '+':
				mem[memptr]++;
				break;
			case '-':
				mem[memptr]--;
				break;
			
			// User input/output
			case '.':
				putchar(mem[memptr]);
				break;
			case ',':
			{
				#ifdef __unix__
					static struct termios oldt, newt;
					tcgetattr( STDIN_FILENO, &oldt);
					newt = oldt;
					newt.c_lflag &= ~(ICANON);
					tcsetattr( STDIN_FILENO, TCSANOW, &newt);
				#endif
				mem[memptr] = getchar();
				#ifdef __unix__
					tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
				#endif
				break;
			}
			
			// Loops
			case '[':
			{
				int nested = 1;
				if(mem[memptr])	break;				
				do
				{
					i++;					
					if(mem[i] == '[') nested++;
					else if(mem[i] == ']') nested--;
				} while(nested);
				break;
			}
			case ']':
			{	
				int nested = 0;
				do
				{
					if(mem[i] == '[') nested++;
					else if(mem[i] == ']') nested--;
					i--;
				} while(nested);
				break;
			}
			default: break;
		}
	}
	#ifdef DEBUG
	printf("Releasing resources!\n");
	#endif
	free(mem);
	return 0;
}

int main(int argc, char *argv[])
{
	// we ofcourse need a file to run
	if(argc < 2)
	{
		printHelp(argv[0]);
		return 1;
	}
	
	// find extra arguments
	for(int i = 2; i < argc; i++)
	{
		if(!strcmp("--mem", argv[i]))
		{
			if(i == argc - 1)
			{
				fprintf(stderr, "--mem needs a value!\n");
				return 1;
			}
			i++;
			if(sscanf(argv[i], "%u", &mem_siz) == 0)
			{
				fprintf(stderr,"Invalid value: %s", argv[i]);
				return 1;
			}
		}
		else if(!strcmp("--help", argv[i]))
		{
			printHelp(argv[0]);
			return 0;
		}
	}
	
	// load the file into memory
	if(loadFile(argv[1]))
	{
		fprintf(stderr, "Error loading file: %s\n", argv[1]);
		return 1;
	}
	
	// starts the interpreter
	if(interpret())
	#ifdef DEBUG
		printf("Execution has finished with errors!\n");
	else	printf("Execution has finished without errors!\n");
	#endif
	return 0;
}
