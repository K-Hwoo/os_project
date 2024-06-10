#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "system.h"
#include "print_util.h"

int main() {
    system("clear");
    char * input;

    // 메모리 setting
    void * virtual_physical_memory = make_dummy_physical_memory(); // in mem_allocate.c

    // ProgrogramManager * pm = create_program_manager( ...... );
    // ProgramPool
    // ProgramQueue

    // 프레임 종합 관리
    FrameList * fl = create_empty_frames_list();
    FrameManager * fm = create_frame_manager(virtual_physical_memory, fl);

    // page manager 생성 후 data에 프로그램 주소 할당
    PageManager *page_manager = create_page_manager(TOTAL_FRAMES);

    // 필요할 때 사용하자
    // const char * tmux_session_name = "terminal";
    // const char * tmux_pane = "1";

    print_minios("");
    printWithDelay("Welcome to the Memory Management Simulation.", 30000);
    print_minios("");
    printWithDelay("================================================", 10000);
    print_minios("");


    // // ==================================== 테스트공간
    // // 프로그램 주소 읽어오는 test
    // unsigned int *test = execute();
    
    // print_page(test);

    // // fill_frames(virtual_physical_memory, pm, fl, test);
    // // show_pf_table(pm, fm);
    // // =====================================


    sleep(1);

    memory_view(virtual_physical_memory, 0, 25); // in mem_allocate.c

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
            show_pf_table(page_manager, fm);
        }

        else if (strcmp(input, "execute") == 0) {
            read_program(virtual_physical_memory, fl, fm, page_manager);
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

// 문제: system.h에 structlib.h를 include하지 않고 Frame 구조체 가져오기.
//      read_program의 입력 인자들을 변경했는데, execute 함수에서 어떻게 그 인자들(vpm, fl, 등)을 가져올지.

// 2차 문제: 위 문제를 system.h에 structlib.h를 include하고
//          frame과 page등의 정의들을 한 곳으로 모아서 헤더파일 참조 안 해도되게 대충 해결한 뒤 실행하니
//          execute.c에서 fill_frames 내부에서 "No Empty Frame ! "을 16번 출력함.
//          각 페이지별 프레임 매칭하는 부분의 오류.
//
//          kernel.c에서 fm과 fl 만드는 부분에서 문제가 있는 건 아닌 거 같은데,
//          아마 page_manager가 서로 달라서 그런 게 아닐까? 왜냐면 execute는 page_manager를 안 받음.
//          -> 같게 만들어도 똑같다.
//
//          first_address 지운 거 때문이었음.
//          이거 해결하니까 이제 segmentation fault가 나타남 하ㅏ
//
//          fill_frames 내부에 문제가 있음. 확인하기

//          24.06.10 / 18:00
//          알고보니 set_page_data에서 메모리 잘못 참조하는 거였음.
//          그러나 해결방법을 몰라서 현재 알아보는 중