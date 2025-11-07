/** A.1 * File Include  **/
#include <stdio.h>  // 기본 입출력 관련 헤더파일 (printf, scanf, gets, puts 등등)
#include <stdlib.h> // 문자열 변환, 의사 난수 생성, "동적 메모리 관리" 관련 헤더파일 (alloc 함수 등)
#include <string.h> // 메모리 블록, 문자열 관련 헤더파일 (mem~ 함수 등)
#include <unistd.h> // POSIX(Linux) API 관련 헤더파일 (read, write 함수 등)
#include <arpa/inet.h>  // 주소 변환 기능 관련 헤더파일 ()
#include <sys/socket.h> // 네트워크 통신을 위한 소켓 인터페이스 자료형, 구조체 정의
#include <errno.h>  // 에러 출력 관련 헤더 (perror 함수 등)

/** B.1 * Definition of New Constants  **/
/** C.1 * Declaration of Variables(LOCAL)  **/
/** C.2 * Declaration of Variables(EXTERN)  **/
/** D.1 * Definition of Functions(LOCAL)  **/
/** D.2 * Definition of Functions(EXTERN)  **/

#define BUF_SIZE 128

int main(int argc, char* argv[])
{
    int sock;   // 소켓 선언
    struct sockaddr_in serv_addr; // 접속할 서버의 주소를 담는 구조체
                                  //sockaddr_in 구조체를 serv_addr이라는 이름으로 선언
    char send_buf[BUF_SIZE] = { 0x00, }; // 128byte 크기의 send_buf 배열 선언
    unsigned int send_len = 0; // send_buf의 길이를 세는 변수 선언
    unsigned int str_len = 0;
    if (argc != 3) { // argv에 0은 실행파일명, 1은 IP, 2는 PORT를 제대로 입력되었는지 확인
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    sock = socket(AF_INET, SOCK_STREAM, 0); // IPv4, SOCKET_STREAM Type, Protocol은 미지정
    if (sock == -1) { // socket() 함수가 제대로 실행되었는지 확인
        perror("socket()");
    }

    memset(&serv_addr, 0, sizeof(serv_addr)); // 접속할 서버의 주소를 담는 구조체 초기화
                                              // serv_addr 구조체를 0으로 set == 초기화
    serv_addr.sin_family = AF_INET; // IPv4
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]); // IP인 2번째 인자로 들어온 값을 cp하여 sin_addr.s_addr에 넣기
                                                    //inet_addr 함수 : 
    serv_addr.sin_port = htons(atoi(argv[2])); // port인 3번째 인자로 들어온 값을 atoi 함수를 통해 숫자로 바꾸고, htons를 통해 Big Endian으로 변경
                                               //파라미터로 입력받았으므로 아스키 값을 정수로 변환
    if (connect(sock,(struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)   //connect 함수로 호출해서 접속
                                                                                // sock를 받아와 socket descriptor를 넣고, 연결할 서버의 IP와 Port를 담아온다. 제대로 값을 담아오지 못하면 오류 출력
        perror("connect()");
    else
        printf("Connected...\n");

    while(1){ // Client to Server를 종료할 때까지
      //사용자 입력
      fputs("Input message (Q) : ", stdout);
      fgets(send_buf, BUF_SIZE, stdin);
      if(!strcmp(send_buf, "Q\n"))
            break;

      //입력 받은 문자열을 write
      write(sock, send_buf, strlen(send_buf)); // sock에 send_buf 크기만큼 send_buf 넣기

      memset(send_buf, 0, BUF_SIZE); // send_buf를 0으로 초기화

      str_len = read(sock, send_buf, BUF_SIZE); // sock를 128byte 크기만큼 읽어와, str_len에 넣기
        if (str_len <= 0) { // 제대로 값이 들어가지 않으면, 오류 출력
            perror("read() : ");
        }
        send_buf[str_len] = '\0';

        printf("Server send : %s",send_buf); // 서버에서 가져오는 값을 출력

    
    }
    
    close(sock); // 소켓 닫기

    return 0;
}
