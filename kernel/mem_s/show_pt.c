#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test_memory_manager.h"

void show_pt(Executable *exe, FrameManager *frame_manager) {
    if (exe == NULL) {
        fprintf(stderr, "Executable is NULL\n");
        return;
    }

    printf("┌──────────────────────────────────────────────────┐\n");
    printf("│             Page Table for Executable            │\n");
    printf("├────────────┬─────────────────────────────────────┤\n");
    printf("│ Total Pages│ %-35d │\n", exe->total_pages);
    printf("│ Total Bytes│ %-35ld │\n", exe->total_bytes);
    printf("├────────────┼────────────┬────────────────────────┤\n");
    printf("│ Page Number│ Frame Addr │ Page Start Address     │\n");
    printf("├────────────┼────────────┼────────────────────────┤\n");

    for (int i = 0; i < exe->total_pages; i++) {
        unsigned char *frame_address = exe->page_table[i].start_address;
        int frame_number = (frame_address - frame_manager->virtual_memory) / PAGE_SIZE;

        printf("│ %-10d │ %-10d │ %-22p │\n", 
            exe->page_table[i].page_number, 
            frame_number, 
            exe->page_table[i].start_address);
    }

    printf("└────────────┴────────────┴────────────────────────┘\n");
}

// 페이지 테이블 보여주는 함수
int show_page_table() {
    // 프로그램 관리자와 실행 파일 생성
    ProgramManager *manager = create_program_manager();
    Executable *exe = create_executable();

    // 가상 메모리 할당 및 프레임 관리자 생성
    unsigned char *virtual_memory = (unsigned char *)malloc(TOTAL_MEMORY_SIZE);
    FrameManager *frame_manager = create_frame_manager(virtual_memory);

    // 예제 프로그램 읽기 (파일 경로는 필요에 따라 수정)
    char *file_path = "program/up_down_game";
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

    // 페이지 테이블 출력
    show_pt(exe, frame_manager);

    // 프로그램 종료 처리 (예제: 첫 번째 프로그램 종료)
    terminate_program(manager, frame_manager, 0);

    // 할당된 메모리 해제
    free_program_manager(manager);
    free_frame_manager(frame_manager);
    free(virtual_memory);

    return 0;
}
