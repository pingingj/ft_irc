/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgarcez- < dgarcez-@student.42lisboa.com > +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 17:58:22 by dgarcez-          #+#    #+#             */
/*   Updated: 2026/08/05 17:57:26 by dgarcez-         ###   ########.fr       */
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
	client.c_pass = false;
	client.nick.finished = false;
	client.user.finished = false;
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

void	Client::handle_pass(std::vector<std::string> split_msg,t_client &clt)
{
	std::cout << "HERE\n";
	send_server_msg(clt.fd, "BANANAmake");
	if (clt.c_pass == true)
	{
		send_server_msg(clt.fd, "Already logged in");
		return ;
	}
	if (split_msg[1] != this->s_pass)
	{
		send_server_msg(clt.fd,"Invalid password");
		return ;
	}
	send_server_msg(clt.fd, "Successfully logged in");
	clt.c_pass = true;
}

void	Client::handle_user(std::vector<std::string> split_msg,t_client &clt)
{
	if (clt.c_pass == false)
	{
		send_server_msg(clt.fd, "You must join the server (password)");
		return ;
	}
	if (clt.user.finished == true)
	{
		send_server_msg(clt.fd, "Can't change user");
		return ;
	}
	if (this->_users.find(split_msg[1]) != this->_users.end())
	{
		send_server_msg(clt.fd, "User already in use");
		return ;
	}
	send_server_msg(clt.fd, "User set");
	clt.user.string = split_msg[1];
	this->_users.insert(split_msg[1]);
	clt.user.finished = true;
}

bool Client::handle_command(std::string command,t_client &clt)
{
	std::vector<std::string> split_msg;
	split_msg = split_char(command,' ');
	std::cout << "Msg size: " << split_msg.size() << std::endl;
	for(size_t i = 0;i < split_msg.size(); i++)
	{
		std::cout << "args: " << split_msg[i] << std::endl;
	}
	if(split_msg[0] == "HELP")
	{
		if(split_msg.size() != 1)
			send_server_msg(clt.fd, "Unknown command, try [HELP]");
		else
			sendHelp(clt);
	}
	else if (split_msg[0] == "PASS")
		handle_pass(split_msg,clt); 
	else if (split_msg[0] == "USER")
		handle_user(split_msg, clt);
	else
		send_server_msg(clt.fd, "Unknown command");
	// else if (split_msg[0] == "USER")
		
	// else if (split_msg[0] == "NICK")
		
	// else if (split_msg[0] == "JOIN")
	// else if (split_msg[0] == "KICK")
	// else if (split_msg[0] == "USER")
	// else if (split_msg[0] == "USER")
	std::cout << "splitmsg !!" << split_msg[0] << "!!" << std::endl;
	return true;
}

void Client::read_buffer(char *buffer,int fd, int bytes)
{
	std::vector<std::string> commands;
	t_client &clt = this->_clients.at(fd);
	clt.buffer.append(buffer, bytes);
	std::cout << "REAL BUFFER LOL " << clt.buffer << std::endl;
	if (clt.buffer.find("\r\n") == std::string::npos)
		return;
	commands = split_string(clt.buffer, "\r\n");
	if (commands[0].empty())
	{
		clt.buffer.erase(clt.buffer.begin(),clt.buffer.end());
		return;
	}
	std::cout << "command size: " << commands.size() << std::endl;
	for(size_t i = 0;i < commands.size();i++)
	{
		std::cout << "Command: " << commands.at(i) << std::endl;
		this->handle_command(commands[i],clt);
	}
	clt.buffer.erase(clt.buffer.begin(),clt.buffer.end());
}
