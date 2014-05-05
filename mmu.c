#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void toLine(FILE *file,int line_number)
{
	rewind(file);

	char buffer[30];
	int current_line = 0;
	//do //skip lines until the desired line (i.e. TLBI) is reached
	while(current_line<line_number)
	{
		if(fgets(buffer,30,file) == NULL)
		{
			printf("error, reached end-of-file");
			exit(0);
		}
		//printf("%s",buffer);
		current_line++;
	}// while(current_line<line_number);
}

int queryTLB(int input,FILE *tlb) //returns a valid address or -1
{
	int VPN = input >> 11; //get the highest 7 bits from the input (arithmetic shift = logical shift when input >= 0)
	int VPO = input & 2047; //get the lower 11 bits from the input

	int TLBT = VPN >> 3; //TLB tag = 4 highest order bits of VPN
	int TLBI = VPN & 7; //TLB index = 3 lowest order bits of VPN

	//toLine(tlb,TLBI,buffer); //jump to TLB index
	toLine(tlb,TLBI); //jump to TLB index

	int valid,tag,ppn,i;

	//check each entry in the index for validity and tag match
	for(i=0;i<2;i++)
	{
		fscanf(tlb,"%d %d %d",&valid,&tag,&ppn);

		if(valid && tag==TLBT)
		{
			//TLB hit, concat PPN with VPO and return the result
			printf("\nFrom TLB - ");
			return (ppn << 11) | VPO;
			
		} else if(i==1)
		{
			return -1;
		}
	}
}

int queryPT(int input,FILE *pt)
{
	int VPN = input >> 11; //get the highest 7 bits from the input (arithmetic shift = logical shift when input >= 0)
	int VPO = input & 2047; //get the lower 11 bits from the input

	toLine(pt,VPN); //jump to page table index

	int valid,ppn,i;

	fscanf(pt,"%d %d",&valid,&ppn);
	if(valid)
	{
		printf("\nFrom page table - ");
		return (ppn << 11) | VPO;
	}
	return -1;
}

int main(int argc, char** argv)
{
	//declare input file pointers
	FILE *tlb,*pt; //TLB and page table

	char argument[80];
	if(argc == 2) //check for enough arguments
	{
		//check for correct command line input, e.g. "run test.tlb test.pt"
		//check extensions in correct order w/ regex

		//load input files
		strcpy(argument,argv[1]);
		strcat(argument,".tlb");
		tlb = fopen(argument,"r");

		strcpy(argument,argv[1]);
		strcat(argument,".pt");
		pt = fopen(argument,"r");
		//tlb = fopen(argv[1],"r");
		//pt = fopen(argv[2],"r");
	} else
	{
		printf("usage: %s <filename>.tlb <filename>.pt\n",argv[0]);
		exit(2);
	}
	if(tlb == NULL || pt == NULL)
	{
		printf("fileread failed");
		exit(3);
	}
	//filereading preparations finished


	int input = 0, output = 0;
	//char buffer[30];
	while(1) //main program loop, exit when user enters -1
	{
		//read in virtual address to query from user
		printf("\nEnter Virtual address in decimal (-1 to exit): ");
		scanf("%d",&input);
		if(input==-1) //exit condition
			break;
		if(input>>18 == 0) //if more than 18 bits, illegal virtual address
		{
			//display hex value of input
			printf("virtual: 0x%x",input);

			output = ((output=queryTLB(input,tlb))==-1) ? queryPT(input,pt) : output;
			if(output==-1)
			{
				printf("\npage fault");
			} else
			{
				printf("Physical address: 0x%x (%d)",output,output);
			}
		} else
		{
			printf("Illegal virtual address");
		}
/*
		//**extract VPN from input
		//shift right 11-bits, can use arithmetic shift operator since operand should be positive
		VPN = input >> 11;
		VPO = input & 2047;
		//extract TLBT and TLBI from VPN
		TLBT = VPN >> 3;
		TLBI = VPN & 7;
		//printf("\nVPN:%d\nTLBT:%d\nTLBI:%d\n",VPN,TLBT,TLBI);

		//****check .tlb line# TLBI for TLBT
		int line_number = 0;
		do //skip lines until the desired line (i.e. TLBI) is reached
		{
			if(fgets(buffer,30,tlb) == NULL)
			{
				printf("error, reached end-of-file");
				exit(0);
			}
			//printf("%s",buffer);
			line_number++;
		} while(line_number<TLBI);

		//the correct line has been obtained, parse for matches with TLBT
		int valid,tag,ppn;
		int i;
		for(i=0;i<2;i++)
		{
			//printf("i = %d",i);
			fscanf(tlb,"%d %d %d",&valid,&tag,&ppn);
			//printf("\nvalid = %d\ntag = %d\nTLBT = %d",valid,tag,TLBT);
			if(valid)
			{
				if(tag==TLBT)
				{
					//TLB hit, concat PPN with VPO
					output = (ppn << 11) | VPO;
					printf("\nPhysical Address: 0x%x (%d) - from the TLB",output,output);
					break;
				}
			} else
			{
				break;
			}
		}

		rewind(tlb);
		rewind(pt);
		*/
	}
	return 0;
}