#include <stdio.h>
#include <unistd.h>

int main()
{
    int a = 1;

    while (1)
    {
        sleep(3);
        printf("a = %d\n", a);

        a++;
    }

    return 0;
}
