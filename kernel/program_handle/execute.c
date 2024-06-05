#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "system.h"
#include "structlib.h"

#define TOTAL_MEMORY_SIZE 65536 // 64KB 전체 메모리 크기
#define PAGE_SIZE 4096 // 4KB 페이지 크기
#define OFFSET_MASK 0xFFF // 12비트 오프셋


int read_program(char * path) { // 프로그램 파일 경로
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        perror("fopen");
        return 1;
    }

    unsigned int *byte = (unsigned int *)malloc(TOTAL_MEMORY_SIZE * sizeof(unsigned int));
    if (byte == NULL) {
        perror("malloc");
        fclose(fp);
        return NULL;
    }
    
    int total_bytes = 0;
    char select; // 읽어온 값 선택하는 변수
    
    // 읽어온 바이트 값이 많으므로 선택적 처리
    while (1) {
        printf("읽어온 바이트 값을 출력하시겠습니까? (y/n): ");
        scanf(" %c", &select);
        if (select == 'y' || select == 'n') break;
        else printf("잘못된 입력입니다. 다시 입력해주세요.");
    }
    
    int i = 0;
    if (select == 'y') {
        while (i < TOTAL_MEMORY_SIZE && (byte[i] = fgetc(fp)) != EOF) {
            // 읽어온 바이트 값을 이용하여 원하는 작업을 수행합니다.
            // 예를 들어, 바이너리 데이터의 특정 부분을 확인하거나 처리할 수 있습니다.
            // 여기서는 총 읽은 바이트 수를 출력하고 카운트합니다.
            total_bytes++;
            printf("%02X ", byte[i]); // 예시로 읽어온 바이트 값을 16진수로 출력하는 예시
            i++;
        }
        i = 0;
    }

    printf("\nTotal bytes read: %d\n", total_bytes);

    // page manager 생성 후 data에 프로그램 주소 할당
    PageManager *page_manager = create_page_manager(total_bytes / (PAGE_SIZE / sizeof(int)));
    set_page_data(page_manager, byte);

    // fill_frame()   <---- 어떻게 불러올지 관건

    check_all_matched(page_manager);
    if (page_manager->is_memory_loaded == 1) {
       프로세서 pool에 넣는다.
    }

    else { // memory load가 됐을 경우
        //enqueue한다
        return;
    }

    fclose(fp);
    return 0;
}

int execute() {
    char path[30]; // 프로그램 이름을 저장할 배열
    char full_path[40];

    printf("실행할 프로그램을 입력하세요 : ");
    scanf("%s", path); // 사용자로부터 파일 경로를 입력 받음

    strcpy(full_path, "program/");
    strcat(full_path, path);

    // read_program 함수로 프로그램 주소 저장
    unsigned int *result = read_program(full_path);
    if (result != 0) {
        printf("디렉토리에 입력한 프로그램이 없습니다. \n");
        return 1;
    }

    if ()

    return 0;
}

/*
unsigned int *read_program(char * path) { // 프로그램 파일 경로
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        perror("fopen");
        return 1;
    }

    unsigned int *byte = (unsigned int *)malloc(TOTAL_MEMORY_SIZE * sizeof(unsigned int));
    if (byte == NULL) {
        perror("malloc");
        fclose(fp);
        return NULL;
    }
    
    int total_bytes = 0;
    char select; // 읽어온 값 선택하는 변수
    
    // 읽어온 바이트 값이 많으므로 선택적 처리
    while (1) {
        printf("읽어온 바이트 값을 출력하시겠습니까? (y/n): ");
        scanf(" %c", &select);
        if (select == 'y' || select == 'n') break;
        else printf("잘못된 입력입니다. 다시 입력해주세요.");
    }
    
    int i = 0;
    if (select == 'y') {
        while (i < TOTAL_MEMORY_SIZE && (byte[i] = fgetc(fp)) != EOF) {
            // 읽어온 바이트 값을 이용하여 원하는 작업을 수행합니다.
            // 예를 들어, 바이너리 데이터의 특정 부분을 확인하거나 처리할 수 있습니다.
            total_bytes++;
            printf("%02X ", byte[i]); // 예시로 읽어온 바이트 값을 16진수로 출력하는 예시
            i++;
        }
        i = 0;
    }
    while (i < TOTAL_MEMORY_SIZE && (byte[i] = fgetc(fp)) != EOF) {
            // 여기서는 총 읽은 바이트 수를 카운트합니다.
            total_bytes++;
            i++;
    }
    printf("\nTotal bytes read: %d\n", total_bytes);

    fclose(fp);
    return byte;
}

unsigned int *execute() {
    char path[30]; // 프로그램 이름을 저장할 배열
    char full_path[40];

    printf("실행할 프로그램을 입력하세요 : ");
    scanf("%s", path); // 사용자로부터 파일 경로를 입력 받음

    strcpy(full_path, "program/");
    strcat(full_path, path);

    // read_program 함수 호출
    unsigned int *result = read_program(full_path);
    if (*result == 0) {
        printf("디렉토리에 입력한 프로그램이 없습니다. \n");
        return;
    }

    return result;
}
*/
