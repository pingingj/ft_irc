/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_irc.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgarcez- < dgarcez-@student.42lisboa.com > +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 17:43:47 by dgarcez-          #+#    #+#             */
/*   Updated: 2026/07/30 18:56:05 by dgarcez-         ###   ########.fr       */
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

typedef struct s_client
{
	std::string user;
	std::string nick;
	bool		registered;
	int			fd;
	
} t_client ;

class Client
{
	private:
		std::string s_pass;
		std::map<int, t_client> _clients;
		bool handle_register(std::string command,t_client clt);
		void sendHelp(t_client clt);
		void handle_pass(std::vector<std::string> split_msg,t_client clt);

		
	public:
		Client();
		Client(std::string server_password);
		void add_client(int fd);
		void read_buffer(std::string buffer,int fd);
		void remove_client(int fd);

};

std::vector<std::string> split_string(std::string s, std::string delimiter);

std::vector<std::string> split_char(const std::string &s, char delim);

#endif