#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define SHM_R 0400
#define SHM_W 0200

struct shm_struct
{
    int buffer[10];
    int in;
    int out;
    int count;
};

int main(void)
{
    volatile struct shm_struct *shmp = NULL;
    int shmid;
    int value;
    int i;
    pid_t pid;

    shmid = shmget(IPC_PRIVATE, sizeof(struct shm_struct), IPC_CREAT | SHM_R | SHM_W);
    shmp = (struct shm_struct *)shmat(shmid, NULL, 0);
    shmp->in = 0;
    shmp->out = 0;
    shmp->count = 0;

    pid = fork();
    if (pid != 0)
    {
        /* Parent is consumer*/
        srand(getpid());
        for (i = 1; i <= 100; i++)
        {
            while (shmp->count == 10)
            {
                ;
            } // if full then wait

            shmp->buffer[shmp->in] = i;
            shmp->in = (shmp->in + 1) % 10;
            shmp->count++; // race condition possible

            printf("Sending %d\n", i);
            fflush(stdout);
            usleep(100000 + rand() % 400001); // 0,1-0,5 s
        }
        wait(NULL); // Waiting for child
        shmdt((void *)shmp);
        shmctl(shmid, IPC_RMID, NULL);
    }
    else
    {
        /* child = consumer */
        srand(getpid());
        for (i = 1; i <= 100; i++)
        {
            while (shmp->count == 0)
            {
                ;
            } // if empty wait

            value = shmp->buffer[shmp->out];
            shmp->out = (shmp->out + 1) % 10;
            shmp->count--; // race condition possible

            printf("Received %d\n", value);
            fflush(stdout);
            usleep(200000 + rand() % 1800001); // 0,2-2 s
        }
        shmdt((void *)shmp);
    }
    return 0;
}