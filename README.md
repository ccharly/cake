## Some cooking in `c++`!

- - -

This projects *will* to brings project/tool management in c++. Making easy to install
compilers, create project skeletons, build system generation and maybe have very basic
package management!

It will look something like this:

```sh
# Creating a new basic project
cake new hello_world
```

```sh
# Build the project (based on some config)
cake build
```

And so on and so on...

- - -

Instead of having config file written in `.json`, `.toml` or whatever popular format, config will
_*probably*_ be written in c++ directly... Something like:

```c++
# Cakefile.cpp
#include <cake/cake.hpp>

// Sources
auto src = cake::src("src/*.cpp");

// Main build target
auto all = cake::rule("all", src, [](auto const& context) {
    std::vector<std::string> objs;
    // Compiling
    for (auto src : context.deps) {
        auto obj = cake::rename(src, ".cpp", ".o");
        cake::cxx.compile(src).output(obj);
        objs.push_back(obj);
    }
    // Linking
    cake::cxx.link(context.rule_name, objs);
});

// Set default rule
cake::default_rule = all;
```
