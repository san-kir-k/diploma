#include "cli_args_parser.h"

CLIArgsParser::CLIArgsParser(int &argc, char **argv)
{
    for (auto i = 1; i < argc; ++i)
    {
        tokens.push_back(std::string(argv[i]));
    }
}

const std::string& CLIArgsParser::GetOption(const std::string &option) const
{
    auto itr = std::find(this->tokens.begin(), this->tokens.end(), option);
    if (itr != tokens.end() && ++itr != tokens.end())
    {
        return *itr;
    }
    static const std::string empty_string("");
    return empty_string;
}

bool CLIArgsParser::IsOptionExists(const std::string &option) const
{
    return std::find(tokens.begin(), tokens.end(), option) != tokens.end();
}