#ifndef DEBUG_CLASS_HPP
#define DEBUG_CLASS_HPP

#include <iostream>

namespace aul {

    class Debug_class {
    public:
        Debug_class();
        Debug_class(const double x);
        Debug_class(const Debug_class&);
        Debug_class(Debug_class&&) noexcept;

        ~Debug_class();

        Debug_class& operator=(const Debug_class&);
        Debug_class& operator=(Debug_class&&) noexcept;

        double value() const;

        friend std::ostream& operator<<(std::ostream&, const Debug_class& rhs);

    private:
		
		double val = 0.0;

    };

}

#endif
