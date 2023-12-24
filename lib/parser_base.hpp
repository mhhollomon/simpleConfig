#pragma once

#include "parser_utils.hpp"

#include <string_view>

namespace simpleConfig {
    struct ParserBase {

        std::string_view src;

        parse_loc current_loc;

        error_list errors;

        ParserBase(std::string_view _src) : src{_src},
            current_loc{_src, 0, 0} {}

        /***********************************************************
         * Error utilities
         ***********************************************************/

        void record_error(const std::string &msg, const parse_loc &l) {
            errors.add(msg, l);
        }

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

        inline bool check_string_end(int pos = 0) {
            return (
                (not valid_pos(pos)) || not ( 
                std::isalnum(peek(pos)) ||
                peek(pos) == '_'
                )
            );
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

        bool match_char(int pos, char c) {
            return (valid_pos(pos) and peek(pos) == c);
        }

        bool match_char(char c) {
            return peek() == c;
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

    };

}