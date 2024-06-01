//test_cfile
#include "memory_manager.h"
#include <string.h>

Executable* create_executable() {
    Executable *exe = (Executable *)malloc(sizeof(Executable));
    exe->pages = NULL;
    exe->page_table = NULL;
    exe->total_pages = 0;
    exe->total_bytes = 0;
    return exe;
}

ProgramManager* create_program_manager() {
    ProgramManager *manager = (ProgramManager *)malloc(sizeof(ProgramManager));
    manager->executables = NULL;
    manager->total_executables = 0;
    return manager;
}

FrameManager* create_frame_manager(unsigned char *virtual_memory) {
    FrameManager *frame_manager = (FrameManager *)malloc(sizeof(FrameManager));
    for (int i = 0; i < TOTAL_FRAMES; i++) {
        frame_manager->frames[i].frame_number = i;
        frame_manager->frames[i].is_allocated = 0;
    }
    frame_manager->virtual_memory = virtual_memory;
    frame_manager->waiting_queue.front = NULL;
    frame_manager->waiting_queue.rear = NULL;
    return frame_manager;
}

int add_page(Executable *exe, Page *page) {
    exe->pages = (Page **)realloc(exe->pages, sizeof(Page *) * (exe->total_pages + 1));
    if (exe->pages == NULL) {
        perror("realloc");
        return 1;
    }
    exe->pages[exe->total_pages] = page;

    exe->page_table = (PageTableEntry *)realloc(exe->page_table, sizeof(PageTableEntry) * (exe->total_pages + 1));
    if (exe->page_table == NULL) {
        perror("realloc");
        return 1;
    }
    exe->page_table[exe->total_pages].page_number = exe->total_pages;
    exe->page_table[exe->total_pages].start_address = page->data;

    exe->total_pages++;
    return 0;
}

int add_executable(ProgramManager *manager, Executable *exe) {
    manager->executables = (Executable **)realloc(manager->executables, sizeof(Executable *) * (manager->total_executables + 1));
    if (manager->executables == NULL) {
        perror("realloc");
        return 1;
    }
    manager->executables[manager->total_executables] = exe;
    manager->total_executables++;
    return 0;
}

int read_program(char *path, Executable *exe) {
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        perror("fopen");
        return 1;
    }

    fseek(fp, 0L, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    exe->total_bytes = file_size;
    int total_pages = (file_size + PAGE_SIZE - 1) / PAGE_SIZE;
    for (int i = 0; i < total_pages; i++) {
        Page *page = (Page *)malloc(sizeof(Page));
        page->page_number = i;
        size_t bytes_read = fread(page->data, 1, PAGE_SIZE, fp);
        if (bytes_read < PAGE_SIZE) {
            memset(page->data + bytes_read, 0, PAGE_SIZE - bytes_read);
        }
        if (add_page(exe, page) != 0) {
            fclose(fp);
            return 1;
        }
    }

    fclose(fp);
    return 0;
}

void free_executable(Executable *exe) {
    for (int i = 0; i < exe->total_pages; i++) {
        free(exe->pages[i]);
    }
    free(exe->pages);
    free(exe->page_table);
    free(exe);
}

void free_program_manager(ProgramManager *manager) {
    for (int i = 0; i < manager->total_executables; i++) {
        free_executable(manager->executables[i]);
    }
    free(manager->executables);
    free(manager);
}

void free_frame_manager(FrameManager *frame_manager) {
    FrameQueueNode *current = frame_manager->waiting_queue.front;
    while (current != NULL) {
        FrameQueueNode *next = current->next;
        free(current);
        current = next;
    }
    free(frame_manager);
}

int allocate_frame(FrameManager *frame_manager) {
    for (int i = 0; i < TOTAL_FRAMES; i++) {
        if (!frame_manager->frames[i].is_allocated) {
            frame_manager->frames[i].is_allocated = 1;
            return i;
        }
    }
    return -1; // No free frame found
}

void map_executables_to_memory(ProgramManager *manager, FrameManager *frame_manager, ProcessPool *pool) {
    for (int i = 0; i < manager->total_executables; i++) {
        Executable *exe = manager->executables[i];
        int all_pages_mapped = 1;
        for (int j = 0; j < exe->total_pages; j++) {
            int frame_number = allocate_frame(frame_manager);
            if (frame_number == -1) {
                fprintf(stderr, "No free frame available for page %d of executable %d\n", j, i);
                enqueue(&frame_manager->waiting_queue, exe);
                all_pages_mapped = 0;
                break;
            }
            unsigned char *frame_address = frame_manager->virtual_memory + frame_number * PAGE_SIZE;
            Page *page = exe->pages[j];
            memcpy(frame_address, page->data, PAGE_SIZE);
            exe->page_table[j].start_address = frame_address;
            printf("Mapped page %d of executable %d to frame %d\n", j, i, frame_number);
        }
        if (all_pages_mapped) {
            load_process(pool, i + 1, "Executable", (unsigned long)exe->page_table[0].start_address, exe->total_bytes);
        }
    }
}

void terminate_program(ProgramManager *manager, FrameManager *frame_manager, ProcessPool *pool, int program_index) {
    if (program_index < 0 || program_index >= manager->total_executables) {
        fprintf(stderr, "Invalid program index %d\n", program_index);
        return;
    }

    Executable *exe = manager->executables[program_index];
    for (int i = 0; i < exe->total_pages; i++) {
        unsigned char *frame_address = exe->page_table[i].start_address;
        int frame_number = (frame_address - frame_manager->virtual_memory) / PAGE_SIZE;

        // Reset the frame content to 00000000
        memset(frame_address, 0, PAGE_SIZE);
        frame_manager->frames[frame_number].is_allocated = 0;
        printf("Freed frame %d for page %d of executable %d\n", frame_number, i, program_index);
    }

    free_executable(exe);

    // Remove the executable from the program manager
    for (int i = program_index; i < manager->total_executables - 1; i++) {
        manager->executables[i] = manager->executables[i + 1];
    }
    manager->total_executables--;

    manager->executables = (Executable **)realloc(manager->executables, sizeof(Executable *) * manager->total_executables);
    if (manager->executables == NULL && manager->total_executables > 0) {
        perror("realloc");
    }

    // Remove from process pool
    for (int i = 0; i < pool->num_processes; i++) {
        if (pool->processes[i].process_id == program_index + 1) {
            for (int j = i; j < pool->num_processes - 1; j++) {
                pool->processes[j] = pool->processes[j + 1];
            }
            pool->num_processes--;
            break;
        }
    }

    process_waiting_queue(manager, frame_manager, pool);
}

void enqueue(FrameQueue *queue, Executable *exe) {
    FrameQueueNode *new_node = (FrameQueueNode *)malloc(sizeof(FrameQueueNode));
    new_node->exe = exe;
    new_node->next = NULL;
    if (queue->rear == NULL) {
        queue->front = new_node;
        queue->rear = new_node;
    } else {
        queue->rear->next = new_node;
        queue->rear = new_node;
    }
}

Executable* dequeue(FrameQueue *queue) {
    if (queue->front == NULL) {
        return NULL;
    }
    FrameQueueNode *temp = queue->front;
    queue->front = queue->front->next;
    if (queue->front == NULL) {
        queue->rear = NULL;
    }
    Executable *exe = temp->exe;
    free(temp);
    return exe;
}

void process_waiting_queue(ProgramManager *manager, FrameManager *frame_manager, ProcessPool *pool) {
    Executable *exe;
    while ((exe = dequeue(&frame_manager->waiting_queue)) != NULL) {
        add_executable(manager, exe);
        map_executables_to_memory(manager, frame_manager, pool);
    }
}

void load_process(ProcessPool* pool, int pid, const char* name, unsigned long base_addr, unsigned long mem_size) {
    if (pool->num_processes < MAX_PROCESSES) {
        ProcessControlBlock* pcb = &pool->processes[pool->num_processes];
        pcb->process_id = pid;
        snprintf(pcb->process_name, MAX_PROCESS_NAME_LENGTH + 1, "%s", name);
        pcb->base_address = base_addr;
        pcb->memory_size = mem_size;
        pool->num_processes++;
    } else {
        printf("Process pool is full. Cannot load more processes.\n");
    }
}

void display_process_info(ProcessControlBlock* pcb) {
    printf("Process ID: %d\n", pcb->process_id);
    printf("Process Name: %s\n", pcb->process_name);
    printf("Base Address: 0x%lx\n", pcb->base_address);
    printf("Memory Size: %lu bytes\n", pcb->memory_size);
}

void free_process_pool(ProcessPool *pool) {
    // 현재는 동적 할당된 메모리가 없기 때문에 따로 해제할 것은 없음
    (void)pool; // 컴파일러 경고 방지
}

// Main 함수에서 사용될 코드
int main() {
    char *file_path = "your_executable_file_path_here";
    ProgramManager *manager = create_program_manager();
    Executable *exe = create_executable();
    unsigned char *virtual_memory = (unsigned char *)malloc(TOTAL_MEMORY_SIZE);
    FrameManager *frame_manager = create_frame_manager(virtual_memory);
    ProcessPool *pool = (ProcessPool *)malloc(sizeof(ProcessPool));
    pool->num_processes = 0;

    if (read_program(file_path, exe) != 0) {
        fprintf(stderr, "Failed to read program\n");
        free_executable(exe);
        free_program_manager(manager);
        free_frame_manager(frame_manager);
        free(virtual_memory);
        free(pool);
        return 1;
    }

    if (add_executable(manager, exe) != 0) {
        fprintf(stderr, "Failed to add executable to manager\n");
        free_executable(exe);
        free_program_manager(manager);
        free_frame_manager(frame_manager);
        free(virtual_memory);
        free(pool);
        return 1;
    }

    map_executables_to_memory(manager, frame_manager, pool);

    // 추가 작업: 메모리 매핑 후 필요한 작업 수행
    for (int i = 0; i < exe->total_pages; i++) {
        printf("Page %d:", exe->pages[i]->page_number);
        for (int j = 0; j < PAGE_SIZE && (i * PAGE_SIZE + j) < exe->total_bytes; j++) {
            printf(" %02X", exe->page_table[i].start_address[j]);
        }
        printf("\n");
    }

    // 프로세스 정보 출력
    for (int i = 0; i < pool->num_processes; i++) {
        printf("Process %d:\n", i + 1);
        display_process_info(&pool->processes[i]);
        printf("\n");
    }

    // Terminate a program (example, terminate the first program)
    terminate_program(manager, frame_manager, pool, 0);

    free_program_manager(manager);
    free_frame_manager(frame_manager);
    free(virtual_memory);
    free(pool);
    return 0;
}


