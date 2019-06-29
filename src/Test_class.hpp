#ifndef TEST_CLASS_HPP
#define TEST_CLASS_HPP

#include <iostream>

namespace aul {

    class Test_class {
    public:
        Test_class();
        Test_class(const double x);
        Test_class(const Test_class&);
        Test_class(Test_class&&) noexcept;

        ~Test_class();

        Test_class& operator=(const Test_class&);
        Test_class& operator=(Test_class&&) noexcept;

        double value() const;

        friend std::ostream& operator<<(std::ostream&, const Test_class& rhs);

    private:
		
		double val = 0.0;

    };

}

#endif
