#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>

#define SHMSIZE 128
#define SHM_R 0400
#define SHM_W 0200

const char *semName1 = "producer_sema";
const char *semName2 = "consumer_sema";

int sem1Value, sem2Value;

int main(int argc, char **argv)
{
    // sem_unlink(semName1);
    // sem_unlink(semName2);
    int status;
    struct shm_struct
    {
        int buffer[10];
        int start;
        int end;
        int elements;
        unsigned empty;
    };

    sem_t *sem_id1 = sem_open(semName1, O_CREAT, O_RDWR, 10);
    sem_t *sem_id2 = sem_open(semName2, O_CREAT, O_RDWR, 0);

    volatile struct shm_struct *shmp = NULL;
    char *addr = NULL;
    pid_t pid = -1;
    int var1 = 0, var2 = 0, shmid = -1;
    struct shmid_ds *shm_buf;

    /* allocate a chunk of shared memory */
    shmid = shmget(IPC_PRIVATE, SHMSIZE, IPC_CREAT | SHM_R | SHM_W);
    shmp = (struct shm_struct *)shmat(shmid, addr, 0);
    shmp->empty = 0;
    shmp->start = 0;
    shmp->end = 0;
    shmp->elements = 0;
    pid = fork();
    if (pid != 0)
    {
        /* here's the parent, acting as producer */
        while (var1 < 100)
        {
            sem_wait(sem_id1);
            /* write to shmem */
            var1++;
            printf("Sending %d\n", var1);
            fflush(stdout);
            // Put var into buffer at end, increment end, increment number of elements
            shmp->buffer[shmp->end] = var1;
            shmp->end = (shmp->end + 1) % 10;
            shmp->elements++;
            shmp->empty = 1;
            sem_getvalue(sem_id1, &sem1Value);
            printf("Current sem1 value is: %d\n", sem1Value);
            sem_post(sem_id2);
            usleep(100000 + rand() % 400001); // 0,1-0,5s sleep timer
        }
        // Async like wait for the other process to finnish emptying the buffer followed by
        wait(NULL);
        shmdt(addr);
        shmctl(shmid, IPC_RMID, shm_buf);
        sem_close(sem_id1);
        sem_close(sem_id2);
        wait(&status);
        sem_unlink(semName1);
        sem_unlink(semName2);
    }
    else
    {
        /* here's the child, acting as consumer */
        while (var2 < 100)
        {
            /* read from shmem */
            sem_wait(sem_id2);

            // Take item from buffer at start, move start 1 step, decrease number of elements
            var2 = shmp->buffer[shmp->start];
            shmp->start = (shmp->start + 1) % 10;
            shmp->elements--;
            shmp->empty = 0;
            printf("Received %d\n", var2);
            fflush(stdout);
            sem_post(sem_id1);
            sem_getvalue(sem_id2, &sem2Value);
            printf("Current sem2 value is: %d\n", sem2Value);
            usleep(200000 + rand() % 1800001); // 0,2-2s sleep timer
        }
        shmdt(addr);
        shmctl(shmid, IPC_RMID, shm_buf);
        sem_close(sem_id1);
        sem_close(sem_id2);
    }
}