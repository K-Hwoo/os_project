#include "test_memory_manager.h"
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

    int byte;
    long byte_count = 0;
    Page *current_page = NULL;

    while ((byte = fgetc(fp)) != EOF) {
        if (byte_count % PAGE_SIZE == 0) {
            if (current_page != NULL) {
                add_page(exe, current_page);
            }
            current_page = (Page *)malloc(sizeof(Page));
            if (current_page == NULL) {
                perror("malloc");
                fclose(fp);
                return 1;
            }
            current_page->page_number = exe->total_pages;
        }

        current_page->data[byte_count % PAGE_SIZE] = byte;
        byte_count++;
    }

    if (current_page != NULL) {
        add_page(exe, current_page);
    }

    exe->total_bytes = byte_count;

    printf("Total bytes read: %ld\n", exe->total_bytes);
    printf("Total pages: %d\n", exe->total_pages);

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

void map_executables_to_memory(ProgramManager *manager, FrameManager *frame_manager) {
    for (int i = 0; i < manager->total_executables; i++) {
        Executable *exe = manager->executables[i];
        for (int j = 0; j < exe->total_pages; j++) {
            int frame_number = allocate_frame(frame_manager);
            if (frame_number == -1) {
                fprintf(stderr, "No free frame available for page %d of executable %d\n", j, i);
                enqueue(&frame_manager->waiting_queue, exe);
                break;
            }
            unsigned char *frame_address = frame_manager->virtual_memory + frame_number * PAGE_SIZE;
            Page *page = exe->pages[j];
            memcpy(frame_address, page->data, PAGE_SIZE);
            exe->page_table[j].start_address = frame_address;
            // printf("Mapped page %d of executable %d to frame %d\n", j, i, frame_number); // 매핑 브리핑
        }
    }
}

void terminate_program(ProgramManager *manager, FrameManager *frame_manager, int program_index) {
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
        // printf("Freed frame %d for page %d of executable %d\n", frame_number, i, program_index); // 프리 브리핑
    }

    free_executable(exe);

    // Remove the executable from the program manager
    for (int i = program_index; i < manager->total_executables - 1; i++) {
        manager->executables[i] = manager->executables[i + 1];
    }
    manager->total_executables--;

    manager->executables = (Executable **)realloc(manager->executables, sizeof(Executable *) * manager->total_executables);
    if (manager->total_executables > 0 && manager->executables == NULL) {
        perror("realloc");
    }

    process_waiting_queue(manager, frame_manager);
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

void process_waiting_queue(ProgramManager *manager, FrameManager *frame_manager) {
    Executable *exe;
    while ((exe = dequeue(&frame_manager->waiting_queue)) != NULL) {
        add_executable(manager, exe);
        map_executables_to_memory(manager, frame_manager);
    }
}

int test_mem_manager() {
    char *file_path = "/program/up_down_game";
    ProgramManager *manager = create_program_manager();
    Executable *exe = create_executable();
    unsigned char *virtual_memory = (unsigned char *)malloc(TOTAL_MEMORY_SIZE);
    FrameManager *frame_manager = create_frame_manager(virtual_memory);

    if (read_program(file_path, exe) != 0) {
        fprintf(stderr, "Failed to read program\n");
        free_executable(exe);
        free_program_manager(manager);
        free_frame_manager(frame_manager);
        free(virtual_memory);
        return 1;
    }

    if (add_executable(manager, exe) != 0) {
        fprintf(stderr, "Failed to add executable to manager\n");
        free_executable(exe);
        free_program_manager(manager);
        free_frame_manager(frame_manager);
        free(virtual_memory);
        return 1;
    }

    map_executables_to_memory(manager, frame_manager);

    // 추가 작업: 메모리 매핑 후 필요한 작업 수행
    for (int i = 0; i < exe->total_pages; i++) {
        printf("Page %d:", exe->pages[i]->page_number);
        for (int j = 0; j < PAGE_SIZE && (i * PAGE_SIZE + j) < exe->total_bytes; j++) {
            printf(" %02X", exe->page_table[i].start_address[j]);
        }
        printf("\n");
    }

    // Terminate a program (example, terminate the first program)
    terminate_program(manager, frame_manager, 0);

    free_program_manager(manager);
    free_frame_manager(frame_manager);
    free(virtual_memory);
    return 0;
}
