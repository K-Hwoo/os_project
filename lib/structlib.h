#ifndef __STRUCTLIB_H__

#define PAGE_SIZE 4096 // 4KB 페이지 크기
#define OFFSET_MASK 0xFFF // 12비트 오프셋

#define TOTAL_MEMORY_SIZE 65536 // 64KB 전체 메모리 크기
#define TOTAL_FRAMES (TOTAL_MEMORY_SIZE / PAGE_SIZE) // 총 프레임 수

// frame_struct.c 에 있는 구조체 및 함수들

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

FrameList * create_empty_frames_list();
void add_empty_frame_sorted(FrameList * empty_frames_list, Frame frame, FrameManager * fm);
void print_empty_frames_list(FrameList * empty_frames_list); 
int count_empty_frames(FrameList * empty_frames_list);
Frame get_first_empty_frame(FrameList * empty_frames_list, FrameManager * fm);
Frame remove_node_at_position(FrameList * empty_frames_list, int position);


FrameManager * create_frame_manager(unsigned char * dummy_physical_mem, FrameList * empty_frames_list);
void free_frame_manager(FrameManager * frame_manager);
void show_frame_status(FrameManager * frame_manager);
void first_frame_list_set(FrameList * empty_frames_list, Frame frame);


void free_frame_manager(FrameManager * frame_manager);
void free_empty_frames_list(FrameList * empty_frames_list);


// program_struct.c 에 있는 구조체들 및 함수들

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

PageManager* create_page_manager(int total_page_size); // create page manager, page data는 비어있는 것으로 생성
void remove_page_manager(PageManager* page_manager); // remove page manager

void show_total_page_status(PageManager* page_manager); // 전체 페이지
void show_page_status(PageManager* page_manager, int page_num); // 페이지 한 개

void set_page_data(PageManager* page_manager, unsigned int *byte); // 페이지에 data 넣으면서 matched_frame 1로 설정해줌
unsigned int get_page_data(PageManager* page_manager, int page_num, int i); // 밖에서for문으로 page_data받아오기(4096번 돌면서)

void set_matched_frame(PageManager* page_manager, int page_num); // page table에 frame과 match되었음
int get_matched_frame(PageManager* page_manager, int page_num); // page table에 frame과 match되었는지 확인
int check_memory_loaded(PageManager* page_manager); // Manager가 관리하는 page들 matching 여부 확인 및 변경

void fill_frames(unsigned char *virtual_physical_memory, PageManager *page_manager, FrameList *fl, FrameManager *fm, unsigned int *byte);
void show_pf_table(PageManager *page_manager, FrameManager *frame_manager);


typedef struct Process {
    PageManager* page_manager;
    struct Process* next;
    char process_name[30];
} Process;

// 링크드 리스트 관리를 위한 구조체
typedef struct ProcessPool {
    Process* head;
} ProcessPool;

void CreateProcessPool(); // 프로세스 풀 생성

// 링크드 리스트에 프로세스 추가
void addProcess(ProcessPool* pool, PageManager* page_manager, char* name);

// 링크드 리스트에서 특정 프로세스를 제거
void removeProcess(ProcessPool* pool, char* name);

// 링크드 리스트에 있는 모든 프로세스 정보 출력
void printProcesses(ProcessPool* pool);

// 메모리 해제를 위해 링크드 리스트의 모든 노드 삭제
void freeProcessPool(ProcessPool* pool);

#endif