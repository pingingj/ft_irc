/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: finn <finn@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/30 16:27:19 by dgarcez-          #+#    #+#             */
/*   Updated: 2026/08/25 17:11:53 by finn             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/ft_irc.hpp"

std::vector<std::string> split_string(std::string s, std::string delimiter)
{
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos)
	{
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    // res.push_back (s.substr (pos_start));
    return res;
}

std::vector<std::string> split_char(const std::string &s, char delim)
{
    std::vector<std::string> result;
    std::stringstream ss (s);
    std::string item;

    while (getline (ss, item, delim)) 
	{
		if(item.empty())
			continue;
        result.push_back (item);
    }
    return result;
}

bool	str_isalnum(std::string &str)
{
	for(size_t i = 0; i < str.size(); i++)
	{
		if (std::isalnum(str[i]) == false)
			return (false);
	}
	return (true);
}

void	send_msg(int fd, std::string msg, int flag)
{
	static std::string res;
	if(flag == 0)
		res += "Server: " + msg;
	if (flag == 1)
		res += msg;
	if (flag == 2)
	{
		res += msg + "\r\n";
		send(fd, res.c_str(), res.size(), 0);
		res.clear();
	}
}

void	send_msg_hex(int fd, std::string response)
{
	response += "\r\n";
	send(fd, response.c_str(), response.size(), 0);
}

void	send_server_msg(int fd, std::string msg)
{
	std::string res = msg + "\r\n";
	std::string sys = "Server: " + res;
	send(fd, sys.c_str(), sys.size(), 0);
}