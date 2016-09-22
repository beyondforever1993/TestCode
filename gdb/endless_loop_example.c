#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int loop = 1;
static void func1();
static void func2();
static void func3();

static void func1()
{
    func2();
}

static void func2()
{
    func3();
}

static void func3()
{
    printf("Execute function is %s\n", __FUNCTION__);
}


int main()
{
    while (loop)
    {
        func1();
        sleep(1);
    }

    return 0;
}
