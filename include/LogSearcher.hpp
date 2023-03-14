/**
 * @file LogSearcher.hpp
 * @author Simon Cahill (contact@simonc.eu)
 * @brief Contains the logic for searching log files within the directory matching the glob values.
 * @version 0.1
 * @date 2023-03-14
 * 
 * @copyright Copyright (c) 2023 Simon Cahill
 */

#ifndef HTTPD_REPORT_GENERATOR_INCLUDE_LOGSEARCHER_HPP
#define HTTPD_REPORT_GENERATOR_INCLUDE_LOGSEARCHER_HPP

/////////////////////
// SYSTEM INCLUDES //
/////////////////////

// stl
#include <string>
#include <filesystem>
#include <vector>

// fmt
#include <fmt/format.h>

/////////////////////
// LOCAL  INCLUDES //
/////////////////////
#include "AppOptions.hpp"

namespace httpdreport {

    namespace fs = std::filesystem;

    using fmt::format;

    using std::string;
    using std::vector;

    /**
     * @brief Header-only implementation of a simple class that searches for log files matching the configured globs.
     * 
     * This class searches, caches and returns two vectors containing the found log files.
     */
    class LogSearcher final {
        public: // +++ Constructor / Destructor +++
            explicit LogSearcher(const AppOptions& opts): m_appOpts(opts) {}
            LogSearcher(const LogSearcher&) = delete;

        public: // +++ Business Logic +++
            void searchLogFiles() { searchLogFiles(m_appOpts.LogDirectory); }

        private: // +++ Private Business +++
            void searchLogFiles(const fs::path& dirPath) {
                try {
                    // we can't use recursive_directory_iterator without having excess boilerplate here.
                    // we'll just do the recursion ourselves.
                    // kinda stupid that recursive_directory_iterator doesn't inherit from directory_iterator...
                    auto dirIter = fs::directory_iterator(dirPath);

                    for (const auto& entry : dirIter) {
                        // filter out any entries we don't want
                        if (
                            !entry.exists() &&
                            !entry.is_regular_file() &&
                            !entry.is_directory() &&
                            (m_appOpts.FollowSymlinks && !entry.is_symlink())) {
                            continue; // these dir entries don't match out criteria
                        }

                        // recurse through directories if required
                        if (
                            (m_appOpts.RecurseDirectories && entry.is_directory()) ||
                            (entry.is_symlink() && entry.symlink_status().type() == fs::file_type::directory)
                        ) {
                            searchLogFiles(entry.path());
                        }

                        // check for symlink files
                        if (entry.is_symlink() && entry.symlink_status().type() != fs::file_type::regular) {
                            continue;
                        }

                        
                    }
                } catch (const fs::filesystem_error& ex) {
                    // exception handling later
                }
            }

        private:
            AppOptions m_appOpts{};

            vector<fs::path> m_accessLogs{};
            vector<fs::path> m_errorLogs{};
    };

}

#endif // HTTPD_REPORT_GENERATOR_INCLUDE_LOGSEARCHER_HPP