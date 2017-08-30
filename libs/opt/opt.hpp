/**
 * The MIT License
 *
 * Copyright (c) 2017 Fabio Massaioli
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

#include <algorithm>
#include <array>
#include <complex>
#include <functional>
#include <iostream>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

#if (__cplusplus >= 201703L)
#include <string_view>
#else
#include <experimental/string_view>
#endif

namespace opt
{

#if (__cplusplus >= 201703L)
using StringView = std::string_view;
#else
using StringView = std::experimental::string_view;
#endif

StringView trim(StringView s);
std::string to_lower(StringView s);


class OptionBase {
public:
    virtual ~OptionBase();

    StringView key() const {
        return key_;
    }

    StringView placeholder() const {
        return placeholder_;
    }

    bool required() const {
        return required_;
    }

    bool is_set() const {
        return set_;
    }

    virtual bool parse(StringView arg, std::ostream& err = std::cerr) = 0;

    std::ostream& error(std::ostream& err) const {
        return err << "error: " << key() << ": ";
    }

protected:
    OptionBase(StringView k, bool r)
        : required_(r), key_(k), placeholder_()
        {}

    OptionBase(StringView k, StringView p, bool r)
        : required_(r), key_(k), placeholder_(p)
        {}

    void placeholder(StringView p) {
        placeholder_ = p;
    }

    void set() {
        set_ = true;
    }

    void reset() {
        set_ = false;
    }

private:
    bool set_ = false, required_;
    StringView key_, placeholder_;
};


bool parse(std::initializer_list<std::reference_wrapper<OptionBase>> opts,
           std::initializer_list<std::reference_wrapper<OptionBase>> kwopts,
           char const* const* first, char const* const* last,
           std::ostream& err = std::cerr);

bool parse(std::initializer_list<std::reference_wrapper<OptionBase>> opts,
           std::initializer_list<std::reference_wrapper<OptionBase>> kwopts,
           std::vector<StringView>& ignored,
           char const* const* first, char const* const* last,
           std::ostream& err = std::cerr);

void usage(StringView program,
           std::initializer_list<std::reference_wrapper<OptionBase>> opts,
           std::initializer_list<std::reference_wrapper<OptionBase>> kwopts,
           std::ostream& out = std::cerr);


template<typename T>
constexpr char type_placeholder[] = "VALUE";

template<>
constexpr char type_placeholder<bool>[] = "(true|1|false|0)";

template<>
constexpr char type_placeholder<StringView>[] = "STRING";

template<>
constexpr char type_placeholder<std::string>[] = "STRING";

template<>
constexpr char type_placeholder<std::intmax_t>[] = "INT";

template<>
constexpr char type_placeholder<std::uintmax_t>[] = "UINT";

template<>
constexpr char type_placeholder<float>[] = "REAL";

template<>
constexpr char type_placeholder<double>[] = "REAL";


enum RequiredTag {
    Required
};

struct Placeholder {
    Placeholder(StringView s) : str(s) {}

    StringView str;
};


template<typename T, bool = std::is_enum<T>::value>
class Option : public OptionBase {
public:
    using value_type = T;

    Option(StringView key, value_type const& value = value_type())
        : OptionBase(key, type_placeholder<T>, false), value_(value)
        {}

    Option(StringView key, RequiredTag, value_type const& value = value_type())
        : OptionBase(key, type_placeholder<T>, true), value_(value)
        {}

    Option(StringView key, Placeholder p,
           value_type const& value = value_type())
        : OptionBase(key, p.str, false), value_(value)
        {}

    Option(StringView key, Placeholder p, RequiredTag,
           value_type const& value = value_type())
        : OptionBase(key, p.str, true), value_(value)
        {}

    value_type const& get() const {
        return value_;
    }

    operator value_type const&() const {
        return value_;
    }

    bool parse(StringView arg, std::ostream& err = std::cerr) override final;

private:
    template<typename, bool>
    friend class Option;

    value_type value_;
};

template<>
bool Option<bool>::parse(StringView, std::ostream&);

template<>
bool Option<StringView>::parse(StringView, std::ostream&);

template<>
bool Option<std::string>::parse(StringView, std::ostream&);

template<>
bool Option<std::intmax_t>::parse(StringView, std::ostream&);

template<>
bool Option<std::intmax_t>::parse(StringView, std::ostream&);

template<>
bool Option<float>::parse(StringView, std::ostream&);

template<>
bool Option<double>::parse(StringView, std::ostream&);


template<typename T>
class Option<std::complex<T>, false> : public OptionBase {
public:
    using value_type = std::complex<T>;

    Option(StringView key, value_type const& value = value_type())
        : OptionBase(key, "[REAL][[(+|-)](j|J|i|I)IMAG]", false), value_(value)
        {}

    Option(StringView key, RequiredTag, value_type const& value = value_type())
        : OptionBase(key, "[REAL][[(+|-)](j|J|i|I)IMAG]", true), value_(value)
        {}

    Option(StringView key, Placeholder p,
           value_type const& value = value_type())
        : OptionBase(key, p.str, false), value_(value)
        {}

    Option(StringView key, Placeholder p, RequiredTag,
           value_type const& value = value_type())
        : OptionBase(key, p.str, true), value_(value)
        {}

    value_type const& get() const {
        return value_;
    }

    operator value_type const&() const {
        return value_;
    }

    bool parse(StringView arg, std::ostream& err = std::cerr) override final;

private:
    template<typename, bool>
    friend class Option;

    value_type value_;
};

template<typename T>
bool Option<std::complex<T>, false>::parse(StringView arg, std::ostream& err) {
    reset();

    if (arg.empty())
        return true;

    auto sep = arg.find("+j"), sep_end = StringView::npos;
    if (sep == StringView::npos)
        sep = arg.find("+J");
    if (sep == StringView::npos)
        sep = arg.find("+i");
    if (sep == StringView::npos)
        sep = arg.find("+I");
    if (sep == StringView::npos)
        sep = arg.find("-j");
    if (sep == StringView::npos)
        sep = arg.find("-J");
    if (sep == StringView::npos)
        sep = arg.find("-i");
    if (sep == StringView::npos)
        sep = arg.find("-I");

    if (sep == StringView::npos) {
        if (arg[0] == 'j' || arg[0] == 'J' || arg[0] == 'i' || arg[0] == 'I') {
            sep = 0;
            sep_end = 1;
        }
    } else {
        sep_end = sep + 2;
    }

    if (arg.find_first_of(" \t\f\r\n") != StringView::npos ||
            (sep_end != StringView::npos &&
                (sep_end == arg.size() ||
                 arg[sep_end] == '+'   ||
                 arg[sep_end] == '-'))) {
        error(err) << "complex value expected (valid format is [REAL][(+|-)(j|J|i|I)IMAG])" << std::endl;
        return false;
    }

    Option<T> opt(key());
    T real = T(), imag = T();

    if (sep > 0) {
        if (!opt.parse(arg.substr(0, sep), err))
            return false;

        real = opt.value_;
    }

    if (sep_end < StringView::npos) {
        if (!opt.parse(arg.substr(sep_end), err))
            return false;

        imag = ((arg[sep] == '-') ? -opt.value_ : opt.value_);
    }

    value_ = { real, imag };

    set();
    return true;
}


template<typename T, std::size_t N, bool Enum>
class Option<std::array<T, N>, Enum> : public OptionBase {
public:
    using value_type = std::array<T, N>;

    Option(StringView key, value_type const& value = value_type())
        : OptionBase(key, default_placeholder(), false), value_(value)
        {}

    Option(StringView key, RequiredTag, value_type const& value = value_type())
        : OptionBase(key, default_placeholder(), true), value_(value)
        {}

    Option(StringView key, Placeholder p,
           value_type const& value = value_type())
        : OptionBase(key, p.str, false), value_(value)
        {}

    Option(StringView key, Placeholder p, RequiredTag,
           value_type const& value = value_type())
        : OptionBase(key, p.str, true), value_(value)
        {}

    value_type const& get() const {
        return value_;
    }

    operator value_type const&() const {
        return value_;
    }

    bool parse(StringView arg, std::ostream& err = std::cerr) override final;

private:
    template<typename, bool>
    friend class Option;

    static StringView default_placeholder();

    value_type value_;
};

template<typename T, std::size_t N, bool Enum>
StringView Option<std::array<T, N>, Enum>::default_placeholder() {
    static std::string p;

    if (!p.empty())
        return p;
    
    Option<T, std::is_enum<T>::value || Enum> opt("");

    p = "{" + std::to_string(N) + "x" + opt.placeholder().to_string() + "}";

    return p;
}

template<typename T, std::size_t N, bool Enum>
bool Option<std::array<T, N>, Enum>::parse(StringView arg, std::ostream& err) {
    reset();

    if (arg.empty())
        return true;

    if (arg.front() != '{' || arg.back() != '}') {
        error(err) << "vector values should be wrapped in curly braces" << std::endl;
        return false;
    }

    arg = trim(arg.substr(1, arg.size() - 2));

    Option<T, std::is_enum<T>::value || Enum> opt(key());
    auto it = value_.begin(), end = value_.end();

    while (arg.size() && it != end) {
        auto comma = std::min(arg.find(','), arg.size());
        auto sub = trim(arg.substr(0, comma));

        if (sub.size()) {
            if (!opt.parse(sub, err))
                return false;

            *it = std::move(opt.value_);
        }

        ++it;
        arg = arg.substr(comma + 1);
    }

    if (arg.size() || it != end) {
        error(err) << "vector of " << N << " elements expected" << std::endl;
        return false;
    }

    set();
    return true;
}


template<typename T, bool Enum>
class Option<std::vector<T>, Enum> : public OptionBase {
public:
    using value_type = std::vector<T>;

    Option(StringView key, value_type const& value = value_type())
        : OptionBase(key, default_placeholder(), false), value_(value)
        {}

    Option(StringView key, RequiredTag, value_type const& value = value_type())
        : OptionBase(key, default_placeholder(), true), value_(value)
        {}

    Option(StringView key, Placeholder p,
           value_type const& value = value_type())
        : OptionBase(key, p.str, false), value_(value)
        {}

    Option(StringView key, Placeholder p, RequiredTag,
           value_type const& value = value_type())
        : OptionBase(key, p.str, true), value_(value)
        {}

    value_type const& get() const {
        return value_;
    }

    operator value_type const&() const {
        return value_;
    }

    bool parse(StringView arg, std::ostream& err = std::cerr) override final;

private:
    template<typename, bool>
    friend class Option;

    static StringView default_placeholder();

    value_type value_;
};

template<typename T, bool Enum>
StringView Option<std::vector<T>, Enum>::default_placeholder() {
    static std::string p;

    if (!p.empty())
        return p;
    
    Option<T, std::is_enum<T>::value || Enum> opt("");

    p = "{" + opt.placeholder().to_string() + ", ... }";

    return p;
}

template<typename T, bool Enum>
bool Option<std::vector<T>, Enum>::parse(StringView arg, std::ostream& err) {
    reset();

    if (arg.empty())
        return true;

    Option<T, std::is_enum<T>::value || Enum> opt(key());

    if (arg.front() != '{' || arg.back() != '}') {
        if (!opt.parse(arg, err))
            return false;

        value_.clear();
        value_.push_back(std::move(opt.value_));

        set();
        return true;
    }

    value_.clear();
    arg = trim(arg.substr(1, arg.size() - 2));

    while (arg.size()) {
        auto comma = std::min(arg.find(','), arg.size());
        auto sub = trim(arg.substr(0, comma));

        if (sub.size()) {
            if (!opt.parse(sub, err))
                return false;

            value_.push_back(std::move(opt.value_));
        } else {
            value_.push_back(T());
        }

        arg = arg.substr(comma + 1);
    }

    set();
    return true;
}


template<typename T, bool Enum>
class Option<std::set<T>, Enum> : public OptionBase {
public:
    using value_type = std::set<T>;

    Option(StringView key, value_type const& value = value_type())
        : OptionBase(key, default_placeholder(), false), value_(value)
        {}

    Option(StringView key, RequiredTag, value_type const& value = value_type())
        : OptionBase(key, default_placeholder(), true), value_(value)
        {}

    Option(StringView key, Placeholder p,
           value_type const& value = value_type())
        : OptionBase(key, p.str, false), value_(value)
        {}

    Option(StringView key, Placeholder p, RequiredTag,
           value_type const& value = value_type())
        : OptionBase(key, p.str, true), value_(value)
        {}

    value_type const& get() const {
        return value_;
    }

    operator value_type const&() const {
        return value_;
    }

    bool parse(StringView arg, std::ostream& err = std::cerr) override final;

private:
    template<typename, bool>
    friend class Option;

    static StringView default_placeholder();

    value_type value_;
};

template<typename T, bool Enum>
StringView Option<std::set<T>, Enum>::default_placeholder() {
    static std::string p;

    if (!p.empty())
        return p;
    
    Option<T, std::is_enum<T>::value || Enum> opt("");

    p = "{" + opt.placeholder().to_string() + ", ... }";

    return p;
}

template<typename T, bool Enum>
bool Option<std::set<T>, Enum>::parse(StringView arg, std::ostream& err) {
    reset();

    if (arg.empty())
        return true;

    Option<T, std::is_enum<T>::value || Enum> opt(key());

    if (arg.front() != '{' || arg.back() != '}') {
        if (!opt.parse(arg, err))
            return false;

        value_.clear();
        value_.insert(std::move(opt.value_));

        set();
        return true;
    }

    value_.clear();
    arg = trim(arg.substr(1, arg.size() - 2));

    while (arg.size()) {
        auto comma = std::min(arg.find(','), arg.size());
        auto sub = trim(arg.substr(0, comma));

        if (sub.size()) {
            if (!opt.parse(sub, err))
                return false;

            value_.insert(std::move(opt.value_));
        }

        arg = arg.substr(comma + 1);
    }

    set();
    return true;
}


// force enum mode
template<typename T>
using EnumOption = Option<T, true>;

// enum option
template<typename T>
class Option<T, true> : public OptionBase {
public:
    using value_type = T;
    using value_map = std::initializer_list<std::pair<StringView, T>>;

    Option(StringView key, value_type const& value = value_type())
        : OptionBase(key, default_placeholder(), false), value_(value)
        {}

    Option(StringView key, RequiredTag,
           value_type const& value = value_type())
        : OptionBase(key, default_placeholder(), true), value_(value)
        {}

    Option(StringView key, Placeholder p,
           value_type const& value = value_type())
        : OptionBase(key, p.str, false), value_(value)
        {}

    Option(StringView key, Placeholder p, RequiredTag,
           value_type const& value = value_type())
        : OptionBase(key, p.str, true), value_(value)
        {}

    value_type const& get() const {
        return value_;
    }

    operator value_type const&() const {
        return value_;
    }

    bool parse(StringView arg, std::ostream& err = std::cerr) override final;

    static const value_map values;

private:
    template<typename, bool>
    friend class Option;

    static StringView default_placeholder();

    value_type value_;
};

template<typename T>
StringView Option<T, true>::default_placeholder() {
    static std::string p;
    
    if (!p.empty())
        return p;

    p = "(";

    auto it = values.begin(), end = values.end();
    if (it != end)
        p += (it++)->first.to_string();

    for (; it != end; ++it)
        p += "|" + it->first.to_string();

    p += ")";

    return p;
}

template<typename T>
bool Option<T, true>::parse(StringView arg, std::ostream& err) {
    reset();

    if (arg.empty())
        return true;

    auto lc = to_lower(arg);

    for (auto it = values.begin(), end = values.end(); it != end; ++it) {
        if (it->first == lc) {
            value_ = it->second;
            set();
            return true;
        }
    }

    error(err) << "invalid value '" << arg << "'" << std::endl;
    return false;
}

} /* namespace opt */
