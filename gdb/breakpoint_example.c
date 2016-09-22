#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int narcissus_count = 0;
static int narcissus_arr[10] = {0, 0};
static void func1();
static void func2();
static void func3();
static void show_narcissus();

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
    int n = 0;
    int a, b ,c;

    for (n = 100; n < 1000; n++)
    {
        a = n / 100; 
        b = (n - a*100) / 10;
        c = n - a*100 - b*10;
        if (n == (a*a*a + b*b*b + c*c*c))
        {
            narcissus_arr[narcissus_count] = n;
            narcissus_count++;
        }
    }
}

static void show_narcissus()
{
    int i = 0;
    
    if (!narcissus_count)
    {
        printf("Not found narcissus number between 100 to 1000.\n");
    }
    else
    {
        printf("All found narcissus number between 100 to 1000 is: ");
    }

    for (i = 0; i < narcissus_count; i++)
    {
        printf("%d ", narcissus_arr[i]);
    }
    printf("\n");
}

int main()
{
    func1();
    show_narcissus();

    return 0;
}
