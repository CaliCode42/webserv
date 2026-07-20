/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/23 10:52:18 by tcali             #+#    #+#             */
/*   Updated: 2026/07/20 13:54:51 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <sstream>

template <typename T>
std::string turnIntoString(T value)
{
    std::stringstream ss;
    ss << value;
    return (ss.str());
}

#endif