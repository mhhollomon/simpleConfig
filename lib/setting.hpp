#pragma once

#include "value_type.hpp"

#include <string>
#include <vector>
#include <map>
#include <exception>
#include <ctype.h>
#include <sstream>
#include <iostream>

using namespace std::literals::string_literals;

using namespace std::literals::string_view_literals;

namespace simpleConfig {
    // These make up the Config Tree that
    // we give to the user.
    class Setting {

        ValType type_;

        // scalar containers (maybe change to std::variant?)
        long integer_;
        double float_;
        bool bool_;
        std::string string_;


        // container for the composite types
        std::vector<Setting> children_;

        // lookup for groups. Allows at(std::string) to be O(1)
        // The int is an index into children_
        std::map<std::string, int> group_;

        // Arrays must all be the same type. Set when the first child is added to the array.
        ValType array_type_ = ValType::NONE;

        void clear_subobjects() {
            children_.clear();
            group_.clear();
            string_.clear();
        }

        template<class T>
        ValType deduce_scalar_type(T v) {
            if constexpr (std::is_same_v<T, bool>) {
                return ValType::BOOL;
            } else if constexpr (std::is_same_v<T, Setting>) {
                return ValType::GROUP;
            } else if constexpr (std::is_convertible_v<T, std::string>) {
                return ValType::STRING;
            } else if constexpr (std::is_integral_v<T>) {
                return ValType::INTEGER;
            } else if constexpr(std::is_floating_point_v<T>) {
                return ValType::FLOAT;
            } else {
                throw std::runtime_error("Could not deduce setting type from value");
            }
        }


    public :
        Setting(ValType t = ValType::BOOL) : type_{t} {}

        Setting(bool b) : type_(ValType::BOOL), bool_(b) {}
        Setting(int i) : type_(ValType::INTEGER), integer_(i) {}
        Setting(long l) : type_(ValType::INTEGER), integer_(l) {}
        Setting(double f) :  type_(ValType::FLOAT), float_(f) {}
        Setting(std::string s) : type_(ValType::STRING), string_(s) {}
        Setting(const char * c) : type_(ValType::STRING), string_(c) {}

        ValType get_type() const { return type_; }

        class group_iterator;
        class group_enumerator {
            friend class Setting;
            friend class group_iterator;
            Setting &parent_;

            group_enumerator(Setting &p) : parent_{p} {}

            public:

            group_iterator& begin() {
                return *(new group_iterator{parent_, parent_.group_.begin()});
            }

            group_iterator& end() {
                return *(new group_iterator(parent_, parent_.group_.end()));
            }
        };

        class group_iterator {
            friend class group_enumerator;
            Setting& parent_;
            std::map<std::string, int>::iterator it_;
            std::pair<std::string, Setting &> *output_ = nullptr;

            group_iterator(Setting &p, std::map<std::string, int>::iterator i) :
                parent_(p), it_(i) {

                    if (it_ != parent_.group_.end()) {
                        output_ = new std::pair<std::string, Setting &>(it_->first, 
                                std::ref(parent_.children_.at(it_->second)));
                    }
                }


            public:
                ~group_iterator() {
                    if (output_) delete output_;
                }
                std::pair<std::string, Setting &> operator*() {
                    if (! output_) throw std::runtime_error("invalid iterator");
                    return *output_;
                }
                std::pair<std::string, Setting &>* operator->() {
                    if (! output_) throw std::runtime_error("invalid iterator");
                    return output_;
                }

                group_iterator& operator++() {
                    ++it_;
                    if (output_) {
                        delete output_;
                        output_ = nullptr;
                    }

                    if (it_ != parent_.group_.end()) {
                        output_ = new std::pair<std::string, Setting &>(it_->first, 
                                std::ref(parent_.children_.at(it_->second)));
                    }
                    return *this;
                }

                bool operator==(const group_iterator *o) const {
                    return (it_ == o->it_);
                }
                bool operator==(const group_iterator &o) const {
                    return (it_ == o.it_);
                }
                bool operator!=(const group_iterator *o) const {
                    return (it_ != o->it_);
                }
                bool operator!=(const group_iterator &o) const {
                    return (it_ != o.it_);
                }

        };

        group_enumerator& enumerate() {
            if (!is_group()) {
                throw std::runtime_error("Can only enumerate groups");
            }

            return *(new group_enumerator(*this));
        }

        Setting & set_value(bool b) {
            if (!is_boolean()) {
                clear_subobjects();
                type_ = ValType::BOOL;
            }
            bool_ = b;
            return *this;
        }

        Setting & set_value(int i) {
            if (!is_integer()) {
                clear_subobjects();
                type_ = ValType::INTEGER;
            }
            integer_ = i;
            return *this;
        }

        Setting & set_value(long i) {
            if (!is_integer()) {
                clear_subobjects();
                type_ = ValType::INTEGER;
            }
            integer_ = i;
            return *this;
        }
        
        Setting & set_value(double f) {
            if (!is_float()) {
                clear_subobjects();
                type_ = ValType::FLOAT;
            }
            float_ = f;
            return *this;
        }


        Setting & set_value(const std::string s) {
            if (!is_string()) {
                clear_subobjects();
                type_ = ValType::STRING;
            }
            string_ = s;
            return *this;
        }

        Setting & set_value(const char *c) {
            if (!is_string()) {
                clear_subobjects();
                type_ = ValType::STRING;
            }
            string_ = c;
            return *this;
        }

        bool is_boolean() const { return (type_ == ValType::BOOL); }
        bool is_integer() const { return (type_ == ValType::INTEGER); }
        bool is_float()   const { return (type_ == ValType::FLOAT); }
        bool is_string()  const { return (type_ == ValType::STRING); }
        bool is_group()   const { return (type_ == ValType::GROUP); }
        bool is_list()    const { return (type_ == ValType::LIST); }
        bool is_array()   const { return (type_ == ValType::ARRAY); }

        bool is_numeric()   const { return (is_integer() or is_float()); }
        bool is_composite() const { return (valtype_is_composite(type_)); }
        bool is_scalar()    const { return (valtype_is_scalar(type_)); }

        void make_list() {
            if (!is_list()) {
                clear_subobjects();
                type_ = ValType::LIST;
            }
        }

        void make_group() {
            if (!is_group()) {
                clear_subobjects();
                type_ = ValType::GROUP;
            }
        }

        void make_array() {
            if (! is_array()) {
                clear_subobjects();
                type_ = ValType::ARRAY;
            }
        }

        bool exists(const std::string child) const {
            if (! is_group()) return false;

            auto const &iter = group_.find(child);
            if (iter == group_.end()) 
                return false;
            else
                return true;
        }

        template<typename T> T get() const {
            if constexpr (std::is_same_v<T, bool>) {
                if (is_boolean()) {
                    return T(bool_);
                } else {
                    throw std::runtime_error("Bad type conversion\n");
                }
            } else if constexpr (std::is_integral_v<T>) {
                if (is_integer())
                    return T(integer_);
                else 
                    throw std::runtime_error("Bad type conversion\n");

            } else if constexpr(std::is_floating_point_v<T>) {
                if (is_float()) {
                    return T(float_);
                } else if (is_integer()) {
                    return T(integer_);
                } else {
                    throw std::runtime_error("Bad type conversion\n");
                }
            } else if constexpr (std::is_convertible_v<std::string, T>) {
                if (is_string()) {
                    return T(string_);
                } else {
                    throw std::runtime_error("Bad type conversion\n");
                }
            } else {
                throw std::runtime_error("Bad type conversion (not a scalar)\n");
            }

        }

        
        template<class T>
        Setting &add_child(T v) {

            if (is_group()) {
                throw std::runtime_error("Group children must have names");

            } else if (is_array()) {
                ValType target_type = deduce_scalar_type(v);
                if (children_.size() > 0) {
                    if (array_type_ != target_type) {
                        throw std::runtime_error("All children of arrays must be the same type");
                    }
                } else {
                    array_type_ = target_type;
                }
                return children_.emplace_back(v);

            } else if (is_list()) {
                return children_.emplace_back(v);

            } else {
                throw std::runtime_error("Setting must be composite to add child");
            }
        }

        Setting &add_child(ValType t) {
            if (is_group()) {
                throw std::runtime_error("Group children must have names");

            } else if (is_list()) {
                return children_.emplace_back(t);

            } else if (is_array()) {
                if (valtype_is_composite(t)) {
                    throw std::runtime_error("Arrays may only have scalar children");
                }

                if (children_.size() > 0) {
                    if (array_type_ != t) {
                        throw std::runtime_error("All children of arrays must be the same type");
                    }
                } else {
                    array_type_ = t;
                }
                return children_.emplace_back(t);
            } else {
                throw std::runtime_error("Setting must be composite to add child");
            }
        }
        
        template<class T>
        Setting &add_child( const std::string &name, T v) {

            if (!is_group()) {
                throw std::runtime_error("Only group children may have names");
            }

            auto [ _, done ] = group_.try_emplace(name, int(group_.size()));

            if (done) {
                // It didn't exists before
                return children_.emplace_back(v);
            } else {
                throw std::runtime_error("Child with given key "s + name + " already exists");

            }
        }

        Setting &add_child( const std::string &name, ValType t) {

            if (!is_group()) {
                throw std::runtime_error("Only group children may have names");
            }

            auto [ _, done ] = group_.try_emplace(name, int(group_.size()));

            if (done) {
                // It didn't exists before
                return children_.emplace_back(t);
            } else {
                throw std::runtime_error("Child with given key "s + name + " already exists");

            }
        }

        // ==== NON EXCEPTION VARIANTS
        template<class T>
        Setting* try_add_child(T v) {

            if (is_group()) {
                return nullptr;

            } else if (is_array()) {
                ValType target_type = deduce_scalar_type(v);
                if (children_.size() > 0) {
                    if (array_type_ != target_type) {
                        return nullptr;
                    }
                } else {
                    array_type_ = target_type;
                }
                return &(children_.emplace_back(v));

            } else if (is_list()) {
                return &(children_.emplace_back(v));

            } else {
                return nullptr;
            }
        }

        Setting* try_add_child(ValType t) {
            
            if (is_group()) {
                // group children must have a name
                return nullptr;

            } else if (is_list()) {
                return &(children_.emplace_back(t));

            } else if (is_array()) {
                if (valtype_is_composite(t) and t != ValType::GROUP) {
                    return nullptr;
                }

                if (children_.size() > 0) {
                    if (array_type_ != t) {
                        return nullptr;
                    }
                } else {
                    array_type_ = t;
                }
                return &(children_.emplace_back(t));
            } else {
                return nullptr;
            }
        }

        template<class T>
        Setting* try_add_child( const std::string &name, T v) {

            if (!is_group()) {
                return nullptr;
            }

            auto [ _, done ] = group_.try_emplace(name, group_.size());

            if (done) {
                // It didn't exists before
                return &(children_.emplace_back(v));
            } else {
                return nullptr;

            }
        }

        Setting* try_add_child( const std::string &name, ValType t) {

            if (!is_group()) {
                return nullptr;
            }

            auto [ _, done ] = group_.try_emplace(name, int(group_.size()));

            if (done) {
                // It didn't exists before
                return &(children_.emplace_back(t));
            } else {
                return nullptr;

            }
        }

        int count() const {
            if (is_scalar()) {
                return 0;
            }

            return int(children_.size());
        }

        ValType array_type() const {
            if (is_array()) {
                return array_type_;
            }

            throw std::runtime_error("Setting is not an array");
        }

        /*=============== Getting Data ================*/

        Setting &at(int idx) {
            if (! is_composite()) {
                throw std::runtime_error("at(int) called on a non-composite");
            }

            // negative indecies count from the end
            //   0    1   2
            //   -    -   -
            //  -3   -2  -1
            if (idx >= int(children_.size()) or idx < -int(children_.size())) {
                throw std::runtime_error("at(int) called with index out of range");
            }

            if (idx < 0) {
                idx += int(children_.size());
            }

            return children_.at(idx);
        }

        Setting *lkup(int idx) {
            if (! is_composite()) {
                return nullptr;
            }

            // negative indecies count from the end
            //   0    1   2
            //   -    -   -
            //  -3   -2  -1
            if (idx >= int(children_.size()) or idx < -int(children_.size())) {
                return nullptr;
            }

            if (idx < 0) {
                idx += int(children_.size());
            }

            return &(children_.at(idx));
        }


        Setting &at(const std::string& name) {
            if (!is_group()) {
                throw std::runtime_error("at(string) called on a non-group");
            }

            auto iter = group_.find(name);
            if (iter == group_.end()) {
                throw std::runtime_error("at(string) : key "s + name + 
                        " does not exist in the group");
            }

            return children_.at(iter->second);
        }

        Setting *lkup(const std::string& name) {
            if (!is_group()) {
                return nullptr;
            }

            auto iter = group_.find(name);
            if (iter == group_.end()) {
                return nullptr;
            }

            return &(children_.at(iter->second));
        }

        Setting &at_path(std::string_view path) {
            auto point_index = path.find('.');
            std::string_view piece;
            if (point_index == std::string::npos) {
                piece = path;
                path = ""sv;
                
            } else {
                piece = path.substr(0, point_index);
                path.remove_prefix(point_index + 1);
            }

            if (piece[0] == '[') {
                int index = std::stol(std::string(piece.substr(1, piece.size()-2)));
                if (path.empty())  {
                    return at(index);
                } else {
                    return at(index).at_path(path);
                }
            } else {
                if (path.empty())  {
                    return at(std::string(piece));
                } else {
                    return at(std::string(piece)).at_path(path);
                }
            }
            throw std::runtime_error("How did we get here ?");

        }

        Setting* lkup_path(std::string_view path) {
            auto point_index = path.find('.');

            Setting* retval = nullptr;

            std::string_view piece;
            if (point_index == std::string::npos) {
                piece = path;
                path = ""sv;
                
            } else {
                piece = path.substr(0, point_index);
                path.remove_prefix(point_index + 1);
            }

            if (piece[0] == '[') {
                int index = std::stol(std::string(piece.substr(1, piece.size()-2)));
                retval = lkup(index);
            } else {
                retval = lkup(std::string(piece));
            }

            if (retval && not path.empty()) {
                retval = retval->lkup_path(path);
            }

            return retval;

        }

        Setting &at_vpath(const std::vector<std::string> &path) {
            Setting *current = this;

            for (auto const &e : path) {
                auto first_char = e.front();

                if (std::isdigit(first_char) or first_char == '-' or first_char == '+' ) {
                    const char * start = e.c_str();
                    char * end;
                    int number = std::strtol(start, &end, 10);
                    if (end == nullptr or (end - start) < int(e.size())) {
                        throw std::runtime_error("at_path element is an invalid decimal integer");
                    }
                    current = &current->at(number);

                } else {
                    current = &current->at(e);
                }
            }

            return *current;
        }


        Setting *lkup_vpath(const std::vector<std::string> &path) {
            Setting *current = this;

            for (auto const &e : path) {
                auto first_char = e.front();

                if (std::isdigit(first_char) or first_char == '-' or first_char == '+' ) {
                    const char * start = e.c_str();
                    char * end;
                    int number = std::strtol(start, &end, 10);
                    if (end == nullptr or (end - start) < int(e.size())) {
                        return nullptr;
                    }
                    current = current->lkup(number);

                } else {
                    current = current->lkup(e);
                }
                if (!current) return nullptr;
            }

            return current;
        }


        template <typename first_t, typename ... pack_t>
        Setting &at_tpath(first_t first_arg, pack_t...args) {
            return at(first_arg).at_tpath(args...);
        }

        template<typename first_t>
        Setting &at_tpath(first_t first_arg) {
            return at(first_arg);
        }

        template <typename first_t, typename ... pack_t>
        Setting *lkup_tpath(first_t first_arg, pack_t...args) {
            auto *f = lkup(first_arg);
            if (not f) return nullptr;
            return f->lkup_tpath(args...);
        }

        template<typename first_t>
        Setting *lkup_tpath(first_t first_arg) {
            return lkup(first_arg);
        }

        auto begin() {
            return children_.begin();
        }

        auto end() {
            return children_.end();
        }

        std::ostream &stream_setting(std::ostream& strm,
            const std::string prefix = "");

    private :

        void stream_scalar(std::ostream& strm,
            const std::string prefix);

        void stream_group(std::ostream& strm,
            const std::string prefix);

        void stream_array(std::ostream& strm,
            const std::string prefix);

        void stream_list(std::ostream& strm,
            const std::string prefix);

    };

}