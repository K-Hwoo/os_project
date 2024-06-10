#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PAGE_SIZE 4096 // 4KB 페이지 크기

#define TOTAL_MEMORY_SIZE 65536 // 64KB 전체 메모리 크기
#define TOTAL_FRAMES (TOTAL_MEMORY_SIZE / PAGE_SIZE) // 총 프레임 수

typedef struct { 
    int frame_number;
    int is_allocated;
    size_t first_address;
} Frame; // 각 frame을 관리

typedef struct {
    Frame frames[TOTAL_FRAMES];
    unsigned char * mem;
} FrameManager; // 전체 frame을 관리

// status가 0인 frame들을 관리하는 양방향 연결 리스트
typedef struct FrameNode {
    Frame frame;
    struct FrameNode * prev;
    struct FrameNode * next;
} FrameNode;

typedef struct {
    FrameNode * head;
    FrameNode * tail;
} FrameList;
//

FrameList * create_empty_frames_list() { // 비어 있는 frame들을 관리하는 양방향 연결 리스트 생성
    FrameList * empty_frames_list = (FrameList *)malloc(sizeof(FrameList));
    empty_frames_list->head = NULL;
    empty_frames_list->tail = NULL;

    return empty_frames_list;
}

// 프레임을 연결리스트에 다시 순서 맞춰서 넣기
void add_empty_frame_sorted(FrameList * empty_frames_list, Frame frame, FrameManager * fm) {
    FrameNode * new_node = (FrameNode *)malloc(sizeof(FrameNode));
    new_node -> frame = frame;
    new_node -> prev = NULL;
    new_node -> next = NULL;

    if (empty_frames_list -> head == NULL) {
        // List is empty
        empty_frames_list -> head = new_node;
        empty_frames_list -> tail = new_node;
    } else if (empty_frames_list -> head -> frame.frame_number >= frame.frame_number) {
        // Insert at the head
        new_node -> next = empty_frames_list -> head;
        empty_frames_list -> head -> prev = new_node;
        empty_frames_list -> head = new_node;
    } else {
        // Insert at the correct position
        FrameNode * current = empty_frames_list -> head;
        while (current -> next != NULL && current -> next -> frame.frame_number < frame.frame_number) {
            current = current->next;
        }
        new_node -> next = current -> next;
        if (current -> next != NULL) {
            new_node -> next -> prev = new_node;
        } else {
            empty_frames_list->tail = new_node;
        }
        current -> next = new_node;
        new_node -> prev = current;
    }

    fm -> frames[frame.frame_number].is_allocated = 0;
}

void print_empty_frames_list(FrameList * empty_frames_list) {
    FrameNode * current = empty_frames_list -> head;
    printf("\n   [ HEAD ] \n\n");
    while (current != NULL) {
        printf("   Frame Number: %3d    First address : %6zu \n", current -> frame.frame_number, current -> frame.first_address);
        current = current->next;
    }
    printf("\n   [ TAIL ] \n\n");
}

// 빈 프레임 연결 리스트 내의 노드 개수 세기
int count_empty_frames(FrameList * empty_frames_list) {
    int count = 0;
    FrameNode * current = empty_frames_list -> head;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}

// 빈 프레임 연결 리스트에서 맨 앞의 프레임 가져오기, 페이지 테이블 할당 될 때 사용
// 항상 먼저 연결 리스트 개수 확인한 후에 실행 (개수가 0 이면 에러 - 0일 때 그냥 함수 실행 안되게)
Frame get_first_empty_frame(FrameList * empty_frames_list, FrameManager * fm) {
    if (empty_frames_list -> head == NULL) {
        Frame error;
        error.frame_number = -1;
        error.is_allocated = -1;
        printf("No empty frame ! \n");
        return error;
    }
    printf("여기는?? - get_first_empty_frame()\n");
    FrameNode * first_node = empty_frames_list -> head;
    Frame first_frame = first_node -> frame;

    fm -> frames[first_frame.frame_number].is_allocated = 1;

    if (empty_frames_list -> head == empty_frames_list -> tail) {
        // Only one node in the list
        empty_frames_list -> head = NULL;
        empty_frames_list -> tail = NULL;
    } else {
        empty_frames_list -> head = first_node -> next;
        empty_frames_list -> head -> prev = NULL;
    }

    free(first_node);

    return first_frame;
}

// 처음 Frame Mangager 생성 될 때 리스트 세팅
void first_frame_list_set(FrameList * empty_frames_list, Frame frame) {
    FrameNode * new_node = (FrameNode *)malloc(sizeof(FrameNode));
    new_node -> frame = frame;
    new_node -> prev = NULL;
    new_node -> next = NULL;

    if (empty_frames_list -> head == NULL) {
        // List is empty
        empty_frames_list -> head = new_node;
        empty_frames_list -> tail = new_node;
    } else {
        empty_frames_list -> tail -> next = new_node;
        new_node -> prev = empty_frames_list -> tail;
        empty_frames_list -> tail = new_node;
    }
}

// empty frames list 생성 후에 포인터 전달
FrameManager * create_frame_manager(unsigned char * dummy_physical_mem, FrameList * empty_frames_list) {
    // 전체 frame을 묶어서 관리하는 자료구조
    FrameManager * frame_manager = (FrameManager *)malloc(sizeof(FrameManager)); 
    frame_manager -> mem = dummy_physical_mem; // 할당받은 dummy 물리 메모리 공간을 연결

    size_t addr = 0;
    
    for (int i = 0; i < TOTAL_FRAMES; i++) {
        frame_manager -> frames[i].frame_number = i; // 정해지고 변하지 않음
        frame_manager -> frames[i].is_allocated = 0; // 계속 변하는 값
        frame_manager -> frames[i].first_address = addr; // 정해지고 변하지 않음
        addr += 4096;

        // 처음 frame manager가 setting 될 때, 모든 frame을 양방향 연결 리스트에 연결
        first_frame_list_set(empty_frames_list, frame_manager->frames[i]);
    }

    printf("Frame 관리 자료구조 세팅 완료 ...");

    return frame_manager;
}

void show_frame_status(FrameManager * frame_manager) {
    printf("%15s  %9s  %15s \n", "Frame Number", "Status", "First Address");
    printf("   ======================================== \n");
    for (int i = 0; i < TOTAL_FRAMES; i++) {
        printf("%13d  %9d  %17zu \n", 
        frame_manager->frames[i].frame_number, frame_manager->frames[i].is_allocated, frame_manager->frames[i].first_address);
    }
}

// 동적 할당 메모리 free 함수 모음

void free_frame_manager(FrameManager * frame_manager) {
    free(frame_manager);
}

void free_empty_frames_list(FrameList * empty_frames_list) {
    FrameNode * current = empty_frames_list -> head;
    while (current != NULL) {
        FrameNode * temp = current;
        current = current -> next;
        free(temp);
    }
    free(empty_frames_list);
}



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
    
    // sizeof(Page): 16408, 16개의 page가 들어갈 수 있는 동적 주소 할당
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
            if (page_manager->pages[i].data[j] != 0) 
                printf("주소: %d, %d페이지, %d번째 - set_page_data()\n", page_manager->pages[i].data[j], i, j);
        }
        page_manager->pages[i].is_matched_frame++; // 페이지별 프레임 매칭될 때 마다 증가
    }
    printf("set_page_data() success\n");
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

// page에 frame 주소 채우기
void fill_frames(unsigned char *virtual_physical_memory, PageManager *page_manager, FrameList *fl, FrameManager *fm, unsigned int *byte) {

    // 프로그램 읽어서 바이트를 pages[].data[]에 넣어줘야 됨
    // 프로그램 읽어오는 함수로 Bytes 읽어오고 읽어온 값으로
    // page 4096개 묶음으로 나누고 배열에 넣으면서 page # 부여

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
        printf("%d %d - fill_frames() 내 get_first_empty_frame()\n", page_manager->pages[i].matched_frame, i);
    }
    ////////////// 위까지 프레임 매칭 ////////////////////
    printf("fill_frames() 절반 수행됨\n");
    // 그리고 frame.first_address를 시작 주소로 두고
    // virtual_memory + frame.first_address부터 4096번 for문 반복해서 data의 프로그램 주소 대입
    // -> physical memory에 값 입력하는 과정. tmux 가운데 pane에 표시되는 주소 데이터값을 변경 시켜줌
    unsigned char *addr_ptr;
    
    for (int j = 0; j < TOTAL_FRAMES; j++) {
        int first_address = page_manager->pages[j].data[0];
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