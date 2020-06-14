#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <poll.h>

#include "socket_server.hpp"
#include "engine/interface.hpp"
#include "utils/logger.hpp"


#define BACKLOG 10


SimpleSocketServer::SimpleSocketServer(int port_, AbstractEngine& engine_)
: port(port_),
  close_scheduled(false),
  engine(engine_)
  {
      ready_mutex.lock();
  }

void SimpleSocketServer::schedule_close() { close_scheduled = true; }


int SimpleSocketServer::start_listening()
{
    int sockfd, new_fd; /* listen on sock_fd, new connection on new_fd */
    struct sockaddr_in my_addr; /* my address information */
    struct sockaddr_in their_addr; /* connector's address information */
    socklen_t sin_size;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return 1;
    }
    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    my_addr.sin_family = AF_INET; /* host byte order */
    my_addr.sin_port = htons(port); /* short, network byte order */
    my_addr.sin_addr.s_addr = INADDR_ANY; /* auto-fill with my IP */
    bzero(&(my_addr.sin_zero), 8); /* zero the rest of the struct */

    if (bind(sockfd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        return 1;
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        return 1;
    }

    log_debug("Server started, listening on {}", port);

    std::vector<RunningConnection> connections;

    struct pollfd *pfds = (struct pollfd *)malloc(sizeof(*pfds) * 1);
    pfds[0].fd = sockfd;
    pfds[0].events = POLLIN;


    ready_mutex.unlock();
    while (!close_scheduled) { /* main accept() loop */
        int poll_count = poll(pfds, 1, 1000);  // 1000 ms
        if (poll_count == -1) {
            perror("poll");
            return 1;
        }

        if (pfds[0].revents && POLLIN) {
            sin_size = sizeof(struct sockaddr_in);
            new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);
            if (new_fd == -1) {
                perror("accept");
            } else {
                log_debug("New connection from {}, binding to {}", inet_ntoa(their_addr.sin_addr), new_fd);
                connections.emplace_back(new_fd, engine);
            }
        }
    }
    for (auto& conn: connections) {
        conn.close_conn();
    }
    close(sockfd);
    return 0;
}
