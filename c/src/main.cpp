#include <iostream>
#include "float-semiring.h"
#include "matrix.h"
#include "polynomial.h"
#include "newton.h"

void test_addition()
{
	FloatSemiring first(0.5);
	FloatSemiring second(0.3);
	FloatSemiring result = first + second;
	std::cout << "- float semiring addition: " << std::endl;
	std::cout << first << " + " << second << " = " << result << std::endl;
}

void test_multiplication()
{
	FloatSemiring first(0.5);
	FloatSemiring second(0.3);
	FloatSemiring result = first * second;
	std::cout << "- float semiring multiplication: " << std::endl;
	std::cout << first << " * " << second << " = " << result << std::endl;
}

Polynomial<FloatSemiring> get_first_polynomial()
{
	// define new polynomial 2xx+7y
	std::map<std::string, FloatSemiring> coeff;
	coeff["xx"] = FloatSemiring(2);
	coeff["z"] = FloatSemiring(7);
	std::set<char> variables;
	variables.insert('x');
	variables.insert('z');
	return Polynomial<FloatSemiring>(variables,coeff);
}

Polynomial<FloatSemiring> get_second_polynomial()
{
	// define new polynomial 5xx+3xy+6yy
	std::map<std::string, FloatSemiring> coeff;
	coeff["xx"] = FloatSemiring(5);
	coeff["xy"] = FloatSemiring(3);
	coeff["yy"] = FloatSemiring(6);
	std::set<char> variables;
	variables.insert('x');
	variables.insert('y');
	return Polynomial<FloatSemiring>(variables,coeff);
}

void test_polynomial_addition()
{
	Polynomial<FloatSemiring> first = get_first_polynomial();
	Polynomial<FloatSemiring> second = get_second_polynomial();
	Polynomial<FloatSemiring> result = first + second;
	std::cout << "- polynomial addition:" << std::endl;
	std::cout << first << " + " << second << " = " << result << std::endl;
}

void test_polynomial_multiplication()
{
	Polynomial<FloatSemiring> first = get_first_polynomial();
	Polynomial<FloatSemiring> second = get_second_polynomial();
	Polynomial<FloatSemiring> result = first * second;
	std::cout << "- polynomial multiplication:" << std::endl;
	std::cout << first << " * " << second << " = " << result << std::endl;
}

std::map<char,FloatSemiring> get_variable_values()
{
	std::map<char,FloatSemiring> values;
	values.insert(values.begin(), std::pair<char,FloatSemiring>('x',FloatSemiring(5)));
	values.insert(values.begin(), std::pair<char,FloatSemiring>('y',FloatSemiring(4)));
	values.insert(values.begin(), std::pair<char,FloatSemiring>('z',FloatSemiring(0)));
	return values;
}

void test_polynomial_evaluation()
{
	Polynomial<FloatSemiring> polynomial = get_second_polynomial();
	std::map<char,FloatSemiring> values = get_variable_values();
	FloatSemiring result = polynomial.eval(values);
	std::cout << "- polynomial evaluation:" << std::endl;
	std::cout << polynomial << " at {x=5, y=4, z=0} = " << result << std::endl;
}

Matrix<FloatSemiring> get_first_matrix()
{
	FloatSemiring elems[] = {
		FloatSemiring(0.5), FloatSemiring(0.9),
		FloatSemiring(0.2), FloatSemiring(0.1)};
	return Matrix<FloatSemiring>(2,2,std::vector<FloatSemiring>(elems, elems+4));
}

Matrix<FloatSemiring> get_second_matrix()
{
	FloatSemiring elems[] = {
		FloatSemiring(0.3), FloatSemiring(0.1),
		FloatSemiring(0.7), FloatSemiring(0.2)};
	return Matrix<FloatSemiring>(2,2,std::vector<FloatSemiring>(elems, elems+4));
}

void test_matrix_addition()
{
	Matrix<FloatSemiring> first = get_first_matrix();
	Matrix<FloatSemiring> second = get_second_matrix();
	Matrix<FloatSemiring> result = first + second;
	std::cout << "- matrix addition:" << std::endl;
	std::cout << first << " + " << std::endl << second << " = " << std::endl << result;
}

void test_matrix_multiplication()
{
	Matrix<FloatSemiring> first = get_first_matrix();
	Matrix<FloatSemiring> second = get_second_matrix();
	Matrix<FloatSemiring> result = first * second;
	std::cout << "- matrix multiplication:" << std::endl;
	std::cout << first << " * " << std::endl << second << " = " << std::endl << result;
}

void test_matrix_transpose()
{
	Matrix<FloatSemiring> matrix = get_first_matrix();
	Matrix<FloatSemiring> result = matrix.transpose();
	std::cout << "- matrix transpose:" << std::endl;
	std::cout << matrix << " transposed " << std::endl << result;
}

void test_matrix_star()
{
	Matrix<FloatSemiring> matrix = get_first_matrix();
	Matrix<FloatSemiring> result = matrix.star();
	std::cout << "- matrix star:" << std::endl;
	std::cout << matrix << " the matrix star of this is: " << std::endl << result;
}

void test_jacobian()
{
	std::vector<char> variables;
	variables.push_back('x');
	variables.push_back('y');
	variables.push_back('z');
	std::vector<Polynomial<FloatSemiring> > polynomials;
	polynomials.push_back(get_first_polynomial());
	polynomials.push_back(get_second_polynomial());
	Matrix<Polynomial<FloatSemiring> > jacobian = Polynomial<FloatSemiring>::jacobian(polynomials, variables);
	std::cout << "- jacobian (x,y,z) of " << std::endl;
	std::cout << polynomials.at(0) << std::endl << polynomials.at(1) << std::endl << " = " << std::endl << jacobian << std::endl;
}

void test_polynomial_matrix_evaluation()
{
	std::vector<char> variables;
	variables.push_back('x');
	variables.push_back('y');
	variables.push_back('z');
	std::vector<Polynomial<FloatSemiring> > polynomials;
	polynomials.push_back(get_first_polynomial());
	polynomials.push_back(get_second_polynomial());
	Matrix<Polynomial<FloatSemiring> > polynomial_matrix = Polynomial<FloatSemiring>::jacobian(polynomials, variables);
	std::map<char,FloatSemiring> values = get_variable_values();
	Matrix<FloatSemiring> result = Polynomial<FloatSemiring>::eval(polynomial_matrix, values);
	std::cout << "- polynomial matrix evaluation" << std::endl;
	std::cout << polynomial_matrix << " at {x=5, y=4, z=0} = " << std::endl << result;
}

std::vector<Polynomial<FloatSemiring> > get_newton_test_polynomials()
{
	std::vector<Polynomial<FloatSemiring> > polynomials;

	// define new polynomial 0.4xy+0.6
	std::map<std::string, FloatSemiring> coeff;
	coeff["xy"] = FloatSemiring(0.4);
	coeff[""] = FloatSemiring(0.6);
	std::set<char> variables;
	variables.insert('x');
	variables.insert('z');
	Polynomial<FloatSemiring> f1 = Polynomial<FloatSemiring>(variables,coeff);

	// define new polynomial 0.3yz+0.4yx+0.3
	coeff.clear();
	coeff["yz"] = FloatSemiring(0.3);
	coeff["yx"] = FloatSemiring(0.4);
	coeff[""] = FloatSemiring(0.3);
	variables.clear();
	variables.insert('x');
	variables.insert('z');
	Polynomial<FloatSemiring> f2 = Polynomial<FloatSemiring>(variables,coeff);

	// define new polynomial 0.3x+0.7
	coeff.clear();
	coeff["x"] = FloatSemiring(0.3);
	coeff[""] = FloatSemiring(0.7);
	variables.clear();
	variables.insert('x');
	variables.insert('z');
	Polynomial<FloatSemiring> f3 = Polynomial<FloatSemiring>(variables,coeff);

	polynomials.push_back(f1);
	polynomials.push_back(f2);
	polynomials.push_back(f3);

	return polynomials;
}

void test_newton()
{
	Newton<FloatSemiring> newton;
	std::vector<char> variables;
	variables.push_back('x');
	variables.push_back('y');
	variables.push_back('z');
	std::cout << "- newton:" << std::endl;
	std::vector<Polynomial<FloatSemiring> > polynomials = get_newton_test_polynomials();
	Matrix<Polynomial<FloatSemiring> > result = newton.solve_fixpoint(polynomials, variables, 10);
}


int main(int argc, char* argv[])
{
	std::cout << "testing..." << std::endl;
//	test_addition();
//	test_multiplication();
//	test_polynomial_addition();
//	test_polynomial_multiplication();
//	test_polynomial_evaluation();
//	test_matrix_addition();
//	test_matrix_multiplication();
//	test_matrix_transpose();
//	test_matrix_star();
//	test_jacobian();
//	test_polynomial_matrix_evaluation();
//	test_newton();

	return 0;
}