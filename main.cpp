/**
 * @file main.cpp
 * @author Simon Cahill (simon@simonc.eu)
 * @brief Simple C++ program that reads auth logs generated by Apache httpd and creates markdown-formatted output
 * @version 0.1
 * @date 2022-01-08
 * 
 * @copyright Copyright (c) 2022 Simon Cahill
 */

#include <iostream>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <errno.h>
#include <regex.h>
#include <string.h>

using std::cerr;
using std::cout;
using std::endl;
using std::function;
using std::getline;
using std::ifstream;
using std::istream;
using std::make_pair;
using std::map;
using std::pair;
using std::string;
using std::stringstream;
using std::vector;

namespace fs = std::filesystem;

/**
 * @brief Enumeration of possible HTTP response/status codes.
 */
enum HttpResponseCode {
    // Info
    CONTINUE                    = 100,
    SWITCHING_PROTOCOLS         = 101,
    PROCESSING                  = 102,
    EARLY_HINTS                 = 103,

    // Success
    OK                          = 200,
    CREATED                     = 201,
    ACCEPTED                    = 202,
    NON_AUTH_INFORMATION        = 203,
    NO_CONTENT                  = 204,
    RESET_CONTENT               = 205,
    PARTIAL_CONTENT             = 206,
    MULTI_STATUS                = 207,
    ALREADY_REPORTED            = 208,
    IM_USED                     = 226,

    // Redirection
    MULTIPLE_CHOICE             = 300,
    MOVED_PERMANENTLY           = 301,
    FOUND                       = 302,
    SEE_OTHER                   = 303,
    NOT_MODIFIED                = 304,
    USE_PROXY                   = 305,
    UNUSED_306                  = 306,
    TEMPORARY_REDIRECT          = 307,
    PERMANENT_REDIRECT          = 308,

    // Client error
    BAD_REQUEST                 = 400,
    UNAUTHORIZED                = 401,
    PAYMENT_REQUIRED            = 402,
    FORBIDDEN                   = 403,
    NOT_FOUND                   = 404,
    METHOD_NOT_ALLOWED          = 405,
    NOT_ACCEPTABLE              = 406,
    PROXY_AUTH_REQUIRED         = 407,
    CONFLICT                    = 409,
    GONE                        = 410,
    LENGTH_REQUIRED             = 411,
    PRECONDITION_FAILED         = 412,
    PAYLOAD_TOO_LARGE           = 413,
    URI_TOO_LONG                = 414,
    UNSUPPORTED_MEDIA_TYPE      = 415,
    RANGE_NOT_SATISFIABLE       = 416,
    EXPECTATION_FAILED          = 417,
    IM_A_TEAPOT                 = 418,
    MISDIRECTED_REQUEST         = 421,
    UNPROCESSABLE_ENTITY        = 422,
    LOCKED                      = 423,
    FAILED_DEPENDENCY           = 424,
    TOO_EARLY                   = 425,
    UPGRADE_REQUIRED            = 426,
    PRECONDITION_REQURED        = 428,
    TOO_MANY_REQUESTS           = 429,
    HEADER_FIELD_TOO_LARGE      = 431,
    UNAVAIL_FOR_LEGAL_REASONS   = 451,

    // Server error
    INTERNAL_SERVER_ERROR       = 500,
    NOT_IMPLEMENTED             = 501,
    BAD_GATEWAY                 = 502,
    SERVICE_UNAVAILABLE         = 503,
    GATEWAY_TIMEOUT             = 504,
    HTTP_VER_NOT_SUPPORTED      = 505,
    VARIANT_ALSO_NEGOTIATES     = 506,
    INSUFFICIENT_STORAGE        = 507,
    LOOP_DETECTED               = 508,
    NOT_EXTENDED                = 510,
    NETWORK_AUTH_REQUIRED       = 511
};

/**
 * @brief Contains data and information about clients accessing the web server.
 */
struct Connection {
    string clientSource; //!< IP address or URL
    string clientId; //!< Just here for completeness' sake. Unreliable information: see https://httpd.apache.org/docs/2.4/logs.html
    string userId; //!< For password-protected files. Should not be trusted otherwise
    string timestamp; //!< Obvious
    string httpRequestMethod; //!< Again, obvious
    string requestUri; //!< e.g.: /myawesomepage.php
    string httpVersion; //!< e.g. HTTP/1.1
    int32_t httpStatusCode; //!< The status code returned to the client
    int64_t responseSize; //!< The size (in B) of the response sent back to the client w/o headers

    Connection() {
        clientSource = string();
        clientId = string();
        userId = string();
        timestamp = string();
        httpRequestMethod = string();
        requestUri = string();
        httpVersion = string();
        httpStatusCode = 0;
        responseSize = 0;
    }

    Connection(const Connection& ref) {
        clientSource = ref.clientSource;
        clientId = ref.clientId;
        userId = ref.userId;
        timestamp = ref.timestamp;
        httpRequestMethod = ref.httpRequestMethod;
        requestUri = ref.requestUri;
        httpVersion = ref.httpVersion;
        httpStatusCode = ref.httpStatusCode;
        responseSize = ref.responseSize;
    }

    string toMarkdownRow() const {
        stringstream stream;

        stream << "|" << clientSource
               << "|" << clientId
               << "|" << userId
               << "|" << timestamp
               << "|" << httpRequestMethod
               << "|" << requestUri
               << "|" << httpVersion
               << "|" << httpStatusCode
               << "|" << responseSize
               << "|" << endl;

        return stream.str();
    }
};

static bool     g_error = false; //!< Set to true if an error occurs
static bool     g_printUniqueIps = true; //!< Whether or not to print a list of unique IDs
static bool     g_printConnectionStats = true; //!< Whether or not to print the connection stats table
static bool     g_readFromStdIn = false; //!< Whether or not to read from stdin
static string   g_logPath = "/var/log/apache2"; //!< The path to the httpd logs

bool                                    isGzipped(const fs::directory_entry& dir);
bool                                    splitString(const string& str, const string& delimiters, vector<string> &tokens); //!< Splits a string by one or more delimiters
bool                                    regexMatch(const char* haystack, const char* needle); //!< Matches a string against a regular expression
Connection                              parseConnectionFromLine(const string&); //!< Attempts to parse a connection out of a log line from httpd
int32_t                                 countHttpResults(const vector<Connection>&, const HttpResponseCode&); //!< Counts the total number of HTTP status/result codes for a given IP
map<string, vector<string>>             readLogFiles(); //!< Reads the log file into memory
map<string, vector<Connection>>         getConnectionAttempts(const vector<string>&); //!< Parses all connections
pair<string, string>                   getSpacerStrings(const uint32_t& width, const string& centreString); //!< Creates two spacer strings to centre a string
string                                  getLongestClientSource(const map<string, vector<Connection>>&, const int32_t& maxWidth = 80); //!< Gets the appropriate width of the client src column
string                                  trimStart(string nonTrimmed, const string& trimChar); //!< Trims the start of a string
string                                  trimEnd(string nonTrimmed, const string& trimChar); //!< Trims the end of a string
string                                  trim(string nonTrimmed, const string& trimChar); //!< Trims a string
void                                    printUniqueIpStats(const map<string, vector<Connection>>&);

int main(int32_t argc, char** argv) {
    const auto lines = readLogFiles();

    map<string, vector<Connection>> totalConnections;

    for (auto iterator = lines.begin(); iterator != lines.end(); iterator++) {
        const auto connections = getConnectionAttempts(iterator->second);

        for (const auto connection : connections) {
            map<string, vector<Connection>>::iterator iterator;
            if ((iterator = totalConnections.find(connection.first)) == totalConnections.end()) {
                totalConnections.emplace(connection);
                continue;
            }

            iterator->second.insert(iterator->second.end() - 1, connection.second.begin(), connection.second.end());
        }
    }

    int totalUniqueIps = totalConnections.size();
    cout << "# HTTPD Report" << endl
         << "## Total Unique IPs: " << totalUniqueIps << endl << endl;

    printUniqueIpStats(totalConnections);
}

/**
 * @brief Parses the log file and retrieves all valid and invalid connection attempts.
 * 
 * @param logLines A vector containing the individual lines of the log file
 * 
 * @return map<string, vector<ConnectionAttempt>> A map containing a table of all unique IPs and their connection attempts.
 * 
 * @remarks For now, this assumes the standard HTTPD log format: "%h %l %u %t \"%r\" %>s %b" common
 */
map<string, vector<Connection>> getConnectionAttempts(const vector<string>& logLines) {
    map<string, vector<Connection>> connections;

    for (const auto& line : logLines) {
        if (line.empty()) { continue; }
        try {
            const Connection connection = parseConnectionFromLine(line);

            if (connections.find(connection.clientSource) == connections.end()) {
                connections.emplace(connection.clientSource, vector<Connection>({ connection }));
                continue;
            }

            connections[connection.clientSource].push_back(connection);
        } catch (...) {
            cerr << "Failed to parse connection info. Skipping..." << endl;
        }
    }

    return connections;
}

map<string, vector<string>> readLogFiles() {
    map<string, vector<string>> output;

    auto _ = [](std::istream& stream, vector<string>& output) {
        string line;
        while (std::getline(stream, line)) {
            if (line.find("HTTP/1.1") != string::npos) {
                output.push_back(line);
            }
        }
    };

    if (g_readFromStdIn) {
        output.emplace("stdin", vector<string>());
        _(std::cin, output["stdin"]);
    } else {
        if (fs::is_directory(g_logPath)) {
            for (const auto& entry : fs::directory_iterator(g_logPath)) {
                if (!entry.is_regular_file() || !entry.exists()) { continue; } // skip all non-file entries
                if (isGzipped(entry)) {
                    cerr << "Gzipped file detected! Will ignore. Use stdin and zcat." << endl;
                    // skip gzipped entries; can't be bothered to unzip files here. Use stdin for that
                    continue;
                }

                ifstream fileStream(entry.path());
                if (!fileStream.good()) {
                    cerr << "Failed to open " << entry.path() << endl;
                    cerr << "Error: " << strerror(errno) << endl;
                }

                output.emplace(entry.path(), vector<string>());
                _(fileStream, output[entry.path()]);
            }
        }
    }

    return output;
}

Connection parseConnectionFromLine(const string& line) {
    Connection connection;
    size_t offset = 0;
    size_t offset2 = 0;

    connection.clientSource = line.substr(offset, (offset2 = line.find(" ")));
    offset = offset2 + 1;
    offset2 = line.find(' ', offset);
    connection.clientId = line.substr(offset, offset2 - offset);
    offset = offset2 + 1;
    offset2 = line.find(' ', offset);
    connection.userId = line.substr(offset, offset2 - offset);
    offset = line.find('[', offset2 + 1) + 1; // don't include the [ in the string
    offset2 = line.find(']', offset);
    connection.timestamp = line.substr(offset, offset2 - offset);
    offset = line.find('"', offset2 + 1) + 1;
    offset2 = line.find('"', offset + 1);

    {
        vector<string> tokens; // split the REQUEST by whitespace
        splitString(line.substr(offset, offset2 - offset), " \t\n\r", tokens);
        connection.httpRequestMethod = tokens[0];
        connection.requestUri = tokens[1];
        connection.httpVersion = tokens[2];
    }

    offset = line.find(' ', offset2);
    offset2 = line.find(' ', offset + 1);
    connection.httpStatusCode = std::atoi(line.substr(offset, offset2 - offset).c_str());
    offset = offset2 + 1;
    connection.responseSize = std::atoi(line.substr(offset).c_str());
   
    return connection;
}

int32_t countHttpResults(const vector<Connection>& connectionList, const HttpResponseCode& statusCode) {
    return std::count_if(connectionList.begin(), connectionList.end(), [&](const Connection& conn) {
        return conn.httpStatusCode == statusCode;
    });
}

string getLongestClientSource(const map<string, vector<Connection>>& totalConnections, const int32_t& maxWidth) {
    string largestString;

    // iterate through all IPs and their respective entries, find the longest string up to maxWidth
    for (const auto& ipMap : totalConnections) {
        if (ipMap.first.length() <= maxWidth && ipMap.first.length() > largestString.length()) {
            largestString = ipMap.first;
        } else if (ipMap.first.length() == maxWidth) {
            return largestString;
        } else if (ipMap.first.length() > maxWidth) {
            break;
        }
    }

    // the list was empty? return an empty string with maxWidth spaces
    return largestString.empty() ? string(' ', maxWidth) : largestString;
}

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
string trim(string nonTrimmed, const string& trimChar) { return trimStart(trimEnd(nonTrimmed, trimChar), trimChar); }

pair<string, string> getSpacerStrings(const uint32_t& width, const string& centreString) {
    string str1 = string(width / 2 - centreString.length() / 2, ' ');
    string str2 = string(width - str1.length() - centreString.length(), ' ');

    return make_pair<string, string>(std::move(str1), std::move(str2));
}

/**
 * @brief Gets a value indicating whether or not a givern directory entry is gzipped or not.
 * 
 * @param entry A const ref to a directory_entry object.
 * 
 * @return true If the file is gzipped
 * @return false Otherwise
 */
bool isGzipped(const fs::directory_entry& entry) {
    ifstream input(entry.path());

    char buffer[2] = {0};
    input.read(&buffer[0], 2);

    return buffer[0] == 0x1f && buffer[1] == 0x8b;
}


void printUniqueIpStats(const map<string, vector<Connection>>& ipMap) {
    const static string HEADER_CLIENT_SRC = "Source";

    for (const auto& mapEntry : ipMap) {
        pair<string, string> sepStrings;

        // cout << "# " << mapEntry.first << endl;

        if (HEADER_CLIENT_SRC.length() < mapEntry.first.length()) {
            sepStrings = getSpacerStrings(mapEntry.first.length(), HEADER_CLIENT_SRC);
        } else {
            sepStrings = getSpacerStrings(HEADER_CLIENT_SRC.length() + 2, HEADER_CLIENT_SRC);
        }

        cout << "|" << sepStrings.first << HEADER_CLIENT_SRC << sepStrings.second
             << "| Total 200 | Total 204 | Total 301 | Total 400 | Total 401 | Total 403 | Total 404 | Total 500 | Total 503 |" << endl
             << "|" << string(sepStrings.first.length() + HEADER_CLIENT_SRC.length() + sepStrings.second.length(), '-')
             << "|-----------|-----------|-----------|-----------|-----------|-----------|-----------|-----------|-----------|" << endl;

        uint32_t total200 = 0;
        uint32_t total204 = 0;
        uint32_t total301 = 0;
        uint32_t total400 = 0;
        uint32_t total401 = 0;
        uint32_t total403 = 0;
        uint32_t total404 = 0;
        uint32_t total500 = 0;
        uint32_t total503 = 0;

        for (const auto& connection : mapEntry.second) {
            switch (connection.httpStatusCode) {
                case OK:                    total200++;     break;
                case NO_CONTENT:            total204++;     break;
                case MOVED_PERMANENTLY:     total301++;     break;
                case BAD_REQUEST:           total400++;     break;
                case UNAUTHORIZED:          total401++;     break;
                case FORBIDDEN:             total403++;     break;
                case NOT_FOUND:             total404++;     break;
                case INTERNAL_SERVER_ERROR: total500++;     break;
                case SERVICE_UNAVAILABLE:   total503++;     break;
                default:                                    break;
            }
        }
        
        cout << "|" << mapEntry.first;

        string tmp = std::to_string(total200);
        sepStrings = getSpacerStrings(11, tmp);
        cout << "|" << sepStrings.first << tmp << sepStrings.second;

        tmp = std::to_string(total204);
        sepStrings = getSpacerStrings(11, tmp);
        cout << "|" << sepStrings.first << tmp << sepStrings.second;

        tmp = std::to_string(total301);
        sepStrings = getSpacerStrings(11, tmp);
        cout << "|" << sepStrings.first << tmp << sepStrings.second;

        tmp = std::to_string(total400);
        sepStrings = getSpacerStrings(11, tmp);
        cout << "|" << sepStrings.first << tmp << sepStrings.second;

        tmp = std::to_string(total401);
        sepStrings = getSpacerStrings(11, tmp);
        cout << "|" << sepStrings.first << tmp << sepStrings.second;

        tmp = std::to_string(total403);
        sepStrings = getSpacerStrings(11, tmp);
        cout << "|" << sepStrings.first << tmp << sepStrings.second;

        tmp = std::to_string(total404);
        sepStrings = getSpacerStrings(11, tmp);
        cout << "|" << sepStrings.first << tmp << sepStrings.second;

        tmp = std::to_string(total500);
        sepStrings = getSpacerStrings(11, tmp);
        cout << "|" << sepStrings.first << tmp << sepStrings.second;

        tmp = std::to_string(total503);
        sepStrings = getSpacerStrings(11, tmp);
        cout << "|" << sepStrings.first << tmp << sepStrings.second;

        cout << "|" << endl << endl << "----------" << endl << endl;
    }
}
