/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: finn <finn@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/11 15:09:52 by dgarcez-          #+#    #+#             */
/*   Updated: 2026/08/24 19:38:38 by finn             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../incs/ft_irc.hpp"

Channel::Channel()
{
	
}

Channel::Channel(Client *client)
{
	this->client_ptr = client;
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
	if (split_msg.size() > 3)
	{
		send_server_msg(clt.fd, "Too many parameters");
		return ;
	}
	std::vector<std::string> channel_nombres = split_char(split_msg[1],',');
	std::vector<std::string> channel_passaggio;
	if(split_msg.size() > 2)
		channel_passaggio = split_char(split_msg[2],',');
	for(size_t i = 0;i < channel_nombres.size();i++)
	{
		send_msg(clt.fd, "Channel ", 0);
		send_msg(clt.fd, channel_nombres[i], 1);
		t_channel &chl = this->channels[channel_nombres[i]];
		if ((channel_nombres[i][0] != '#' && channel_nombres[i][0] != '&') ||  std::count(channel_nombres[i].begin(), channel_nombres[i].end(), '#') > 1 || std::count(channel_nombres[i].begin(), channel_nombres[i].end(), '&') > 1)
		{
			send_msg(clt.fd, " Invalid channel name (ex: #channel_name or &channel_name)",2);
			continue ;	
		}
		if(std::count(channel_nombres[i].begin(), channel_nombres[i].end(), '#') == 1 && std::count(channel_nombres[i].begin(), channel_nombres[i].end(), '&') == 1)
		{
			send_msg(clt.fd, " Invalid channel name (ex: #channel_name or &channel_name)",2);
			continue ;	
		}
		if (chl.name.empty())
		{
			chl.name = channel_nombres[i];
			chl.invite_only = false;
			chl.user_limit_bool = false;
			chl.user_limit = 0;
			chl.topic_change = true;
			chl.admins.insert(clt.fd);
			chl.clt_fds.insert(clt.fd);
			if (channel_passaggio.size() > 0)
			{
				chl.password.exists = true;
				chl.password.string = channel_passaggio[0];
				channel_passaggio.erase(channel_passaggio.begin());
				send_msg(clt.fd, " Channel created with password",2);
			}
			else
			{
				chl.password.exists = false;
				send_msg(clt.fd, " Channel created",2);
			}
			chl.clt_counter += 1;
			clt.channels.insert(channel_nombres[i]);
		}
		else
		{
			if(chl.invite_only == true)
			{
				send_msg(clt.fd, " Channel is invite only",2);
				if (channel_passaggio.size() > 0)
					channel_passaggio.erase(channel_passaggio.begin());
				continue ;
			}
			if (chl.user_limit_bool == true && chl.clt_counter >= chl.user_limit)
			{
				send_msg(clt.fd, " Channel user limit reached",2);
				if (channel_passaggio.size() > 0)
					channel_passaggio.erase(channel_passaggio.begin());
				continue ;
			}
			if (clt.channels.find(channel_nombres[i]) != clt.channels.end())
			{
				send_msg(clt.fd, " Already in channel",2);
				if (channel_passaggio.size() > 0)
					channel_passaggio.erase(channel_passaggio.begin());
				continue ;
			}
			if(chl.password.exists == true)
			{
				if(channel_passaggio.size() > 0)
				{
					if(channel_passaggio[0] != chl.password.string)
					{
						send_msg(clt.fd," Wrong channel password",2);
						channel_passaggio.erase(channel_passaggio.begin());
						continue;
					}
					channel_passaggio.erase(channel_passaggio.begin());
				}
				else
				{
					send_msg(clt.fd," Channel is password protected",2);
					continue ;
				}
			}
			chl.clt_fds.insert(clt.fd);
			clt.channels.insert(channel_nombres[i]);
			chl.clt_counter += 1;
			send_msg(clt.fd, " Joined channel",2);
		}
	}
}

void	Channel::handle_part(std::string split_msg, t_client &clt)
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
	if (this->check_admin(this->channels[split_msg], clt.fd) == true)
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

void Channel::handle_privmsg(std::vector<std::string> split_msg, t_client &clt, std::string msg)
{
	if (split_msg.size() < 2)
	{
		send_server_msg(clt.fd, "Missing channel name");
		return ;
	}
	if (split_msg.size() < 3)
	{
		send_server_msg(clt.fd, "Missing message");
		return ;
	}
	if (split_msg[2][0] == ':')
		msg = msg.substr(msg.find(" :") + 2);
	else
		msg = split_msg[2];
	std::vector<std::string> inoa = split_char(split_msg[1],',');
	for(size_t i = 0;i < inoa.size();i++)
	{
		if ((inoa[i].find("#") != std::string::npos|| inoa[i].find("&") != std::string::npos) && clt.channels.find(inoa[i]) == clt.channels.end())
		{
			send_msg(clt.fd, "Not in channel ", 0);
			send_msg(clt.fd, inoa[i], 1);
			send_msg(clt.fd, " or doesn't exist", 2);
			continue;
		}
		if(inoa[i].find("#") == std::string::npos && inoa[i].find("&") == std::string::npos)
		{

			if(this->client_ptr->search_client_list(inoa[i], clt, msg) == false)
			{
				send_msg(clt.fd, "Nick ", 0);
				send_msg(clt.fd, inoa[i], 1);
				send_msg(clt.fd, " doesn't exist", 2);
			}
			continue;
		}
		send_channel_msg(inoa[i], clt, msg);
		// std::set<int>::iterator fd_it;
		// std::string chop;
		// for (fd_it = this->channels[inoa[i]].clt_fds.begin(); fd_it != this->channels[inoa[i]].clt_fds.end(); ++fd_it)
		// {
		// 	chop = ": ";
		// 	if (this->channels[inoa[i]].admins.find(clt.fd) != this->channels[inoa[i]].admins.end())
		// 		chop += "@";
		// 	std::string prefix = inoa[i] + chop + clt.nick.string + "($" + clt.user.string + "):";
		// 	send_msg(*fd_it, prefix, 1);
		// 	send_msg(*fd_it, msg, 2);
		// }
	}
}

void	Channel::send_channel_msg(std::string channel_name, t_client &clt, std::string msg)
{
	std::set<int>::iterator fd_it;
	std::string chop;
	for (fd_it = this->channels[channel_name].clt_fds.begin(); fd_it != this->channels[channel_name].clt_fds.end(); ++fd_it)
	{
		chop = ": ";
		if (this->check_admin(this->channels[channel_name], clt.fd) == true)
			chop += "@";
		std::string prefix = channel_name + chop + clt.nick.string + "($" + clt.user.string + "):";
		send_msg(*fd_it, prefix, 1);
		send_msg(*fd_it, msg, 2);
	}
}

void	Channel::handle_kick(std::vector<std::string> split_msg, t_client &clt, std::string cmd)
{
	if (split_msg.size() < 3)
	{
		if (split_msg.size() < 2 || (split_msg[1][0] != '#' && split_msg[1][0] != '&'))
		{
			send_server_msg(clt.fd, "Missing channel name");
			return ;
		}
		send_server_msg(clt.fd, "Missing nick to kick");
		return ;
	}
	std::string reason = "";
	if (split_msg.size() > 3 && split_msg[3][0] == ':')
		reason = cmd.substr(cmd.find(" :") + 2);
	else if (split_msg.size() > 3)
		reason = split_msg[3];
	std::vector<std::string> channel_names = split_char(split_msg[1], ',');
	std::vector<std::string> nicknames = split_char(split_msg[2], ',');
	for(size_t i = 0;i < channel_names.size();i++)
	{
		t_channel &chl = this->channels[channel_names[i]];
		if (this->check_admin(chl, clt.fd) == false)
		{
			send_msg(clt.fd, "Not an admin in channel ", 0);
			send_msg(clt.fd, channel_names[i], 1);
			send_msg(clt.fd, " or doesn't exist", 2);
			continue ;
		}
		for (size_t j = 0; j < nicknames.size(); j++)
		{
			int clt_fd = this->client_ptr->get_client_fd(nicknames[j]);
			t_client *cur_nick = this->client_ptr->get_client(clt_fd);
			if (clt_fd == -1 || cur_nick->channels.find(channel_names[i]) == cur_nick->channels.end())
			{
				send_msg(clt.fd, "Nick ", 0);
				send_msg(clt.fd, nicknames[j], 1);
				send_msg(clt.fd, " not in channel ", 1);
				send_msg(clt.fd, channel_names[i], 2);
				continue;
			}
			std::string msg;
			if (reason.empty() == true)
				msg = "Successfully kicked " +  nicknames[j] + " out of " + channel_names[i];
			else
				msg = "Successfully kicked " +  nicknames[j] + " out of " + channel_names[i] + " reason:" + reason;
			send_channel_msg(channel_names[i], clt, msg);
			cur_nick->channels.erase(channel_names[i]);
			this->channels[channel_names[i]].clt_counter--;
			this->channels[channel_names[i]].clt_fds.erase(cur_nick->fd);
			if (this->check_admin(this->channels[channel_names[i]], cur_nick->fd) == true)
				this->channels[channel_names[i]].admins.erase(cur_nick->fd);
		}
	}
}

// TOPIC #42  :topic 

void	Channel::handle_topic(std::vector<std::string> split_msg, t_client &clt, std::string cmd)
{
	if (split_msg.size() < 2)
	{
		send_server_msg(clt.fd, "Missing channel name");
		return ;
	}
	std::string topic;
	if (split_msg.size() < 3)
		topic = split_msg[1];
	else
	{
		if (split_msg[2][0] == ':')
			topic = cmd.substr(cmd.find(" :") + 2);
		else
			topic = split_msg[2];
	}
	t_channel &chl = this->channels[split_msg[1]];
	if(chl.name.empty() || chl.clt_fds.find(clt.fd) == chl.clt_fds.end())
	{
		send_server_msg(clt.fd, "Not in the channel");
		return ;
	}
	if (chl.topic_change == true && this->check_admin(chl, clt.fd) == false)
	{
		send_server_msg(clt.fd, "Can't change topic of channel");
		return ;
	}
	chl.topic = topic;
	std::string msg = " changed the channel " + split_msg[1] + " topic to:" + topic;
	send_channel_msg(split_msg[1], clt, msg);
}

void	Channel::handle_mode(std::vector<std::string> split_msg,t_client &clt)
{
	bool	mode;
	if (split_msg.size() < 2)
	{
		send_server_msg(clt.fd, "Missing channel name");
		return ;
	}
	if (split_msg.size() < 3)
	{
		send_server_msg(clt.fd, "Missing channel mode options to add or remove (+/-itkol)");
		return ;
	}
	// mode #42 +tlo-o 20
	if(split_msg[2][0] != '+' && split_msg[2][0] != '-')
	{
		send_server_msg(clt.fd, "Channel options must start with + (enable) or - (disable)");
		return;
	}
	t_channel &chl = this->channels[split_msg[1]];
	if (chl.name.empty())
	{
		send_server_msg(clt.fd, "Channel doesn't exist");
		return ;
	}
	if (this->check_admin(chl, clt.fd) == false)
	{
		send_server_msg(clt.fd, "Can't change mode of channel");
		return ;
	}
	size_t j = 3;
	for(size_t i = 0;i<  split_msg[2].size();i++)
	{
		if (split_msg[2][i] == '+')
			mode = true;
		else if (split_msg[2][i] == '-')
			mode = false;
		else
		{
			if(split_msg[2][i] == 'i')
			{
				chl.invite_only = mode;
				if(mode == true)
					send_channel_msg(chl.name,clt,"Channel is now invite only");
				else
					send_channel_msg(chl.name,clt,"Channel is now open");
			}
			else if(split_msg[2][i] == 't')
				chl.topic_change = mode;
			else if (split_msg[2][i] == 'k')
			{
				if(mode == true)
				{
					if(split_msg[j].empty() == true)
					{
						send_server_msg(clt.fd,"No password given");
						continue;
					}
					chl.password.string = split_msg[j];
					j++;
				}
				chl.password.exists = mode;
			}
			else if (split_msg[2][i] == 'o')
			{
				if(mode == true)
				{
					if(split_msg[j].empty() == false)
					{
						int clt_fd = this->client_ptr->get_client_fd(split_msg[j]);
						if(check_admin(chl,clt_fd) == true)
						{
							send_msg(clt.fd,split_msg[j],0);
							send_msg(clt.fd," is already an operator in channel ",1);
							send_msg(clt.fd,chl.name,2);
						}
						chl.admins.insert(clt.fd);
						send_msg(clt_fd,"You are now a operator in channel ",0);
						send_msg(clt_fd,chl.name,2);
						j++;
					}
				}
				else
				{
					if(split_msg[j].empty() == false)
					{
						int clt_fd = this->client_ptr->get_client_fd(split_msg[j]);
						if(check_admin(chl,clt_fd) == false)
						{
							send_msg(clt.fd,split_msg[j],0);
							send_msg(clt.fd," is not an operator in channel ",1);
							send_msg(clt.fd,chl.name,2);
						}
						chl.admins.erase(clt.fd);
						send_msg(clt_fd,"You are no longer an operator in channel ",0);
						send_msg(clt_fd,chl.name,2);
						j++;
					}
				}
			}
			else if (split_msg[2][i] == 'l')
			{
				if(mode == true)
				{
					if(split_msg[j].empty() == false)
					{
						if(split_msg[j].find_first_not_of("0123456789") != std::string::npos)
						{
							send_server_msg(clt.fd,"Invalid limit amount");
							j++;
							continue;
						}
						long u_limit = atol(split_msg[j].c_str());
						if(u_limit > std::numeric_limits<int>::max())
							u_limit = std::numeric_limits<int>::max();
						chl.user_limit = u_limit;
					}
				}
				chl.user_limit_bool = mode;
			}
			else
				send_server_msg(clt.fd, "Invalid channel mode");
		}
	}
}

void Channel::channel_commands(std::vector<std::string> split_msg, t_client &clt, std::string command)
{
	if (split_msg[0] == "PART")
		this->handle_part(split_msg[1], clt);
	else if(split_msg[0] == "PRIVMSG")
		this->handle_privmsg(split_msg, clt, command);
	else if (split_msg[0] == "KICK")
		this->handle_kick(split_msg, clt, command);
	else if (split_msg[0] == "TOPIC")
		this->handle_topic(split_msg, clt, command);
	else if (split_msg[0] == "MODE")
		this->handle_mode(split_msg, clt);
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
		this->handle_part(channel, clt);
	}
}

bool	Channel::check_admin(t_channel &chl,size_t clt_fd)
{
	if (chl.admins.find(clt_fd) == chl.admins.end())
		return (false);
	return (true);
}