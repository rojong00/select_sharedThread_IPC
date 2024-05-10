#include "wrapSharedMutex.h"

void child_process(int idenfier, int *pCnt, int shmId)
{
	char msg[MAXBUF];
	struct sockaddr_in addr = {0,};
	int n, sockfd, num = 0;
	srandom(getpid());
	/* Create socket and connect to server */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(2000);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) != 0)
    {
        printf("err in sockfd of child\n");
        exit(-1);
    }

    int writefd;
    char fifoName[10];
    sprintf(fifoName, "fifo_%d", idenfier);
    if ( (writefd = open(fifoName, O_WRONLY)) > 2)
    {
        puts("write fd already made, and will use it");
    }
    else
    {
        //close(writefd);
        if (mkfifo(fifoName, 0666) < 0)
        {
            perror("err in making fifo");
            exit(1);
        }
        if ( (writefd = open(fifoName, O_WRONLY)) < 0 )
        {
            puts("err in open write fd 2");
        }
    }

    sharedMtx mtx = shared_mutex_init("/trySharedMtx");
    if (mtx.ptr == NULL || mtx.chkCreated == 0)
    {
        exit(9);
    }
    puts("children while loop start");

    int sl = 0;
	while(1)
	{
	    num++;

        shared_mutex_lock(&mtx);
        (*pCnt)++;
        if (*pCnt >= 11)
        {
            shared_mutex_unlock(&mtx);

            //strncpy(msg, "Exit", strlen("Exit")+1);
            sprintf(msg, "Exit%c", '\0');
            n = write(writefd, msg, strlen(msg)+1);
            printf("to fifo(from %d): %d\n", getpid(), n);
            break;
        }
        else 
        {
            shared_mutex_unlock(&mtx);
        }

        sl = (random()%5) + 1;
        sleep(sl);
  	    sprintf(msg, "Test message %dth from client %d(%ds)", num, getpid(), sl);
  	    n = write(sockfd, msg, strlen(msg)+1);	/* Send message */
        if (n < 0)
        {
            fprintf(stderr, "write failed : %d(%s)\n", getpid(), strerror(errno));
        }
        else
        {
            n = write(writefd, msg, strlen(msg)+1);
            if (n < 0)
            {
                fprintf(stderr, "write(fifo) failed : %d\n", getpid());
            }
        }
    }

    if (shmdt(pCnt) == -1)
    {
        puts("Err in shmdt");
        exit(7);
    }
    /*
    if (shmctl(shmId, IPC_RMID, 0) == -1)
    {
        puts("Err in shmctt");
        exit(8);
    }
    */

    if (close(sockfd)==-1)
        exit(10);
    if (close(writefd) == -1)
        exit(11);
    if (unlink(fifoName) == -1)
        exit(12);
    if (shared_mutex_close(&mtx) == -1)
        exit(13);
    //shared_mutex_destroy(&mtx);
}

int main()
{
	char buffer[MAXBUF];
	int fds[5];
	struct sockaddr_in addr;
	struct sockaddr_in client;
	int n, i,max=0;
	uint32_t addrLen;
	int sockfd;
	fd_set rset, tmpSet;
    struct timeval timeout;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&addr, 0, sizeof (addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(2000);
	addr.sin_addr.s_addr = INADDR_ANY;
	bind(sockfd,(struct sockaddr*)&addr ,sizeof(addr));
	listen (sockfd, 5); 

    //shared mem
    key_t shmKey;
    int shmId = 0;
    int *pCnt = NULL;
    shmKey = ftok ("/dev/null", 5);       
    printf("shmkey for p = %d\n", shmKey);
    if ((shmId = shmget(shmKey, sizeof(int), 0644 | IPC_CREAT)) < 0)
    {
        perror ("shmget\n");
        exit (5);
    }
    pCnt = (int *)shmat(shmId, NULL, 0);
    *pCnt = 0;

	for(i=0;i<5;i++)
	{
		if(fork() == 0)
		{
			child_process(i, pCnt, shmId);
			exit(0);
		}
	}

	for (i=0;i<5;i++) 
	{
	  memset(&client, 0, sizeof (client));
	  addrLen = sizeof(client);
	  fds[i] = accept(sockfd,(struct sockaddr*)&client, &addrLen);
      if (fds[i] == -1)
      {
          exit(-1);
      }
	  if (fds[i] > max)
          max = fds[i];
	}
  
	FD_ZERO(&rset);
	for (i = 0; i< 5; i++ ) {
		FD_SET(fds[i], &rset);
	}

    puts("-1");
    sharedMtx mtx = shared_mutex_init("/trySharedMtx");
    if (mtx.ptr == NULL || mtx.chkCreated == 0)
    {
        exit(9);
    }

    puts("0");
    while(1){
        tmpSet = rset;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
	
	   	puts("round again");
		int ret = select(max+1, &tmpSet, NULL, NULL, &timeout);
        if ( ret == -1 )
        {
            puts("Err in select()\n");
            exit(1);
        }
        /*
        else if ( ret == 0 )
        {
            puts("Err in select() timeout\n");
            exit(1);
        }
        */
        else{
		    //printf("RET is %d\n", ret);
			for(i=0;i<5;i++)
            {
				if (FD_ISSET(fds[i], &tmpSet))
                {
					memset(buffer,0,MAXBUF);
					n = read(fds[i], buffer, MAXBUF);
		            if ( n < 0 )
		            {
		                printf("read err in %d\n", i);
		                FD_CLR(fds[i], &tmpSet);
		            }
		            else
		            {
		                //cnt++;
                        puts(buffer);
		            }
				}
		    } // for loop about fds
            shared_mutex_lock(&mtx);
	        if (*pCnt >= 10)
	            {
                    shared_mutex_unlock(&mtx);
	                break;
                }
            shared_mutex_unlock(&mtx);
        } // if ret if okay
    }

    //while (waitpid(-1, NULL, 0) > 0)
    while (waitpid(-1, NULL, WNOHANG) > 0)
    {
        /*
        if (errno == ECHILD)
            break;
            */
        continue;
    }


    close(sockfd);
    for (i = 0 ; i < 5 ; i++)
        close(fds[i]);

    if (shmdt(pCnt) == -1)
    {
        puts("Err in shmdt");
        exit(7);
    }
    if (shmctl(shmId, IPC_RMID, 0) == -1)
    {
        puts("Err in shmctt _ main");
        exit(8);
    }
    if (shared_mutex_close(&mtx))
    {
        return 10;
    }
    /*
    if (shared_mutex_destroy(&mtx))
    {
        return 1;
    }
    */
    
    puts("__5");

    return 0;
}
