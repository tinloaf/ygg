#ifndef CMDLINE_HPP
#define CMDLINE_HPP

#include <cstring>
#include <string>
#include <vector>

class Cmdline {
public:
  void
  init(int argc, char ** argv)
  {
    this->remaining_argv = new char *[argc + 1];
    this->remaining_argc = 0;

		// program name
		this->remaining_argv[0] = argv[0];
		this->remaining_argc++;
		
    int i = 1;

    while (i < argc) {
      bool parsed = false;

      if (std::strcmp(argv[i], "--papi") == 0) {
	this->parse_papi(argv[i + 1]);
	i++;
	parsed = true;
      }

      if (std::strcmp(argv[i], "--sizes") == 0) {
	this->parse_sizes(argv[i + 1]);
	i++;
	parsed = true;
      }

      if (!parsed) {
	this->remaining_argv[this->remaining_argc] = argv[i];
	this->remaining_argc++;
      }
      i++;
    }

		this->remaining_argv[this->remaining_argc] = nullptr;
  }

	char **
	get_remaining_argv() {
		return this->remaining_argv;
	}

	int
	get_remaining_argc() {
		return this->remaining_argc;
	}
	
  const std::vector<std::string>
  get_papi_types() const
  {
    return this->papi_types;
  }

  const std::vector<std::pair<size_t, size_t>>
  get_sizes() const
  {
    return this->sizes;
  }

  static Cmdline &
  get()
  {
    static Cmdline cmdline;

    return cmdline;
  }

  ~Cmdline() { delete (this->remaining_argv); }

private:
  int remaining_argc;
  char ** remaining_argv;

  Cmdline() : remaining_argv(nullptr){};

  void
  parse_papi(const char * arg)
  {
    char * str = (char *)malloc(strlen(arg) + 1);
    std::strncpy(str, arg, strlen(arg) + 1);
    char * tok = std::strtok(str, ",");
    this->papi_types.clear();

    while (tok != nullptr) {
      this->papi_types.push_back(std::string(tok));
      tok = std::strtok(nullptr, ",");
    }

    free(str);
  }

  void
  parse_sizes(const char * arg)
  {
    char * str = (char *)malloc(strlen(arg) + 1);
    std::strncpy(str, arg, strlen(arg) + 1);
    char * tok = std::strtok(str, ":");
    this->sizes.clear();
    while (tok != nullptr) {
      size_t exponent = (size_t)std::atol(tok);
      tok = std::strtok(nullptr, ",");
      size_t iterations = (size_t)std::atol(tok);
      tok = std::strtok(nullptr, ":");
      this->sizes.emplace_back(exponent, iterations);
    }
  }

  std::vector<std::string> papi_types;
  std::vector<std::pair<size_t, size_t>> sizes;
  size_t max_exponent;
};

#endif
