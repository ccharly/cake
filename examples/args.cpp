#include <cake/args.hpp>

struct options {
  bool h;
  std::string command;
  struct {
      std::string config;
  } c_new;
};

using cake::required;
using cake::metavar;
using cake::help;

auto opts = options{};
auto config = cake::args::configure([](auto& args) {
  args
    .template arg<bool>({"-h", "--help"}, { help("The config file") }, opts.h)
    .command("new", "Creates new project", [](auto& c) {
      c.then([]() { opts.command = "new"; })
        .template arg<std::string>({"-c", "--c", "--config"},
                { required(), metavar("<config>"), help("The config file") },
                opts.c_new.config
                )
        .command("foo", "Foo project", [](auto& c) {})
        .command("bar", "Bar project", [](auto& c) {})
        ;
    })
    .command("test", "Test project", [](auto& c) {
      c.then([]() { opts.command = "new"; })
        .template arg<std::string>({"-c", "--c", "--config"},
                { required(), metavar("<config>"), help("The config file") },
                opts.c_new.config
                )
        ;
    });
});

int main(int argc, char** argv) {
  printf("%s", config.usage(argv[0]).c_str());
  config.parse(argc, argv);
  return 0;
}
