// structlib.h로 이동

#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stdio.h>
#include <stdlib.h>

#define PAGE_SIZE 4096 // 4KB 페이지 크기
#define TOTAL_MEMORY_SIZE 65536 // 64KB 전체 메모리 크기
#define TOTAL_FRAMES (TOTAL_MEMORY_SIZE / PAGE_SIZE) // 총 프레임 수

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
void map_executables_to_memory(ProgramManager *manager, FrameManager *frame_manager);
void terminate_program(ProgramManager *manager, FrameManager *frame_manager, int program_index);
void enqueue(FrameQueue *queue, Executable *exe);
Executable* dequeue(FrameQueue *queue);
void process_waiting_queue(ProgramManager *manager, FrameManager *frame_manager);

#endif // MEMORY_MANAGER_H
