#include "Debug_class.hpp"
#include <iostream>

using std::cout;
using std::endl;

using std::ostream;

namespace aul {

	Debug_class::Debug_class() {
		cout << "Constructed\t" << '[' << this << ']' << endl;
	}

	Debug_class::Debug_class(const double x) :
	   val(x)
	{
		cout << "Val constructed\t" << '[' << this << ']' << endl;
	}

	Debug_class::Debug_class(const Debug_class& test_class) :
		val(test_class.val)
	{
		cout << "Copy constructed" << '[' << this << ']' << " <-- "<< '[' << &test_class << ']' << endl;
	}

	Debug_class::Debug_class(Debug_class&& test_class) noexcept :
		val(test_class.val)
	{
		test_class.val = 0.0;
		cout << "Move constructed" << '[' << this << ']' << " <-- "<< '[' << &test_class << ']' << endl;
	}

	Debug_class::~Debug_class() {
		cout << "Destructed\t" << '[' << this << ']' << endl;
	}

	Debug_class& Debug_class::operator=(const Debug_class& test_class) {
		this->val = test_class.val;

		cout << "Copy assigned\t" << '[' << this << ']' << " <-- "<< '[' << &test_class << ']' << endl;

		return *this;
	}

	Debug_class& Debug_class::operator=(Debug_class&& test_class) noexcept {
		this->val = test_class.val;
		test_class.val = 0.0;

		cout << "Move assigned\t" << '[' << this << ']' << " <-- "<< '[' << &test_class << ']' << endl;

		return *this;
	}

	double Debug_class::value() const {
		return val;
	} 

	ostream& operator<<(ostream& out, const Debug_class& rhs) {
		out << rhs.value();

		return out;
	}

}
