/**
 * @file AppOptions.hpp
 * @author Simon Cahill (contact@simonc.eu)
 * @brief Contains the AppOptions struct which contains the options as configured via command-line parameters for each instance.
 * @version 0.1
 * @date 2023-03-14
 * 
 * @copyright Copyright (c) 2023 Simon Cahill
 */

#ifndef HTTPDREPORT_GENERATOR_INCLUDE_APPOPTIONS_HPP
#define HTTPDREPORT_GENERATOR_INCLUDE_APPOPTIONS_HPP

/////////////////////
// SYSTEM INCLUDES //
/////////////////////

// stl
#include <string>
#include <vector>

/////////////////////
// LOCAL  INCLUDES //
/////////////////////
#include "resources/Resources.hpp"

namespace httpdreport {

    using std::string;
    using std::vector;

    /**
     * @brief Structure containing all options required for each running instance of
     * httpd-hit-report.
     */
    struct AppOptions final {

        bool            FollowSymlinks{false}; //!< Whether or not to follow symlinks
        bool            ReadFromStdin{false}; //!< Whether or not to read from stdin.
        bool            ReadGzippedFiles{false}; //!< Whether or not to read files compressed with gzip
        bool            RecurseDirectories{false}; //!< Whether or not to recurse through subdirectors in LogDirectory

        string          AccessFileGlob{"*.access.log*"}; //!< The glob used to search access logs
        string          ErrorFileGlob{"*.error.log*"}; //!< The glob used to search error logs

        string          LogDirectory{resources::DEFAULT_LOG_PATH}; //!< The directory in which to search for logs

        string          OutputFile{}; //!< The output file destination (or empty or output is stdout)

        vector<string>  InputFiles{}; //!< Arbitray input files passed via command line

    };

}

#endif // HTTPDREPORT_GENERATOR_INCLUDE_APPOPTIONS_HPP