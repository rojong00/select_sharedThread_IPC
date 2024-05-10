#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

// FIFO like pipe
// Enables the communication between processors using FIFO file name
int main(int argc, char *argv[])
{
    if ( argc < 2 )
    {
        puts("arg is lacked");
        return 1;
    }

    char buff[100] = {0,};
    int readfd = open(argv[1], O_RDONLY);
    if ( 0 > readfd )
    {
        puts("err in open readfd");
        return 2;
    }


    int n = 0;
    puts("in while loop for readfd");
    while ((n = read(readfd, buff, 100)) > 0)
    {
        buff[n] = '\0';

        if ( strncmp(buff, "Exit", strlen("Exit")) == 0)
        {
            printf("END : %s\n", argv[1]);
            break;
        }

        printf("<%s> %s\n", argv[1], buff);
    }

    close(readfd);
    return 0;
}
