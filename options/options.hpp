#pragma once

#include <algorithm>
#include <array>
#include <complex>
#include <functional>
#include <iostream>
#include <string>
#include <experimental/string_view>
#include <vector>

namespace sdr { namespace opt
{

using StringView = std::experimental::string_view;

StringView trim(StringView s);
std::string to_lower(StringView s);


class OptionBase {
public:
    virtual ~OptionBase();

    StringView key() const {
        return key_;
    }

    bool is_set() const {
        return set_;
    }

    virtual bool parse(StringView arg, std::ostream& err = std::cerr) = 0;

    std::ostream& error(std::ostream& err) const {
        return err << "error: " << key() << ": ";
    }

protected:
    OptionBase(StringView k) : key_(k) {}

    void set() {
        set_ = true;
    }

    void reset() {
        set_ = false;
    }

private:
    bool set_ = false;
    StringView key_;
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


template<typename T>
class Option : public OptionBase {
public:
    using value_type = T;

    Option(StringView key, value_type const& value = value_type())
        : OptionBase(key), value_(value)
        {}

    value_type const& get() const {
        return value_;
    }

    operator value_type const&() const {
        return value_;
    }

    bool parse(StringView arg, std::ostream& err = std::cerr) override final;
private:
    value_type value_;
};

template<>
bool Option<StringView>::parse(StringView, std::ostream&);

template<>
bool Option<std::string>::parse(StringView, std::ostream&);

template<>
bool Option<bool>::parse(StringView, std::ostream&);

template<>
bool Option<std::intmax_t>::parse(StringView, std::ostream&);

template<>
bool Option<std::intmax_t>::parse(StringView, std::ostream&);

template<>
bool Option<float>::parse(StringView, std::ostream&);

template<>
bool Option<double>::parse(StringView, std::ostream&);


template<typename T>
class Option<std::complex<T>> : public OptionBase {
public:
    using value_type = std::complex<T>;

    Option(StringView key, value_type const& value = value_type())
        : OptionBase(key), value_(value)
        {}

    value_type const& get() const {
        return value_;
    }

    operator value_type const&() const {
        return value_;
    }

    bool parse(StringView arg, std::ostream& err = std::cerr) override final;
private:
    value_type value_;
};

template<typename T>
bool Option<std::complex<T>>::parse(StringView arg, std::ostream& err) {
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
        error(err) << "complex value expected (valid format is REAL[+-][iIjJ]IMAG)" << std::endl;
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


template<typename T, std::size_t N>
class Option<std::array<T, N>> : public OptionBase {
public:
    using value_type = std::array<T, N>;

    Option(StringView key, value_type const& value = value_type())
        : OptionBase(key), value_(value)
        {}

    value_type const& get() const {
        return value_;
    }

    operator value_type const&() const {
        return value_;
    }

    bool parse(StringView arg, std::ostream& err = std::cerr) override final;
private:
    value_type value_;
};

template<typename T, std::size_t N>
bool Option<std::array<T, N>>::parse(StringView arg, std::ostream& err) {
    reset();

    if (arg.empty())
        return true;

    if (arg.front() != '{' || arg.back() != '}') {
        error(err) << "vector values should be wrapped in curly braces" << std::endl;
        return false;
    }

    arg = trim(arg.substr(1, arg.size() - 2));

    Option<T> opt(key());
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


template<typename T>
class Option<std::vector<T>> : public OptionBase {
public:
    using value_type = std::vector<T>;

    Option(StringView key, value_type const& value = value_type())
        : OptionBase(key), value_(value)
        {}

    value_type const& get() const {
        return value_;
    }

    operator value_type const&() const {
        return value_;
    }

    bool parse(StringView arg, std::ostream& err = std::cerr) override final;
private:
    value_type value_;
};

template<typename T>
bool Option<std::vector<T>>::parse(StringView arg, std::ostream& err) {
    reset();

    if (arg.empty())
        return true;

    if (arg.front() != '[' || arg.back() != ']') {
        error(err) << "list values should be wrapped in square brackets" << std::endl;
        return false;
    }

    arg = trim(arg.substr(1, arg.size() - 2));

    Option<T> opt(key());

    value_.clear();

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

} /* namespace opt */ } /* namespace sdr */
