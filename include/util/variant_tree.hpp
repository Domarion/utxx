//----------------------------------------------------------------------------
/// \file  variant_tree.hpp
//----------------------------------------------------------------------------
/// \brief This file contains a tree class that can hold variant values.
//----------------------------------------------------------------------------
// Author:  Serge Aleynikov
// Created: 2010-07-10
//----------------------------------------------------------------------------
/*
***** BEGIN LICENSE BLOCK *****

This file may be included in various open-source projects.

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

#ifndef _UTIL_VARIANT_TREE_HPP_
#define _UTIL_VARIANT_TREE_HPP_

#include <util/variant.hpp>
#include <util/typeinfo.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/throw_exception.hpp>

namespace boost {
namespace property_tree {
    // Custom translator that works with util::variant instead of std::string.
    // This translator is used to read/write values from files.
    template <>
    struct translator_between<util::variant, std::string>
    {
        typedef translator_between<util::variant, std::string> type;
        typedef std::string     external_type;
        typedef util::variant   internal_type;

        boost::optional<external_type> get_value(const internal_type& value) const {
            return boost::optional<external_type>(
                value.type() == internal_type::TYPE_NULL ? "" : value.to_string());
        }

        boost::optional<internal_type> put_value(const external_type& value) const {
            try {
                long n = lexical_cast<long>(value);
                for(external_type::const_iterator it=value.begin(), end=value.end(); it!=end; ++it)
                    if (*it < '0' || *it > '9')
                        throw false;
                return boost::optional<internal_type>(n);
            } catch (...) {}
            try {
                double n = lexical_cast<double>(value);
                return boost::optional<internal_type>(n);
            } catch (...) {}
            if (value == "true" || value == "false")
                return boost::optional<internal_type>(value[0] == 't');
            return boost::optional<internal_type>(value);
        }
    };

} // namespace property_tree
} // namespace boost

namespace util {

namespace detail {

    // Custom translator that works with variant instead of std::string
    // This translator is used to get/put values through explicit get/put calls.
    template <class Ext>
    struct variant_translator
    {
        typedef Ext external_type;
        typedef variant internal_type;

        /*
        typedef boost::mpl::joint_view<variant::int_types,
                boost::mpl::vector<bool,double>
            > valid_non_string_types;
        */
        typedef variant::valid_types valid_types;

        external_type get_value(const internal_type& value) const {
            return value.get<external_type>();
        }

        template<typename T>
        typename boost::disable_if<
            boost::is_same<
                boost::mpl::end<valid_types>::type,
                boost::mpl::find<variant::valid_types, T>
            >, 
            internal_type>::type
        put_value(T value) const {
            return variant(value);
        }
    };

    typedef boost::property_tree::basic_ptree<
            std::string,                    // Key type
            variant,                        // Data type
            std::less<std::string>          // Key comparison
    > basic_variant_tree;

} // namespace detail

class variant_tree : public detail::basic_variant_tree
{
    typedef detail::basic_variant_tree base;
    typedef variant_tree self_type;
public:
    typedef boost::property_tree::ptree_bad_path bad_path;

    variant_tree() {}

    template <typename Source>
    static void read_info(Source& src, variant_tree& tree) {
        boost::property_tree::info_parser::read_info(src, tree);
    }

    template <typename Target>
    static void write_info(Target& tar, variant_tree& tree) {
        boost::property_tree::info_parser::write_info(tar, tree);
    }

    template <typename Target, typename Settings>
    static void write_info(Target& tar, variant_tree& tree, const Settings& tt) {
        boost::property_tree::info_parser::write_info(tar, tree, tt);
    }

    template <class T>
    T get_value() const {
        using boost::throw_exception;

        if(boost::optional<T> o =
                base::get_value_optional<T>(detail::variant_translator<T>())) {
            return *o;
        }
        BOOST_PROPERTY_TREE_THROW(boost::property_tree::ptree_bad_data(
            std::string("conversion of data to type \"") +
            typeid(T).name() + "\" failed", base::data()));
    }

    template <class T>
    T get_value(const T& default_value) const {
        return base::get_value(default_value, detail::variant_translator<T>());
    }

    std::string get_value(const char* default_value) const {
        return base::get_value(std::string(default_value),
            detail::variant_translator<std::string>());
    }

    template <class T>
    boost::optional<T> get_value_optional() const {
        return base::get_value(detail::variant_translator<T>());
    }

    template <class T>
    void put_value(const T& value) {
        base::put_value(value, detail::variant_translator<T>());
    }

    template <class T>
    T get(const path_type& path) const {
        try {
            return base::get_child(path).BOOST_NESTED_TEMPLATE
                get_value<T>(detail::variant_translator<T>());
        } catch (boost::bad_get& e) {
            std::stringstream s;
            s << "Cannot convert value to type '" << type_to_string<T>() << "'";
            throw bad_path(s.str(), path);
        }
    }

    template <class T>
    T get(const path_type& path, const T& default_value) const {
        try {
            return base::get(path, default_value, detail::variant_translator<T>());
        } catch (boost::bad_get& e) {
            throw bad_path("Wrong or missing value type", path);
        }
    }

    std::string get(const path_type& path, const char* default_value) const {
        return base::get(path, std::string(default_value),
            detail::variant_translator<std::string>());
    }

    template <class T>
    boost::optional<T> get_optional(const path_type& path) const {
        return base::get_optional(path, detail::variant_translator<T>());
    }

    template <class T>
    void put(const path_type& path, const T& value) {
        base::put(path, value, detail::variant_translator<T>());
    }

    template <class T>
    self_type& add(const path_type& path, const T& value) {
        return static_cast<self_type&>(
            base::add(path, value, detail::variant_translator<T>()));
    }

    void swap(variant_tree& rhs) {
        base::swap(rhs);
    }
    void swap(boost::property_tree::basic_ptree<
        std::string, variant, std::less<std::string> >& rhs) {
        base::swap(rhs);
    }

    self_type &get_child(const path_type &path) {
        return static_cast<self_type&>(base::get_child(path));
    }

    /** Get the child at the given path, or throw @c ptree_bad_path. */
    const self_type &get_child(const path_type &path) const {
        return static_cast<const self_type&>(base::get_child(path));
    }

    /** Get the child at the given path, or return @p default_value. */
    self_type &get_child(const path_type &path, self_type &default_value) {
        return static_cast<self_type&>(base::get_child(path, default_value));
    }

    /** Get the child at the given path, or return @p default_value. */
    const self_type &get_child(const path_type &path,
                               const self_type &default_value) const {
        return static_cast<const self_type&>(base::get_child(path, default_value));
    }

    /** Get the child at the given path, or return boost::null. */
    boost::optional<self_type &> get_child_optional(const path_type &path) {
        boost::optional<base&> o = base::get_child_optional(path);
        if (!o)
            return boost::optional<self_type&>();
        return boost::optional<self_type&>(static_cast<self_type&>(*o));
    }

    /** Get the child at the given path, or return boost::null. */
    boost::optional<const self_type &>
    get_child_optional(const path_type &path) const {
        boost::optional<const base&> o = base::get_child_optional(path);
        if (!o)
            return boost::optional<const self_type&>();
        return boost::optional<const self_type&>(static_cast<const self_type&>(*o));
    }

    self_type &put_child(const path_type &path, const self_type &value) {
        return static_cast<self_type&>(base::put_child(path, value));
    }
};

} // namespace util

#endif // _UTIL_VARIANT_TREE_HPP_
