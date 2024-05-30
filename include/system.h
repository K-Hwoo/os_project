// include/linux/sched.h
//SSU struct task_struct

void minisystem();

void printWithDelay(const char *str, int delay); // 한글자씩 출력하는 함수 (in kernel/mem_s/print_util.c)
void print_minios(char* str);

void * memory_set(void * ptr, int value, size_t num);
void * make_dummy_physical_memory();