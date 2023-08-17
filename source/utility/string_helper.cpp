#include "string_helper.h"

namespace sigma::detail {
    std::string format_ending(
        u64 count,
        const std::string& singular,
        const std::string& plural
    ) {
        return count == 1 ? singular : plural;
    }

    std::string create_caret_underline(
        const std::string& str, 
        u64 start,
        u64 end
    ) {
        std::string underline;

        for (u64 i = 0; i < str.length(); i++) {

            if (i >= start && i < end) {
                underline += "^";
            }
            else {
                // account for tabs, newlines, and carriage returns
                switch (str[i]) {
                case '\t': underline += "\t"; break; // tab
                case '\n': underline += "\n"; break; // newline
                case '\r': underline += "\r"; break; // carriage return
                default:   underline += " ";  break; // other characters
                }
            }
        }

        return underline;
    }

    outcome::result<u64> string_to_hex(
        const std::string& str
    ) {
        std::stringstream ss;
        ss << std::hex << str;

        u64 hex_value;
        if (!(ss >> (hex_value))) {
            return outcome::failure(
                error::emit<error_code::string_to_hexadecimal_failed>(str)
            );
        }

        return hex_value;
    }

    bool is_hex(char c) {
        return std::isdigit(c) || (std::tolower(c) >= 'a' && std::tolower(c) <= 'f');
    }

    bool is_bin(char c) {
        return c == '0' || c == '1';
    }

    std::string escape_string(
        const std::string& input
    ) {
        // todo: update to support various hexadecimal and binary strings
        std::string output;
        for (const char ch : input) {
            if (ch == '\\' || ch == '\'' || ch == '\"' || ch == '\a' || ch == '\b' || ch == '\f' || ch == '\n' || ch == '\r' || ch == '\t' || ch == '\v' || ch == '\x1b') {
                output.push_back('\\');
                switch (ch) {
                case '\\': output.push_back('\\'); break;
                case '\'': output.push_back('\''); break;
                case '\"': output.push_back('\"'); break;
                case '\a': output.push_back('a'); break;
                case '\b': output.push_back('b'); break;
                case '\f': output.push_back('f'); break;
                case '\n': output.push_back('n'); break;
                case '\r': output.push_back('r'); break;
                case '\t': output.push_back('t'); break;
                case '\v': output.push_back('v'); break;
                case '\x1b':
                    output.append("x1b");
                    break;
                }
            }
            else {
                output.push_back(ch);
            }
        }
        return output;
    }

    std::pair<u64, std::string> remove_leading_spaces(
        const std::string& str
    ) {
        u64 first_non_space = 0;

        while (first_non_space < str.size() && std::isspace(str[first_non_space])) {
            ++first_non_space;
        }

        return { first_non_space, str.substr(first_non_space) };
    }
}