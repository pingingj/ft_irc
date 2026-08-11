/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_irc.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgarcez- <dgarcez-@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 17:43:47 by dgarcez-          #+#    #+#             */
/*   Updated: 2026/08/11 16:34:37 by dgarcez-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_IRC_HPP
#define FT_IRC_HPP


#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdlib>
#include <fcntl.h>
#include <cerrno>
#include <sys/epoll.h>
#include <map>
#include <vector>
#include <sstream>
#include <cctype>
#include <set>
#include <signal.h>
#include <cstdlib>
#include <algorithm>

typedef struct s_reg
{
	std::string string;
	bool exists;
} t_reg;

typedef struct s_client
{
	std::string buffer;
	t_reg	user;
	t_reg	nick;
	bool 	c_pass;
	bool	registered;
	int		fd;
	bool	in_channel;
	std::set<std::string> channels;
} t_client;

typedef struct s_channel
{
	std::set<int> clt_fds;
	std::set<int> admins;
	std::string name;
	std::string topic;
	bool invite_only;
	bool topic_change;
	t_reg password;
	size_t user_limit;
	size_t clt_counter;
} t_channel;

class Client
{
	private:
		std::map<int, t_client> _clients;
		std::set<std::string> _users;

	public:
		Client();
		Client(const Client &obj);
		Client &operator=(const Client &obj);
		~Client();
		void add_client(int fd);
		t_client &get_client(int fd);
		void remove_client(int fd);
		void sendHelp(t_client clt);
		void handle_pass(std::vector<std::string> split_msg, t_client &clt, std::string s_pass);
		void handle_user(std::vector<std::string> split_msg,t_client &clt);
		void handle_nick(std::vector<std::string> split_msg, t_client &clt);
};


class Channel
{
	private:
		std::map<std::string,t_channel> channels;
	public:
		Channel();
		Channel(const Channel &obj);
		Channel &operator=(const Channel &obj);
		~Channel();
		void		handle_join(std::vector<std::string> split_msg, t_client &clt);
		static void channel_commands(t_client clt);
};

class Server
{
	private:
		std::string _pass;
		Client _client;
		Channel _channel;
	public:
		Server();
		Server(std::string s_pass);
		Server(const Server &obj);
		Server &operator=(const Server &obj);
		~Server();
		void	server(char *port);
		void	read_buffer(char *buffer,int fd, int bytes);
		bool	handle_command(std::string command,t_client &clt);
};

std::vector<std::string> split_string(std::string s, std::string delimiter);

std::vector<std::string> split_char(const std::string &s, char delim);

void	send_server_msg(int fd, std::string msg);

#endif