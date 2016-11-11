#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>

static long x=0;
int number[4]={1,2,3,4};

static void *thr_fn(void *arg){
  int *num=(int *)arg;
  while(1){
    printf("Hello world from thread name%d.\n",*num);
    sleep(1);
  }
}

int main(){
  pthread_t tid[4];
  int i=0,err;

  for(i;i<4;i++){
    int err=pthread_create(tid+i,NULL,thr_fn,(int *)(number+i));
    if(err!=0){
      printf("Can't create thread: %d\n",strerror(err));
      exit(1);
    }
  }

  while(1){
    printf("Hello worle from main thread!\n");
    sleep(1);
  }
  
  return 0;

}
