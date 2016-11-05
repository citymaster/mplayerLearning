#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <fcntl.h>  
#include <sys/stat.h>  
#include <sys/types.h>  
#include <string.h>  
#include <pthread.h>
  
/**********************全局变量定义区*****************/  
int fd_fifo;                    //创建有名管道，用于向mplayer发送命令  
int fd_pipe[2];                 //创建无名管道,用于从mplayer读取命令  
  
void *get_pthread(void *arg)  
{  
    char buf[100];  
    while(1)  
    {  
        printf("please input you cmd:");  
        fflush(stdout);  
        fgets(buf,sizeof(buf),stdin);       //从标准输入获取数据  
        buf[strlen(buf)]='\0';  
        printf("*%s*\n",buf);             
        if(write(fd_fifo,buf,strlen(buf))!=strlen(buf))  
            perror("write");                    //将命令写入命名管道  
    }  
}  
  
void *print_pthread(void *arg)  
{  
    char buf[100];  
    close(fd_pipe[1]);  
    int size=0;  
    while(1)  
    {  
        size=read(fd_pipe[0],buf,sizeof(buf));  //从无名管道的写端读取信息打印在屏幕上  
        buf[size]='\0';  
        printf("th msg read form pipe is %s\n",buf);  
    }  
}  
  
int main(int argc, char *argv[])  
{  
    int fd;  
    char buf[100];  
    pid_t pid;  
      
    unlink("/tmp/my_fifo");                 //如果明明管道存在，则先删除  
    mkfifo("/tmp/my_fifo",O_CREAT|0666);  
    perror("mkfifo");  
      
    if (pipe(fd_pipe)<0 )                    //创建无名管道  
    {  
        perror("pipe error\n");  
        exit(-1);  
    }  
  
    pid=fork();  
    if(pid<0)  
    {  
        perror("fork");  
    }  
    if(pid==0)                              //子进程播放mplayer  
    {  
        close(fd_pipe[0]);  
        dup2(fd_pipe[1],1);                 //将子进程的标准输出重定向到管道的写端  
        fd_fifo=open("/tmp/my_fifo",O_RDWR);  
        execlp("mplayer","mplayer","-slave","-quiet","-input","file=/tmp/my_fifo","../fade.mp3",NULL);  
    }  
    else  
    {  
        pthread_t tid1;  
        pthread_t tid2;  
        fd_fifo=open("/tmp/my_fifo",O_RDWR);  
        if(fd<0)  
            perror("open");  
              
        pthread_create(&tid1,NULL,get_pthread,NULL);        //从键盘获取控制信息  
        pthread_create(&tid2,NULL,print_pthread,NULL);      //打印从无名管道收到的信息  
        pthread_join(tid1,NULL);  
        pthread_join(tid2,NULL);  
    }  
    return 0;  
}
