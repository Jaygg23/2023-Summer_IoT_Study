#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <error.h>

#define BUF_SIZE 100
#define MAX_CLNT 256

void *handle_clnt(void * arg);
void send_msg(char* msg, int len, int count);
void error_handling(char* masg);

int clnt_cnt=0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;

int main(int argc, char* argv[])
{
    int serv_sock; //서버 소켓 선언
    int clnt_sock; //클라이언트 소켓 선언

    struct sockaddr_in serv_adr; //sys/socket.h 헤더파일 내에 선언되어있는 구조체를 통해
    struct sockaddr_in clnt_adr; //serv_addr과 clnt_addr 구조체 선언
    int clnt_adr_sz;
    pthread_t t_id;
    

    if (argc != 2) //argc는 인자의 개수를 저장. 즉, 인자가 2개 [./<실행파일> <PORT>] 형식으로 적도록 조건문 작성
    {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    pthread_mutex_init(&mutx,NULL);//mutex init
    serv_sock = socket(PF_INET, SOCK_STREAM, 0); 
    if (serv_sock == -1)
        perror("socket() : "); 
    

    memset(&serv_adr, 0, sizeof(serv_adr)); 
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);    
    serv_adr.sin_port = htons(atoi(argv[1])); 


    if (bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) //serv_sock에 &serv_addr을 통해 serv_addr 크기만큼 할당 == 주소할당, 안됐다면 오류 출력
        perror("bind() : ");
    
    if (listen(serv_sock, 5) == -1) //serv_sock을 통해 접속 요청을 대기, 한 번에 5대까지 대기. 실패할 경우 오류 출력
        perror("listen() : ");

    
    
    while(1){
        clnt_adr_sz = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz); //연결을 기다리는 serv_sock에 접속되는 Client의 주소 및 포트정보를 &clnt_addr에 저장하고 그 길이를 clnt_addr_size에 저장
         if (clnt_sock == -1) //clnt_sock가 생성되지 않으면 accept() 오류 출력
            perror("accept() : ");
        else
            printf("Connected client\n");

        pthread_mutex_lock(&mutx);
        clnt_socks[clnt_cnt++]=clnt_sock;
        pthread_mutex_unlock(&mutx);

        pthread_create(&t_id,NULL,handle_clnt,(void*)&clnt_sock);
        pthread_detach(t_id);
        printf("Connected client IP : %s \n",inet_ntoa(clnt_adr.sin_addr));
    }
        
    close(serv_sock); //사용했던 구조체 정리 
    return 0;
}

void* handle_clnt(void* arg){
    int clnt_sock =* ((int*)arg);
    int str_len =0,i;
    char msg[BUF_SIZE];
    while((str_len=read(clnt_sock,msg,sizeof(msg)))!=0)
    {
        send_msg(msg,str_len,clnt_sock);
    }
    pthread_mutex_lock(&mutx);
    for(i=0;i<clnt_cnt;i++)
    {
        if(clnt_sock==clnt_socks[i])
        {
            while(i++<clnt_cnt-1)
                clnt_socks[i] = clnt_socks[i+1];
            break;
        }
    }
    clnt_cnt--;
    pthread_mutex_unlock(&mutx);
    close(clnt_sock);
    return NULL;
}

void send_msg(char* msg, int len, int count)
{
    int i;
    pthread_mutex_lock(&mutx);
    for(i=0;i<clnt_cnt;i++)
    {
        if(i==count-4) continue;
        write(clnt_socks[i],msg,len);

    }
    pthread_mutex_unlock(&mutx);
}

void error_handling(char* msg)
{
    fputs(msg,stderr);
    fputc('\n',stderr);
    exit(1);
}
