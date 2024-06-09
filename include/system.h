// include/linux/sched.h
//SSU struct task_struct

void minisystem();

void printWithDelay(const char *str, int delay); // 한글자씩 출력하는 함수 (in kernel/mem_s/print_util.c)
void print_minios(char* str);

void * memory_set(void * ptr, int value, size_t num);
void * make_dummy_physical_memory();

int prepare_dummy_physical_memory_reorder(void * start_adr, void * end_adr);
void memory_view(unsigned char *memory, size_t from, size_t to);

int read_program(void *virtual_physical_memory, FrameList *fl, FrameManager *fm);
int execute();