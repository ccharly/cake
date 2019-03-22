#ifndef CAKE_ERRORS_HPP
#define CAKE_ERRORS_HPP

#include <string>
#include <exception>

namespace cake {

class error : public std::exception {
    std::string _message;

    public:
    error(std::string const& m) : std::exception(), _message(m) {
    }

    const char* what() const noexcept override {
        return _message.c_str();
    }
};

}

#endif
