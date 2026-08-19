/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgarcez- <dgarcez-@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/11 15:09:52 by dgarcez-          #+#    #+#             */
/*   Updated: 2026/08/19 17:44:28 by dgarcez-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../incs/ft_irc.hpp"

Channel::Channel()
{
	
}

Channel::~Channel()
{

}

Channel::Channel(const Channel &obj)
{
	if(this != &obj)
		return;
	return;	
}


Channel &Channel::operator=(const Channel &obj)
{
	(void)obj;
	return (*this);
}


// void Channel::channel_options(t_channel &chl)
// {
// 	// while(true)
// 	// {
// 	// 	char buffer[1024];
// 	// 	int bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
// 	// }
// }

void Channel::handle_join(std::vector<std::string> split_msg, t_client &clt)
{
	if (clt.registered == false)
	{
		send_server_msg(clt.fd, "User is not registered");
		return ;
	}
	if (split_msg.size() < 2)
	{
		send_server_msg(clt.fd, "Missing channel name");
		return ;
	}
	if (split_msg[1][0] != '#' || std::count(split_msg[1].begin(), split_msg[1].end(), '#') > 1)
	{
		send_server_msg(clt.fd, "Invalid channel name (ex: #channel_name)");
		return ;	
	}
	if (split_msg.size() > 3)
	{
		send_server_msg(clt.fd, "Too many parameters");
		return ;
	}
	t_channel &chl = this->channels[split_msg[1]];
	if (chl.name.empty())
	{
		chl.name = split_msg[1];
		chl.invite_only = true;
		chl.user_limit = 20;
		chl.topic_change = true;
		chl.admins.insert(clt.fd);
		chl.clt_fds.insert(clt.fd);
		if (split_msg.size() == 3)
		{
			chl.password.exists = true;
			chl.password.string = split_msg[2];
			send_server_msg(clt.fd, "Channel created with password");
		}
		else
		{
			chl.password.exists = false;
			send_server_msg(clt.fd, "Channel created");
		}
		chl.clt_counter += 1;
		clt.channels.insert(split_msg[1]);
		return ;
	}
	else
	{
		if (chl.clt_counter >= chl.user_limit)
		{
			send_server_msg(clt.fd, "Channel user limit reached");
			return ;
		}
		if (clt.channels.find(split_msg[1]) != clt.channels.end())
		{
			send_server_msg(clt.fd, "Already in channel");
			return ;
		}
		chl.clt_fds.insert(clt.fd);
		clt.channels.insert(split_msg[1]);
		chl.clt_counter += 1;
		send_server_msg(clt.fd, "Joined channel");
	}
	// if (split_msg[1] == "CHECK")
	// {
	// 	send_server_msg(clt.fd, "Your current NICKNAME is ");
	// 	send_server_msg(clt.fd, clt.nick.string);
	// 	return ;
	// }
}

void	Channel::handle_exit(std::string split_msg, t_client &clt)
{
	if (split_msg.size() < 2)
	{
		send_server_msg(clt.fd, "Missing channel name to exit");
		return ;
	}
	if (clt.channels.find(split_msg) == clt.channels.end())
	{
		send_server_msg(clt.fd, "Not in this channel or doesn't exist");
		return ;
	}
	clt.channels.erase(split_msg);
	this->channels[split_msg].clt_counter--;
	this->channels[split_msg].clt_fds.erase(clt.fd);
	if (this->channels[split_msg].admins.find(clt.fd) != this->channels[split_msg].admins.end())
	{
		this->channels[split_msg].admins.erase(clt.fd);
		if (this->channels[split_msg].admins.size() <= 0)
		{
			this->channels[split_msg].admins.insert(*this->channels[split_msg].clt_fds.begin());
			send_msg(*this->channels[split_msg].clt_fds.begin(), "You have become an admin in ", 1);
			send_msg(*this->channels[split_msg].clt_fds.begin(), split_msg, 2);
		}
	}
	if (this->channels[split_msg].clt_counter <= 0)
	{
		this->channels.erase(split_msg);
		if (clt.disconnected == false)
			send_server_msg(clt.fd, "Successfuly left the channel, channel has been deleted.");
		return ;
	}
	if (clt.disconnected == false)
		send_server_msg(clt.fd, "Successfuly left the channel");
}

void Channel::handle_privmsg(std::vector<std::string> split_msg, t_client &clt)
{
	if (split_msg[1].size() < 2)
	{
		send_server_msg(clt.fd, "Missing channel name");
		return ;
	}
	if (clt.channels.find(split_msg[1]) == clt.channels.end())
	{
		send_server_msg(clt.fd, "Not in this channel or doesn't exist");
		return ;
	}
	std::set<int>::iterator fd_it;
	for (fd_it = this->channels[split_msg[1]].clt_fds.begin(); fd_it != this->channels[split_msg[1]].clt_fds.end(); ++fd_it)
	{
		std::string prefix = split_msg[1] + ": " + clt.nick.string + "(@" + clt.user.string + "):";
		send_msg(*fd_it, prefix, 1);
		for (size_t i = 2; i < split_msg.size() - 1; i++)
		{
			send_msg(*fd_it, split_msg[i], 1);
			send_msg(*fd_it, " ", 1);
		}
		send_msg(*fd_it, split_msg.back(), 2);
	}

}

void Channel::channel_commands(std::vector<std::string> split_msg, t_client &clt, Channel &chl)
{
	if (split_msg[0] == "EXIT")
		chl.handle_exit(split_msg[1], clt);
	else if(split_msg[0] == "PRIVMSG")
		chl.handle_privmsg(split_msg, clt);
	else
		send_server_msg(clt.fd, "Unknown command");
}

void	Channel::disconnect_channels(t_client &clt)
{
	std::set<std::string>::iterator c_it;
	for (c_it = clt.channels.begin(); c_it != clt.channels.end();)
	{
		std::string channel = *c_it;
		++c_it;
		this->handle_exit(channel, clt);
	}
}