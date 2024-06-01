//test_first
#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stdio.h>
#include <stdlib.h>

#define PAGE_SIZE 4096 // 4KB 페이지 크기
#define TOTAL_MEMORY_SIZE 65536 // 64KB 전체 메모리 크기
#define TOTAL_FRAMES (TOTAL_MEMORY_SIZE / PAGE_SIZE) // 총 프레임 수
#define MAX_PROCESS_NAME_LENGTH 50
#define MAX_PROCESSES 100

typedef struct {
    unsigned char data[PAGE_SIZE];
    int page_number;
} Page;

typedef struct {
    int page_number;
    unsigned char *start_address;
} PageTableEntry;

typedef struct {
    Page **pages;
    PageTableEntry *page_table;
    int total_pages;
    long total_bytes;
} Executable;

typedef struct {
    Executable **executables;
    int total_executables;
} ProgramManager;

typedef struct {
    int frame_number;
    int is_allocated;
} Frame;

typedef struct FrameQueueNode {
    Executable *exe;
    struct FrameQueueNode *next;
} FrameQueueNode;

typedef struct {
    FrameQueueNode *front;
    FrameQueueNode *rear;
} FrameQueue;

typedef struct {
    Frame frames[TOTAL_FRAMES];
    unsigned char *virtual_memory;
    FrameQueue waiting_queue;
} FrameManager;

// ProcessControlBlock와 ProcessPool 정의
typedef struct {
    int process_id;
    char process_name[MAX_PROCESS_NAME_LENGTH + 1];
    unsigned long base_address;
    unsigned long memory_size;
} ProcessControlBlock;

typedef struct {
    ProcessControlBlock processes[MAX_PROCESSES];
    int num_processes;
} ProcessPool;

// Function prototypes
Executable* create_executable();
ProgramManager* create_program_manager();
FrameManager* create_frame_manager(unsigned char *virtual_memory);
int add_page(Executable *exe, Page *page);
int add_executable(ProgramManager *manager, Executable *exe);
int read_program(char *path, Executable *exe);
void free_executable(Executable *exe);
void free_program_manager(ProgramManager *manager);
void free_frame_manager(FrameManager *frame_manager);
int allocate_frame(FrameManager *frame_manager);
void map_executables_to_memory(ProgramManager *manager, FrameManager *frame_manager, ProcessPool *pool);
void terminate_program(ProgramManager *manager, FrameManager *frame_manager, ProcessPool *pool, int program_index);
void enqueue(FrameQueue *queue, Executable *exe);
Executable* dequeue(FrameQueue *queue);
void process_waiting_queue(ProgramManager *manager, FrameManager *frame_manager, ProcessPool *pool);
void load_process(ProcessPool* pool, int pid, const char* name, unsigned long base_addr, unsigned long mem_size);
void display_process_info(ProcessControlBlock* pcb);

#endif // MEMORY_MANAGER_H


