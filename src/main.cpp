/**
 * @file main.cpp
 * @author Simon Cahill (contact@simonc.eu)
 * @brief Contains the program's entry point and option parsing.
 * @version 0.1
 * @date 2023-03-14
 * 
 * @copyright Copyright (c) 2023 Simon Cahill.
 */

/////////////////////
// SYSTEM INCLUDES //
/////////////////////

// stl
#include <iostream>
#include <string>

// fmt
#include <fmt/format.h>

// libc
#include <getopt.h>

/////////////////////
// LOCAL  INCLUDES //
/////////////////////
#include "AppOptions.hpp"
#include "LogSearcher.hpp"
#include "resources/Resources.hpp"

using fmt::format;

using std::cerr;
using std::cout;
using std::endl;
using std::string;

//=======================================
// Prototypes
//=======================================
int parseArgs(const int32_t argc, char* const* argv); //!< Parses incoming command-line arguments

void printHelp(); //!< Prints the help text to the terminal
void printVersion(); //!< Prints the version info to the terminal

static httpdreport::AppOptions g_appOptions{};

int main(const int32_t argc, char* const* argv) {
    if (auto retCode = parseArgs(argc, argv); retCode > 0) {
        return retCode - 1;
    }

    return 0;
}

/**
 * @brief Parses command-line arguments coming into the application and sets options internally.
 * 
 * @param argc The arg count.
 * @param argv The arguments passed to the application.
 * 
 * @return int 0 if regular execution shall continue, >0 if application should exit with (>0) - 1.
 */
int parseArgs(const int32_t argc, char* const* argv) {
    static const string SHORT_OPTS = "hvsgFRra:e:o:l:";
    static const option OPTIONS[] = {
        { "help",       no_argument,        nullptr, 'h' },
        { "version",    no_argument,        nullptr, 'v' },
        { "stdin",      no_argument,        nullptr, 's' },
        { "gzip",       no_argument,        nullptr, 'g' },
        { "follow",     no_argument,        nullptr, 'F' },
        { "recurse",    no_argument,        nullptr, 'R' },
        { "recurse",    no_argument,        nullptr, 'r' },
        { "access",     required_argument,  nullptr, 'a' },
        { "error",      required_argument,  nullptr, 'e' },
        { "output",     required_argument,  nullptr, 'o' },
        { "log-dir",    required_argument,  nullptr, 'l' },
        { nullptr,      no_argument,        nullptr,  0  }
    };

    int32_t optChar = 0;
    while ((optChar = getopt_long(argc, argv, SHORT_OPTS.c_str(), OPTIONS, nullptr)) != -1) {
        switch (optChar) {
            case 1:
                g_appOptions.InputFiles.emplace_back(optarg);
                break;
            case 'h':
                printHelp();
                return 1;
            case 'v':
                printVersion();
                return 1;
            case 'o':
                g_appOptions.OutputFile = optarg;
                break;
            case 's':
                g_appOptions.ReadFromStdin = true;
                break;
            case 'g':
                g_appOptions.ReadGzippedFiles = true;
                break;
            case 'a':
                g_appOptions.AccessFileGlob = optarg;
                break;
            case 'e':
                g_appOptions.ErrorFileGlob = optarg;
                break;
            case 'l':
                g_appOptions.LogDirectory = optarg;
                break;
            case 'F':
                g_appOptions.FollowSymlinks = true;
                break;
            case 'R':
            case 'r':
                g_appOptions.RecurseDirectories = true;
                break;
            default:
                break;
        }
    }

    return 0;
}

void printHelp() {
    printVersion();
    static const auto DEFAULT_APPOPTS = httpdreport::AppOptions{};

    using namespace httpdreport::resources;
    cout << format(
R"(
    {0:s}
    Usage:
        {1:s} # normal execution, must be run as root
        {1:s} [input files] # define input files
        {1:s} [-options]

    Switches:
        --help,     -h              Prints this text and exits
        --version,  -v              Prints the version information and exits
        --stdin,    -s              Read from stdin instead of searching for logs under {2:s}
        --gzip,     -g              Allow reading from gzip-compressed files
        --follow,   -F              Follow symlinks
        --recurse,  -R/r            Recurse through subdirectories

    Arguments:
        --access,   -a[glob]        Set the glob pattern for access log files. Default: {3:s}
        --error,    -e[glob]        Set the glob pattern for error log files. Default: {4:s}
        --output,   -o[file]        Set the output file (otherwise stdout is used)

)", APP_DESCRIPTION, APP_NAME, DEFAULT_LOG_PATH, DEFAULT_APPOPTS.AccessFileGlob, DEFAULT_APPOPTS.ErrorFileGlob) << endl;
}

void printVersion() {
    using namespace httpdreport::resources;
    cout << APP_NAME << "\t" << APP_VERSION << endl;
}
