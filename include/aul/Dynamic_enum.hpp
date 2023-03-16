#ifndef AUL_DYNAMIC_ENUM_HPP
#define AUL_DYNAMIC_ENUM_HPP

#include "containers/Array_map.hpp"

#include <string>
#include <type_traits>

namespace aul {
    
    /// 
    /// \tparam Tag A type used as a tag to identify unique instantiations of 
    /// the Dynamic_enum class
    /// \tparam I Integral backing type 
    template<class Tag, class I = int>
    class Dynamic_enum {
        static_assert(std::is_integral<I>::value, "Backing type must be integral type");
    public:

        //=================================================
        // Type aliases
        //=================================================

        using backing_type = I;

        //=================================================
        // -ctors
        //=================================================

        ///
        /// \param v Initialize an instance of the enum value with a bit pattern
        /// equal to the supplied pattern in v
        explicit Dynamic_enum(backing_type v):
            value(v) {}

        ///
        /// \param name Name of enum value to assign. If the name does not
        /// correspond to an enum value, the current object is assigned the
        /// default value
        explicit Dynamic_enum(const std::string& name):
            value(get_value(name)) {}

        Dynamic_enum() = default;
        Dynamic_enum(const Dynamic_enum&) = default;
        Dynamic_enum(Dynamic_enum&&) = default;
        ~Dynamic_enum() = default;

        //=================================================
        // Assignment operators
        //=================================================

        Dynamic_enum& operator=(const Dynamic_enum& rhs) = default;
        Dynamic_enum& operator=(Dynamic_enum&&) = default;

        //=================================================
        // Comparison operators
        //=================================================

        ///
        /// \param lhs
        /// \param rhs
        /// \return
        [[nodiscard]]
        friend bool operator==(Dynamic_enum lhs, Dynamic_enum rhs) {
            return lhs.value == rhs.value;
        }

        ///
        /// \param lhs
        /// \param rhs
        /// \return
        [[nodiscard]]
        friend bool operator!=(Dynamic_enum lhs, Dynamic_enum rhs) {
            return lhs.value != rhs.value;
        }

        //=================================================
        //  Conversion operators
        //=================================================

        ///
        /// \return
        [[nodiscard]]
        explicit operator backing_type() const {
            return value;
        }

        //=================================================
        // Static functions
        //=================================================

        ///
        /// Specifies the default value that Dynamic_enum objects should have
        /// upon default construction or other other default-setting cases.
        ///
        /// \param v Value to use as default in future
        static void set_default_value(backing_type v) {
            default_value = v;
        }

        ///
        /// Retrieves the default value that Dynamic_enum objects should have
        /// upon default construction or other other default-setting cases.
        ///
        /// \return The value used as as default by Dynamic_enum objects
        static backing_type get_default_value() {
            return default_value;
        }

        ///
        /// Creates an association between the specified enum value and a
        /// string. If an association for the specified enum value already
        /// exists, then replaced the existing association.
        ///
        /// \param val Value to insert into set of recognized enum values
        /// \param name The name to associated with the specified enum value.
        /// Ideally, should not be an empty string.
        static void insert_enum_value(backing_type val, const std::string& name) {
            values_to_names.insert(val, name);
            names_to_values.insert(name, val);
        }

        ///
        /// Remove an entry from the set of recognized enum values. Has no
        /// effect if the specified value is not already recognized. All
        /// Dynamic_enum objects with the specified value will effectively be
        /// rendered invalid. Care should be taken to ensure that all such
        /// objects are discarded or that the possibility that the value they
        /// have will be reassigned to something different in the future.
        ///
        /// \param val Value to remove from set of recognized enums values.
        static void erase_enum_value(backing_type val){
            auto it = values_to_names.find(val);

            if (it == values_to_names.end()) {
                return;
            }

            values_to_names.erase(values_to_names.find(it));
            names_to_values.erase(names_to_values.find(*it));
        }

        ///
        /// \param e An arbitrary dynamic enum value
        /// \return A const reference to a std::string object containing the
        /// name associated with the specified enum value. If no such string
        /// exists returns a const reference to an empty string object.
        [[nodiscard]]
        friend const std::string& to_string(Dynamic_enum e) {
            static std::string empty_string{""};

            auto it = values_to_names.find(e.value);
            if (*it == values_to_names.end()) {
                return empty_string;
            } else {
                return *it;
            }
        }

    private:

        //=================================================
        // Instance members
        //=================================================

        backing_type value = default_value;

        //=================================================
        // Static members
        //=================================================

        static aul::Array_map<backing_type, std::string> values_to_names{};
        
        static aul::Array_map<std::reference_wrapper<std::string>, backing_type> names_to_values{};

        static backing_type default_value = backing_type{0};

        //=================================================
        // Helper functions
        //=================================================

        [[nodiscard]]
        backing_type get_value(const std::string& name) {
            auto it = names_to_values.find(name);
            if (it == names_to_values.end()) {
                return default_value;
            } else {
                return *it;
            }
        }

    };

}

#endif //AUL_DYNAMIC_ENUM_HPP
