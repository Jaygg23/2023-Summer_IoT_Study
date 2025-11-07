/***** A.1 * File Include                       *******************************/
#include <stdio.h> //기본 입출력 관련 헤더파일(printf, scanf, gets, puts 등)
#include <stdlib.h> //문자열 변환, 의사 난수 생성, '동적 메모리 관리' 관련 헤더파일(alloc 함수 등)
#include <string.h> //메모리 블록, 문자열 관련 헤더파일(mem~ 함수 등)
#include <unistd.h> //POSIX(Linux) API 관련 헤더파일(read, write 함수 등)
#include <arpa/inet.h> // 주소 변환 기능 관련 헤더파일
#include <sys/socket.h> //네트워크 통신을 위한 소켓 인터페이스 자료형, 구조체 정의
#include <error.h> //에러 출력 관련 헤더(perror 함수 등)
/***** B.1 * Definition of New Constants        *******************************/
/***** C.1 * Declaration of Variables(LOCAL)    *******************************/
/***** C.2 * Declaration of Variables(EXTERN)   *******************************/
/***** D.1 * Definition of Functions(LOCAL)     *******************************/
/***** D.2 * Definition of Functions(EXTERN)    *******************************/

#define BUF_SIZE 128

int main(int argc, char* argv[])
{
    int serv_sock; //서버 소켓 선언
    int clnt_sock; //클라이언트 소켓 선언

    struct sockaddr_in serv_addr; //sys/socket.h 헤더파일 내에 선언되어있는 구조체를 통해
    struct sockaddr_in clnt_addr; //serv_addr과 clnt_addr 구조체 선언
    socklen_t clnt_addr_size = 0; //socketlen_t는 소켓 관련 매개 변수에 사용되는 것으로 길이 및 크기 값에 대한 정의

    char read_buf[BUF_SIZE] = { 0x00, }; //Client로부터 읽어올 데이터 저장 배열
    unsigned int read_len = 0; //read_buf의 길이를 저장할 변수 선언

    if (argc != 2) //argc는 인자의 개수를 저장. 즉, 인자가 2개 [./<실행파일> <PORT>] 형식으로 적도록 조건문 작성
    {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

//to make socket, call socket function
    serv_sock = socket(AF_INET, SOCK_STREAM, 0); //IPv4, SOCK_STREAM Type, Protocol 미지정
    if (serv_sock == -1) //소켓이 생성되지 않아, socket descriptor가 0 이상의 값이 나오지 않으면 소켓이 생성되지 않았다는 오류 출력
        perror("socket() : "); //perror 함수 : 사용자가 입력한 문자열 뒤에 에러에 해당하는 메시지 출력

    memset(&serv_addr, 0, sizeof(serv_addr)); //serv_addr을 serv_addr의 사이즈만큼 0으로 설정 == 초기화
    serv_addr.sin_family = AF_INET; //주소체계를 IPv4로 설정
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //들어오는 모든 32bit로 저장된 IP주소를 Network-Byte-Order(Big Endian)으로 변환하여 s_addr에 저장
                                                   //INADDR_ANY : 내 네트워크 카드로 들어오는 ip 패킷을 다 볼것임
    serv_addr.sin_port = htons(atoi(argv[1])); //argv[0]은 실행파일명, 정상적으로 입력했다면 argv[1]은 포트번호이지만, 입력값은 문자열이기 때문에 atoi()를 통해 정수형으로 변환 후, 
                                               //short 크기(2Byte) 데이터를 Network-Byte-Order(Big Endian)으로 변환하여 sin_port에 저장
                                               //포트번호 argv[1]가 아스키 값으로 전달되었으므로 int형으로 변환

    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) //serv_sock에 &serv_addr을 통해 serv_addr 크기만큼 할당 == 주소할당, 안됐다면 오류 출력
        perror("bind() : ");
    
    if (listen(serv_sock, 5) == -1) //serv_sock을 통해 접속 요청을 대기, 한 번에 5대까지 대기. 실패할 경우 오류 출력
        perror("listen() : ");

    clnt_addr_size = sizeof(clnt_addr); //clnt_addr_size를 clnt_addr의 크기만큼 선언. client 주소가 담긴 구조체 size 할당
    
    while(1){
         clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size); //연결을 기다리는 serv_sock에 접속되는 Client의 주소 및 포트정보를 &clnt_addr에 저장하고 그 길이를 clnt_addr_size에 저장
         if (clnt_sock == -1) //clnt_sock가 생성되지 않으면 accept() 오류 출력
            perror("accept() : ");
        else
            printf("Connected client\n");

        //데이터 수신
        while(1){ //Client로부터 오는 입력을 계속 입력 받기
            read_len = read(clnt_sock, read_buf, BUF_SIZE); //socket descriptor에 저장된 값을 read_buf에 최대 128byte 크기를 받아옴
            if(read_len <= 0) //음수면 echo 서비스 종료
                break;
            printf("Echo data : %s",read_buf);

            write(clnt_sock,read_buf,read_len);
            memset(read_buf, 0, BUF_SIZE); //용도가 끝난 read_buf는 0으로 초기화
        }
        close(clnt_sock); //사용했던 구조체 정리
        printf("Disconnect...\n");
    }
    close(serv_sock); //사용했던 구조체 정리 
    return 0;
}
