#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void toLine(FILE *file,int line_number) //positions a given file pointer to a given line number in the file
{
	rewind(file);

	char buffer[30];
	int current_line = 0;
	//skip lines until the desired line (i.e. TLBI) is reached
	while(current_line<line_number)
	{
		if(fgets(buffer,30,file) == NULL) //get the line and advance file pointer, if NULL then EOF was reached
		{
			printf("error, reached end-of-file");
			exit(0);
		}

		current_line++;
	}
}

int queryTLB(int input,FILE *tlb) //query the TLB for a valid address or return -1
{
	int VPN = input >> 11; //get the highest 7 bits from the input (arithmetic shift = logical shift when input >= 0)
	int VPO = input & 2047; //get the lower 11 bits from the input

	int TLBT = VPN >> 3; //TLB tag = 4 highest order bits of VPN
	int TLBI = VPN & 7; //TLB index = 3 lowest order bits of VPN


	toLine(tlb,TLBI); //jump to TLB index

	int valid,tag,ppn,i;

	//check each entry in the index for validity and tag match
	for(i=0;i<2;i++)
	{
		fscanf(tlb,"%d %d %d",&valid,&tag,&ppn); //read the 'valid' bit, tag, and PPN from the page table

		if(valid && tag==TLBT) //if address is valid and tag matches then return new physical address
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

int queryPT(int input,FILE *pt) //query the page table and return an address
{
	int VPN = input >> 11; //get the highest 7 bits from the input (arithmetic shift = logical shift when input >= 0)
	int VPO = input & 2047; //get the lower 11 bits from the input

	toLine(pt,VPN); //jump to page table index

	int valid,ppn,i;

	fscanf(pt,"%d %d",&valid,&ppn); //read the 'valid' bit and PPN from the page table
	if(valid)
	{
		printf("\nFrom page table - "); //if a valid address, then return physical address
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
		//load input files
		strcpy(argument,argv[1]);
		strcat(argument,".tlb");
		tlb = fopen(argument,"r");

		strcpy(argument,argv[1]);
		strcat(argument,".pt");
		pt = fopen(argument,"r");

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

	//main program loop, exit when user enters -1
	while(1)
	{
		//read in virtual address to query from user
		printf("\nEnter Virtual address in decimal (-1 to exit): ");
		scanf("%d",&input);
		if(input==-1) //break from loop/end program if user enters -1
			break;

		if(input>>18 == 0) //if more than 18 bits, illegal virtual address
		{
			//display hex value of input
			printf("virtual: 0x%x",input);

			//query TLB, if result is -1 then address wasn't cached, so then do page walk, else keep original result
			output = ((output=queryTLB(input,tlb))==-1) ? queryPT(input,pt) : output;
			if(output==-1) //if result is -1 from queryTLB and queryPT then page fault
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
	}
	return 0;
}