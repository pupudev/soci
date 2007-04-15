//
// Copyright (C) 2004-2007 Maciej Sobczak, Stephen Hutton
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef SOCI_TYPE_CONVERSION_H_INCLUDED
#define SOCI_TYPE_CONVERSION_H_INCLUDED

#include "into-type.h"
#include "use-type.h"

#include <vector>

namespace soci
{

// default traits class type_conversion, acts as pass through for Row::get()
// when no actual conversion is needed.
template<typename T> 
struct type_conversion
{
    typedef T base_type;
    static T from(T const &t) { return t; }
};

namespace details
{

// this class is used to ensure correct order of construction
// of into_type and use_type elements that use type_conversion

template <typename T>
struct base_value_holder
{
    typename type_conversion<T>::base_type val_;
};

// Automatically create into_type from a type_conversion

template <typename T>
class conversion_into_type
    : private base_value_holder<T>,
      public into_type<typename type_conversion<T>::base_type>
{
public:
    typedef typename type_conversion<T>::base_type BASE_TYPE;

    conversion_into_type(T &value)
        : into_type<BASE_TYPE>(details::base_value_holder<T>::val_),
          value_(value) {}
    conversion_into_type(T &value, eIndicator &ind)
        : into_type<BASE_TYPE>(details::base_value_holder<T>::val_, ind),
          value_(value) {}

private:
    void convert_from()
    {
        value_ = type_conversion<T>::from(details::base_value_holder<T>::val_);
    }

    T &value_;
};

// Automatically create into_type for conversion with boost::optional

template <typename T>
class conversion_into_type<boost::optional<T> >
    : private base_value_holder<T>,
      public into_type<typename type_conversion<T>::base_type>
{
public:
    typedef typename type_conversion<T>::base_type BASE_TYPE;

    conversion_into_type(boost::optional<T> &opt)
        : into_type<BASE_TYPE>(details::base_value_holder<T>::val_, ind_),
          opt_(opt) {}

private:

    // convert_from is called here automatically
    // by post_fetch from the into_type base class,
    // and the indicator is already properly set
    void convert_from()
    {
        if (ind_ == eOK)
        {
            opt_ = type_conversion<T>::from(
                details::base_value_holder<T>::val_);
        }
        else
        {
            opt_.reset();
        }
    }

    boost::optional<T> &opt_;
    eIndicator ind_;
};

// Automatically create use_type from a type_conversion

template <typename T>
class conversion_use_type
    : private details::base_value_holder<T>,
      public use_type<typename type_conversion<T>::base_type>
{
public:
    typedef typename type_conversion<T>::base_type BASE_TYPE;

    conversion_use_type(T &value, std::string const &name = std::string())
        : use_type<BASE_TYPE>(details::base_value_holder<T>::val_, name),
          value_(value) {}
    conversion_use_type(T &value, eIndicator &ind, std::string const &name 
            = std::string())
        : use_type<BASE_TYPE>(details::base_value_holder<T>::val_, ind, name),
          value_(value) {}

    void convert_from()
    {
        value_ = type_conversion<T>::from(details::base_value_holder<T>::val_);
    }
    void convert_to()
    {
        details::base_value_holder<T>::val_ = type_conversion<T>::to(value_);
    }

private:
    T &value_;
};

// Automatically create use_type from a type_conversion with boost::optional

template <typename T>
class conversion_use_type<boost::optional<T> >
    : private details::base_value_holder<T>,
      public use_type<typename type_conversion<T>::base_type>
{
public:
    typedef typename type_conversion<T>::base_type BASE_TYPE;

    conversion_use_type(boost::optional<T> &opt,
        std::string const &name = std::string())
        : use_type<BASE_TYPE>(details::base_value_holder<T>::val_, ind_, name),
          opt_(opt) {}

private:
    void convert_from()
    {
        if (ind_ == eOK)
        {
            opt_ = type_conversion<T>::from(
                details::base_value_holder<T>::val_);
        }
        else
        {
            opt_.reset();
        }
    }

    void convert_to()
    {
        if (opt_.is_initialized())
        {
            details::base_value_holder<T>::val_ = type_conversion<T>::to(opt_);
        }
        else
        {
            ind_ = eNull;
        }
    }

    boost::optional<T> &opt_;
    eIndicator ind_;
};

// this class is used to ensure correct order of construction
// of vector based into_type and use_type elements that use type_conversion

template <typename T>
struct base_vector_holder
{
    base_vector_holder(std::size_t sz = 0) : vec_(sz) {}
    std::vector<typename type_conversion<T>::base_type> vec_;
};

// Automatically create a std::vector based into_type from a type_conversion

template <typename T>
class conversion_into_type<std::vector<T> >
    : private details::base_vector_holder<T>,
      public into_type<std::vector<typename type_conversion<T>::base_type> >
{
public:
    typedef typename std::vector<typename type_conversion<T>::base_type>
        BASE_TYPE;

    conversion_into_type(std::vector<T> &value)
        : details::base_vector_holder<T>(value.size()),
          into_type<BASE_TYPE>(details::base_vector_holder<T>::vec_),
          value_(value) {}

    conversion_into_type(std::vector<T> &value, std::vector<eIndicator> &ind)
        : details::base_vector_holder<T>(value.size()),
          into_type<BASE_TYPE>(details::base_vector_holder<T>::vec_, ind),
          value_(value) {}

    virtual std::size_t size() const
    {
        return details::base_vector_holder<T>::vec_.size();
    }
    virtual void resize(std::size_t sz)
    {
        value_.resize(sz);
        details::base_vector_holder<T>::vec_.resize(sz);
    }

private:
    void convert_from()
    {
        std::size_t const sz = details::base_vector_holder<T>::vec_.size();

        for (std::size_t i = 0; i != sz; ++i)
        {
            value_[i] = type_conversion<T>::from(
                details::base_vector_holder<T>::vec_[i]);
        }
    }

    std::vector<T> &value_;
};


// Automatically create a std::vector based use_type from a type_conversion
template <typename T>
class conversion_use_type<std::vector<T> >
     : private details::base_vector_holder<T>,
       public use_type<std::vector<typename type_conversion<T>::base_type> >
{
public:
    typedef typename std::vector<typename type_conversion<T>::base_type>
        BASE_TYPE;

    conversion_use_type(std::vector<T> &value)
        : details::base_vector_holder<T>(value.size()),
          use_type<BASE_TYPE>(details::base_vector_holder<T>::vec_),
          value_(value) {}

    conversion_use_type(std::vector<T> &value, std::vector<eIndicator> const &ind,
        std::string const &name=std::string())
        : details::base_vector_holder<T>(value.size()),
          use_type<BASE_TYPE>(details::base_vector_holder<T>::vec_, ind, name),
          value_(value) {}

private:
    void convert_from()
    {
        std::size_t const sz = details::base_vector_holder<T>::vec_.size();
        for (std::size_t i = 0; i != sz; ++i)
        {
            value_[i] = type_conversion<T>::from(
                details::base_vector_holder<T>::vec_[i]);
        }
    }
    void convert_to()
    {
        std::size_t const sz = value_.size();
        for (std::size_t i = 0; i != sz; ++i)
        {
            details::base_vector_holder<T>::vec_[i]
                = type_conversion<T>::to(value_[i]);
        }
    }

    std::vector<T> &value_;
};

template <typename T>
into_type_ptr do_into(T &t, user_type_tag)
{
    return into_type_ptr(new conversion_into_type<T>(t));
}

template <typename T>
use_type_ptr do_use(T &t, std::string const &name, user_type_tag)
{
    return use_type_ptr(new conversion_use_type<T>(t, name));
}

// type_conversion specializations must use a stock type as the base_type.
// Each such specialization automatically creates a use_type and an into_type.
// This code is commented out, since it causes problems in those environments
// where std::time_t is an alias to int.
// 
// template<>
// struct type_conversion<std::time_t>
// {
//     typedef std::tm base_type;
//     static std::time_t from(std::tm& t) { return mktime(&t); }
//     static std::tm to(std::time_t& t) { return *localtime(&t); }
// };

} // namespace details
} // namespace soci

#endif
