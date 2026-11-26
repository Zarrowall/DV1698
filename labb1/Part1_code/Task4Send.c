#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <limits.h>

#define PERMS 0644
struct my_msgbuf
{
    long mtype;
    int number;
};

int main(void)
{
    struct my_msgbuf buf;
    int msqid;
    key_t key;
    int min = 1;
    system("touch msgq.txt");

    if ((key = ftok("msgq.txt", 'B')) == -1)
    {
        perror("ftok");
        exit(1);
    }

    if ((msqid = msgget(key, PERMS | IPC_CREAT)) == -1)
    {
        perror("msgget");
        exit(1);
    }
    printf("message queue: ready to send messages.\n");
    buf.mtype = 1; /* we don't really care in this case */

    for (int i = 0; i < 51; ++i)
    {
        buf.number = rand() % ((INT_MAX - 1) - min + 1) + min;

        if (i == 50)
        {
            buf.number = -1;
        }

        printf("%d: Sending \"%d\"\n", i + 1, buf.number);
        if (msgsnd(msqid, &buf, sizeof(buf.number), 0) == -1) /* +1 for '\0' */
            perror("msgsnd");
    }

    printf("message queue: done sending messages.\n");
    return 0;
}
