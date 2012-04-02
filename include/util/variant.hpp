//----------------------------------------------------------------------------
/// \file  variant.hpp
//----------------------------------------------------------------------------
/// \brief This file contains a variant class that represents a subtype of
/// boost::variant that can hold integer/bool/double/string values.
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


#ifndef _UTIL_VARIANT_HPP_
#define _UTIL_VARIANT_HPP_

#include <boost/variant.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/find.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/joint_view.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/static_assert.hpp>
#include <boost/cstdint.hpp>
#include <stdexcept>
#include <string.h>
#include <stdio.h>

namespace util {

struct null {};

class variant: public boost::variant<null, bool, long, double, std::string>
{
    typedef boost::variant    <null, bool, long, double, std::string> base;
    typedef boost::mpl::vector<null, bool, long, double, std::string> internal_types;

    struct string_visitor: public boost::static_visitor<std::string> {
        std::string operator () (null v) const { return "<NULL>"; }
        std::string operator () (bool v) const { return v ? "true" : "false"; }
        std::string operator () (long v) const {
            std::stringstream s; s << v; return s.str();
        }
        std::string operator () (double v) const {
            char buf[128];
            snprintf(buf, sizeof(buf)-1, "%f", v);
            // Remove trailing zeros.
            for (char* p = buf+strlen(buf); p > buf && *(p-1) != '.'; p--)
                if (*p == '0')
                    *p = '\0';
            return buf;
        }
        std::string operator () (const std::string& v) const { return v; }
    };

    template <typename T>
    T       get(T*)       const { return boost::get<T>(*this); }
    variant get(variant*) const { return variant(*this); }
public:
    typedef boost::mpl::vector<
        int,int16_t,long,uint16_t,uint32_t,uint64_t> int_types;

    typedef boost::mpl::joint_view<
        int_types,
        boost::mpl::vector<null,bool,double,std::string>
    > valid_types;

    typedef boost::mpl::joint_view<
        int_types,
        boost::mpl::vector<null,bool,double,std::string,variant>
    > valid_get_types;

    enum value_type {
          TYPE_NULL
        , TYPE_BOOL
        , TYPE_INT
        , TYPE_DOUBLE
        , TYPE_STRING
    };

    variant() : base(null()) {}
    explicit variant(bool           a) : base(a)        {}
    explicit variant(int16_t        a) : base((long)a)  {}
    explicit variant(int            a) : base((long)a)  {}
    explicit variant(long           a) : base(a)        {}
    explicit variant(uint16_t       a) : base((long)a)  {}
    explicit variant(uint32_t       a) : base((long)a)  {}
    explicit variant(uint64_t       a) : base((long)a)  {}
    explicit variant(double         a) : base(a)        {}
    explicit variant(const char*    a) : base(std::string(a)) {}
    explicit variant(const std::string& a) : base(a) {}
    variant(value_type v, const std::string& a) { from_string(v, a); }

    void operator= (int16_t     a)  { *(base*)this = (long)a; }
    void operator= (int         a)  { *(base*)this = (long)a; }
    void operator= (int64_t     a)  { *(base*)this = (long)a; }
    void operator= (uint16_t    a)  { *(base*)this = (long)a; }
    void operator= (uint32_t    a)  { *(base*)this = (long)a; }
    void operator= (uint64_t    a)  { *(base*)this = (long)a; }
    void operator= (const char* a)  { *(base*)this = std::string(a); }

    template <typename T>
    void operator= (T a) {
        typedef typename boost::mpl::find<valid_types, T>::type type_iter;
        BOOST_STATIC_ASSERT(
            (!boost::is_same<
                boost::mpl::end<valid_types>::type,
                type_iter >::value)
        );
        *(base*)this = a;
    }

    value_type  type()     const { return static_cast<value_type>(which()); }
    const char* type_str() const {
        static const char* s_types[] = { "null", "bool", "int", "double", "string" };
        return s_types[type()];
    }

    /// Set value to null.
    void clear() { *(base*)this = null(); }

    /// Returns true if the value is null.
    bool is_null() const { return type() == TYPE_NULL; }

    bool                to_bool()   const { return boost::get<bool>(*this); }
    long                to_int()    const { return boost::get<long>(*this); }
    long                to_float( ) const { return boost::get<double>(*this); }
    const std::string&  to_str()    const { return boost::get<std::string>(*this); }
    const char*         c_str()     const { return boost::get<std::string>(
                                                    *this).c_str(); }

    /// Convert value to string.
    std::string to_string() const {
        variant::string_visitor v;
        return boost::apply_visitor(v, *this);
    }

    template <typename T>
    T get() const {
        // Make sure given type is a valid variant type.
        // Note to user: an error raised here indicates that
        // there's a compile-time attempt to convert a variant to 
        // unsupported type.
        typedef typename boost::mpl::find<valid_get_types, T>::type valid_iter;
        BOOST_STATIC_ASSERT(
            (!boost::is_same<boost::mpl::end<valid_get_types>::type, valid_iter>::value));

        // For integers - cast them to long type when fetching from the variant.
        typedef typename boost::mpl::find<int_types, T>::type int_iter;
        typedef typename boost::mpl::if_<
                boost::is_same<boost::mpl::end<int_types>::type, int_iter>,
                T,
                long
            >::type type;
        type* dummy(NULL);
        return get(dummy);
    }

    void from_string(value_type v, const std::string& a) {
        switch (v) {
            case TYPE_NULL:     *this = null(); break;
            case TYPE_BOOL:     *this = a == "true" || a == "yes";      break;
            case TYPE_INT:      *this = boost::lexical_cast<long>(a);   break;
            case TYPE_DOUBLE:   *this = boost::lexical_cast<double>(a); break;
            case TYPE_STRING:   *this = a;
            default:            throw_type_error(type());
        }
    }

    bool operator== (const variant& rhs) const {
        if (type() == TYPE_NULL || rhs.type() == TYPE_NULL || type() != rhs.type())
            return false;
        switch (type()) {
            case TYPE_BOOL:     return to_bool()   == rhs.to_bool();
            case TYPE_INT:      return to_int()    == rhs.to_int();
            case TYPE_DOUBLE:   return to_float()  == rhs.to_float();
            case TYPE_STRING:   return to_string() == rhs.to_string();
            default:            throw_type_error(type());
                                return false; // just to pease the compiler
        }
    }

    bool operator< (const variant& rhs) const {
        if (type() == TYPE_NULL || rhs.type() == TYPE_NULL) return false;
        if (type() < rhs.type()) return true;
        if (type() > rhs.type()) return false;
        switch (type()) {
            case TYPE_BOOL:     return to_bool()    < rhs.to_bool();
            case TYPE_INT:      return to_int()     < rhs.to_int();
            case TYPE_DOUBLE:   return to_float()   < rhs.to_float();
            case TYPE_STRING:   return to_string()  < rhs.to_string();
            default:            throw_type_error(type());
                                return false; // just to pease the compiler
        }
    }

    bool operator> (const variant& rhs) const {
        if (type() == TYPE_NULL || rhs.type() == TYPE_NULL) return false;
        if (type() > rhs.type()) return true;
        if (type() < rhs.type()) return false;
        switch (type()) {
            case TYPE_BOOL:     return to_bool()    > rhs.to_bool();
            case TYPE_INT:      return to_int()     > rhs.to_int();
            case TYPE_DOUBLE:   return to_float()   > rhs.to_float();
            case TYPE_STRING:   return to_string()  > rhs.to_string();
            default:            throw_type_error(type());
                                return false; // just to pease the compiler
        }
    }

private:
    static void throw_type_error(value_type v) {
        std::stringstream s;
        s << "Unknown type " << v;
        throw std::runtime_error(s.str());
    }
};

static inline std::ostream& operator<< (std::ostream& out, const variant& a) {
    return out << a.to_string();
}

} // namespace util

#endif // _UTIL_VARIANT_HPP_
