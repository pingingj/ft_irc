/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: finn <finn@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 17:43:43 by dgarcez-          #+#    #+#             */
/*   Updated: 2026/08/21 14:18:27 by finn             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/ft_irc.hpp"

bool g_exit_flag = false;

void	action_handler(int signal)
{
	if (signal == SIGINT)
		g_exit_flag = true;
}

Server::Server()
{
	
}

Server::Server(std::string s_pass) : _client(), _channel(&_client)
{
	this->_pass = s_pass;
}

Server::~Server()
{
	
}

Server::Server(const Server &obj)
{
	if(this != &obj)
		return;
	return;	
}


Server &Server::operator=(const Server &obj)
{
	(void)obj;
	return (*this);
}

void	Server::server(char *port)
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
	{
		close(ServerSocket);
		throw std::runtime_error("Error: Bind failed\n");
	}
	if (listen(ServerSocket, 5) < 0)//waits for stuff to happen to the server
		throw std::runtime_error("Error: Listen failed\n");
	int epfd = epoll_create1(0);
	epoll_event Server;
	Server.events = EPOLLIN;//event is recieve data
	Server.data.fd = ServerSocket;
	epoll_ctl(epfd, EPOLL_CTL_ADD,ServerSocket, &Server);
	while(true)
	{
		if(g_exit_flag == true)
		{
			close(epfd);
			close(ServerSocket);
			return ;
		}
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
				int client_fd = accept(ServerSocket, NULL, NULL);
				std::cout << "NEW USER JOINED"  << std::endl;
				epoll_event ev;
				ev.events = EPOLLIN;
				ev.data.fd = client_fd;
				this->_client.add_client(client_fd);
				epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev);
			}
			else
			{
				char buffer[1024];
				int bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
				if(bytes > 512)
					send_server_msg(fd,"Message to big");
				else if (bytes <= 0) 
				{
					t_client &clt = this->_client.get_client(fd);
					clt.disconnected = true;
					this->_channel.disconnect_channels(clt);
					std::cout << "USER DISCONNECTED" << std::endl;
					epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
					this->_client.remove_client(fd);
				}
				else 
				{
					buffer[bytes] = '\0';
					// std::cout << "Server: " << buffer;
					this->read_buffer(buffer,fd, bytes);
					
				}
			}
		}
	}
	close(ServerSocket);
}

bool parseword(char *av)
{
	std::string pass(av);

	if(pass.empty())
	{
		std::cerr << "Error: Empty server password" << std::endl;
		return(false);
	}
	if(pass.size() > 64)
		std::cerr << "Error: Server Password too massive" << std::endl;
	for(size_t i = 0;i < pass.size();i++)
	{
		if(!isalnum(pass[i]))
		{
			std::cerr << "Error: Invalid password must be alpha numeric" << std::endl;
			return(false);
		}
	}
	return (true);
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
		if(!parseword(av[2]))
			return(1);
		struct sigaction sign;
		sign.sa_handler = &action_handler;
		sign.sa_flags = 0;
		sigemptyset(&sign.sa_mask);
		sigaction(SIGINT, &sign, NULL);
		Server server(av[2]);
		server.server(av[1]);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
}