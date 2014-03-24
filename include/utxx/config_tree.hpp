//----------------------------------------------------------------------------
/// \file config_tree.hpp
/// \author Serge Aleynikov
//----------------------------------------------------------------------------
/// Defines config_tree, config_path, and config_error classes for
/// ease of configuration management.
//----------------------------------------------------------------------------
// Copyright (C) 2010 Serge Aleynikov <saleyn@gmail.com>
// Created: 2010-06-21
//----------------------------------------------------------------------------
/*
***** BEGIN LICENSE BLOCK *****

This file is a part of utxx open-source project.

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

#ifndef _UTXX_CONFIG_TREE_HPP_
#define _UTXX_CONFIG_TREE_HPP_

#include <utxx/variant_tree.hpp>

namespace utxx {

    /// Container for storing configuration data.
    typedef variant_tree            config_tree;
    typedef tree_path               config_path;

    typedef variant_tree_error      config_error;
    typedef variant_tree_bad_data   config_bad_data;
    typedef variant_tree_bad_path   config_bad_path;

} // namespace utxx

#endif // _UTXX_CONFIG_TREE_HPP_

