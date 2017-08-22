#include "options.hpp"

#include <cctype>
#include <iterator>
#include <map>

using namespace sdr;
using namespace sdr::opt;

static const std::intmax_t int_powers[] = {
    1, 10, 100,
    1000, 10000, 100000,
    1000000, 10000000, 100000000,
    1000000000, 10000000000, 100000000000,
    1000000000000, 10000000000000, 100000000000000,
    1000000000000000, 10000000000000000, 100000000000000000,
    1000000000000000000
};

static inline std::intmax_t pow10i(std::size_t exp) {
    return int_powers[exp];
}

static const std::map<char, std::size_t> int_units{
    { 'k', 3 },
    { 'K', 3 },
    { 'M', 6 },
    { 'G', 9 },
    { 'T', 12 },
    { 'P', 15 },
    { 'E', 18 }
};

static const std::map<char, float> float_units{
    { 'z', 1e-21 },
    { 'a', 1e-18 },
    { 'f', 1e-15 },
    { 'p', 1e-12 },
    { 'n', 1e-9 },
    { 'u', 1e-6 },
    { 'm', 1e-3 },
    { 'k', 1e3 },
    { 'M', 1e6 },
    { 'G', 1e9 },
    { 'T', 1e12 },
    { 'P', 1e15 },
    { 'E', 1e18 }
};


StringView sdr::opt::trim(StringView s) {
    auto start = s.find_first_not_of(" \t\f\r\n");
    if (start == StringView::npos)
        return StringView();

    auto end = s.find_first_of(" \t\f\r\n", start);
    if (end != StringView::npos)
        end -= start;

    return s.substr(start, end);
}

std::string sdr::opt::to_lower(StringView s) {
    std::string result;
    std::transform(s.begin(), s.end(), std::back_inserter(result), ::tolower);
    return result;
}


OptionBase::~OptionBase() {}


bool sdr::opt::parse(std::initializer_list<std::reference_wrapper<OptionBase>> opts,
                     std::initializer_list<std::reference_wrapper<OptionBase>> kwopts,
                     char const* const* first, char const* const* last,
                     std::ostream& err) {
    std::map<StringView, OptionBase*> kwmap;

    for (auto& opt: opts)
        kwmap[opt.get().key()] = &opt.get();

    for (auto& opt: kwopts)
        kwmap[opt.get().key()] = &opt.get();

    auto pos_it = std::begin(opts), pos_end = std::end(opts);

    for (auto it = first; it != last; ++it) {
        StringView arg = *it;
        auto assign = arg.find('=');
        if (assign != StringView::npos) {
            auto opt = kwmap.find(arg.substr(0, assign));
            if (opt != kwmap.end()) {
                if (!opt->second->parse(trim(arg.substr(assign+1)), err))
                    return false;
            }
        } else {
            auto opt = kwmap.find(arg);
            if (opt != kwmap.end() && dynamic_cast<Option<bool>*>(opt->second)) {
                opt->second->parse("true", err);
                continue;
            }

            while (pos_it != pos_end && pos_it->get().is_set())
                ++pos_it;

            if (pos_it != pos_end) {
                if (!pos_it->get().parse(trim(arg), err))
                    return false;
            }
        }
    }

    return true;
}

bool sdr::opt::parse(std::initializer_list<std::reference_wrapper<OptionBase>> opts,
                     std::initializer_list<std::reference_wrapper<OptionBase>> kwopts,
                     std::vector<StringView>& ignored,
                     char const* const* first, char const* const* last,
                     std::ostream& err) {
    std::map<StringView, OptionBase*> kwmap;

    for (auto& opt: opts)
        kwmap[opt.get().key()] = &opt.get();

    for (auto& opt: kwopts)
        kwmap[opt.get().key()] = &opt.get();

    auto pos_it = std::begin(opts), pos_end = std::end(opts);

    for (auto it = first; it != last; ++it) {
        StringView arg = *it;
        auto assign = arg.find('=');
        if (assign != StringView::npos) {
            auto opt = kwmap.find(arg.substr(0, assign));
            if (opt != kwmap.end()) {
                if (!opt->second->parse(trim(arg.substr(assign+1)), err))
                    return false;
            } else {
                ignored.push_back(arg);
            }
        } else {
            auto opt = kwmap.find(arg);
            if (opt != kwmap.end() && dynamic_cast<Option<bool>*>(opt->second)) {
                opt->second->parse("true", err);
                continue;
            }

            while (pos_it != pos_end && pos_it->get().is_set())
                ++pos_it;

            if (pos_it != pos_end) {
                if (!pos_it->get().parse(trim(arg), err))
                    return false;
            } else {
                ignored.push_back(arg);
            }
        }
    }

    return true;
}


template<>
bool Option<StringView>::parse(StringView arg, std::ostream&) {
    value_ = arg;
    set();
    return true;
}

template<>
bool Option<std::string>::parse(StringView arg, std::ostream&) {
    value_ = arg.to_string();
    set();
    return true;
}

template<>
bool Option<bool>::parse(StringView arg, std::ostream& err) {
    reset();

    if (arg.empty())
        return true;

    if (arg == "true" || arg == "1")
        value_ = true;
    else if (arg == "false" || arg == "0")
        value_ = false;
    else {
        error(err) << "boolean value expected" << std::endl;
        return false;
    }

    set();
    return true;
}

// Parse integer options
// fixed-point values with SI unit prefixes are supported
template<>
bool Option<std::intmax_t>::parse(StringView arg, std::ostream& err) {
    reset();

    if (arg.empty())
        return true;

    auto s = arg.to_string();
    try {
        std::size_t pos = 0;
        value_ = std::stoll(s, &pos, 0);

        if (pos < s.size()) {
            if (s[0] == '0')
                // No fixed point/units for octal and hexadecimal values
                throw nullptr;

            std::size_t decimal_digits = 0;
            std::uintmax_t decimal = 0;

            if (s[pos] == '.') {
                auto end = s.find_first_not_of("0123456789", ++pos);
                if (end == pos) {
                    error(err) << "digits expected after point" << std::endl;
                    return false;
                } else if (end == std::string::npos)
                    throw nullptr;

                decimal_digits = end - pos;

                s.erase(0, pos);
                decimal = std::stoull(s, &pos);

                if (pos < decimal_digits)
                    throw nullptr;

                s.erase(0, decimal_digits);
            } else {
                s.erase(0, pos);
            }

            auto unit = int_units.find(s[0]);
            if (unit == int_units.end() || s.size() > 1) {
                error(err) << "invalid unit '" << s << '\'' << std::endl;
                return false;
            }

            if (decimal_digits > unit->second)
                throw nullptr;

            value_ *= pow10i(unit->second);
            value_ += decimal*pow10i(unit->second - decimal_digits);
        }
    } catch (std::out_of_range&) {
        error(err) << "option value out of range" << std::endl;
    } catch (...) {
        error(err) << "integer value expected" << std::endl;
        return false;
    }

    set();
    return true;
}

// Parse unsigned integer options
// fixed-point values with SI unit prefixes are supported
template<>
bool Option<std::uintmax_t>::parse(StringView arg, std::ostream& err) {
    reset();

    if (arg.empty())
        return true;

    auto s = arg.to_string();
    try {
        if (s[0] == '-')
            // Should be unsigned
            throw nullptr;

        std::size_t pos = 0;
        value_ = std::stoull(s, &pos, 0);

        if (pos < s.size()) {
            if (s[0] == '0')
                // No fixed point/units for octal and hexadecimal values
                throw nullptr;

            std::size_t decimal_digits = 0;
            std::uintmax_t decimal = 0;

            if (s[pos] == '.') {
                auto end = s.find_first_not_of("0123456789", ++pos);
                if (end == pos) {
                    error(err) << "digits expected after point" << std::endl;
                    return false;
                } else if (end == std::string::npos)
                    throw nullptr;

                decimal_digits = end - pos;

                s.erase(0, pos);
                decimal = std::stoull(s, &pos);

                if (pos < decimal_digits)
                    throw nullptr;

                s.erase(0, decimal_digits);
            } else {
                s.erase(0, pos);
            }

            auto unit = int_units.find(s[0]);
            if (unit == int_units.end() || s.size() > 1) {
                error(err) << "invalid unit '" << s << '\'' << std::endl;
                return false;
            }

            if (decimal_digits > unit->second)
                throw nullptr;

            value_ *= pow10i(unit->second);
            value_ += decimal*pow10i(unit->second - decimal_digits);
        }
    } catch (std::out_of_range&) {
        error(err) << "option value out of range" << std::endl;
    } catch (...) {
        error(err) << "unsigned integer value expected" << std::endl;
        return false;
    }

    set();
    return true;
}

// Parse single precision floating-point options
// SI unit prefixes are supported
template<>
bool Option<float>::parse(StringView arg, std::ostream& err) {
    reset();

    if (arg.empty())
        return true;

    try {
        std::size_t pos = 0;
        value_ = std::stof(arg.to_string(), &pos);
        if (pos < arg.size()) {
            auto unit = float_units.find(arg[pos]);
            if (unit == float_units.end() || pos+1 < arg.size()) {
                error(err) << "invalid unit '" << arg.substr(pos) << '\'' << std::endl;
                return false;
            }

            value_ *= unit->second;
        }
    } catch (std::out_of_range&) {
        error(err) << "option value out of range" << std::endl;
    } catch (...) {
        error(err) << "floating-point value expected" << std::endl;
        return false;
    }

    set();
    return true;
}

// Parse double precision floating-point options
// SI unit prefixes are supported
template<>
bool Option<double>::parse(StringView arg, std::ostream& err) {
    reset();

    if (arg.empty())
        return true;

    try {
        std::size_t pos = 0;
        value_ = std::stod(arg.to_string(), &pos);
        if (pos < arg.size()) {
            auto unit = float_units.find(arg[pos]);
            if (unit == float_units.end() || pos+1 < arg.size()) {
                error(err) << "invalid unit '" << arg.substr(pos) << '\'' << std::endl;
                return false;
            }

            value_ *= unit->second;
        }
    } catch (std::out_of_range&) {
        error(err) << "option value out of range" << std::endl;
    } catch (...) {
        error(err) << "floating-point value expected" << std::endl;
        return false;
    }

    set();
    return true;
}
