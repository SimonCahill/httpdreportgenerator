/**
 * @file Extensions.hpp
 * @author Simon Cahill (contact@simonc.eu)
 * @brief This header contains useful extension methods I find myself requiring everywhere... I should put these in their own library.
 * @version 0.1
 * @date 2023-03-14
 * 
 * @copyright Copyright (c) 2023 Simon Cahill
 */

#ifndef HTTPD_REPORT_GENERATOR_INCLUDE_EXTENSIONS_HPP
#define HTTPD_REPORT_GENERATOR_INCLUDE_EXTENSIONS_HPP
/////////////////////
// SYSTEM INCLUDES //
/////////////////////

// stl
#include <string>
#include <filesystem>
#include <fstream>
#include <functional>
#include <regex>

// libc
#include <regex.h>

namespace httpdreport {

    namespace fs = std::filesystem;

    using std::ifstream;
    using std::function;
    using std::regex;
    using std::regex_match;
    using std::string;
    using std::vector;

    /**
     * @brief Splits a given string by the passed delimiters in to a vector.
     *
     * @param str string& The string to split.
     * @param delimiters string& A string containing the delimiters to split by (chars).
     * @param tokens vector<string>& A vector that will contain the elements.
     * 
     * @return true If tokens were found.
     * @return false Otherwise.
     */
    bool splitString(const string& str, const string& delimiters, vector<string> &tokens) {
        // Skip delimiters at beginning.
        string::size_type lastPos = str.find_first_not_of(delimiters, 0);
        // Find first "non-delimiter".
        string::size_type pos = str.find_first_of(delimiters, lastPos);

        while (string::npos != pos || string::npos != lastPos) {
            // Found a token, add it to the vector.
            tokens.push_back(str.substr(lastPos, pos - lastPos));
            // Skip delimiters.
            lastPos = string::npos == pos ? string::npos : pos + 1;
            // Find next "non-delimiter"
            pos = str.find_first_of(delimiters, lastPos);
        }

        return tokens.size() > 0;
    }

    /**
     * @brief Matches a string against a regular expression.
     * 
     * @param haystack The string to compare
     * @param needle The expression to compare against
     * 
     * @return true If the string matches the expression
     * @return false Otherwise
     */
    bool regexMatch(const char* haystack, const char* needle) {
        int nRet;
        regex_t sRegEx;

        if (regcomp(&sRegEx, needle, REG_EXTENDED | REG_NOSUB) != 0) {
            return false;
        }

        nRet = regexec(&sRegEx, haystack, (size_t) 0, NULL, 0);
        regfree(&sRegEx);

        if (nRet != 0) {
            return false;
        }

        return true;
    }

    /**
     * @brief Trims the beginning of a given string.
     *
     * @param nonTrimmed The non-trimmed string.
     * @param trimChar The character to trim off the string. (default: space)
     *
     * @return The trimmed string.
     */
    string trimStart(string nonTrimmed, const string& trimChar) {
        function<bool(char)> shouldTrimChar = [=](char c) -> bool { return trimChar.size() == 0 ? isspace(c) : trimChar.find(c) != string::npos; };

        nonTrimmed.erase(nonTrimmed.begin(), find_if(nonTrimmed.begin(), nonTrimmed.end(), not1(shouldTrimChar)));

        return nonTrimmed;
    }

    /**
     * @brief Trims the end of a given string.
     * @param nonTrimmed The non-trimmed string.
     * @param trimChar The character to trim off the string. (default: space)
     *
     * @return The trimmed string.
     */
    string trimEnd(string nonTrimmed, const string& trimChar) {
        function<bool(char)> shouldTrimChar = [=](char c) -> bool { return trimChar.size() == 0 ? isspace(c) : trimChar.find(c) != string::npos; };
        nonTrimmed.erase(find_if(nonTrimmed.rbegin(), nonTrimmed.rend(), not1(shouldTrimChar)).base(), nonTrimmed.end());

        return nonTrimmed;
    }

    /**
     * @brief Trims both the beginning and the end of a given string.
     *
     * @param nonTrimmed The non-trimmed string.
     * @param trimChar The character to trim off the string. (default: space)
     *
     * @return The trimmed string.
     */
    string trim(const string& nonTrimmed, const string& trimChar) { return trimStart(trimEnd(nonTrimmed, trimChar), trimChar); }

    /**
     * @brief Gets a value indicating whether or not a givern directory entry is gzipped or not.
     * 
     * @param entry A const ref to a directory_entry object.
     * 
     * @return true If the file is gzipped
     * @return false Otherwise
     */
    bool isGzipped(const fs::directory_entry& entry) {
        if (entry.file_size() < 2) { return false; }

        ifstream input(entry.path());

        char buffer[2] = {0};
        input.read(&buffer[0], 2);

        return buffer[0] == 0x1f && buffer[1] == 0x8b;
    }

}

#endif