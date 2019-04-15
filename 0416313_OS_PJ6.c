#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>

#define tlb_size 16
#define page_size 256

int check_tlb(FILE *fp_in, FILE *fp_out, int input);
int check_page(FILE *fp_in, FILE *fp_out, int input);
int check_store(FILE *fp_in, FILE *fp_out, int input);
void initial();

FILE *fptr;
int page_hit, page_falut;
int tlb_hit, tlb_miss;
int tlb[15] = {0};
int page_tab[256] = {0};
int h_addr_now;
int l_addr_now;
int tlb_index;
unsigned char physic_index[5];
int current;
int addr;

int check_tlb(FILE *fp_in, FILE *fp_out, int input){
	for(int i = 0;i < tlb_size;i++){
		if((tlb[i] >> 8) == input){
			tlb_hit++;
			h_addr_now = (tlb[i] % 256);
			current = (h_addr_now << 8) + l_addr_now;
			return 1;
		}
	}
	
	check_page(fp_in, fp_out, input);
	
	tlb_miss++;							//FIFO
	tlb_index++;
	if(tlb_index >= tlb_size)
		tlb_index = 0;
	
	return 0;
}

int check_page(FILE *fp_in, FILE *fp_out, int input){
	if(page_tab[(input)] != 0){
		tlb[tlb_index] = (input << 8) + page_tab[(input)];
		page_hit++;
		current = (page_tab[(input)] << 8) + l_addr_now;
		return 1;
	}
	
	check_store(fp_in, fp_out, input);
	page_falut++;
	
	return 0;
}

int check_store(FILE *fp_in, FILE *fp_out, int input){
	fptr = fopen("BACKING_STORE.bin", "rb");

	fseek(fptr, input, SEEK_SET);
	fread(physic_index, sizeof(physic_index), 1, fptr);
	
	tlb[tlb_index] = (input << 8) + addr;
	page_tab[input] = addr;
	
	current = (addr << 8) + l_addr_now;
	addr++;
	printf("*******%s******\n", &physic_index);
	
	fclose(fptr);
	return 0;
}

void initial(){
	tlb_hit = 0;
	tlb_miss = 0;
	page_hit = 0;
	page_falut = 0;
	h_addr_now = 0;
	l_addr_now = 0;
	tlb_index = 0;
	addr = 0;
}

int main(int argc, char **argv) {
	float page_rate, tlb_rate; 
	if( argc != 2 ) {
		fprintf(  stdout,  "Usage:  ./parser  [filename]\n"  );
		return 0;
	}
	
	FILE *fp_out1 = fopen("0416313_address.txt", "w");
	FILE *fp_out2 = fopen("0416313_value.txt", "w");
	FILE *fp_in = fopen( argv[1], "r" );

	if( fp_in == NULL )  {
		fprintf( stdout, "Open  file  error\n" );
		return 0;
	}
	initial();
	
	int index = 0;
	while(feof(fp_in) == 0){
		fscanf(fp_in, "%d", &index);
		
		l_addr_now = index % 256;
		check_tlb(fp_in, fp_out1, (index >> 8));
		
		fprintf(fp_out1, "%d\n", current);
		fprintf(fp_out2, "%d\n", current);
	}
	
	printf("Page-faults: %d\n", page_falut);
	page_rate = page_falut*100/(page_falut + page_hit);
	printf("Page-fault rate: %f%\n", page_rate);
	printf("TLB hit times: %d\n", tlb_hit);
	tlb_rate = (tlb_hit*100/(tlb_hit + tlb_miss));
	printf("TLB hit rate: %f%\n", tlb_rate);
	
	fclose(fp_in);
	return 0;
}