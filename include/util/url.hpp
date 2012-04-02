//----------------------------------------------------------------------------
/// \file  path.hpp
//----------------------------------------------------------------------------
/// \brief Collection of general purpose functions for path manipulation.
//----------------------------------------------------------------------------
// Copyright (c) 2010 Serge Aleynikov <saleyn@gmail.com>
// Created: 2010-05-06
//----------------------------------------------------------------------------
/*
***** BEGIN LICENSE BLOCK *****

This file is may be included in several open-source projects.

Copyright (C) 2010 Serge Aleynikov <saleyn@gmail.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

***** END LICENSE BLOCK *****
*/
#ifndef _UTIL_URL_HPP_
#define _UTIL_URL_HPP_

#include <string>

namespace util {

    /// Types of connections supported by this class.
    enum connection_type { UNDEFINED, TCP, UDP, UDS };

    namespace detail {
        /// Convert connection type to string.
        const char* connection_type_to_str(connection_type a_type);
    }

    /// Server ddress information holder
    struct addr_info {
        connection_type proto;
        std::string addr;
        std::string port;
        std::string path;

        std::string to_string() const;
    };

    /// URL Parsing
    /// @param a_url string in the form <tt>tcp://host:port</tt>
    bool parse_url(const std::string& a_url, addr_info& a_info);

} // namespace util

#endif // _UTIL_URL_HPP_

