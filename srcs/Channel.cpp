/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgarcez- <dgarcez-@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/11 15:09:52 by dgarcez-          #+#    #+#             */
/*   Updated: 2026/08/11 17:04:51 by dgarcez-         ###   ########.fr       */
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
		send_server_msg(clt.fd, "Invalid channel name");
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