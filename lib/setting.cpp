#include <setting.hpp>



namespace simpleConfig {

    void Setting::stream_scalar(std::ostream& strm,
            const std::string prefix) {

        switch (type_) {
            case ValType::INTEGER :
                strm << integer_;
                break;
            case ValType::FLOAT :
                strm << float_;
                break;
            case ValType::BOOL :
                strm << std::boolalpha << bool_;
                break;
            case ValType::STRING :
                strm << '"' << string_ << '"';
                break;
            default :
                break;
        }

    }

    void Setting::stream_group(std::ostream& strm,
            const std::string prefix) {

        bool is_first = true;
        for (auto [name, value] : enumerate() ) {
            if (is_first) {
                is_first = false;
            } else {
                strm << ",\n";
            }
            strm << prefix << name << " : ";
            if (value.is_scalar()) {
                value.stream_scalar(strm, "");
            } else if (value.is_group()) {
                strm << "{\n";
                value.stream_group(strm, prefix + "  "s);
                strm << prefix << "}";
            } else if (value.is_array()) {
                value.stream_array(strm, prefix + "  "s);
            } else if (value.is_list()) {
                value.stream_list(strm, prefix + "  "s);
            }
        }
        if (not is_first) {
            strm << "\n";
        }


    }


    void Setting::stream_array(std::ostream& strm,
            const std::string prefix) {

        strm << "[";

        std::string delim = "";
        if (array_type_ == ValType::GROUP) {
            strm << "\n";
            delim = "\n";
        } else {
            strm << " ";
            delim = " ";
        }

        std::string new_prefix = prefix + "  "s;
        bool is_first = true;
        for (auto &c : *this ) {
            if (is_first) {
                is_first = false;
            } else {
                strm << "," << delim;
            }
            if (c.is_scalar()) {
                c.stream_scalar(strm, "");
            } else if (c.is_group()) {
                strm << prefix << "{\n";
                c.stream_group(strm, prefix + "  "s);
                strm << prefix << "}";
            } else {
                throw std::runtime_error("Array with bad element type");
            }
        }

        strm << " ]";
    }

    void Setting::stream_list(std::ostream& strm,
            const std::string prefix) {

        strm << "(";


        std::string new_prefix = prefix + "  "s;
        bool is_first = true;
        bool last_was_composite = false;
        for (auto &c : *this ) {
            if (is_first) {
                is_first = false;
            } else {
                strm << ",";
            }
            if (c.is_scalar()) {
                strm << " ";
                c.stream_scalar(strm, "");
                last_was_composite = false;
            } else if (c.is_group()) {
                strm << "\n" << prefix << " {\n";
                c.stream_group(strm, prefix + "  "s);
                strm << prefix << "}";
                last_was_composite = true;
            } else if (c.is_list()) {
                c.stream_list(strm, prefix + "  "s);
                last_was_composite = true;
            }
        }
        if (last_was_composite) {
            strm << "\n" << prefix << ")";
        } else {
            strm << " )";
        }
    }

    std::ostream &Setting::stream_setting(std::ostream& strm,
            const std::string prefix) {

        if (is_scalar()) {
            stream_scalar(strm, prefix);
        } else if (is_group()) {
            stream_group(strm, prefix);
        } else if (is_array()) {
            stream_array(strm, prefix);
        } else if (is_list()) {
            stream_list(strm, prefix);
        }

        return strm;

    }

}