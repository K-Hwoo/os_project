#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "system.h"
#include "print_util.h"
#include "structlib.h"

void fill_frames(unsigned char *virtual_physical_memory, PageManager *page_manager, FrameList *fl, FrameManager *fm, unsigned int *byte);

// Page# & Frame# 표 생성자 (분할 화면에 띄우는 역할은 아직 X)
void show_pf_table(PageManager *page_manager, FrameManager *frame_manager);

int main() {
    system("clear");
    char * input;

    // 메모리 setting
    void * virtual_physical_memory = make_dummy_physical_memory(); // in memory_allocate.c

    // ProgrogramManager * pm = create_program_manager( ...... );
    // ProgramPool
    // ProgramQueue

    FrameList * fl = create_empty_frames_list();
    FrameManager * fm = create_frame_manager(virtual_physical_memory, fl);
    //

    // 필요할 때 사용하자
    // const char * tmux_session_name = "terminal";
    // const char * tmux_pane = "1";

    print_minios("");
    printWithDelay("Welcome to the Memory Management Simulation.", 30000);
    print_minios("");
    printWithDelay("================================================", 10000);
    print_minios("");


    // // ==================================== 작업공간
    // // 프로그램 주소 읽어오는 test
    // unsigned int *test = execute();
    
    // print_page(test);

    // // fill_frames(virtual_physical_memory, pm, fl, test);
    // // show_pf_table(pm, fm);
    // // =====================================




    sleep(1);

    memory_view(virtual_physical_memory, 0, 25); // in memory_allocate.c

    while(1) {
        // readline을 사용하여 입력 받기
        input = readline("커맨드를 입력하세요(종료:exit) : ");

        if (strcmp(input,"exit") == 0) {
            printWithDelay("Bye, See you next time.", 30000);
            sleep(1);
            free(input);
            system("tmux kill-session -t terminal");
            break;
        }
        
        if (strcmp(input, "minisystem") == 0) {
            minisystem();
        }

        else if (strcmp(input, "help") == 0) {
            print_minios("Sorry, It will be implemented soon.");
            print_minios("");
        }

        else if (strcmp(input, "show_m") == 0) {
            size_t first, end;

            print_minios("Memory addresses are accessible from 0 to 65535.");
            print_minios("");
            printf("Which address should you print first : ");
            scanf("%zu", &first);
            printf("Which address should you print end : ");
            scanf("%zu", &end);
            print_minios("");
            memory_view(virtual_physical_memory, first, end);
        }
     
        else if (strcmp(input, "show_f") == 0) {
            show_frame_status(fm);
        }
        
        else if (strcmp(input, "show_efl") == 0) {
            print_empty_frames_list(fl);
        }

        else if (strcmp(input, "show_pt") == 0) {

        }

        else if (strcmp(input, "execute") == 0) {
            execute();
        }

        else if (strcmp(input, "terminate") == 0) {
        }

        // 테스트용
        else if (strcmp(input, "pop_f") == 0) {
            printf("첫번째거 팝 완료");
            get_first_empty_frame(fl, fm);
        }


        else if (strcmp(input, "push") == 0) {
            int b;
            printf("몇번 넣어? ");
            scanf("%d", &b);
            printf("넣기 완료");
            add_empty_frame_sorted(fl, fm->frames[b], fm);
        }

        else {
            system(input); 
            print_minios("");
        }
    }

    return 0;
}

// page에 frame 주소 채우기. 총 5개의 인자 받아옴. 많은 거 같기도..
void fill_frames(unsigned char *virtual_physical_memory, PageManager *page_manager, FrameList *fl, FrameManager *fm, unsigned int *byte) {

    // 프로그램 읽어서 바이트를 pages[].data[]에 넣어줘야 됨
    // 프로그램 읽어오는 함수로 Bytes 읽어오고 읽어온 값으로
    // page 4096개 묶음으로 나누고 배열에 넣으면서 page # 부여

    set_page_data(page_manager, byte); // execute.c로 옮기면 코드가 중복되므로 지우기

    // count_empty_frames으로 fl에 frame 몇 개 남았나 확인하고
    // page 개수보다 모자라면 return
    if (count_empty_frames(fl) < TOTAL_FRAMES) {
        printf("프레임 개수가 페이지보다 부족합니다.\n");
        return;
    }
    // 안 모자라면 get_first_empty_frame 함수로 하나씩 가져와서 frame.frame_number 매칭
    for (int i = 0; i < TOTAL_FRAMES; i++) {
        // 각 페이지별 프레임 매칭
        page_manager->pages[i].matched_frame = get_first_empty_frame(fl, fm).frame_number;
        
        // 각 페이지별 프레임 매칭 여부 설정
        set_matched_frame(page_manager, i);
    }
    ////////////// 위까지 프레임 매칭 ////////////////////
    
    // 그리고 frame.first_address를 시작 주소로 두고
    // virtual_memory + frame.first_address부터 4096번 for문 반복해서 data의 프로그램 주소 대입
    // physical memory에 값 입력하는 과정. tmux 가운데 pane에 표시되는 주소 데이터값을 변경 시켜줌
    unsigned char *addr_ptr;
    for (int j = 0; j < TOTAL_FRAMES; j++) {
        for (int i = 0; i < PAGE_SIZE; i++) {
            addr_ptr = virtual_physical_memory + get_first_empty_frame(fl, fm).first_address;
            *addr_ptr = page_manager->pages[j].data[i];
        }
    }
    



    // 이 pages를 page_manager로 관리 ------------------------------

}

// 페이지 테이블 생성자 (분할 화면에 띄우는 역할은 아직 X)
void show_pf_table(PageManager *page_manager, FrameManager *frame_manager) {
    if (page_manager == NULL) {
        fprintf(stderr, "PageManager is NULL\n");
        return;
    }

    // matching 여부 파악해서 matching 됐으면 페이지 테이블 출력
    if (check_all_matched(page_manager) == 1) {
        
        printf("┌────────────┬────────────┐\n");
        printf("│   Page #   │   Frame #  │\n");
        printf("├────────────┼────────────┤\n");

        for (int i = 0; i < page_manager->allocated_pages; i++) {
            printf("│ %-10d │ %-10d │\n", 
                    page_manager->pages[i].page_number,
                    page_manager->pages[i].matched_frame
                    );
        }
        printf("└────────────┴────────────┘\n");
    }

    else {
        printf("페이지가 프레임과 매칭이 되지 않았습니다.\n");
    }
}


// page 출력 결과 테스트용
void print_page(unsigned int *byte) {
    size_t elements_per_page = PAGE_SIZE / sizeof(unsigned int);
    size_t start = 0 * elements_per_page; // 1페이지라고 가정
    size_t end = start + elements_per_page;

    // 마지막 페이지의 경우, 총 바이트 수를 넘어갈 수 있으므로 끝을 조정합니다.
    if (end > 33600 / sizeof(unsigned int)) { // 33600: up_down_game 크기. 크기 읽어오는 부분 구현 필요
        end = 33600 / sizeof(unsigned int);
    }

    printf("Page %d (addresses %zu to %zu):\n", 1, start, end - 1); // 1페이지
    for (size_t i = start; i < end; i++) {
        printf("%02X ", byte[i]); // 4바이트씩 16진수로 출력
        if ((i + 1) % 4 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

// fill_frame 함수를 program_struct.c로 옮겨야 함.