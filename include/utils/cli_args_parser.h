#pragma once

#include <vector>
#include <string>
#include <algorithm>

class CLIArgsParser
{
public:
    explicit CLIArgsParser(int &argc, char **argv);

    ~CLIArgsParser() = default;

    const std::string& GetOption(const std::string &option) const;

    bool IsOptionExists(const std::string &option) const;

private:
    std::vector <std::string> tokens;
};