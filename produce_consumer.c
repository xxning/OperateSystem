#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>
#include<signal.h>
#include<unistd.h>

typedef int buffer_item;
#define BUFFER_SIZE 5

pthread_mutex_t mutex;
sem_t empty;
sem_t full;

buffer_item *buffer;
int head=0,rear=0;
int buffer_state=0;//0表示buffer未满，1表示buffer满 

static void sig_int(int signo);
int insert_item(buffer_item item);
int remove_item(buffer_item *item);
static void *thr_producer(void *param);
static void *thr_consumer(void *param);

int producer_num,consumer_num;
int test_time;
char *filename;
int *state_producer,*state_consumer;
FILE *fp;

int main(int argc,char *argv[]){
	
	switch(argc){
		case 1: printf("Please enter 4 parameters that this program needs!\n");exit(0);
		case 5: break;
		default: printf("Please enter the parameters correctly!\n");exit(0);	
	}
//初始化变量
	buffer=(buffer_item *)malloc(BUFFER_SIZE*sizeof(buffer_item)); 
	filename=(char *)malloc(32*sizeof(char));
//读取命令行参数	
	producer_num=atoi(argv[1]);
	consumer_num=atoi(argv[2]);
	test_time=atoi(argv[3]);
	filename=argv[4];
	
//初始化互斥量和信号量
	pthread_mutex_init(&mutex,NULL);
	sem_init(&empty,0,BUFFER_SIZE);
	sem_init(&full,0,0);	
//初始化状态数组
	if(!(state_producer=(int *)malloc(producer_num*sizeof(int)))){
		printf("malloc error!\n");
	}	
	if(!(state_consumer=(int *)malloc(consumer_num*sizeof(int)))){
		printf("malloc error!\n");
	}
//输出文件
	fp=fopen(filename,"w");
	fprintf(fp,"producer_num:%d\n",producer_num);
	fprintf(fp,"consumer_num:%d\n",consumer_num); 
//ctrl+c
    	if(signal(SIGINT,sig_int)==SIG_ERR)
		exit(1);	
//创建线程	
	pthread_t *tid_producer;
	if(!(tid_producer=(pthread_t *)malloc(producer_num*sizeof(pthread_t)))){
	  printf("malloc error!\n");
	}
	pthread_t *tid_consumer;
	if(!(tid_consumer=(pthread_t *)malloc(consumer_num*sizeof(pthread_t)))){
	  printf("malloc error!\n");
	}

	int i,err;
	for(i=0;i<producer_num;i++){
		err=pthread_create(tid_producer+i,NULL,thr_producer,state_producer+i);
		if(err!=0){
	      printf("Can't create thread:%d\n",strerror(err));
	      exit(1);
	    }
	}
	for(i=0;i<consumer_num;i++){
		err=pthread_create(tid_consumer+i,NULL,thr_consumer,state_consumer+i);
		if(err!=0){
	      printf("Can't create thread:%d\n",strerror(err));
	      exit(1);
	    }
	}
//退出	 	
	for(i=0;i<test_time;i++) 
		sleep(1);
	for(i=0;i<producer_num;i++){
		pthread_cancel(tid_producer[i]);
		pthread_join(tid_producer[i],NULL);	
	}
	for(i=0;i<consumer_num;i++){
		pthread_cancel(tid_consumer[i]);
		pthread_join(tid_consumer[i],NULL);
	}
	printf("Test is over!\n");	
	
	return 0;
	
}

static void *thr_producer(void *param){
	
	buffer_item item;
	int sleep_time;
	int *state=(int *)param; 
	
	while(1){
		*state=1;
		item=rand();
		sleep_time=rand()%10;
		sleep(sleep_time);
		*state=5;
		sem_wait(&empty);
		*state=7;
		pthread_mutex_lock(&mutex);
		*state=3;
		if(insert_item(item))	
			printf("insert error!\n");
		//else  printf("producer produced %d\n",item);		
		pthread_mutex_unlock(&mutex);
		sem_post(&full);		
	}
}

static void *thr_consumer(void *param){
	
	buffer_item item;
	int sleep_time;
	int *state=(int *)param; 
	
	while(1){
		*state=2;
		sleep_time=rand()%10;
		sleep(sleep_time);
		*state=6;
		sem_wait(&full);
		*state=7;
		pthread_mutex_lock(&mutex);
		*state=4;
		if(remove_item(&item))
			printf("remove error!\n");
		//else  printf("consumer consumed %d\n",item);
		pthread_mutex_unlock(&mutex);
		sem_post(&empty);	
	}
}

int insert_item(buffer_item item){
	
	if(head==rear){
		if(buffer_state) return -1;
		else{
			buffer[head]=item;
			rear=(rear+1)%BUFFER_SIZE;
			return 0;
		}
	}
	else{
		buffer[rear]=item;
		rear=(rear+1)%BUFFER_SIZE;
		if(head==rear) buffer_state=1;
		return 0;
	}	
}


int remove_item(buffer_item *item){
	
	if(head==rear){
		if(buffer_state){
			*item=buffer[head];
			head=(head+1)%BUFFER_SIZE;
			buffer_state=0;
			return 0;
		}
		else 
			return -1;	
	}
	else{
		*item=buffer[head];
		head=(head+1)%BUFFER_SIZE;
		return 0;
	}
}

static void sig_int(int signo){
	
	int i,length;
	printf("\n");
	for(i=0;i<producer_num;i++){
		printf("The state of producer %d: %d\n",i+1,state_producer[i]);
		fprintf(fp,"The state of producer %d: %d\n",i+1,state_producer[i]);
	}
	for(i=0;i<consumer_num;i++){
		printf("The state of consumer %d: %d\n",i+1,state_consumer[i]);
		fprintf(fp,"The state of consumer %d: %d\n",i+1,state_consumer[i]);
	}
	length=(rear-head)%BUFFER_SIZE;
	if(length==0){
		if(buffer_state){
			printf("The buffer is full.\n");
			fprintf(fp,"The buffer is full.\n");
		}
		else {
			printf("The buffer is empty.\n");
			fprintf(fp,"The buffer is empty.\n");
		}
	}
	else{
		if(length<0) length=length+BUFFER_SIZE;
		printf("There are %d items in buffer now.\n",length);
		fprintf(fp,"There are %d items in buffer now.\n",length);	
	}
}
