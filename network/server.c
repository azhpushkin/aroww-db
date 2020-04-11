#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BACKLOG 10 /* how many pending connections queue will hold */

typedef struct thread_args {
    int* socket;
    void (*callback)(char*, char*, int*);
} thread_args;

void* socket_thread(void* ptr)
{
    thread_args* args = (thread_args*)ptr;
    char buffer[512];
    char ret[512];
    int socket = *(args->socket);
    int result;

    while (1) {
        memset(buffer, 0, 500);
        memset(ret, 0, 500);
        result = recv(socket, buffer, 512, 0);
        if (result == 0) {
            printf("Connection closed!\n");
            break;
        }
        if (result == -1) {
            perror("Error receiving from connection!");
            break;
        }
        (args->callback)(buffer, ret, &result);
        if (result == 0) {
            send(socket, "OK.", strlen("OK."), 0);
        } else if (result == 1) {
            send(socket, ret, strlen(ret), 0);
        } else {
            send(socket, "ERROR!!", strlen("ERROR!"), 0);
        }
        
    }

    close(socket);
    pthread_exit(0);
}

int listen_on_port(int port, void (*callback)(char*, char*, int*))
{
    int sockfd, new_fd; /* listen on sock_fd, new connection on new_fd */
    struct sockaddr_in my_addr; /* my address information */
    struct sockaddr_in their_addr; /* connector's address information */
    socklen_t sin_size;
    pthread_t thread;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    my_addr.sin_family = AF_INET; /* host byte order */
    my_addr.sin_port = htons(port); /* short, network byte order */
    my_addr.sin_addr.s_addr = INADDR_ANY; /* auto-fill with my IP */
    bzero(&(my_addr.sin_zero), 8); /* zero the rest of the struct */

    if (bind(sockfd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr))
        == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    while (1) { /* main accept() loop */
        sin_size = sizeof(struct sockaddr_in);
        if ((new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size)) == -1) {
            perror("accept");
            continue;
        }
        printf("server: got connection from %s\n", inet_ntoa(their_addr.sin_addr));
        thread_args args = { &new_fd, callback };
        pthread_create(&thread, 0, socket_thread, (void*)&args);
        pthread_detach(thread);
    }

    return 0;
}

// int main() {
//     return start_listening();
// }
