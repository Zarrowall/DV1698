#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define SHMSIZE 128
#define SHM_R 0400
#define SHM_W 0200

int main(int argc, char **argv)
{
	struct shm_struct
	{
		int buffer[10];
		int start;
		int end;
		int elements;
		unsigned empty;
	};
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
			/* write to shmem */
			var1++;
			while (shmp->elements == 10)
				; /* busy wait until queue no longer full */
			printf("Sending %d\n", var1);
			fflush(stdout);
			shmp->buffer[shmp->end] = var1;
			shmp->end = (shmp->end + 1) % 10;
			shmp->elements++;
			shmp->empty = 1;
			usleep(100000 + rand() % 400001);
		}
		wait(NULL);
		shmdt(addr);
		shmctl(shmid, IPC_RMID, shm_buf);
	}
	else
	{
		/* here's the child, acting as consumer */
		while (var2 < 100)
		{
			/* read from shmem */
			while (shmp->elements == 0)
				; /* busy wait until queue not empty */
			var2 = shmp->buffer[shmp->start];
			shmp->start = (shmp->start + 1) % 10;
			shmp->elements--;
			shmp->empty = 0;
			printf("Received %d\n", var2);
			fflush(stdout);
			usleep(200000 + rand() % 1800001);
		}
		shmdt(addr);
		shmctl(shmid, IPC_RMID, shm_buf);
	}
}