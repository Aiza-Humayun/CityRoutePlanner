/*
 * =============================================================================
 * File    : Json.h
 * Part    : C (Foundations, Interface, Web)
 * Purpose : Small helpers that build the JSON text the engine prints.
 *           Every reply of the engine is ONE line of JSON, and these four
 *           functions are the only place where quotes, escapes and number
 *           formatting are handled, so everyone's replies look the same.
 * Course  : Lab 1 - basic functions and strings (no STL containers needed).
 *
 * FUNCTIONS (interface frozen in Section 4.7 of the team plan)
 *   jsonQuote(s)     "abc" with quotes; escapes " and \ (and line breaks)
 *   jsonError(msg)   {"ok":false,"error":"msg"}
 *   jsonMessage(msg) {"ok":true,"message":"msg"}
 *   numToStr(v)      12.5 -> "12.5", 3.0 -> "3", at most 2 decimals
 *
 * All functions are 'inline' so this header can be included by many files
 * without linker errors ("multiple definition").
 * =============================================================================
 */
#pragma once
#include <string>

/*
 * jsonQuote: turns plain text into a JSON string literal.
 * Input : any text.
 * Output: the text wrapped in double quotes, with these characters escaped:
 *           "  ->  \"        \  ->  \\
 *           line feed, carriage return, tab -> \n  \r  \t
 *           any other control character (below space) -> \u00XX
 *         Escaping line breaks matters: a raw line break would split the reply
 *         into two lines and break the Python bridge.
 * Time  : O(length of s)
 */
inline std::string jsonQuote(const std::string& s) {
    static const char* HEX_DIGITS = "0123456789abcdef";
    std::string out = "\"";
    for (size_t i = 0; i < s.size(); i++) {
        char c = s[i];
        unsigned char uc = (unsigned char)c;
        if (c == '"')       out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else if (c == '\t') out += "\\t";
        else if (uc < 0x20) {                       // other control characters
            out += "\\u00";
            out += HEX_DIGITS[uc >> 4];
            out += HEX_DIGITS[uc & 0x0F];
        }
        else out += c;                              // normal character (UTF-8 bytes pass through)
    }
    out += "\"";
    return out;
}

/*
 * jsonError: failure reply.  jsonError("Unknown location: Sadar")
 *            gives {"ok":false,"error":"Unknown location: Sadar"}
 * Time: O(length of msg)
 */
inline std::string jsonError(const std::string& msg) {
    return "{\"ok\":false,\"error\":" + jsonQuote(msg) + "}";
}

/*
 * jsonMessage: simple success reply.  jsonMessage("Location added")
 *              gives {"ok":true,"message":"Location added"}
 * Time: O(length of msg)
 */
inline std::string jsonMessage(const std::string& msg) {
    return "{\"ok\":true,\"message\":" + jsonQuote(msg) + "}";
}

/*
 * numToStr: number to text with AT MOST 2 decimals and no trailing zeros.
 *   12.5 -> "12.5"    3.0 -> "3"    3.456 -> "3.46"    -0.001 -> "0"
 * We do the rounding by hand (multiply by 100, round, split) so we need no
 * extra headers and the result never uses scientific notation.
 * NaN or absurdly large values (beyond +-1e15) give "0" instead of garbage.
 * Time: O(1)
 */
inline std::string numToStr(double v) {
    if (v != v || v > 1e15 || v < -1e15) return "0";   // v != v is true only for NaN

    bool negative = v < 0;
    if (negative) v = -v;

    long long scaled = (long long)(v * 100.0 + 0.5);   // value in hundredths, rounded
    if (scaled == 0) negative = false;                 // avoid "-0"

    long long whole = scaled / 100;                    // part before the decimal point
    int frac = (int)(scaled % 100);                    // the two decimal digits as a number

    std::string out = negative ? "-" : "";
    out += std::to_string(whole);
    if (frac != 0) {
        out += ".";
        out += (char)('0' + frac / 10);                // first decimal digit
        if (frac % 10 != 0) out += (char)('0' + frac % 10);   // second one only if not zero
    }
    return out;
}
