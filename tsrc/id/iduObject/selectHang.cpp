/** 
 *  Copyright (c) 1999~2017, Altibase Corp. and/or its affiliates. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License, version 3,
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
 
/* is_shutdown_select.c

Experiment to determine behaviour of select() after a shutdown()
is performed on one or both ends of a TCP socket pair.
*/
#include <idl.h>

#define errExit(msg) { printf("errno = %d\n", errno); perror(msg);  }

#define PORT_NUM 50000          /* Port number for server */

int
main(int argc, char *argv[])
{
    struct sockaddr_in svaddr;
    int fd[3], optval, j;
    fd_set rfds, wfds;
    int nfds;
    struct timeval tmo = { 0, 0 };      /* For polling select() */
    struct linger mylinger;

    fd[0] = socket(AF_INET, SOCK_STREAM, 0);    /* Listening socket */ 
    if (fd[0] == -1) errExit("socket");

    optval = 1;
    if (setsockopt(fd[0], SOL_SOCKET, SO_REUSEADDR, &optval,
                   sizeof(optval)) == -1) errExit("setsockopt");


//     mylinger.l_onoff = 1;
//     mylinger.l_linger = 0;
    
//     if (setsockopt(fd[0], SOL_SOCKET, SO_LINGER, &mylinger,
//                    sizeof(linger)) == -1) errExit("setsockopt");

    
    /* Bind to wildcard host address + well-known port, and mark as
       listening socket */

    memset(&svaddr, 0, sizeof(svaddr));
    svaddr.sin_family = AF_INET;
    svaddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Wildcard address */
    svaddr.sin_port = htons(PORT_NUM); if (bind(fd[0], (struct sockaddr *)
                                                &svaddr, sizeof(svaddr)) == -1)
        errExit("bind");

    if (listen(fd[0], 5) == -1) errExit("listen");

    printf("Listening socket (fd=%d) set up okay\n", fd[0]);

    /* Active sockets */

    fd[1] = socket(AF_INET, SOCK_STREAM, 0);
    if (fd[1] == -1) errExit("socket");
    svaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (connect(fd[1], (struct sockaddr *) &svaddr, sizeof(svaddr)) == -1)
        errExit("connect");

    printf("Active socket (fd=%d) set up okay\n", fd[1]);

    fd[2] = accept(fd[0], NULL, NULL);
    if (fd[2] == -1) errExit("accept");

    printf("Connection established (fd=%d)\n", fd[2]);
    
    printf("do Shutdown \n");
    if (shutdown(fd[2], SHUT_RDWR) == -1)
    {
        errExit("shutdown (active socket)");
    }
    //close(fd[2]);
    printf("shutdown(%d, %d) errno=%d\n", fd[2], (int)SHUT_RDWR, errno);

    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    nfds = 0;

//     for (j = 0; j < 3; j++) {
//         FD_SET(fd[j], &rfds);
//         FD_SET(fd[j], &wfds);

//         if (fd[j] >= nfds)
//             nfds = fd[j] + 1;
//     } /* for */

    FD_SET(fd[2], &wfds);
    nfds = fd[2] + 1;

    fprintf(stderr, "before sleep \n");
    if (select(nfds, NULL/*&rfds*/, &wfds, NULL, NULL) == -1) errExit("select");

    for (j = 0; j < 3; j++) {
        printf("%d: ", fd[j]);
        if (FD_ISSET(fd[j], &rfds))
            printf("r ");
        if (FD_ISSET(fd[j], &wfds))
            printf("w ");
        printf("\n");
    } /* for */

    exit(EXIT_SUCCESS);
} /* main */
