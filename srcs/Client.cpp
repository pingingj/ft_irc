/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgarcez- < dgarcez-@student.42lisboa.com > +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 17:58:22 by dgarcez-          #+#    #+#             */
/*   Updated: 2026/07/30 19:01:29 by dgarcez-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/ft_irc.hpp"

Client::Client()
{
	
}

Client::Client(std::string server_password)
{
	this->s_pass = server_password;
}


void Client::add_client(int fd)
{
	t_client client;
	client.fd = fd;
	client.registered = false;
	this->_clients.insert(std::make_pair(fd,client));
}

void send_server_msg(int fd,std::string msg)
{
	std::string res = msg + "\r\n";
	send(fd,res.c_str(),res.size(),0);
}
void	Client::sendHelp(t_client clt)
{
	if(clt.registered == true)
	{
		(void)clt;
	}
	else
	{
		send_server_msg(clt.fd,"User is not registered!\nFollow these steps to register:");
		send_server_msg(clt.fd,"Step 1: Enter server password with [PASS (server_password)] ");
		send_server_msg(clt.fd,"Step 2: Enter a unique user name with [USER (your_username)] ");
		send_server_msg(clt.fd,"Step 3: Enter a nickname with [NICK (your_nickname)] ") 	;
	}
}

void	Client::handle_pass(std::vector<std::string> split_msg,t_client clt)
{
	(void)split_msg;
	(void)clt;
	std::cout << "HERE\n";
}

bool Client::handle_register(std::string command,t_client clt)
{
	std::vector<std::string> split_msg;
	split_msg = split_char(command,' ');
	std::cout << "Msg size: " << split_msg.size() << std::endl;

	if(split_msg[0] == "HELP")
	{
		if(split_msg.size() != 1)
			std::cout <<"Unknown command, try [HELP]" << std::endl;
		sendHelp(clt);
	}
	else if (split_msg[0] == "PASS")
		handle_pass(split_msg,clt); 
		
	// else if (split_msg[0] == "USER")
		
	// else if (split_msg[0] == "NICK")
		
	// else if (split_msg[0] == "JOIN")
	// else if (split_msg[0] == "KICK")
	// else if (split_msg[0] == "USER")
	// else if (split_msg[0] == "USER")
	std::cout << "!!" << split_msg[0] << "!!" << std::endl;
	return true;
}

void Client::read_buffer(std::string buffer,int fd)
{
	// std::cout << "banana\n";
	t_client clt;

	std::vector<std::string> commands;
	clt = this->_clients.at(fd);
	commands = split_string(buffer,"\r\n");
	std::cout << "command size: " << commands.size() << std::endl;
	for(size_t i = 0;i < commands.size();i++)
	{
		std::cout << "Command: " << commands.at(i) << std::endl;
		this->handle_register(commands[i],clt);
	}
	
}
