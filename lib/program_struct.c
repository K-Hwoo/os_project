#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "system.h"

#define PAGE_SIZE 4096 // 4KB 페이지 크기
#define OFFSET_MASK 0xFFF // 12비트 오프셋 마스크

// program 주소 관리 page 구조체 및 함수들

typedef struct { // page struct
    unsigned int data[PAGE_SIZE];
    int page_number;
    int matched_frame; // 각 page와 frame matching된 값
    int is_matched_frame; // 각 page와 frame matching 여부 판단
    size_t first_address; //page first address
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

size_t get_page_first_address(PageManager* page_manager, int get_page_number){//원하는 page의 first address 가져오기
    return page_manager->pages[get_page_number].first_address;
}

void set_first_address(PageManager* page_manager, int page_num, size_t first_addr){//addr넣을때 맨 처음에 이것도 실행
    page_manager->pages[page_num].first_address = first_addr;
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
	    if(page_manager->pages[i].is_matched_frame == 1){
		    matched++; // matching 판단 기준 수정 필요
		}
	}
	if(matched == page_manager->allocated_pages){
	    page_manager->is_memory_loaded = 1;
	    return 1; //all_matched
	}
	else{
	    return 0; // not matched
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
