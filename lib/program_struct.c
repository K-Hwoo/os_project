#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "system.h"
#include "structlib.h"

// program 주소 관리 page 구조체 및 함수들

typedef struct { // page struct
    unsigned int data[PAGE_SIZE];
    int page_number;
    int matched_frame; // 각 page와 frame matching된 값
    int is_matched_frame; // 각 page와 frame matching 여부 판단
} Page;

typedef struct { // page manager struct
    Page *pages; // dynamic allocation
    int allocated_pages; // allocated pages
    int is_memory_loaded; // is making page table finished?
} PageManager;

PageManager* create_page_manager(int total_page_size) { // create page manager, page data는 비어있는 것으로 생성
    PageManager* page_manager = (PageManager*)malloc(sizeof(PageManager));
    if (page_manager == NULL) { // error
        return NULL;
    }
    page_manager->pages = (Page*)malloc(total_page_size * sizeof(Page));
    if (page_manager->pages == NULL) { // error
        free(page_manager);
        return NULL;
    }
    for (int i = 0; i < total_page_size; i++) {
        page_manager->pages[i].page_number = i;
		page_manager->pages[i].is_matched_frame = -1;
    }
    page_manager->allocated_pages = total_page_size;
    page_manager->is_memory_loaded = 0; // not loaded
   
    return page_manager;
}

void remove_page_manager(PageManager* page_manager) { // remove page manager
    free(page_manager->pages); // free pages
    free(page_manager); // free page manager
}

void show_total_page_status(PageManager* page_manager) {//전체 페이지
    printf("number_of_pages    page_is_memory_loaded\n");
    printf("      %d                       %d       \n", page_manager->allocated_pages, page_manager->is_memory_loaded);
}

void show_page_status(PageManager* page_manager, int page_num){//페이지 한 개
    printf("page_number           is_matched_frame          first_address\n");
    printf("      %d                     %d                     %ld\n", page_manager->pages[page_num].page_number, page_manager->pages[page_num].is_matched_frame, page_manager->pages[page_num].first_address);
}

// page하나당 is_matched_frame 1씩 증가
// 따라서 전체 페이지 수와 is_matched_frame과 값이 다르면 memory load 안됨.
void set_page_data(PageManager* page_manager, unsigned int *byte) { ///? two for loop
    int k = 0;
    for (int i = 0; i < page_manager->allocated_pages; i++) {
        for (int j = 0; j < PAGE_SIZE / sizeof(int); j++) {
            page_manager->pages[i].data[j] = byte[k];
            k++;
        }
        page_manager->pages[i].is_matched_frame++; // 페이지별 프레임 매칭될 때 마다 증가
    }
    // for문을 함수 밖에 놓으면 함수가 실행될 때마다 k값이 초기화돼서 안으로 옮겨주었습니다
    // 그리고 unsigned int인 걸 고려해서 PAGE_SIZE / sizeof(int)만큼만 반복합니다
}

unsigned int get_page_data(PageManager* page_manager, int page_num, int i){//밖에서for문으로 page_data받아오기(4kb돌면서)
    return page_manager->pages[page_num].data[i];
}
void set_matched_frame(PageManager* page_manager, int page_num){ //page table에 frame과 match되었음
    page_manager->pages[page_num].is_matched_frame = 1;
}

int get_matched_frame(PageManager* page_manager, int page_num){//page table에 frame과 match되었는지 확인
    return page_manager->pages[page_num].is_matched_frame;
}

int check_memory_loaded(PageManager* page_manager){ // Manager가 관리하는 page들 matching 여부 확인 및 변경
    int matched=0;
    for(int i = 0; i < page_manager->allocated_pages; i++){
	    if(page_manager->pages[i].is_matched_frame == 1){ // set_page_data 함수와 연계
		    matched++;
		}
	}
	if(matched == page_manager->allocated_pages){
	    page_manager->is_memory_loaded = 1;
	    return 1; // all_matched
	}
	else{
	    return 0; // not matched
	}
}

// page에 frame 주소 채우기. 총 5개의 인자 받아옴.
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
    // -> physical memory에 값 입력하는 과정. tmux 가운데 pane에 표시되는 주소 데이터값을 변경 시켜줌
    unsigned char *addr_ptr;
    
    for (int j = 0; j < TOTAL_FRAMES; j++) {
        int first_address = (int)get_first_empty_frame(fl, fm).first_address;
        addr_ptr = virtual_physical_memory + first_address;
        
        for (int i = 0; i < PAGE_SIZE; i++) {
            *(addr_ptr + i) = page_manager->pages[j].data[i];
        }
    }
}

// 페이지 테이블 생성자 (분할 화면에 띄우는 역할은 아직 X)
void show_pf_table(PageManager *page_manager, FrameManager *frame_manager) {
    if (page_manager == NULL) {
        fprintf(stderr, "PageManager is NULL\n");
        return;
    }

    // matching 여부 파악해서 matching 됐으면 페이지 테이블 출력
    if (check_memory_loaded(page_manager) == 1) {
        
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

/*
pool 에 첫번째로 들어가

fl 에서 가용가능한 프레임 갯수를 확인한다음에


===========
page num     frame
0       |  13   40000 44960

1       |  2

2       |  8

3       |  4

4       |  x
==========
return


추가 함수를 만들어서
fl을 인자로 받고
frame 칸에다 하나씩 채워

virtual_physical_memory로 접근을 해서

void * memory_set(void * ptr, int value, size_t num) {
    unsigned char * p = (unsigned char *)ptr;
    while (num--) {
        *p++ = (unsigned char)value;
    }
    return ptr;
}
40000 44960
for문을 돌려  0 4095
virtual_physical_memory++ = 페이지 한 오프셋 값이 메모리 번지 하나에 계속 채워지게


신호 보내서 그 분할 화면에 프로그램 띄우기
if memory_load == 1{
    "tmux send-keys -t %s.%s 'cat %s' C-m"
}



===========
page num     frame
0       |

1       |

2       |

3       |

4       |
==========


===========
page num     frame
0       |

1       |

2       |

3       |

4       |
==========


===========
page num     frame
0       |

1       |

2       |

3       |

4       |
==========


===================================
===========
page num     frame
0       |  x

1       |  x

2       |  x

3       |  x

4       |  x
==========
===================================
*/
