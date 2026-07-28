/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgarcez- < dgarcez-@student.42lisboa.com > +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 17:43:43 by dgarcez-          #+#    #+#             */
/*   Updated: 2026/07/28 18:35:22 by dgarcez-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/ft_irc.hpp"

void	server(char *port)
{
	int ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (ServerSocket < 0)
		throw std::runtime_error("Error: Socket failed");
	sockaddr_in ServerAddr;
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(std::atoi(port));
	ServerAddr.sin_addr.s_addr = INADDR_ANY;
	int	opt = 1;
	if (setsockopt(ServerSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)//make it so wh we lose the server we can open again faster
		throw std::runtime_error("Error: Setsockopt failed");
	if (bind(ServerSocket, (struct sockaddr *)&ServerAddr, sizeof(ServerAddr)) < 0)//bind the port so ony one server at a time
		throw std::runtime_error("Error: Bind failed\n");
	if (listen(ServerSocket, 5) < 0)//waits for stuff to happen to the server
		throw std::runtime_error("Error: Listen failed\n");
	int epfd = epoll_create1(0);
	epoll_event Server;
	Server.events = EPOLLIN;//event is recieve data
	Server.data.fd = ServerSocket;
	epoll_ctl(epfd, EPOLL_CTL_ADD,ServerSocket, &Server);
	while(true)
	{
		epoll_event events[64];
		int n = epoll_wait(epfd, events, 64, -1);//wait or fd to have an event
		if(n < 0)
		{
			if (errno == EINTR)
				continue;
			throw std::runtime_error("Error: Epoll failed\n");
		}
		for (int i = 0; i < n; i++)
		{
			int fd = events[i].data.fd;

			if (events[i].events & (EPOLLHUP | EPOLLERR | EPOLLRDHUP))
			{
				epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
				close(fd);
				continue;
			}
			if (fd == ServerSocket)
			{
				int client = accept(ServerSocket, NULL, NULL);
				std::cout << "NEW USER JOINED\n";
				epoll_event ev;
				ev.events = EPOLLIN;
				ev.data.fd = client;
				epoll_ctl(epfd, EPOLL_CTL_ADD, client, &ev);
			}
			else
			{
				char buffer[1024];
				int bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
				if (bytes <= 0) 
				{
					std::cout << "USER DISCONNECTED\n"; 
					epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
					close(fd);
					// _cli.removeCli(fd);
				}
				else 
				{
					buffer[bytes] = '\0';
					std::cout << buffer;
					// _cli.clientRead(fd, buffer, bytes);
        		}
				// buffer = 0;
			}
		}
	}
	close(ServerSocket);
}

int main(int ac, char *av[])
{
	if(ac != 3)
	{
		std::cerr << "Error: Invalid Arguments(./ircserv [port] [password])\n";
		return (1);
	}
	try
	{
		server(av[1]);

	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
}