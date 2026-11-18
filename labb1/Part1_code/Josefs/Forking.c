#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    unsigned i;
    unsigned niterations = 100;
    
    pid_t child1 = fork();
    if (child1 == 0)
    {
        for (int i = 0; i < niterations; i++)
        {
            printf("A = %d, ", i);
        } 
        printf("\n");
        return 0;
    }


    pid_t child2 = fork();
    if (child2 == 0)
    {
        for (int i = 0; i < niterations; i++)
        {
            printf("C = %d, ", i);
        } 
        printf("\n");
        return 0;
    }


    for (int i = 0; i < niterations; ++i)
    {
        printf("B = %d, ", i);

    }
    
    
    printf("Förälder: eget pid = %d\n", getpid());
    printf("\nFörälder: barn 1 har pid %d, barn 2 har pid %d\n", child1, child2);

    return 0;

    
    
}
