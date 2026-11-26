#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

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

    if ((key = ftok("msgq.txt", 'B')) == -1)
    {
        perror("ftok");
        exit(1);
    }

    if ((msqid = msgget(key, PERMS)) == -1)
    { /* connect to the queue */
        perror("msgget");
        exit(1);
    }
    printf("message queue: ready to receive messages.\n");

    for (;;)
    { /* normally receiving never ends but just to make conclusion */
        /* this program ends with string of end */
        if (msgrcv(msqid, &buf, sizeof(buf.number), 0, 0) == -1)
        {
            perror("msgrcv");
            exit(1);
        }
        printf("recvd: \"%d\"\n", buf.number);
        if (buf.number == -1)
            break;
    }

    if (msgctl(msqid, IPC_RMID, NULL) == -1)
    {
        perror("msgctl");
        exit(1);
    }
    printf("message queue: done receiving messages.\n");
    system("rm msgq.txt");
    return 0;
}
