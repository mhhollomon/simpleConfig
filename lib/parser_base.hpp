#pragma once

#include "parser_utils.hpp"

#include "error_reporter.hpp"

#include <string>
#include <string_view>
#include <optional>
#include <vector>
#include <iostream>
#include <charconv>

#if NDEBUG

#define ENTER

#define RETURN(retval) { return retval; }

#define RETURN_B(retval) { return retval; }

#define RETURN_NULLOPT { return std::nullopt; }

#else

#define ENTER std::cout << "Entering function " << __FUNCTION__ \
        << " offset = " << current_loc.offset << "\n"

#define RETURN(retval) { std::cout << "leaving function " << __FUNCTION__ \
        << " returning " << retval << "\n"; \
        return retval; }

#define RETURN_B(retval) { std::cout << "leaving function " << __FUNCTION__ \
        << " returning " << std::boolalpha << retval << "\n"; \
        return retval; }


#define RETURN_NULLOPT { std::cout << "leaving function " << __FUNCTION__ \
        << " returning nullopt\n"; \
        return std::nullopt; }

#endif

namespace simpleConfig {
    struct ParserBase : public ErrorReporter {

        std::string_view src_text;

        parse_loc current_loc;


        ParserBase(std::string_view text, const std::string &parser_name,
            error_list &errlist) :
            
            ErrorReporter(parser_name, errlist), 
            src_text{text},
            current_loc{text, 0, 0}
        {}

        using ErrorReporter::record_error;

        void record_error(const std::string & msg) {
            record_error(msg, current_loc);
        }



        /***********************************************************
         * Input utilities
         ***********************************************************/

        inline void consume(int count) {
            // note : others are responsible for line number
            current_loc.offset += count;
            current_loc.sv.remove_prefix(count);
        }

        inline bool eoi() { return current_loc.sv.size() == 0; }

        inline char peek() { if (!eoi()) { return current_loc.sv[0]; }
            else { return '\x00'; } }

        inline char peek(int pos) { return current_loc.sv[pos]; }

        inline bool valid_pos(int pos) {
            return (pos >= 0 and (unsigned)pos < current_loc.sv.size()); 
        }

        inline bool check_string(std::string_view o) {
            return current_loc.sv.compare(0, o.size(), o) == 0;
        }
        
        inline bool match_string(std::string_view o) {
            bool matched = check_string(o);
            if (matched) consume(o.size());
            return matched;
        }

        inline bool match_sep(int pos = 0) {
            return match_chars(pos, ";,");
        }

        void optional_sep() {
            skip();
            if (match_sep())
                consume(1);
        }

        inline bool check_string_end(int pos = 0) {
            return (
                (not valid_pos(pos)) || not ( 
                std::isalnum(peek(pos)) ||
                peek(pos) == '_'
                )
            );
        }

        inline bool match_keyword(std::vector<std::string_view> v) {
            ENTER;
            auto loc = current_loc;
            for (auto const &e : v ) {
                if (match_string(e)) {
                    if (check_string_end()) {
                        RETURN(true);
                        return true;
                    }
                    current_loc = loc;
                }
            }

            RETURN(false);
            return false;
        }

        //##############   match_name #######################
        // Used to match the key for a key/value pair
        // When in schema_mode a single '*' is valid
        // as is a name beginning with '_'.
        // Otherwise, it must start with a letter
        // and only consist of letters, numbers and underbars.
        std::optional<std::string> match_name(bool schema_mode = false) {

            int pos = 0;
            if (schema_mode && peek(pos) == '*') {
                if (!check_string_end(1))
                    return std::nullopt;
                consume(1);
                
                return std::string("*");

            } else if (std::isalpha(peek(pos))) {
                pos += 1;
            } else {
                return std::nullopt;
            }

            while (valid_pos(pos)) {
                if ( match_chars(pos, "_-") or
                        std::isalnum(peek(pos))) {
                    pos += 1;
                } else break;
            }


            auto retval = current_loc.sv.substr(0, pos);
            consume(pos);

            return std::string(retval);
        }

        //##############   CHAR utils #######################

        bool match_char(int pos, char c) {
            return (valid_pos(pos) and peek(pos) == c);
        }

        bool match_char(char c) {
            return peek() == c;
        }

        bool expect_char(char c) {
            if (match_char(c)) {
                consume(1);
                return true;
            }

            return false;
        }

        bool match_chars(int pos, const char * c) {
            if (!valid_pos(pos)) return false;
            char t  = peek(pos);

            while (*c) {
                if (*c == t) return true;
                ++c;
            }

            return false;
        }

        //##############   match_integer_value  #################
        // handles both base 10 and hex
        std::optional<long> match_integer_value() {

            skip();
            ENTER;

            if (match_string("0x") or match_string("0X")) {
                // integer in hex format - from_chars doesn't like the 0x prefix
                // can't be anything else so commit.
                long seen_num;
                auto [ ptr, ec] = std::from_chars(current_loc.sv.data()+2,
                        current_loc.sv.data()+current_loc.sv.size(), seen_num, 16);
                if (ec == std::errc()) {
                    int pos = ptr-current_loc.sv.data();
                    if (valid_pos(pos) and std::isalnum(*ptr)) {
                        // End of the number wasn't at a word boundary.
                        record_error("Hex prefix, but invalid hex number followed");
                        RETURN_NULLOPT
                    }
                    consume(pos);
                    RETURN(seen_num);
                } else {
                    record_error("Hex prefix, but invalid hex number followed");
                    RETURN_NULLOPT
                }

            } else if (match_chars(0, "+-0123456789")) {
                // either a base-10 integer or float.

                int copy_size = std::min(current_loc.sv.size(), size_t(100));
                std::string subject(std::string(current_loc.sv.substr(0, copy_size)));

                try {
                    size_t pos = 0;
                    long seen_num = std::stol(subject, &pos);

                    if (valid_pos(pos) and (std::isalnum(peek(pos)) or match_char(pos, '.'))) {
                        // must be at a word boundary.
                        RETURN_NULLOPT
                    } else {
                        consume(pos);
                        RETURN(seen_num);
                    }
                } catch(std::exception &e) {
                    RETURN_NULLOPT
                }

                RETURN_NULLOPT

            }

            RETURN_NULLOPT
        }

        //##############   match_double_value  #################
        
        std::optional<double>  match_double_value() {

            skip();
            ENTER;

            if (match_chars(0, "+-0123456789")) {
                // either a base-10 integer or float.

                int copy_size = std::min(current_loc.sv.size(), size_t(100));
                std::string subject(std::string(current_loc.sv.substr(0, copy_size)));

                try {
                    size_t pos = 0;
                    double seen_num = std::stod(subject, &pos);
                    if (valid_pos(pos) and std::isalnum(peek(pos))) {
                        RETURN_NULLOPT;
                    } else {
                        consume(pos);
                        RETURN(seen_num);
                    }
                } catch(std::exception &e) {
                    RETURN_NULLOPT;
                }

                RETURN_NULLOPT;
            }

            RETURN_NULLOPT;
        }

        //##############   match_bool_value  #################

        std::optional<bool> match_bool_value() {

            skip();
            ENTER;

            // if there aren't enough chars then short circuit
            if (!valid_pos(3)) {
                RETURN_NULLOPT;
            }

            int pos = 0;
            if (match_chars(pos, "Ff")) {
                pos += 1;
                if (! match_chars(pos, "Aa")) RETURN_NULLOPT;
                pos += 1;
                if (! match_chars(pos, "Ll")) RETURN_NULLOPT;
                pos += 1;
                if (! match_chars(pos, "Ss")) RETURN_NULLOPT;
                pos += 1;
                if (! match_chars(pos, "Ee")) RETURN_NULLOPT; 
                pos += 1;
                // must be at end of word
                if (!check_string_end(pos)) RETURN_NULLOPT;

                consume(pos);
                RETURN_B(false);

            } else if (match_chars(pos, "Tt")) {
                pos += 1;
                if (! match_chars(pos, "Rr")) RETURN_NULLOPT; 
                pos += 1;
                if (! match_chars(pos, "Uu")) RETURN_NULLOPT;
                pos += 1;
                if (! match_chars(pos, "Ee")) RETURN_NULLOPT;
                pos += 1;
                // must be at end of word
                if (!check_string_end(pos)) RETURN_NULLOPT;

                consume(pos);
                RETURN_B(true);
            }

            RETURN_NULLOPT;

        }


        /****************************************************************
        * SKIP Processing
        ****************************************************************/
        bool skip() {

            //
            // States for the state machine
            //
            enum SKIP_STATE { skNormal, skLine, skBlock };

            SKIP_STATE state = skNormal;

            //
            // we'll be using this a lot.
            // So abbreviate it with a reference.
            //
            auto &cl = current_loc;

            //
            // When we enter a comment, we'll record where it started.
            // If, at the end, we're still looking for comment terminator,
            // we can use this location to put out an error message.
            //
            parse_loc comment_loc;

            bool skipping = true;
            int line_count = 0;
            while (skipping and not eoi()) {

                //std::cout << "Loop top pos = " << cl.offset <<
                //    " state = " << state <<
                //    " char = '" << peek() << "'\n";
                switch (state) {
                    case skNormal : // Normal state
                        if (peek(0) == '\n') {
                            line_count += 1;
                            consume(1);
                        } else if (std::isspace(peek(0))) {
                            consume(1);
                        } else if (peek(0) == '#') {
                            //std::cout << "--- Saw hash comment start\n";
                            comment_loc = cl;
                            consume(2);
                            state = skLine;
                        } else if (check_string("//")) {
                            //std::cout << "--- Saw line comment start\n";
                            comment_loc = cl;
                            consume(2);
                            state = skLine;
                        } else if (check_string("/*")) {
                            //std::cout << "--- Saw block comment start\n";
                            comment_loc = cl;
                            consume(2);
                            state = skBlock;
                        } else {
                            skipping = false;
                        }
                        break;
                    case skLine: // Line comment
                        if (peek(0) == '\n') {
                            //std::cout << "--- line comment end\n";
                            state = skNormal;
                            line_count += 1;
                        }
                        consume(1);
                        break;
                    case skBlock: // Block comment
                        if (match_string("*/")) {
                            //std::cout << "--- block comment end\n";
                            state = skNormal;
                        } else {
                            if (peek(0) == '\n') {
                                line_count += 1;
                            }
                            consume(1);
                        }
                        break;
                }
            }

            if (state != skNormal) {
                record_error("Unterminated comment starting here", comment_loc);
                return false;
            }

            current_loc.line += line_count;

            return true;
        };


    };

}