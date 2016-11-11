#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> 
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <pwd.h>


char *getcurrentdir(void)
{
	char *currentdir;
	int size = 32;
	while (1) {
		currentdir = (char *)malloc(size);
		if (getcwd(currentdir,size) == currentdir)
			return currentdir;
		else size *= 2;
	}
}

void show_hostname_and_currentdir(void)
{
	char hostname[32];
	struct passwd *my_passwd = NULL;
	int my_passwd_len = 0;
	char *dir = NULL;
	int i = 0;

	if ( gethostname(hostname,32) ){
			printf("Get hostname Error~\n");
			exit(0);
		}

	my_passwd = getpwuid( getuid() );
	my_passwd_len = strlen(my_passwd->pw_dir);
	
	dir = getcurrentdir();

	for(i = 0; i<my_passwd_len; i++)
		if(dir[i] != my_passwd->pw_dir[i])
			break;
	if(i == my_passwd_len) {
		char *dir_tmp = NULL;
		int size_tmp = 0;
		
		size_tmp = strlen(dir) - my_passwd_len + 1;
		dir_tmp = (char *)malloc(size_tmp);
		dir_tmp[0] = '~';
		for(; ; i++)
			if(( dir_tmp[i-my_passwd_len+1] = dir[i]) == '\0')
				break;
		free(dir);
		dir = dir_tmp;
		dir_tmp = NULL;
	}

	printf(">>%s@%s: %s$ ", my_passwd->pw_name, hostname, dir);
	free(dir);
}

/*********************************************************************/
char *read_order(char *buffer)
{
	char cmd_char;
	char *order_in;
	int i=0;

	cmd_char = getchar();
	while(cmd_char != '\n') {
		buffer[i] = cmd_char;
		cmd_char = getchar();
		i++;
	}
	buffer[i] = '\0';	

	if((order_in=(char *)malloc(sizeof(char)*(i+1)))==0) {
		printf("malloc error~\n");
		return NULL;
	}
	strcpy(order_in, buffer);
	return order_in;
}

char *order_name(const char *input)
{
	int i,len;
	char *order;
	
	len=strlen(input);
	if( (order=(char *)malloc((len+1)*sizeof(char))) ==0) {
		printf("malloc error~\n");
		return NULL;
	}
	order[0]='\0';
	
	for (i=0;i<=len;i++)
		order[i]=input[i];

	return order;
}

int number(const char *buffer)	
{
	int i=0,k=0,flag=0;
	int len=strlen(buffer);

	for (i=0;i<len;i++){
		if(buffer[i]==' '||buffer[i]=='<'||buffer[i]=='>'||buffer[i]=='	') {
			flag=0;
			continue;
		} else {
			if(flag==0){
				flag=1;
				k++;
			}
		}
	}
	return k;
}

char **analyse(const char *input)	
{	
	int i=0,j=0,k=0;	
	int len;
	char *tmp;
	char **arg;

	len=strlen(input);

	if( (tmp=(char *)malloc((len+1)*sizeof(char))) ==0) {
		printf("malloc error~\n");
		return NULL;
	}
	

	k=number(input);
	if((arg=(char **)malloc((k+1)*sizeof(char *)))==0) {
		printf("malloc error~\n");
		return NULL;
	}

	for (i=0,j=0,k=0;i<=len;i++)
	{
		if(input[i]==' '||input[i]=='<'||input[i]=='>'||input[i]=='	'||input[i]=='\0') {
			if(j == 0) continue;
			else {
				tmp[j] = '\0';
				j++;
				arg[k] = (char *)malloc(sizeof(char)*j);
				
				strcpy(arg[k], tmp);
				j=0; 
				k++;
			}
		} else {   
			tmp[j]=input[i];
			j++;
		}
	}
	
	arg[k]=NULL;
	free(tmp);
	return arg;
}

char *file_cmd(const char *order)		
{
	char *path=NULL, *p=NULL;
	char *buffer=NULL;
	int i=0,len=0;

	path = getenv("PATH");
	strcat(path,":");
	p = path;
	len = strlen(path)+strlen(order)+2;

	if((buffer=(char *)malloc(len*sizeof(char)))==0) {
		printf("malloc error~\n");
		return NULL;
	}

	while(*p != '\0') {
		if(*p != ':') buffer[i++] = *p;
		else {
			buffer[i++] = '/';
			buffer[i] = '\0';
			strcat(buffer,order);
			if(access(buffer,F_OK) == 0) return buffer;
			else i=0;
		}
		p++;
	}
	strcpy(buffer,order);
	if(access(buffer,F_OK) == 0) return buffer;
	return NULL;
}

void show_history(int his_front, int size, char **history)
{
	int i=0;
	int p = his_front;
	char *tmp_out;
	for(i=0;i<size;i++){
		tmp_out = (char *)malloc(128*sizeof(char ));
		strcpy(tmp_out,history[p]);
		printf("%-3d%s\n",i+1,tmp_out);
		p++;
		p = p%16;
		free(tmp_out);
	}
}

int redirect(char *buffer,int his_front, int size, char **history)
{
	char i=0,j=0,len=0,len_a=0;
	char *file_in=NULL,*file_out=NULL;
	char *order=NULL;
	int flag_in=0,flag_out=0,fd_in=0,fd_out=0;
	char **order_a;
	char *path=NULL;
	pid_t pid;
	int status;
	
	len = strlen(buffer);
	file_in = (char *)malloc((len+1)*(sizeof(char)));
	file_out = (char *)malloc((len+1)*(sizeof(char)));
	order = (char *)malloc((len+1)*(sizeof(char)));

	for(i=0;i<len;i++) {
		if( buffer[i] == '<' ) {
			flag_in = 1;
			break;
		}
		else if( buffer[i] == '>' ) {
			flag_out = 1;
			break;
		}
		else
			order[i] = buffer[i];
	}
	order[i] = '\0';
	i++;
	
	while ((buffer[i]==' '||buffer[i]=='	') && i<len) i++;
	j=0;
	file_out[0]='\0';
	file_in[0]='\0';
	if(flag_in == 1) {
		while (i<=len){
			if(buffer[i]=='<') {
				file_in[j]='\0';
				break;
			}
			file_in[j]=buffer[i];
			i++;
			j++;
		}
	}
	if(flag_out == 1) {	
		while (i<=len) {
			if(buffer[i]=='<') {
				file_out[j]='\0';
				break;
			}
			file_out[j]=buffer[i];
			i++;
			j++;
		}
	}

	if (i<len)
	{
		j=0;
		if (flag_out>0 && buffer[i]=='<')
		{
			i++;
			flag_in=1;

			while ((buffer[i]==' '||buffer[i]=='	')&&i<len) i++;
			while (i<=len) {
				file_in[j]=buffer[i];
				i++;
				j++;
			}
		}
		else if (flag_in>0 && buffer[i]=='>')
		{
			i++;
			flag_out=1;
			while ((buffer[i]==' '||buffer[i]=='	')&&i<len)
				i++;
			while (i<=len) {
				file_out[j]=buffer[i];
				i++;
				j++;
			}
		}
	}

	len_a = number(order);
	order_a = analyse(order);
	if( strcmp(order_a[0], "exit") == 0) {
		printf("GoodBye From Hao Huang~\n");

		for(i=0;i<len_a;i++) free(order_a[i]);
		free(order_a);
		free(order);
		exit(0);
		exit(0);
		return 0;
	}
	if (strcmp(order_a[0],"cd")==0) {
		if(order_a[1]!=NULL){ 
			if(chdir(order_a[1])<0)printf("CD :No such path~\n"); 
		}
		for(i=0;i<len_a;i++) free(order_a[i]);
		free(order_a);
		free(order);
		return 0;
	}
	if (strcmp(order_a[0],"history")==0){
		show_history(his_front, size, history);
		free(order_a);
		free(order);
		return 0;
	}

	path = file_cmd(order_a[0]);
	if(path==NULL)	
	{
		printf("command not found~\n");

		for(i=0;i<len_a;i++)free(order_a[i]);
		free(order_a);
		free(order);
		return 0;
	}

	if((pid = fork()) == 0) {
		if(flag_in == 1)
			fd_in = open(file_in, O_RDONLY, S_IRUSR|S_IWUSR );
		if(fd_in == -1) {
			printf("open error~\n");
			return 0;
		}
		if(fd_in > 0)
			dup2(fd_in,STDIN_FILENO);
		if(flag_out == 1)
			fd_out = open(file_out,O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR );
		if(fd_out == -1) {
			printf("open error~\n");
			return 0;
		}
		if(fd_out > 0)
			dup2(fd_out,STDOUT_FILENO);

		execv(path,order_a);
		exit(0);		
	} else                     
		pid=waitpid(pid, &status, 0);
	
	free(file_out);
	free(file_in);
	free(path);
	for(i=0;i<len_a;i++) free(order_a[i]);
	free(order_a);
	return 0;
}
/*********************************************************************/

int pipe_num(const char *buffer)
{
	int sum=0,i=0;

	for (i=0;i<strlen(buffer);i++)
	if(buffer[i]=='|')sum++;
	return sum;
}

int pipe_cmd(char * buffer)
{
	int i=0,j=0,k=0,num_of_pipe=0,len=0;
	int **fd;
	char **order;
	int status;
	int *child;
	
	len = strlen(buffer);
	num_of_pipe = pipe_num(buffer);
	if( (order=(char **)malloc((num_of_pipe+1)*sizeof(char *))) == 0 ) {
		printf("malloc error~\n");
		return 0;
	}
	for(i=0;i<num_of_pipe+1;i++) {
		if( (order[i]=(char *)malloc((len+1)*sizeof(char))) == 0 ) {
			printf("malloc error~\n");
			return 0;
		}
	}

	if( (child=(int *)malloc((num_of_pipe+1)*sizeof(char *))) == 0 ) {
		printf("malloc error~\n");
		return 0;
	}
	if( (fd=(int **)malloc(num_of_pipe*sizeof(int *))) == 0 ) {
		printf("malloc error~\n");
		return 0;
	}
	for(i=0;i<num_of_pipe;i++) {
		if( (fd[i]=(int *)malloc(2*sizeof(int))) == 0 ) {
			printf("malloc error~\n");
			return 0;
		}
	}

	for (i=0;i<=len;i++) {
		if (buffer[i]!='|') {
			order[k][j]=buffer[i];
			j++;
		} else {
			order[k][j]='\0';
			k++;
			j=0;
		}
	}
	for(i=0;i<k;i++) {
		if(pipe(fd[i]) == -1) {
			printf("creat pipe error~\n");
			return 0;
		}
	}
	i=0;
	if( (child[i]=fork()) == 0 )
	{
		close(fd[i][0]);
		if(fd[i][1] != STDOUT_FILENO) {
			if(dup2(fd[i][1], STDOUT_FILENO) == -1) {
				printf("dup error~\n");
				return 0;
			}
			close(fd[i][1]);
		}
		redirect(order[i],0,0,NULL);	
		exit(0);
	} else {
		waitpid(child[i],&status,0);
		close(fd[i][1]);
	}
	i++;
	while(i<k){
		if ((child[i]=fork())==0){
			if(fd[i][0] != STDIN_FILENO) {
				if(dup2(fd[i-1][0], STDIN_FILENO) == -1) {
					printf("dup error\n");
					return 0;
				}
				close(fd[i-1][0]);
				if(dup2(fd[i][1], STDOUT_FILENO) == -1) {
					printf("dup error~\n");
					return 0;
				}
				close(fd[i][1]);
			}
			redirect(order[i],0,0,NULL);
			exit(0);
		} else {
			waitpid(child[i],&status,0);
			close(fd[i][1]);
			i++;
		}
	}
	if((child[i] = fork()) == 0) 
	{
		close(fd[i-1][1]);
		if(fd[i-1][0] != STDIN_FILENO) {
			if(dup2(fd[i-1][0], STDIN_FILENO) == -1) {
				printf("dup error~\n");
				return 0;
			}
			close(fd[i-1][0]);
		}
		redirect(order[i],0,0,NULL);
		exit(0);
	} else {
		waitpid(child[i], NULL, 0);
		close(fd[i-1][1]);
	}

	for(i=0;i<k;i++) free(fd[i]);
	free(fd);
	for(i=0;i<k+1;i++) free(order[i]);
	free(order);
	free(child);
	return 0;
}

/*/////////////////////////////////////////////////////////////////////////*/
int main(void) 
{
	char *path,*buffer;
	char *order_in,*order;
	int i=0,k=0,pipe,number;
	char **history;
	int his_front=0;
	int his_end=0;
	int size=0;
	
	if((buffer=(char *)malloc(128*(sizeof(char))))==0) {
		printf("error! can't malloc enough space for buffer\n");
		return 0;
	}
	history = (char **)malloc(16*sizeof(char *));
	for(i=0;i<16;i++) {
		history[i] = (char *)malloc(128*sizeof(char ));
		history[i][0]='\0';
	}
	printf("Welcome to Hao Huang shell~\n");

	while(1)
	{
		path=getcwd(NULL,0);
		show_hostname_and_currentdir();

		order_in=read_order(buffer);
		if(order_in==NULL)
			continue;
		order=order_name(order_in);

		/*--add_history--*/
		if( his_end == his_front && size ==0) {
			strcpy(history[his_end], order_in);
			his_end = (his_end+1)%16;
			size++;
		}
		else if( (his_end+1)%16 == his_front && size ==15) {
			strcpy(history[his_end], order_in);
			size = 16;
		}
		else if((his_end+1)%16 == his_front && size == 16) {
			his_end = (his_end+1)%16;
			his_front = (his_front+1)%16;
			strcpy(history[his_end], order_in);
		} else {
			strcpy(history[his_end], order_in);
			his_end = (his_end+1)%16;
			size++;
		}
		/*------------*/

		if(strlen(order)!=0) {
			k=pipe_num(order);
			if(k!=0)
				pipe_cmd(order);
			else
				redirect(order,his_front,size,history);
		}

		free(order);
		free(order_in);
		free(path);
	}
	for(i=0;i<16;i++) free(history[i]);
	free(history);
	return 0;
}

