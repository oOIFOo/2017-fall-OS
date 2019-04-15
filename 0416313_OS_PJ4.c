#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define buffer_size 2
int ash_counter = 0, misty_counter = 0, Brock_counter = 0;

typedef struct{
  int time;
  int id;
}seat;

struct {
  pthread_mutex_t mutex;
  int top;
  seat buffer[buffer_size];
} shared_stack;

void *ash(void *arg){
  int tmp;
  seat daul;
  daul.time = 2;
  daul.id = 2;
  while(1){
	  pthread_mutex_lock(&shared_stack.mutex);
	  while(shared_stack.top + 1 == buffer_size){
		pthread_mutex_unlock(&shared_stack.mutex);
		printf("seat is full\n");
		sleep(10);
		pthread_mutex_lock(&shared_stack.mutex);
	  }
	  shared_stack.buffer[++shared_stack.top] = daul;
	  pthread_mutex_unlock(&shared_stack.mutex);
	  ash_counter++;
	  if(ash_counter == 3){
		pthread_exit(&tmp);
	  }
	  sleep(rand()%10 + 1);
	}
}

void *misty(void *arg){
  int tmp;
  seat daul;
  daul.time = 3;
  daul.id = 3;
  while(1){
	  pthread_mutex_lock(&shared_stack.mutex);
	  while(shared_stack.top + 1 == buffer_size){
		pthread_mutex_unlock(&shared_stack.mutex);
		printf("seat is full\n");
		sleep(1);
		pthread_mutex_lock(&shared_stack.mutex);
	  }
	  shared_stack.buffer[++shared_stack.top] = daul;
	  pthread_mutex_unlock(&shared_stack.mutex);
	  misty_counter++;
	  if(misty_counter == 2){
		pthread_exit(&tmp);
	  }
	  sleep(rand()%10 + 1);
	}
}

void *Brock(void *arg){
	int tmp;
	seat daul;
	daul.time = 5;
	daul.id = 5;
	pthread_mutex_lock(&shared_stack.mutex);
	while(shared_stack.top + 1 == buffer_size){
		pthread_mutex_unlock(&shared_stack.mutex);
		printf("seat is full\n");
		sleep(1);
		pthread_mutex_lock(&shared_stack.mutex);
	}
	shared_stack.buffer[++shared_stack.top] = daul;
	pthread_mutex_unlock(&shared_stack.mutex);
	pthread_exit(&tmp);
}

void *master(void *arg){
  int count;
  while(1){
	seat data;
	pthread_mutex_lock(&shared_stack.mutex);
	while(shared_stack.top == -1){
		pthread_mutex_unlock(&shared_stack.mutex);
		printf("master is sleep\n");
		sleep(1);
		pthread_mutex_lock(&shared_stack.mutex);
	}
	printf("the master is waken by a trainer\n");
	data = shared_stack.buffer[shared_stack.top--];
	
	for(int i = data.time;i > 0; i--){
		if(data.id == 2) printf("the master is battle with ash\n");
		else if(data.id == 3) printf("the master is battle with misty\n");
		else if(data.id == 5) printf("the master is battle with brock\n");
		sleep(1);
	}
	
	if(data.id == 2) {
		printf("the battle with ash is over\n");
		if(ash_counter == 3){
			printf("the battle of ash is over\n");
			count++;
	 	}
	}
	else if(data.id == 3) {
		printf("the battle with misty is over\n");
		if(misty_counter == 2){
			printf("the battle of misty is over\n");
			count++;
		}
	}
	else if(data.id == 5){
		printf("the battle with brock is over\n");
		printf("the battle of brock is over\n");
		count++;
	}
	if(count == 3) 	pthread_exit(NULL);
	pthread_mutex_unlock(&shared_stack.mutex);
  }
}

int main() { 
  pthread_t thr[4];
  pthread_attr_t attr[4];
  for(int i = 0;i < 4;i++) pthread_attr_init(&attr[i]);
  
  pthread_mutex_init(&shared_stack.mutex, NULL);
  shared_stack.top = -1;
  
  
  if(pthread_create (&thr[0], NULL, master, NULL) != 0) printf("error\n"); 
  pthread_create (&thr[1], NULL, ash, NULL);
  pthread_create (&thr[2], NULL, misty, NULL);
  pthread_create (&thr[3], NULL, Brock, NULL);
  
  sleep(100);
}