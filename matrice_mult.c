#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

double **A,**B,**C;//C=A*B
int row_A,row_B,row_C,col_A,col_B,col_C;//记录各个矩阵大小 

typedef struct node{
  int row;
  int col;
}node;

static void *thr_fn(void* N){       
  int row=(((node*)N)->row),col=(((node*)N)->col);
  int i;
  double sum=0;
  for(i=0;i<col_A;i++)
    sum+=A[row][i]*B[i][col];

  C[row][col]=sum;
}

int main(int argc,char *argv[]){
	switch(argc){
		case 1: printf("Please enter four numbers to determine the dimension of matrice A and B.\n");exit(0);
		case 5: break;
		default: printf("Please enter the parameters correctly!\n");exit(0);
	}

	row_C=row_A=atoi(argv[1]);
	col_A=atoi(argv[2]);
	row_B=atoi(argv[3]);
	col_C=col_B=atoi(argv[4]);
	if(col_A!=row_B){
	  printf("Error:col_A!=row_B\n");
	  return 0;
	}

	int i;

//分配空间 
	if(!(A=(double **)malloc(row_A*sizeof(double *)))){
	   printf("malloc error!");
	   return 0;
	}
	for(i=0;i<row_A;i++){
	   if(!(A[i]=(double *)malloc(col_A*sizeof(double)))){
	      printf("malloc error!");
	      return 0;
	   }
	}

	if(!(B=(double **)malloc(row_B*sizeof(double *)))){
	   printf("malloc error!");
	   return 0;
	}
	for(i=0;i<row_B;i++){
	   if(!(B[i]=(double *)malloc(col_B*sizeof(double)))){
	      printf("malloc error!");
	      return 0;
	   }
	}

	if(!(C=(double **)malloc(row_C*sizeof(double *)))){
	   printf("malloc error!");
	   return 0;
	}
	for(i=0;i<row_C;i++){
	   if(!(C[i]=(double *)malloc(col_C*sizeof(double)))){
	      printf("malloc error!");
	      return 0;
	   }
	}
//读入数据	
	int j,k,state,head,line;
	char c;
	char *buf;
	double num;
	FILE *fp;

	if(!(buf=(char *)malloc(20*sizeof(char)))){
	  printf("malloc error!");
	  return 0;
	}
	fp=fopen("file_A","t");
	i=j=k=state=0;
	line=1;
	fscanf(fp,"%c",&c);
	while(c!=EOF){
	  if((c>='0'&&c<='9')||(c=='.')){
	    if(state==1) state=0;
	    if(head==1)  head=0;
	    buf[k++]=c;	    
	    fscanf(fp,"%c",&c);
	    continue;
	  }
	  else if(c==' '){
	    if(j==0&&head==1){
		printf("file_A:Please don't use space at the head of each line!\n");
		return 0;
	    }
	    if(state==0) state=1;
	    else {
		printf("file_A:Please don't use too many space\n");
		return 0;
     	    }
	    buf[k]='\0';
	    A[i][j]=atof(buf);
	    j++;
	    k=0;
	    fscanf(fp,"%c",&c);
	    continue;
	  }
	  else if(c=='\n'){
	    if(state==1) state=0;
	    if(head==0)  head=1;
	    if(line==row_A) {
		buf[k]='\0';
	    	A[i][j]=atof(buf);		
		break;
	    }
	    line++;
 	    if(j!=(col_A-1)){
		printf("1Data error of file_A!Please check again!\n");
	  	return 0;
	    }
	    buf[k]='\0';
	    A[i][j]=atof(buf);
	    i++;
	    k=j=0;
	    fscanf(fp,"%c",&c);
	    continue;
	  }
	  else {
	    printf("2Data error of file_A!Please check again!\n");
	    return 0;
	  }
	}
	if(!((i==(row_A-1))&&(j==(col_A-1)))){
	  printf("3Data error of file_A!Please check again!\n");
	  return 0;
	}
	fclose(fp);

	fp=fopen("file_B","r");
	i=j=k=state=0;
	line=1;
	fscanf(fp,"%c",&c);
	while(c!=EOF){
	  if((c>='0'&&c<='9')||c=='.'){
	    if(state==1) state=0;
	    if(head==1)  head=0;
	    buf[k++]=c;	    
	    fscanf(fp,"%c",&c);
	    continue;
	  }
	  else if(c==' '){
	    if(j==0&&head==1){
		printf("file_B:Please don't use space at the head of each line!\n");
		return 0;
	    }
	    if(state==0) state=1;
	    else {
		printf("file_B:Please don't use too many space\n");
		return 0;
     	    }
	    buf[k]='\0';
	    B[i][j]=atof(buf);
	    j++;
	    k=0;
	    fscanf(fp,"%c",&c);
	    continue;
	  }
	  else if(c=='\n'){
	    if(state==1) state=0;
	    if(head==0)  head=1;
	    if(line==row_B) {
		buf[k]='\0';
	    	B[i][j]=atof(buf);		
		break;
	    }
	    line++;
 	    if(j!=(col_B-1)){
		printf("Data error of file_B!Please check again!\n");
	  	return 0;
	    }
	    buf[k]='\0';
	    B[i][j]=atof(buf);
	    i++;
	    k=j=0;
	    fscanf(fp,"%c",&c);
	    continue;
	  }
	  else {
	    printf("Data error of file_B!Please check again!\n");
	    return 0;
	  }
	}
	if(!((i==(row_B-1))&&(j==(col_B-1)))){
	  printf("Data error of file_B!Please check again!\n");
	  return 0;
	}
	fclose(fp);
//计算	    
	pthread_t *tid;
	if(!(tid=(pthread_t *)malloc((row_C*col_C)*sizeof(pthread_t)))){
	  printf("malloc error!\n");
	}
	int err;
	for(i=0;i<row_C;i++){
	  for(j=0;j<col_C;j++){
	    node *N=(node *)malloc(sizeof(node));
	    N->row=i;
	    N->col=j;
	    err=pthread_create(&tid[i*col_C+j],NULL,thr_fn,N);
	    if(err!=0){
	      printf("Can't create thread:%d\n",strerror(err));
	      exit(1);
	    }
	  }
	}

    for(i=0;i<row_C*col_C;i++)
	  pthread_join(tid[i],NULL);
//写入结果 
	fp=fopen("file_C","w");
	for(i=0;i<row_C;i++){
	  for(j=0;j<col_C;j++){
	    if(j==(col_C-1))
	      fprintf(fp,"%.3f\n",C[i][j]);
	    else 
	      fprintf(fp,"%.3f ",C[i][j]);
	  }
	}	
	fclose(fp);
		
	return 0;	

}
