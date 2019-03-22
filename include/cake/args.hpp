#ifndef CAKE_ARGS_HPP
#define CAKE_ARGS_HPP

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <iostream>
#include "./error.hpp"

namespace cake {

using arg_name_vector_container_t = std::vector<std::string>;
struct arg_name_vector : arg_name_vector_container_t {
    using arg_name_vector_container_t::arg_name_vector_container_t;

    arg_name_vector(std::string const& arg)
        : arg_name_vector_container_t({arg}) {
    }
};

class arg_error : public error {
    public:
    using error::error;
};

template <typename T>
struct arg_parser;

template <>
struct arg_parser<std::string> {
    std::function<void (std::string const&)> then;

    void operator()(std::string const& arg) {
        then(arg);
    }
};

template <>
struct arg_parser<bool> {
    std::function<void (bool const&)> then;

    void operator()(std::string const&) {
        then(true);
    }
};

struct arg_info {
    bool required;
    bool requires_arg;
    const char* help;
    const char* metavar;
};

struct arg_info_setter {
    virtual ~arg_info_setter() {};
    virtual void set_options(arg_info& options) = 0;
};

struct arg_info_setter_required : arg_info_setter {
    arg_info_setter_required() {
    }

    virtual void set_options(arg_info& options) {
        options.required = true;
    }
};

struct arg_info_setter_metavar : arg_info_setter {
    const char* _metavar;

    arg_info_setter_metavar(const char* m)
        : _metavar(m) {
    }

    virtual void set_options(arg_info& options) {
        options.metavar = _metavar;
    }
};

struct arg_info_setter_help : arg_info_setter {
    const char* _help;

    arg_info_setter_help(const char* m)
        : _help(m) {
    }

    virtual void set_options(arg_info& options) {
        options.help = _help;
    }
};

std::shared_ptr<arg_info_setter> required() {
    return std::shared_ptr<arg_info_setter>(new arg_info_setter_required());
}

std::shared_ptr<arg_info_setter> metavar(const char* m) {
    return std::shared_ptr<arg_info_setter>(new arg_info_setter_metavar(m));
}

std::shared_ptr<arg_info_setter> help(const char* m) {
    return std::shared_ptr<arg_info_setter>(new arg_info_setter_help(m));
}

using arg_info_vector = std::vector<std::shared_ptr<arg_info_setter>>;

struct arg {
  arg_name_vector arg_name;
  arg_info info;
  std::function<void (const char*)> handler;
};

class args {

  std::string _help;
  std::map<std::string, std::unique_ptr<args>> _commands;
  std::map<std::string, arg> _args;
  std::vector<arg> _flat_args;
  std::function<void ()> _then;

  public:
  args() = default;
  args(args const&) = delete;

  public:
  template <typename F>
  args& command(std::string const& name, F f) {
    _commands[name] = std::make_unique<args>();
    f(*_commands[name]);
    return *this;
  }

  template <typename F>
  args& command(std::string const& name, std::string const& help, F f) {
    command(name, f);
    _help = help;
    return *this;
  }

  public:
  template <typename F>
  args& then(F f) {
    _then = f;
    return *this;
  }

  public:
  template <typename T>
  bool requires_arg() const {
      return !std::is_same<T, bool>();
  }

  public:
  template <typename T>
  args& arg(arg_name_vector const& arg_name, arg_info_vector const& arg_info_setters, std::function<void (T const&)> f) {
    arg_info info;
    for (auto const& info_setter : arg_info_setters) {
        info_setter->set_options(info);
    }
    info.requires_arg = requires_arg<T>();
    auto arg = ::cake::arg{arg_name, info, arg_parser<T>{f}};
    for (auto flag : arg_name) {
        _args[flag] = arg;
    }
    _flat_args.push_back(arg);
    return *this;
  }

  template <typename T, typename F>
  args& arg(arg_name_vector const& arg_name, std::function<void (T const&)> f) {
    return arg<T>(arg_name, {}, f);
  }

  template <typename T>
  args& arg(arg_name_vector const& arg_name, arg_info_vector const& arg_info_setters, T& val) {
    return arg<T>(arg_name, arg_info_setters, [&](auto v) { val = v; });
  }

  template <typename T>
  args& arg(arg_name_vector const& arg_name, T& val) {
    return arg<T>(arg_name, {}, [&](auto v) { val = v; });
  }

  public:
  class argv_reader {
      int _i;
      int _argc;
      char** _argv;

      public:
      argv_reader(int argc, char** argv)
          : _i(1), _argc(argc), _argv(argv) {
      }

      argv_reader& next() {
          _i++;
          return *this;
      }

      bool end() const {
          return _i >= _argc;
      }

      const char* current() {
          return _argv[_i];
      }
  };

  public:
  class config {
    std::unique_ptr<::cake::args> _args;

    public:
    template <typename Configure>
    config(Configure configure)
      : _args(std::make_unique<::cake::args>()) {
      configure(*_args);
    }

    std::string usage(std::string const& bin) const {
      std::string u = "usage: " + bin;
      usage_walk(*_args, u);
      return u;
    }

    void parse(int argc, char** argv) {
      argv_reader r(argc, argv);
      parse_walk(*_args, r);
    }

    private:
    static void usage_walk(::cake::args const& args, std::string& u, std::string const& prefix = "") {
      for (auto const& arg : args._flat_args) {
          const char* beg = "";
          const char* end = "";
          u += " ";
          if (arg.info.required) {
            if (arg.arg_name.size()) {
              beg = "(";
              end = ")";
            }
          } else {
            beg = "[";
            end = "]";
          }
          u += beg;
          if (arg.arg_name.size()) {
            u += arg.arg_name[0];
            if (arg.info.requires_arg) {
                u += std::string(" ") + arg.info.metavar;
            }
            for (int i = 1; i < arg.arg_name.size(); ++i) {
                u += " | " + arg.arg_name[i];
                if (arg.info.requires_arg) {
                    u += std::string(" ") + arg.info.metavar;
                }
            }
          }
          u += end;
      }
      if (args._commands.size()) {
          u += " <command> [<args>]\n" + prefix + "-- where commands must be one of:\n";
          for (auto const& command : args._commands) {
              u += prefix + "   " + command.first;
              usage_walk(*command.second, u, prefix + "   ");
              // If containing sub-commands, do not include the \n because the last sub-command
              // will already use one
              if (command.second->_commands.size() == 0) {
                u += "\n";
              }
          }
      }
    }

    static void parse_walk(::cake::args const& args, argv_reader& r) {
      if (args._then) {
        args._then();
      }
      if (r.end()) {
          return;
      }
      for (auto const& arg : args._args) {
          auto arg_name = arg.first;
          auto arg_data = arg.second;
          if (r.current() == arg_name) {
              if (arg_data.info.requires_arg) {
                r.next();
                if (r.end()) {
                    throw arg_error(std::string("option '") + arg_name
                            + "' requires argument: " + arg_data.info.metavar);
                }
                arg_data.handler(r.current());
              } else {
                arg_data.handler(r.current());
              }
              r.next();
              if (r.end()) {
                  return;
              }
          }
      }
      for (auto const& command : args._commands) {
          if (r.current() == command.first) {
              return parse_walk(*command.second, r.next());
          }
      }
    }
  };

  template <typename Configure>
  static config configure(Configure configure) {
    return config(configure);
  }
};

}

#endif
