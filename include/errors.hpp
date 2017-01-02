#pragma once

class program_error : public std::runtime_error {
public:
    explicit program_error(const std::string &message, int code = 0)
        : runtime_error(message), code(code) {}
    virtual ~program_error() {}
    int code{0};
};

#define DEFINE_CHILD_ERROR(error, parent) \
    class error : public parent {         \
        using parent::parent;             \
    }
#define DEFINE_ERROR(error) DEFINE_CHILD_ERROR(error, program_error)
