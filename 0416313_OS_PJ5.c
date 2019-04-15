#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>

#define consumer_num 5
#define resource_num 3

struct {
  pthread_mutex_t mutex;
  int a;
  int b;
  int c;
  int unsafe;
} shared_resource;

typedef struct{
  int a;
  int b;
  int c;
}consume_source;

void *thread_runner(int id);
int release(int consumer_id, consume_source source);
int request(int consumer_id, consume_source source);
void print_state(int consumer_id, consume_source source, int code, int request);
void initial();

consume_source maximun[consumer_num];
consume_source allocation[consumer_num];
consume_source need[consumer_num];

int loop = 1;
int finish[consumer_num];

void *thread_runner(int id){
	int flag;
	consume_source allocate;
	
	while(1){
		allocate.a = rand()%(maximun[id].a+1);
		allocate.b = rand()%(maximun[id].b+1);
		allocate.c = rand()%(maximun[id].c+1);
		
		pthread_mutex_lock(&shared_resource.mutex);
		flag = request(id, allocate);
		print_state(id, allocate, flag, 1);
		pthread_mutex_unlock(&shared_resource.mutex);
		
		sched_yield();
		
		if(flag < 0 && flag != -3){
			allocate.a = rand()%(maximun[id].a+1);
			allocate.b = rand()%(maximun[id].b+1);
			allocate.c = rand()%(maximun[id].c+1);
			
			pthread_mutex_lock(&shared_resource.mutex);
			flag = release(id, allocate);
			print_state(id, allocate, flag, 0);
			pthread_mutex_unlock(&shared_resource.mutex);
			
			sched_yield();
		}
		else if(flag >= 2) {
			loop = 0;
			break;
		}
	}
	pthread_exit(0);
}

int release(int consumer_id, consume_source source){
	if(source.a > allocation[consumer_id].a | source.b > allocation[consumer_id].b | source.c > allocation[consumer_id].c){
		return -1;
	}
	else if(source.a == 0 && source.b == 0 && source.c == 0){
		return 0;
	}
	
	allocation[consumer_id].a -= source.a;
	allocation[consumer_id].b -= source.b;
	allocation[consumer_id].c -= source.c;
	
	need[consumer_id].a += source.a;
	need[consumer_id].b += source.b;
	need[consumer_id].c += source.c;
	
	shared_resource.a += source.a;
	shared_resource.b += source.b;
	shared_resource.c += source.c;
	
	return 1;
}

int request(int consumer_id, consume_source source){
	consume_source tmp;
	tmp.a = allocation[consumer_id].a + source.a;
	tmp.b = allocation[consumer_id].b + source.b;
	tmp.c = allocation[consumer_id].c + source.c;
							
	if(source.a > shared_resource.a | source.b > shared_resource.b | source.c > shared_resource.c){			//check error
		return -2;
	}
	else if(tmp.a > need[consumer_id].a | tmp.b > need[consumer_id].b | tmp.c > need[consumer_id].c){
		return -1;
	}
	else if(source.a == 0 && source.b == 0 && source.c == 0){
		return 0;
	}
	
	allocation[consumer_id].a = tmp.a;											//update data
	allocation[consumer_id].b = tmp.b;
	allocation[consumer_id].c = tmp.c;
	
	need[consumer_id].a = maximun[consumer_id].a - allocation[consumer_id].a;
	need[consumer_id].b = maximun[consumer_id].b - allocation[consumer_id].b;
	need[consumer_id].c = maximun[consumer_id].c - allocation[consumer_id].c;
	
	shared_resource.a -= source.a;
	shared_resource.b -= source.b;
	shared_resource.c -= source.c;
	
	for(int i = 0;i < consumer_num;i++){
		if(finish[i] == 0){
			if(need[i].a <= shared_resource.a && need[i].b <= shared_resource.b && need[i].c <= shared_resource.c)
				break;
		}
		if(i == consumer_num - 1)
			shared_resource.unsafe = 1;
	}

	if(allocation[consumer_id].a ==  maximun[consumer_id].a && 
	   allocation[consumer_id].b ==  maximun[consumer_id].b && 
	   allocation[consumer_id].c ==  maximun[consumer_id].c)
	{
		shared_resource.a += allocation[consumer_id].a;
		shared_resource.b += allocation[consumer_id].b;
		shared_resource.c += allocation[consumer_id].c;
		
		allocation[consumer_id].a = 0;
		allocation[consumer_id].b = 0;
		allocation[consumer_id].c = 0;
		
		need[consumer_id].a = 0;
		need[consumer_id].b = 0;
		need[consumer_id].c = 0;
		
		finish[consumer_id] = 1;
		
		for(int i = 0;i < consumer_num;i++){
			if(finish[consumer_id] != 1) break;
			if(i = consumer_num-1) return 3;
		}
		return 2;
	}
	else if(shared_resource.unsafe == 1){
		shared_resource.unsafe = 0;
		
		allocation[consumer_id].a -= source.a;											//restore data
		allocation[consumer_id].b -= source.b;
		allocation[consumer_id].c -= source.c;
		
		need[consumer_id].a = maximun[consumer_id].a - allocation[consumer_id].a;
		need[consumer_id].b = maximun[consumer_id].b - allocation[consumer_id].b;
		need[consumer_id].c = maximun[consumer_id].c - allocation[consumer_id].c;
		
		shared_resource.a += source.a;
		shared_resource.b += source.b;
		shared_resource.c += source.c;
		return -3;
	}
	else{
		return 1;
	}
}

void print_state(int consumer_id, consume_source source, int code, int request){
	//printf("\n********%d***********\n", code);
	if(request == 1){
		if(code == -3){
			printf("request %d %d %d\n", source.a, source.b, source.c);
			printf("request code -3 : consumer %d's request fail since state is unsafe\n", consumer_id);
		}
		else if(code == -2){
			printf("request %d %d %d\n", source.a, source.b, source.c);
			printf("request code -2 : consumer %d's request exceed available\n", consumer_id);
		}
		else if(code == -1){
			printf("request %d %d %d\n", source.a, source.b, source.c);
			printf("request code -1 : consumer %d's request exceed need\n", consumer_id);
		}
		else if(code == 0){
			printf("request %d %d %d\n", source.a, source.b, source.c);
			printf("request code 0 : consumer %d's doesn't request\n", consumer_id);
		}
		else if(code == 1){
			printf("request %d %d %d\n", source.a, source.b, source.c);
			printf("request code 3 : consumer %d's request succeeds\n", consumer_id);
		}
		else if(code == 2){
			printf("request %d %d %d\n", source.a, source.b, source.c);
			printf("request code 2 : consumer %d's request succeeds and finish\n", consumer_id);
		}
		else if(code == 3){
			printf("request %d %d %d\n", source.a, source.b, source.c);
			printf("request code 3 : consumer %d's request succeeds and all consumer finish\n", consumer_id);
		}
	}
	else{
		if(code == -1){
			printf("release %d %d %d\n", source.a, source.b, source.c);
			printf("release code -1 : consumer %d's release exceed allocation\n", consumer_id);
		}
		else if(code == 0){
			printf("release %d %d %d\n", source.a, source.b, source.c);
			printf("release code 0 : consumer %d's doesn't release\n", consumer_id);;
		}
		else if(code == 1){
			printf("release %d %d %d\n", source.a, source.b, source.c);
			printf("release code 1 : consumer %d's release succeeds\n", consumer_id);
		}
	}
	
	printf("\n");
	printf("current state\n");
	printf("\n");
	printf("available\n");
	printf("source  %-5d%-5d%-5d\n",shared_resource.a, shared_resource.b, shared_resource.c);
	printf("\n");
	for(int i = 0;i < 12;i++) printf(" ");
	printf("maxium");
	for(int i = 0;i < 9;i++) printf(" ");
	printf("allocation");
	for(int i = 0;i < 5;i++) printf(" ");
	printf("need\n");
	
	for(int i = 0;i < 5;i++){
		printf("consumer %-3d%-5d%-5d%-5d%-5d%-5d%-5d%-5d%-5d%-5d"
						,i , maximun[i].a, maximun[i].b ,maximun[i].c, 
							 allocation[i].a, allocation[i].b, allocation[i].c,
							 need[i].a, need[i].b, need[i].c);
		printf("\n");
	}
	
	for(int i = 0;i < 50;i++) printf("=");
	printf("\n");
}

void initial(){
	srand(time(NULL));
	
	shared_resource.a = 10;
	shared_resource.b = 5;
	shared_resource.c = 7;
	
	shared_resource.unsafe = 0;
	
	for(int i = 0;i < consumer_num;i++)
		finish[i] = 0;
	
	for(int i = 0;i < consumer_num;i++){
		maximun[i].a = (rand()%10);
		maximun[i].b = (rand()%5);
		maximun[i].c = (rand()%7);
	}
	
	for(int i = 0;i < consumer_num;i++){
		need[i].a = maximun[i].a;
		need[i].b = maximun[i].b;
		need[i].c = maximun[i].c;
	}
	
	for(int i = 0;i < consumer_num;i++){
		allocation[i].a = 0;
		allocation[i].b = 0;
		allocation[i].c = 0;
	}
}

int main() {
  while(loop){
	  pthread_attr_t attr[3];
	  pthread_t thr[3]; 
	  for(int i = 0;i < 3;i++) pthread_attr_init(&attr[i]);
	  
	  initial();
	  pthread_mutex_init(&shared_resource.mutex, NULL);
	  
	  for(int i = 0;i < 5;i++) pthread_create(&thr[i], NULL, thread_runner, i);
	  
	  for(int i = 0;i < 5;i++) pthread_join(&thr[i], NULL);
  }
  return 0;
}