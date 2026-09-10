/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgarcez- < dgarcez-@student.42lisboa.com > +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 17:58:22 by dgarcez-          #+#    #+#             */
/*   Updated: 2026/09/10 18:52:12 by dgarcez-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/ft_irc.hpp"

Client::Client()
{
	
}

Client::Client(Channel *channel)
{
	this->channel_ptr = channel;
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
	client.nick.string = "*";
	client.user.exists = false;
	client.disconnected = false;
	this->_clients.insert(std::make_pair(fd, client));
}

void Client::remove_client(int fd)
{
	std::map<int, t_client>::iterator it = this->_clients.find(fd);
	if (it == this->_clients.end())
		return;
	if (it->second.nick.exists)
		this->_nicks.erase(it->second.nick.string);
	this->_clients.erase(it);
	close(fd);
}

void Client::sendHelp(t_client clt)
{
	if (clt.registered == true)
	{
		send_server_msg(clt.fd,"JOIN -> /JOIN <channel_name>,(<channel_name>) :to join a channel");
		send_server_msg(clt.fd,"PART -> /PART <channel_name>,(<channel_name>) :to leave a channel");
		send_server_msg(clt.fd,"PRIVMSG -> /PRIVMSG <nick_name/channel_name>,(<nickname/channel_name>) :to send a privmsg to a channel/user");
		send_server_msg(clt.fd,"WHO -> /WHO <channel_name> :to check users in the channel");
		send_msg(clt.fd,"\nOPERATOR COMMANDS:",2);
		send_server_msg(clt.fd,"KICK -> /KICK <channel_name> <nickname> (reason) :to kick a user from a channel");
		send_server_msg(clt.fd,"INVITE -> /INVITE <nickname> <channel_name> :to invite user to a channel");
		send_server_msg(clt.fd,"TOPIC -> /TOPIC <channel_name> (<new_topic>) :to see/(change) the channel topic");
		send_server_msg(clt.fd,"MODE -> /MODE <channel_name> <[+|-]i|k|t|o|l> (<parameters>) :to join a channel");
	}
	else
	{
		send_server_msg(clt.fd,"User is not registered!\nFollow these steps to register:");
		send_server_msg(clt.fd,"Step 1: Enter server password with [PASS (server_password)] ");
		send_server_msg(clt.fd,"Step 2: Enter a unique nick name with [NICK (your_nickname)] ");
		send_server_msg(clt.fd,"Step 3: Enter a username with [USER (<username> <hostname> <servername> <realname>)] ");
	}
}

void Client::handle_pass(std::vector<std::string> split_msg, t_client &clt, std::string s_pass)
{
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
		send_msg(clt.fd, ":server 464 " + clt.nick.string + " :Password incorrect",2);
		return ;
	}
	send_server_msg(clt.fd, "Successfully logged in");
	clt.c_pass = true;
}

void Client::handle_user(std::vector<std::string> split_msg, t_client &clt, std::string cmd)
{
	std::string msg;
	if (split_msg.size() < 5)
	{
		msg = ":server 461 " + clt.nick.string + " USER :Not enough parameters";
		send_msg(clt.fd,msg,2);
		return ;
	}
	if (str_isalnum(split_msg[1]) == false)
	{
		send_server_msg(clt.fd, "Username must be alpha numeric");
		return ;
	}
	if (clt.user.exists == true)
	{
		msg = ":server 462 * " + split_msg[1] +  " :You may not reregister";
		send_msg(clt.fd,msg,2);
		return ;
	}
	if (split_msg[4][0] == ':')
		clt.real_name = cmd.substr(cmd.find(" :") + 2);
	else
		clt.real_name = split_msg[4];
	std::string feeback = split_msg[1] + " :User set";
	send_msg(clt.fd, feeback,2);
	clt.user.string = split_msg[1];
	clt.user.exists = true;
}

void	Client::change_nick(std::vector<std::string> split_msg, t_client &clt)
{
	std::set<int>::iterator fd_it;
	std::map<std::string,t_channel> var  = this->channel_ptr->get_channels();
	std::string response = ":" + clt.nick.string + "!" + clt.user.string + "@hostname " + "NICK " +  ":" + split_msg[1] + "\r\n";
	for(std::set<std::string>::iterator it = clt.channels.begin();it != clt.channels.end();it++)
	{
		t_channel channel = var[*it];
		for (fd_it = channel.clt_fds.begin(); fd_it != channel.clt_fds.end(); ++fd_it)
			send(*fd_it,response.c_str(),response.size(),0);
	}
}

void Client::handle_nick(std::vector<std::string> split_msg, t_client &clt)
{
	std::string msg;
	if (split_msg.size() < 2)
	{
		msg = ":server 461 " + clt.nick.string + " NICK :Not enough parameters";
		send_msg(clt.fd, msg, 2);
	}
	else if (isalpha(split_msg[1][0]) == false || str_isalnum(split_msg[1]) == false)
	{
		std::string msg = ":server 432 * " + split_msg[1] +  " " + ":Erroneous nickname";
		send_msg(clt.fd,msg,2);
	}
	else if (split_msg[1].size() > 9)
	{
		send_server_msg(clt.fd, "NICKNAME too massive");
		return;
	}
	else if (this->_nicks.find(split_msg[1]) != this->_nicks.end())
	{
		std::string msg = ":server 433 * " + split_msg[1] +  " " + ":Nick already in use";
		send_msg(clt.fd,msg,2);
	}
	else if (clt.nick.exists == true)
	{
		std::string response = ":" + clt.nick.string + "!" + clt.user.string + "@hostname " + "NICK " +  ":" + split_msg[1] + "\r\n";
		change_nick(split_msg,clt);
		this->_nicks.erase(clt.nick.string);
		this->_nicks.insert(std::make_pair(split_msg[1],clt.fd));
		clt.nick.string = split_msg[1];
	}
	else
	{
		clt.nick.exists = true;
		clt.nick.string = split_msg[1];
		this->_nicks.insert(std::make_pair(split_msg[1],clt.fd));
		std::string feeback = split_msg[1] + " :Nickname set";
		send_msg(clt.fd, feeback,2);
		return;
	}
}

// void Client::handle_fast(t_client &clt)
// {
// 	clt.c_pass = true;
// 	clt.nick.exists = true;
// 	clt.user.exists = true;
// 	clt.nick.string = "nick1";
// 	clt.user.string = "user1";
// 	clt.real_name = "realname1";
// 	this->_nicks.insert(std::make_pair(clt.nick.string,clt.fd));
// 	clt.registered = true;
// }

// void Client::handle_fast2(t_client &clt)
// {
// 	clt.c_pass = true;
// 	clt.nick.exists = true;
// 	clt.user.exists = true;
// 	clt.nick.string = "nick2";
// 	clt.user.string = "user2";
// 	clt.real_name = "realname2";
// 	this->_nicks.insert(std::make_pair(clt.nick.string,clt.fd));
// 	clt.registered = true;
// }

bool Server::handle_command(std::string command, t_client &clt)
{
	std::vector<std::string> split_msg;
	if(command.empty())
		return(false);
	split_msg = split_char(command, ' ');
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
		this->_client.handle_user(split_msg, clt, command);
	else if (split_msg[0] == "NICK")
		this->_client.handle_nick(split_msg, clt);
	else if (split_msg[0] == "JOIN")
		this->_channel.handle_join(split_msg, clt);
	else if (split_msg[0] == "QUIT")
	{
		this->_channel.disconnect_channels(clt, this->epfd);
		return (false);
	}
	else if(clt.registered == true)
		this->_channel.channel_commands(split_msg, clt, command);
	else
		send_server_msg(clt.fd, "Unknown command");
	if(clt.registered == false && clt.nick.exists && clt.user.exists && clt.c_pass)
	{
		clt.registered = true;
		std::string feeback = ":server 001 " + clt.nick.string + " :Welcome to IRC! :D";
		send_msg(clt.fd, feeback,2);
	}
	return (true);
}

t_client *Client::get_client(int fd)
{
	std::map<int, t_client>::iterator it = this->_clients.find(fd);
	if (it == this->_clients.end())
		return NULL;
	return &it->second;
}

bool Client::search_client_list(std::string inoa, t_client &clt, std::string msg)
{
	std::map<std::string,int>::iterator it = this->_nicks.find(inoa);
	if(it != this->_nicks.end())
	{
		std::string response = ":" + clt.nick.string + "!" + clt.user.string + "@hostname " + "PRIVMSG " + inoa +  " :" + msg + "\r\n";
		send(it->second,response.c_str(),response.size(),0);
		return(true);
	}
	return(false);
}

int Client::get_client_fd(std::string nick)
{
	std::map<std::string,int>::iterator it = this->_nicks.find(nick);
	if(it != this->_nicks.end())
		return(it->second);
	return(-1);
}

void Server::read_buffer(char *buffer, int fd, int bytes)
{
	t_client *clt = this->_client.get_client(fd);
	clt->buffer.append(buffer, bytes);

	size_t end;

	while ((end = clt->buffer.find("\r\n")) != std::string::npos)
	{
		std::string command = clt->buffer.substr(0, end);
		if(command.size() > 510)
		{
			send_server_msg(fd,"Message to big");
			t_client *clt = this->_client.get_client(fd);
			clt->buffer.erase(clt->buffer.begin(), clt->buffer.end());
			continue;
		}
		clt->buffer.erase(0, end + 2);

		if (command.empty())
			continue;

		if (this->handle_command(command, *clt) == false)
			return;
	}
}