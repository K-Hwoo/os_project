#define PAGE_SIZE 4096 // 4KB 페이지 크기
#define OFFSET_MASK 0xFFF // 12비트 오프셋

#define TOTAL_MEMORY_SIZE 65536 // 64KB 전체 메모리 크기
#define TOTAL_FRAMES (TOTAL_MEMORY_SIZE / PAGE_SIZE) // 총 프레임 수

// program_struct.c 에 있는 구조체들 및 함수들
// program 주소 관리 page 구조체 및 함수들

typedef struct { // page struct
    int data[PAGE_SIZE];
    int page_number;
	int matched_frame; //page_table만들때 frame과 matching이 되었는가?
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

void set_page_data(PageManager* page_manager, unsigned int *byte); // for문으로 i, j loop 돌리기
int get_page_data(PageManager* page_manager, int page_num, int i); // 밖에서for문으로 page_data받아오기(4096번 돌면서)

size_t get_page_first_address(PageManager* page_manager, int get_page_number); // 원하는 page의 first address 가져오기
void set_first_address(PageManager* page_manager, int page_num, size_t first_addr); // addr넣을때 맨 처음에 이것도 실행

void set_matched_frame(PageManager* page_manager, int page_num); // page table에 frame과 match되었음
int get_matched_frame(PageManager* page_manager, int page_num); // page table에 frame과 match되었는지 확인
int check_all_matched(PageManager* page_manager); // Manager가 관리하는 page들 matching 여부 확인 및 변경


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
//

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
