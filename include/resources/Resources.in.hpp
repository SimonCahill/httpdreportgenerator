/**
 * @file Resources.hpp
 * @author Simon Cahill (contact@simonc.eu)
 * @brief Contains resources required by the application at runtime.
 * @version 0.1
 * @date 2023-03-14
 * 
 * @copyright Copyright (c) 2023 Simon Cahill
 */

#ifndef HTTPDREPORT_GENERATOR_INCLUDE_RESOURCES_RESOURCES_HPP
#define HTTPDREPORT_GENERATOR_INCLUDE_RESOURCES_RESOURCES_HPP

/////////////////////
// SYSTEM INCLUDES //
/////////////////////

// stl
#include <string>

// libc
#include <stdint.h>


namespace httpdreport::resources {

    using std::string;
    using std::string_view;

    const inline string APP_VERSION = R"(@PROJECT_VERSION@)"; //!< Gets the application's version as configured in CMakeLists.txt

    const inline string APP_NAME = R"(@PROJECT_NAME@)"; //!< Gets the application's name as configured in CMakeLists.txt

    const inline string APP_DESCRIPTION = R"(@PROJECT_DESCRIPTION@)"; //!< Gets the application's description as configured in CMakeLists.txt

    const inline string DEFAULT_LOG_PATH = R"(@httpdreport_LOG_LOCATION@)"; //!< The default path in which to search for log files

}

#endif // HTTPDREPORT_GENERATOR_INCLUDE_RESOURCES_RESOURCES_HPP