/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgarcez- <dgarcez-@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 17:58:22 by dgarcez-          #+#    #+#             */
/*   Updated: 2026/08/19 17:39:48 by dgarcez-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/ft_irc.hpp"

Client::Client()
{
	
}


Client::~Client()
{
	for (std::map<int, t_client >::iterator it = this->_clients.begin(); it != this->_clients.end(); ++it)
		close(it->first);
}

Client::Client(const Client &obj)
{
	if(this != &obj)
		return;
	return;	
}


Client &Client::operator=(const Client &obj)
{
	(void)obj;
	return (*this);
}

void Client::add_client(int fd)
{
	t_client	client;

	client.fd = fd;
	client.registered = false;
	client.c_pass = false;
	client.nick.exists = false;
	client.user.exists = false;
	client.disconnected = false;
	this->_clients.insert(std::make_pair(fd, client));
}

void Client::remove_client(int fd)
{
	t_client &clt = this->_clients.at(fd);
	_users.erase(clt.user.string);
	_clients.erase(clt.fd);
	close(fd);
}

void Client::sendHelp(t_client clt)
{
	if (clt.registered == true)
	{
		(void)clt;
	}
	else
	{
		send_server_msg(clt.fd,"User is not registered!\nFollow these steps to register:");
		send_server_msg(clt.fd,"Step 1: Enter server password with [PASS (server_password)] ");
		send_server_msg(clt.fd,"Step 2: Enter a unique user name with [USER (your_username)] ");
		send_server_msg(clt.fd,"Step 3: Enter a nickname with [NICK (your_nickname)] ");
	}
}

void Client::handle_pass(std::vector<std::string> split_msg, t_client &clt, std::string s_pass)
{
	std::cout << "HERE\n";
	if (clt.c_pass == true)
	{
		send_server_msg(clt.fd, "Already logged in");
		return ;
	}
	if (split_msg.size() < 2)
	{
		send_server_msg(clt.fd, "Missing password");
		return ;
	}
	if (split_msg.size() != 2)
	{
		send_server_msg(clt.fd, "Password is only 1 word");
		return ;
	}
	if (split_msg[1] != s_pass)
	{
		send_server_msg(clt.fd, "Invalid password");
		return ;
	}
	send_server_msg(clt.fd, "Successfully logged in");
	clt.c_pass = true;
}

void Client::handle_user(std::vector<std::string> split_msg, t_client &clt)
{
	if (clt.c_pass == false)
	{
		send_server_msg(clt.fd, "You must join the server (password)");
		return ;
	}
	if (split_msg.size() < 2)
	{
		send_server_msg(clt.fd, "Missing username");
		return ;
	}
	if (split_msg.size() != 2)
	{
		send_server_msg(clt.fd, "Wrong format e.g: USER (username)");
		return ;
	}
	if (str_isalnum(split_msg[1]) == false)
	{
		send_server_msg(clt.fd, "Username must be alpha numeric");
		return ;
	}
	if (split_msg[1] == "CHECK")
	{
		send_server_msg(clt.fd, "Your current USERNAME is ");
		send_server_msg(clt.fd, clt.user.string);
		return ;
	}
	if (clt.user.exists == true)
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
	clt.user.exists = true;
}

void Client::handle_nick(std::vector<std::string> split_msg, t_client &clt)
{
	if (clt.c_pass == false)
	{
		send_server_msg(clt.fd, "You must join the server (password)");
		return ;
	}
	if (split_msg.size() < 2)
	{
		send_server_msg(clt.fd, "Missing nickname");
		return ;
	}
	if (split_msg.size() != 2)
	{
		send_server_msg(clt.fd, "Wrong format e.g: NICK (nickname)");
		return ;
	}
	if (str_isalnum(split_msg[1]) == false)
	{
		send_server_msg(clt.fd, "Nickname must be alpha numeric");
		return ;
	}
	if (clt.user.exists == false)
	{
		send_server_msg(clt.fd, "You must have a USERNAME first");
		return ;
	}
	if (split_msg[1] == "CHECK")
	{
		send_server_msg(clt.fd, "Your current NICKNAME is ");
		send_server_msg(clt.fd, clt.nick.string);
		return ;
	}
	if (clt.nick.exists == true)
	{
		send_server_msg(clt.fd, "NICKNAME successfully changed");
		clt.nick.string = split_msg[1];
		return;
	}
	send_server_msg(clt.fd, "Nick set");
	clt.nick.string = split_msg[1];
	clt.nick.exists = true;
	clt.registered = true;
}

bool Server::handle_command(std::string command, t_client &clt)
{
	std::vector<std::string> split_msg;
	split_msg = split_char(command, ' ');
	std::cout << "Msg size: " << split_msg.size() << std::endl;
	for (size_t i = 0; i < split_msg.size(); i++)
	{
		std::cout << "args: " << split_msg[i] << std::endl;
	}
	if (split_msg[0] == "HELP")
	{
		if (split_msg.size() != 1)
			send_server_msg(clt.fd, "Unknown command, try [HELP]");
		else
			this->_client.sendHelp(clt);
	}
	else if (split_msg[0] == "PASS")
		this->_client.handle_pass(split_msg, clt, _pass);
	else if (split_msg[0] == "USER")
		this->_client.handle_user(split_msg, clt);
	else if (split_msg[0] == "NICK")
		this->_client.handle_nick(split_msg, clt);
	else if (split_msg[0] == "JOIN")
		this->_channel.handle_join(split_msg, clt);	
	else if(clt.registered == true)
		this->_channel.channel_commands(split_msg, clt, this->_channel);
	else
		send_server_msg(clt.fd, "Unknown command");
	std::cout << "splitmsg !!" << split_msg[0] << "!!" << std::endl;
	return (true);
}

t_client &Client::get_client(int fd)
{
		return(this->_clients.at(fd));
}
void Server::read_buffer(char *buffer, int fd, int bytes)
{
	std::vector<std::string> commands;
	t_client &clt = this->_client.get_client(fd);
	clt.buffer.append(buffer, bytes);
	std::cout << "REAL BUFFER LOL " << clt.buffer << std::endl;
	if (clt.buffer.find("\r\n") == std::string::npos)
		return ;
	commands = split_string(clt.buffer, "\r\n");
	if (commands[0].empty())
	{
		clt.buffer.erase(clt.buffer.begin(), clt.buffer.end());
		return ;
	}
	std::cout << "command size: " << commands.size() << std::endl;
	for (size_t i = 0; i < commands.size(); i++)
	{
		std::cout << "Command: " << commands.at(i) << std::endl;
		this->handle_command(commands[i], clt);
	}
	clt.buffer.erase(clt.buffer.begin(), clt.buffer.end());
}
