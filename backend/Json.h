
 
#pragma once
#include <string>


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
