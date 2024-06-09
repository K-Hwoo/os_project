#include <stdio.h>
#include <stdlib.h>

// 프로세스 정보를 담는 구조체
typedef struct Process {
    PageManager* page_manager;
    struct Process* next;
    int process_num;
} Process;

// 링크드 리스트 관리를 위한 구조체
typedef struct ProcessPool {
    Process* head;
} ProcessPool;

void CreateProcessPool(){// 프로세스 풀 생성
    ProcessPool pool;
    pool.head = NULL;
}

// 링크드 리스트에 프로세스 추가
void addProcess(ProcessPool* pool, PageManager* page_manager, int process_number) {
    // 새로운 프로세스를 생성
    Process* newProcess = (Process*)malloc(sizeof(Process));
    newProcess->page_manager = page_manager;
    newProcess->next = NULL;
    newProcess->process_num = process_number;

    // 리스트가 비어있다면 새로운 프로세스를 헤드로 지정
    if (pool->head == NULL) {
        pool->head = newProcess;
        return;
    }

    // 리스트의 끝을 찾아서 새로운 프로세스를 연결
    Process* current = pool->head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = newProcess;
}

// 링크드 리스트에서 특정 프로세스를 제거
void removeProcess(ProcessPool* pool, int process_number) {
    Process* current = pool->head;
    Process* prev = NULL;

    // 헤드에서 시작하여 해당 Process number를 가진 프로세스 찾기
    while (current != NULL && current->process_num != process_number) {
        prev = current;
        current = current->next;
    }

    // 특정 Process number를 가진 프로세스를 찾지 못한 경우
    if (current == NULL) {
        printf("Process with Process %d not found.\n", process_number);
        return;
    }

    // 특정 Process number를 가진 프로세스를 찾은 경우
    // 이전 노드와 다음 노드를 연결하고 해당 노드 메모리 해제
    if (prev == NULL) {
        // 삭제할 노드가 헤드인 경우
        pool->head = current->next;
    } else {
        // 삭제할 노드가 헤드가 아닌 경우
        prev->next = current->next;
    }
    free(current);
    printf("Process with Process %d removed.\n", process_number);
}

// 링크드 리스트에 있는 모든 프로세스 정보 출력
void printProcesses(ProcessPool* pool) {
    Process* current = pool->head;
    while (current != NULL) {
        printf("process_number: %d \n", current->process_num);
        current = current->next;
    }
}

// 메모리 해제를 위해 링크드 리스트의 모든 노드 삭제
void freeProcessPool(ProcessPool* pool) {
    Process* current = pool->head;
    while (current != NULL) {
        Process* temp = current;
        current = current->next;
        free(temp);
    }
    pool->head = NULL;
}
