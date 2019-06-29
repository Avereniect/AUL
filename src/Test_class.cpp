#include "Test_class.hpp"
#include <iostream>

using std::cout;
using std::endl;

using std::ostream;

namespace aul {

	Test_class::Test_class() {
		cout << "Constructed\t" << '[' << this << ']' << endl;
	}

	Test_class::Test_class(const double x) :
	   val(x)
	{
		cout << "Val constructed\t" << '[' << this << ']' << endl;
	}

	Test_class::Test_class(const Test_class& test_class) :
		val(test_class.val)
	{
		cout << "Copy constructed" << '[' << this << ']' << " <-- "<< '[' << &test_class << ']' << endl;
	}

	Test_class::Test_class(Test_class&& test_class) noexcept :
		val(test_class.val)
	{
		test_class.val = 0.0;
		cout << "Move constructed" << '[' << this << ']' << " <-- "<< '[' << &test_class << ']' << endl;
	}

	Test_class::~Test_class() {
		cout << "Destructed\t" << '[' << this << ']' << endl;
	}

	Test_class& Test_class::operator=(const Test_class& test_class) {
		this->val = test_class.val;

		cout << "Copy assigned\t" << '[' << this << ']' << " <-- "<< '[' << &test_class << ']' << endl;

		return *this;
	}

	Test_class& Test_class::operator=(Test_class&& test_class) noexcept {
		this->val = test_class.val;
		test_class.val = 0.0;

		cout << "Move assigned\t" << '[' << this << ']' << " <-- "<< '[' << &test_class << ']' << endl;

		return *this;
	}

	double Test_class::value() const {
		return val;
	} 

	ostream& operator<<(ostream& out, const Test_class& rhs) {
		out << rhs.value();

		return out;
	}

}
