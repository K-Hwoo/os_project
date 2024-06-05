#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test_memory_manager.h"

void show_detail(ProgramManager *manager) {
    if (manager == NULL) {
        fprintf(stderr, "ProgramManager is NULL\n");
        return;
    }

    printf("┌─────────────────────────┐\n");
    printf("│   Detailed Information  │\n");
    printf("├────────────┬────────────┤\n");
    printf("│ Executable │ Page Number│\n");
    printf("├────────────┼────────────┤\n");

    for (int i = 0; i < manager->total_executables; i++) {
        Executable *exe = manager->executables[i];
        for (int j = 0; j < exe->total_pages; j++) {
            printf("│ %-10d │ %-10d │ %p \n", 
                   i, 
                   exe->pages[j]->page_number, 
                   exe->page_table[j].start_address);
        }
    }

    printf("└────────────┴────────────┘\n");
}

int show_detail_table() {
    // 프로그램 관리자와 실행 파일 생성
    ProgramManager *manager = create_program_manager();
    Executable *exe = create_executable();

    // 가상 메모리 할당 및 프레임 관리자 생성
    unsigned char *virtual_memory = (unsigned char *)malloc(TOTAL_MEMORY_SIZE);
    FrameManager *frame_manager = create_frame_manager(virtual_memory);

    // 파일 경로 설정
    char file_path[256] = "program/up_down_game";

    // 예제 프로그램 읽기
    if (read_program(file_path, exe) != 0) {
        fprintf(stderr, "프로그램을 읽는 데 실패했습니다\n");
        free_executable(exe);
        free_program_manager(manager);
        free_frame_manager(frame_manager);
        free(virtual_memory);
        return 1;
    }

    // 실행 파일을 프로그램 관리자에 추가
    if (add_executable(manager, exe) != 0) {
        fprintf(stderr, "실행 파일을 프로그램 관리자에 추가하는 데 실패했습니다\n");
        free_executable(exe);
        free_program_manager(manager);
        free_frame_manager(frame_manager);
        free(virtual_memory);
        return 1;
    }

    // 메모리에 실행 파일 매핑
    map_executables_to_memory(manager, frame_manager);

    // 프로세스의 상세 정보 출력
    show_detail(manager);

    // 프로그램 종료 처리 (예제: 첫 번째 프로그램 종료)
    terminate_program(manager, frame_manager, 0);

    // 할당된 메모리 해제
    free_program_manager(manager);
    free_frame_manager(frame_manager);
    free(virtual_memory);

    return 0;
}
