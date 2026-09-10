/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: finn <finn@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/11 15:09:52 by dgarcez-          #+#    #+#             */
<<<<<<< HEAD
/*   Updated: 2026/09/10 19:35:20 by dgarcez-         ###   ########.fr       */
=======
/*   Updated: 2026/09/10 19:21:49 by finn             ###   ########.fr       */
>>>>>>> d9b209aaae4fb0d96e35c33b77685bea8ce48d6f
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

void Channel::join_detail(t_channel &chl, t_client &clt)
{
	std::string msg;
	if (chl.topic.empty() == true)
	{
		msg = ":server 331 " + clt.nick.string + " " + chl.name + " :No topic is set";
		send_msg(clt.fd, msg, 2);
	}
	else
	{
		msg = ":server 332 " + clt.nick.string + " " + chl.name + " :" + chl.topic;
		send_msg(clt.fd, msg, 2);
	}
	std::string response = ":server 353 " + clt.nick.string + " = " + chl.name + " :";
	std::set<int>::iterator it = chl.clt_fds.begin();
	for(; it != chl.clt_fds.end(); it++)
	{
		t_client *cur_nick = this->client_ptr->get_client(*it);
		if (check_admin(chl, *it) == true)
			response += "@";
		response += cur_nick->nick.string + " ";
	}
	std::string end = ":server 366 " + clt.nick.string + " " + chl.name + " :End of /NAMES list";

	send_msg(clt.fd, response, 2);
	send_msg(clt.fd, end, 2);
}
bool Channel::join_channel(t_client &clt,t_channel &chl,std::vector<std::string> *channel_passaggio,std::vector<std::string> channel_nombres,int i)
{
	std::string error;
	
	if (clt.channels.find(channel_nombres[i]) != clt.channels.end())
	{
		std::string msg = " Already in " + channel_nombres[i];
		send_msg(clt.fd, msg, 2);
		if ((*channel_passaggio).size() > 0)
			(*channel_passaggio).erase((*channel_passaggio).begin());
		return(false) ;
	}
	if (chl.user_limit_bool == true && chl.clt_counter >= chl.user_limit)
	{
		error = ":server 471 " + clt.nick.string + " " + channel_nombres[i] + " :Cannot join channel (+l)";
			send_msg(clt.fd, error, 2);
		if ((*channel_passaggio).size() > 0)
			(*channel_passaggio).erase((*channel_passaggio).begin());
		return(false) ;
	}
	if(chl.invite_only == true)
	{
		if (chl.whitelist.find(clt.fd) == chl.whitelist.end())
		{
			error = ":server 473 " + clt.nick.string + " " + channel_nombres[i] + " :Cannot join channel (+i)";
			send_msg(clt.fd, error, 2);
			if ((*channel_passaggio).size() > 0)
				(*channel_passaggio).erase((*channel_passaggio).begin());
			return(false) ;
		}
	}
	if(chl.password.exists == true)
	{
		if((*channel_passaggio).size() > 0)
		{
			if((*channel_passaggio)[0] != chl.password.string)
			{
				error = ":server 475 " + clt.nick.string + " " + channel_nombres[i] + " :Cannot join channel (+k)";
				send_msg(clt.fd, error, 2);
				(*channel_passaggio).erase((*channel_passaggio).begin());
				return(false);
			}
			(*channel_passaggio).erase((*channel_passaggio).begin());
		}
		else
		{
			error = ":server 475 " + clt.nick.string + " " + channel_nombres[i] + " :Cannot join channel (+k)";
			send_msg(clt.fd, error, 2);
			return(false) ;
		}
	}
	if ((*channel_passaggio).size() > 0)
		(*channel_passaggio).erase((*channel_passaggio).begin());
	chl.clt_fds.insert(clt.fd);
	clt.channels.insert(channel_nombres[i]);
	chl.clt_counter += 1;
	send_channel_msg(channel_nombres[i], clt, "", "JOIN");
	send_msg(0,"",3);
	join_detail(chl, clt);
	return(true);
}
void Channel::handle_join(std::vector<std::string> split_msg, t_client &clt)
{
	std::string error;
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
		t_channel &chl = this->channels[channel_nombres[i]];
		if ((channel_nombres[i][0] != '#' && channel_nombres[i][0] != '&') ||  std::count(channel_nombres[i].begin(), channel_nombres[i].end(), '#') > 1 || std::count(channel_nombres[i].begin(), channel_nombres[i].end(), '&') > 1)
		{
			error = ":server 476 " + clt.nick.string + " " + channel_nombres[i] + " :Bad Channel Mask";
			send_msg(clt.fd, error,2);
			continue ;	
		}
		if(std::count(channel_nombres[i].begin(), channel_nombres[i].end(), '#') == 1 && std::count(channel_nombres[i].begin(), channel_nombres[i].end(), '&') == 1)
		{
			error = ":server 476 " + clt.nick.string + " " + channel_nombres[i] + " :Bad Channel Mask";
			send_msg(clt.fd, error,2);
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
			}
			else
				chl.password.exists = false;
			chl.clt_counter += 1;
			clt.channels.insert(channel_nombres[i]);
			send_channel_msg(channel_nombres[i],clt,"","JOIN");
			send_msg(0,"",3);
			join_detail(chl, clt);
		}
		else
		{
			if(join_channel(clt,chl,&channel_passaggio,channel_nombres,i) == false)
				continue;
		}
	}
}

void	Channel::handle_part(std::vector<std::string> split_msg, t_client &clt,bool disconnect)
{
	(void)disconnect;
	std::string error;
	if (split_msg.size() < 2)
	{
		error = ":server 461 " + clt.nick.string + " PART :Not enough parameters";
		send_msg(clt.fd, error, 2);
		return ;
	}
	std::vector<std::string> chl_names = split_char(split_msg[1], ',');
	print_container(chl_names);
	for(size_t i = 0; i < chl_names.size(); i++)
	{
		if (clt.channels.find(chl_names[i]) == clt.channels.end())
		{
			error = ":server 442 " + clt.nick.string + " " + chl_names[i] + " :You're not on that channel";
			send_msg(clt.fd, error, 2);
			return ;
		}
		send_channel_msg(chl_names[i], clt, "", split_msg[0]);
		clt.channels.erase(chl_names[i]);
		this->channels[chl_names[i]].clt_counter--;
		this->channels[chl_names[i]].clt_fds.erase(clt.fd);
		this->channels[chl_names[i]].whitelist.erase(clt.fd);
		if (this->check_admin(this->channels[chl_names[i]], clt.fd) == true)
		{
			this->channels[chl_names[i]].admins.erase(clt.fd);
			if (this->channels[chl_names[i]].admins.size() <= 0 && this->channels[chl_names[i]].clt_counter > 0)
			{
				this->channels[chl_names[i]].admins.insert(*this->channels[chl_names[i]].clt_fds.begin());
				t_client *cur_nick = this->client_ptr->get_client(*this->channels[chl_names[i]].clt_fds.begin());
				std::string response = " +o " + cur_nick->nick.string;
				send_channel_msg(this->channels[chl_names[i]].name,clt,response,"MODE");
			}
		}
		if (this->channels[chl_names[i]].clt_counter <= 0)
			this->channels.erase(chl_names[i]);
	}
}

void Channel::handle_privmsg(std::vector<std::string> split_msg, t_client &clt, std::string msg)
{
	std::string error;
	if (split_msg.size() < 2)
	{
		error = ":server 411 " + clt.nick.string + " :No recipient given";
		send_msg(clt.fd, error, 2);
		return ;
	}
	if (split_msg.size() < 3)
	{
		error = ":server 412 " + clt.nick.string + " :No text to send";
		send_msg(clt.fd, error, 2);
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
			error = ":server 401 " + clt.nick.string + " " + inoa[i] + " :No such channel";
			send_msg(clt.fd, error, 2);
			continue;
		}
		if(inoa[i].find("#") == std::string::npos && inoa[i].find("&") == std::string::npos)
		{
			if(this->client_ptr->search_client_list(inoa[i], clt, msg) == false)
			{
				error = ":server 401 " + clt.nick.string + " " + inoa[i] + " :No such channel";
				send_msg(clt.fd, error, 2);
			}
			continue;
		}
		send_channel_msg(inoa[i], clt, msg,"PRIVMSG");
	}
}

void	Channel::send_channel_msg(std::string channel_name, t_client &clt, std::string msg,std::string command)
{
	std::set<int>::iterator fd_it;
	std::string chop;
	bool disconnect = false;
	if(command == "DISCONNECT")
	{
		disconnect = true;
		command = "PART";
	}
	for (fd_it = this->channels[channel_name].clt_fds.begin(); fd_it != this->channels[channel_name].clt_fds.end(); ++fd_it)
	{
		chop = ":";
		if (this->check_admin(this->channels[channel_name], clt.fd) == true && command != "JOIN" && command != "PART") 
			chop += "@";
		std::string response = chop + clt.nick.string + "!" + clt.user.string + "@hostname " + command + " ";
		if(command != "INVITE")
			response += channel_name;
		else
			response += msg + " " + channel_name;
		if(msg.empty() == false && command != "INVITE")
			response += " " + msg;
		response += "\r\n";
		if(*fd_it != clt.fd || command == "JOIN" || (command == "PART" && disconnect == false) || command == "TOPIC" || command == "MODE" || command == "KICK"  || command == "INVITE")
			send(*fd_it,response.c_str(),response.size(),0);
	}
}

void	Channel::handle_kick(std::vector<std::string> split_msg, t_client &clt, std::string cmd)
{
	std::string error;
	if (split_msg.size() < 3)
	{
		error = ":server 461 " + clt.nick.string + " KICK :Not enough parameters";
		send_msg(clt.fd, error, 2);
		return ;
	}
	std::string reason = "";
	if (split_msg.size() > 3 && split_msg[3][0] == ':')
		reason = cmd.substr(cmd.find(" :") + 2);
	else if (split_msg.size() > 3)
		reason = split_msg[3];
	std::vector<std::string> nicknames = split_char(split_msg[2], ',');
	if (this->channels.find(split_msg[1]) == this->channels.end())
	{
		error = ":server 403 " + clt.nick.string + " " + split_msg[1] + ":No such channel";
		send_msg(clt.fd, error, 2);
		return;
	}
	t_channel &chl = this->channels[split_msg[1]];
	if (this->check_admin(chl, clt.fd) == false)
	{
		error = ":server 482 " + clt.nick.string + " " + chl.name + " :You're not channel operator";
		send_msg(clt.fd, error, 2);
		return ;
	}
	for (size_t j = 0; j < nicknames.size(); j++)
	{
		int clt_fd = this->client_ptr->get_client_fd(nicknames[j]);
		t_client *cur_nick = this->client_ptr->get_client(clt_fd);
		if (clt_fd == -1 || cur_nick->channels.find(split_msg[1]) == cur_nick->channels.end())
		{
			error = ":server 442 " + clt.nick.string + " " + split_msg[1] + ":You're not on that channel";
			send_msg(clt.fd, error, 2);
			continue;
		}
		std::string msg;
		if (reason.empty() == true)
			msg = cur_nick->nick.string;
		else
			msg = cur_nick->nick.string + " " + reason;
		send_channel_msg(split_msg[1], clt, msg,"KICK");
		cur_nick->channels.erase(split_msg[1]);
		chl.clt_counter--;
		chl.clt_fds.erase(cur_nick->fd);
		chl.whitelist.erase(cur_nick->fd);
		if (this->check_admin(chl, cur_nick->fd) == true)
			chl.admins.erase(cur_nick->fd);
	}
	if (chl.clt_counter <= 0)
		this->channels.erase(chl.name);
}

void	Channel::handle_topic(std::vector<std::string> split_msg, t_client &clt, std::string cmd)
{
	std::string error;
	if (split_msg.size() < 2)
	{
		error = ":server 461 " + clt.nick.string + " TOPIC :Not enough parameters";
		send_msg(clt.fd, error, 2);
		return ;
	}
	std::string topic;
	if (this->channels.find(split_msg[1]) == this->channels.end())
	{
		error = ":server 403 " + clt.nick.string + " " + split_msg[1] + ":No such channel";
		send_msg(clt.fd, error, 2);
		return;
	}
	t_channel &chl = this->channels[split_msg[1]];
	if (split_msg.size() < 3)
	{
		if(chl.topic.empty())
		{
			error = ":server 331 " + clt.nick.string + " " + chl.name + " :No topic is set";
			send_msg(clt.fd, error, 2);
			return;
		}
		error = ":server 332 " + clt.nick.string + " " + chl.name + " :" + chl.topic;
		send_msg(clt.fd, error, 2);
		return;
	}
	else
	{
		if (split_msg[2][0] == ':')
			topic = cmd.substr(cmd.find(" :") + 2);
		else
			topic = split_msg[2];
	}
	if(chl.name.empty() || chl.clt_fds.find(clt.fd) == chl.clt_fds.end())
	{
		error = ":server 442 " + clt.nick.string + " " + chl.name + " :You're not on that channel";
		send_msg(clt.fd, error, 2);
		return ;
	}
	if (chl.topic_change == true && this->check_admin(chl, clt.fd) == false)
	{
		error = ":server 482 " + clt.nick.string  + " " + chl.name + " :You're not channel operator";
		send_msg(clt.fd, error, 2);
		return ;
	}
	chl.topic = topic;
	send_channel_msg(split_msg[1],clt,topic,"TOPIC");
}

void Channel::mode_check(t_client &clt,t_channel &chl,std::string str)
{
	std::string response = ":IRC 324 " + clt.nick.string + " " + str + " +";
	if(chl.invite_only == true)
		response += "i";
	if(chl.topic_change == true)
		response += "t";
	if(chl.password.exists == true)
		response += "k";
	if(chl.user_limit_bool == true)
		response += "l";
	if(chl.password.exists == true)
		response += " " + chl.password.string;
	if(chl.user_limit_bool == true)
	{
		std::ostringstream ss;
		ss << chl.user_limit;
		std::string s = ss.str();
		response += " " + s;
	}
	response += "\r\n";
	send(clt.fd,response.c_str(),response.size(),0);
}

bool Channel::mode_operator(bool mode,std::vector<std::string> split_msg,t_client &clt,t_channel &chl,size_t *j)
{
	std::string msg;
	
	if(mode == true)
	{
		if(split_msg.size() > (*j) && split_msg[(*j)].empty() == false)
		{
			int clt_fd = this->client_ptr->get_client_fd(split_msg[(*j)]);
			if (clt_fd == -1 || chl.clt_fds.find(clt_fd) == chl.clt_fds.end())
			{
				msg = ":server 401 " + clt.nick.string + " " + split_msg[(*j)] + " :No such nick";
				send_msg(clt.fd, msg, 2);
				(*j)++;
				return (false);
			}
			chl.admins.insert(clt_fd);
			msg = " +o " + split_msg[(*j)];
			send_channel_msg(chl.name,clt,msg,"MODE");
			(*j)++;
		}
		else
			send_server_msg(clt.fd, "Missing nickname to give operator");
	}
	else
	{
		if(split_msg.size() > (*j) && split_msg[(*j)].empty() == false)
		{
			int clt_fd = this->client_ptr->get_client_fd(split_msg[(*j)]);
			if (clt_fd == -1 || chl.clt_fds.find(clt_fd) == chl.clt_fds.end())
			{
				send_server_msg(clt.fd, "Nickname not found");
				(*j)++;
				return (false);
			}
			if(check_admin(chl,clt_fd) == false)
			{
				send_msg(clt.fd,split_msg[(*j)],0);
				send_msg(clt.fd," is not an operator in channel ",1);
				send_msg(clt.fd,chl.name,2);
				(*j)++;
				return (false);
			}
			chl.admins.erase(clt_fd);
			std::string response = " -o " + split_msg[(*j)];
			send_channel_msg(chl.name,clt,response,"MODE");
			(*j)++;
		}
		else
			send_server_msg(clt.fd, "Missing nickname to remove operator");
	}
	return (true);
}

bool Channel::mode_limit(bool mode,std::vector<std::string> split_msg,t_client &clt,t_channel &chl,size_t *j)
{
	std::string msg;

	if(mode == true)
	{
		if(split_msg.size() > (*j) && split_msg[(*j)].empty() == false)
		{
			if(split_msg[(*j)].find_first_not_of("0123456789") != std::string::npos)
			{
				send_server_msg(clt.fd,"Invalid limit amount");
				(*j)++;
				return(false);
			}
			long u_limit = atol(split_msg[(*j)].c_str());
			if(u_limit > std::numeric_limits<int>::max())
			{
				split_msg[(*j)] = "2147483647";
				u_limit = std::numeric_limits<int>::max();
			}
			chl.user_limit = u_limit;
			msg = " +l " + split_msg[(*j)];
			send_channel_msg(chl.name, clt, msg, "MODE");
			(*j)++;
		}
		else
		{
			send_server_msg(clt.fd, "Missing user limit");
			return (false);
		}
	}
	else
		send_channel_msg(chl.name, clt, "-l", "MODE");
	chl.user_limit_bool = mode;
	return(true);
}

void	Channel::handle_mode(std::vector<std::string> split_msg,t_client &clt)
{
	std::string msg;
	bool	mode;
	if (split_msg.size() < 2)
	{
		send_server_msg(clt.fd, "Missing channel name");
		return ;
	}
	if (this->channels.find(split_msg[1]) == this->channels.end())
	{
		msg = ":server 442 " + clt.nick.string + " " + split_msg[1] + " :Not in the channel"; 
		send_msg(clt.fd, msg, 2);
		return;
	}
	t_channel &chl = this->channels[split_msg[1]];
	if (split_msg.size() < 3)
	{
		mode_check(clt,chl,split_msg[1]);
		return ;
	}
	if(split_msg[2][0] != '+' && split_msg[2][0] != '-')
	{
		send_server_msg(clt.fd, "Channel options must start with + (enable) or - (disable)");
		return;
	}
	size_t j = 3;
	for(size_t i = 0;i < split_msg[2].size();i++)
	{
		if (this->check_admin(chl, clt.fd) == false)
		{
			msg = ":server 482 " + clt.nick.string + " " + chl.name + " :You're not channel operator";
			send_msg(clt.fd, msg, 2);
			return ;
		}
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
					send_channel_msg(chl.name,clt,"+i","MODE");
				else
					send_channel_msg(chl.name,clt,"-i","MODE");
			}
			else if(split_msg[2][i] == 't')
			{
				chl.topic_change = mode;
				if (mode == true)
					send_channel_msg(chl.name, clt, "+t","MODE");
				else
					send_channel_msg(chl.name, clt, "-t","MODE");
			}
			else if (split_msg[2][i] == 'k')
			{
				if(mode == true)
				{
					if(j >= split_msg.size() || split_msg[j].empty() == true)
					{
						send_server_msg(clt.fd, "No password given");
						continue;
					}
					chl.password.string = split_msg[j];
					msg = " +k " + split_msg[j];
					send_channel_msg(chl.name, clt, msg, "MODE");
					j++;
				}
				else
					send_channel_msg(chl.name, clt, "-k", "MODE");
				chl.password.exists = mode;
			}
			else if (split_msg[2][i] == 'o')
			{
				if(mode_operator(mode,split_msg,clt,chl,&j) == false)
					continue;
			}
			else if (split_msg[2][i] == 'l')
			{
				if(mode_limit(mode,split_msg,clt,chl,&j) == false)
					continue;
			}
			else
				send_server_msg(clt.fd, "Invalid channel mode");
		}
	}
}

void	Channel::handle_invite(std::vector<std::string> split_msg, t_client &clt)
{
	std::string error;
	if (split_msg.size() < 3)
	{
		error = ":server 461 " + clt.nick.string + " INVITE :Not enough parameters";
		send_msg(clt.fd, error, 2);
		return ;
	}
	// if (this->channels.find(split_msg[2]) == this->channels.end())
	// {
	// 	error = ":server 403 " + clt.nick.string + " " + split_msg[1] + ":No such channel";
	// 	send_msg(clt.fd, error, 2);
	// 	return;
	// }
	if (clt.channels.find(split_msg[2]) == clt.channels.end())
	{
		error = ":server 442 " + clt.nick.string + " " + split_msg[1] + " :You're not on that channel";
		send_msg(clt.fd,error,2);
		return;
	}
	t_channel &chl = this->channels[split_msg[2]];
	int cur_fd = this->client_ptr->get_client_fd(split_msg[1]);
	if (cur_fd == -1)
	{
		error = ":server 401 " + clt.nick.string + " " + split_msg[1] + " :No such nick";
		send_msg(clt.fd,error,2);
		return;
	}
	if(chl.clt_fds.find(cur_fd) != chl.clt_fds.end())
	{
		error = ":server 443 " + clt.nick.string + " " + split_msg[1] + " :Is already on channel";
		send_msg(clt.fd,error,2);
		return;
	}
	if (check_admin(chl, clt.fd) == false)
	{
		error = ":server 482 " + clt.nick.string + " " + chl.name + " :You're not channel operator";
		send_msg(clt.fd, error, 2);
		return;
	}
	if(chl.invite_only == true)
	{
		t_client *cur_clt = this->client_ptr->get_client(cur_fd);
		cur_clt->invitations.insert(chl.name);
		chl.whitelist.insert(cur_fd);
	}
	send_channel_msg(chl.name, clt, split_msg[1], "INVITE");
	std::string msg = "You have been invited to the channel " + chl.name; 
	send_msg(cur_fd,msg,2);
}

void Channel::handle_who(std::vector<std::string> split_msg, t_client &clt)
{
	std::string error;
	if (split_msg.size() < 2)
	{
		send_server_msg(clt.fd, "Missing channel name");
		return ;
	}
	if (this->channels.find(split_msg[1]) == this->channels.end())
	{
		error = ":server 441 " + clt.nick.string + " " + split_msg[1] + ":Not in channel";
		send_msg(clt.fd, error, 2);
		return;
	}
	t_channel &chl = this->channels[split_msg[1]];
	std::set<int>::iterator it = chl.clt_fds.begin();
	for(; it != chl.clt_fds.end(); it++)
	{
		t_client *cur_nick = this->client_ptr->get_client(*it);
		std::string response = ":server 352 " + clt.nick.string + " " + chl.name + " " + cur_nick->user.string +  " hostname IRC " + cur_nick->nick.string + " H";
		if (check_admin(chl, *it) == true)
			response += "@";
		response += " :0 " + cur_nick->real_name;
		send_msg(clt.fd, response, 2);
	}
	std::string end = ":server 315 " + clt.nick.string + " " + chl.name + " :End of /WHO list.";
	send_msg(clt.fd, end, 2);
}

void Channel::channel_commands(std::vector<std::string> split_msg, t_client &clt, std::string command)
{
	if (split_msg[0] == "PART")
		this->handle_part(split_msg, clt,false);
	else if(split_msg[0] == "PRIVMSG")
		this->handle_privmsg(split_msg, clt, command);
	else if (split_msg[0] == "KICK")
		this->handle_kick(split_msg, clt, command);
	else if (split_msg[0] == "INVITE")
		this->handle_invite(split_msg, clt);
	else if (split_msg[0] == "TOPIC")
		this->handle_topic(split_msg, clt, command);
	else if (split_msg[0] == "MODE")
		this->handle_mode(split_msg, clt);
	else if (split_msg[0] == "WHO")
		this->handle_who(split_msg, clt);
	else
		send_server_msg(clt.fd, "Unknown command");
}

void	Channel::disconnect_channels(t_client &clt, int epfd)
{
	std::set<std::string>::iterator c_it;
	std::set<std::string>::iterator inv_it;
	std::vector <std::string> vec;
	std::string	channel;
	std::cout << "ASDASDSADSADISADASDASDA\n";
	std::cout << "INV size " << clt.invitations.size() << std::endl;
	vec.push_back("DISCONNECT");
	for (c_it = clt.channels.begin(); c_it != clt.channels.end();c_it++)
		channel += *c_it + ",";
	vec.push_back(channel);
	for (inv_it = clt.invitations.begin(); inv_it != clt.invitations.end(); inv_it++)
	{
		if(this->channels.find(*inv_it) == this->channels.end())
			continue;
		t_channel &chl = this->channels[*inv_it];
		std::cout << *inv_it << std::endl;
		chl.whitelist.erase(clt.fd);
	}
	this->handle_part(vec, clt,true);
	std::cout << "USER DISCONNECTED" << std::endl;
	epoll_ctl(epfd, EPOLL_CTL_DEL, clt.fd, NULL);
	this->client_ptr->remove_client(clt.fd);
}

bool	Channel::check_admin(t_channel &chl,size_t clt_fd)
{
	if (chl.admins.find(clt_fd) == chl.admins.end())
		return (false);
	return (true);
}

std::map<std::string,t_channel> Channel::get_channels()
{
	return(this->channels);
} 