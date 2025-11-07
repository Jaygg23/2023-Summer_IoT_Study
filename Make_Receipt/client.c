#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <error.h>
#include <time.h>

typedef struct { //서버로부터 받은 데이터를 저장하는 구조체. 구조체 멤버로는 각 물품의 종류와 수량, 예산, 이름
    char Fruits;
    char Fruit_cnt;
    char Snacks;
    char Snack_cnt;
    char Beverages;
    char Beverages_cnt;
    char Vegetables;
    char Vegetables_cnt;
    int budget;
    char customerName[20];
}read_buf;

typedef struct { //각 물품의 이름과 가격을 저장하는 구조체
    char name[20]; //물품의 이름을 나타내는 배열
    int number; //서버에서 넘어온 수량 저장
    int price; //txt파일에서 price 파싱한 값 저장
} Item;

#define MAX_ITEMS 5 //각 섹션에서 허용되는 최대 물품 수

Item items[4][5]; // 물품과 가격을 저장할 구조체 2차원 배열
int num[4]; //선택된 물품들의 인덱스를 저장하는 배열
int number[5] = {0x00, }; //각 물품의 수량을 저장하는 배열
read_buf received_data = {0x00, }; //서버로부터 받은 데이터를 저장할 구조체 변수
int budget;

Item final_item[5] = {0x00, }; //결과로 선택된 물품들의 세부 정보를 저장하는 배열

void parse_price_file(Item items[4][5]) //price_tag.txt 파일 물품 이름, 가격 파싱하여 2차원 배열에 저장하는 함수
{
    char buffer[128];
    FILE* fp; //price_tag.txt 파일 열기
    fp = fopen("price_tag.txt", "r");

    if (fp == NULL) {
        fprintf(stderr, "오류\n");
        return;
    }

    int line[4] = { 3, 2, 2, 2 }; //각 섹션마다 파일에서 건너뛸 라인 수

    for (int cnt_i = 0; cnt_i < 4; cnt_i++) { //4개의 섹션에 대하여 반복
        for (int cnt_j = 0; cnt_j < line[cnt_i]; cnt_j++) { //건너뛸 라인 수만큼 파일에서 읽어옴. 각 섹션의 시작 부분으로 이동
            fgets(buffer, sizeof(buffer), fp); //파일에서 문자열을 읽어와 buffer 배열에 저장
        }
        for (int cnt_k = 0; cnt_k < 5; cnt_k++) { //각 섹션에서 물품의 이름과 가격을 5개씩 읽어와서 items 배열에 저장
            if (fgets(buffer, sizeof(buffer), fp) == NULL) //파일의 끝에 도달하면 루프 종료
                break;

            char* name = strtok(buffer, " \t"); //문자열을 탭으로 분리. 첫 번째 호출에서는 물품의 이름이 추출되어 name 포인터에 저장
            int price = atoi(strtok(NULL, " \t")); //물품의 가격이 추출되어 정수로 변환되어 price에 저장

            strcpy(items[cnt_i][cnt_k].name, name); //물품의 이름을 items 배열에 저장
            items[cnt_i][cnt_k].price = price; //물품의 가격을 items 배열에 저장
        }
    }

    if (fp != NULL) {
        fclose(fp);
    }
}

void print_received_data(Item items[4][5], int budget, char* customername) { //서버로부터 받은 데이터 출력 함수
    printf("\n");
    printf("<< 구매 목록과 금액 >>\n");

for (int i = 0; i < 4; i++) { //4개의 섹션에 대해 반복문 수행
    for (int j = 0; j < 5; j++){ //각 섹션의 물품들에 대해 반복문 수행
        if(j == num[i]){ //j가 num[i]와 같으면, 해당 섹션에서 고객이 선택한 물품
            printf("%s : %d\n", items[i][j].name, items[i][j].price); //선택된 물품의 이름과 가격을 출력
            strcpy(final_item[i].name, items[i][j].name); //final_item 배열에 선택된 물품의 이름 복사 -> 영수증 출력 시 사용
            final_item[i].price = items[i][j].price; //final_item 배열에 선택된 물품의 가격을 저장 -> 영수증 출력 시 사용
            continue;
        }
    }
}
    budget = received_data.budget; //고객이 입력한 예산 정보를 received_data 구조체에서 가져와 budget 변수에 저장
    strcpy(customername, received_data.customerName); //고객의 이름 정보를 received_data 구조체에서 가져와 customername 배열에 저장
    printf("\n");
    printf("예산 : %d\n", budget);
    printf("이름 : %s\n", customername);
 
}

void printReceiptData(Item items[4][5], int budget, char* name) { //영수증 txt 파일 생성 함수
    char filename[30];
    sprintf(filename, "%s's receipt.txt", name);

    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        fprintf(stderr, "Error opening receipt.txt\n");
        return;
    }

    fprintf(fp, "[%s's receipt]\n", name);
    
    //현재 날짜 출력
    time_t timer;
    struct tm* t;
    timer = time(NULL);
    t = localtime(&timer);

    fprintf(fp, "[날짜 : %d년 %d월 %d일]\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
    fprintf(fp, "\n");
    fprintf(fp, "\n");
    fprintf(fp, "%-15s%-15s%-15s%-15s\n", "Product", "Price", "Quantity", "Amount");
    fprintf(fp, "----------------------------------------------------\n");

    int sum = 0;
    for (int cnt_i = 0; cnt_i < 4; cnt_i++) { //선택 물품에 대한 정보 출력
        fprintf(fp, "%-15s%-15d%-15d%-15d\n", final_item[cnt_i].name, final_item[cnt_i].price, number[cnt_i], (final_item[cnt_i].price) * (number[cnt_i]));
        sum += ((final_item[cnt_i].price) * (number[cnt_i]));
    }
    fprintf(fp, "----------------------------------------------------\n");
    fprintf(fp, "합계금액                                     %d원\n", sum);
    fprintf(fp, "----------------------------------------------------\n");
    fprintf(fp, "받은금액                                     %d원\n", received_data.budget);
    fprintf(fp, "거스름돈                                     %d원\n", (received_data.budget) - (sum));
    fprintf(fp, "----------------------------------------------------\n");

    fclose(fp);
}

void sendFileToServer(int sock, const char* filename) { //서버에 txt 파일 전송 함수
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error opening %s\n", filename);
        return;
    }

    char buffer[1024];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, sizeof(char), sizeof(buffer), fp)) > 0) {
        if (send(sock, buffer, bytesRead, 0) == -1) {
            perror("send() : ");
            fclose(fp);
            return;
        }
    }
    fclose(fp);
}

int main(int argc, char *argv[]) { 
    int sock;
    struct sockaddr_in servAddr;
    char send_buf[128];

    char name[20];

    if (argc != 3) {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    FILE* fp = fopen("price_tag.txt", "r");
    if (fp == NULL) {
        fprintf(stderr, "Error opening price_tag.txt\n");
        return 1;
    }

    int section_lines[] = { 3, 2, 2, 4 }; // 각 섹션의 라인 수

    parse_price_file(items);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket()");
        exit(1);
    }

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(argv[1]);
    servAddr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr*)&servAddr, sizeof(servAddr)) == -1) {
        perror("connect()");
        exit(1);
    }

    printf("Input name: ");
    scanf("%s", send_buf);

    if (strcmp(send_buf, "Quit") == 0)
        return 0;

    write(sock, send_buf, strlen(send_buf) + 1); //널문자가 포함되므로 +1을 해줘야 입력한 문자열이 끝까지 나옴. (문자열의 길이에 널 종료 문자를 포함한 값)

    memset(&received_data, 0, sizeof(received_data));

       
    if (read(sock, &received_data, sizeof(received_data)) == -1) {
        perror("read() : ");
        exit(1);
    }

    num[0] = received_data.Fruits; 
    num[1] = received_data.Snacks; 
    num[2] = received_data.Beverages; 
    num[3] = received_data.Vegetables; 

    number[0] = received_data.Fruit_cnt;
    number[1] = received_data.Snack_cnt;
    number[2] = received_data.Beverages_cnt;
    number[3] = received_data.Vegetables_cnt;

    print_received_data(items, budget, name);

    memset(send_buf, 0, sizeof(send_buf));

    printReceiptData(items, budget, name);
    
    char filename[128];
    sprintf(filename, "%s's receipt.txt", name);
    sendFileToServer(sock, filename);

    close(sock);
    return 0;
}
