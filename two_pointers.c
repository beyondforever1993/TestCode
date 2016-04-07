#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define dprintf(fmt, args...) do{printf("line:%d "fmt, __LINE__, ##args);}while(0)

static int test(int test1, void **test2)
{
    char *ptr = NULL;

    ptr = malloc(30);
    if (ptr == NULL)
    {
        perror("malloc failed\n");
        return -1;
    }
    memset(ptr, 0, 30);

    snprintf(ptr, 30, "Welcome!");
    dprintf("%s\n", ptr);
    dprintf("test2's address:%p\n", &test2);
    dprintf("test2's value:%p\n", test2);

    *test2 = ptr;
    dprintf("malloc ptr value:%p\n", ptr);
    dprintf("*test2's value:%p\n", *test2);

    return test1;
}

int main()
{
    //& * 成对操作
    void *ptr = NULL;

    dprintf("ptr's address: %p\n", &ptr);
    test(4, &ptr);

    dprintf("ptr: %s\n", (char *)ptr);

    free(ptr);

    return 0;
}
