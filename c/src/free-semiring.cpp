#include <sstream>
#include "free-semiring.h"

FreeSemiring::FreeSemiring(Var var)
{
	this->type = Element;
	this->elem = new Var(var);
}

FreeSemiring::FreeSemiring(const FreeSemiring& term)
{
	this->type = term.type;
	switch(this->type)
	{
	case Element:
		this->elem = term.elem;
		break;
	case Addition:
	case Multiplication:
		this->right_ptr = term.right_ptr;
	case Star:
		this->left_ptr = term.left_ptr;
		break;
	}
}

FreeSemiring::FreeSemiring(optype type, FreeSemiring left, FreeSemiring right)
{
	this->type = type;
	this->left_ptr = std::make_shared<FreeSemiring>(left);
	this->right_ptr = std::make_shared<FreeSemiring>(right);
}

FreeSemiring::FreeSemiring(optype type, FreeSemiring left)
{
	this->type = type;
	this->left_ptr = std::make_shared<FreeSemiring>(left);
}

FreeSemiring::~FreeSemiring()
{
}

FreeSemiring FreeSemiring::operator +(const FreeSemiring& term) const
{
	return FreeSemiring(Addition, *this, term);
}

FreeSemiring FreeSemiring::operator *(const FreeSemiring& term) const
{
	return FreeSemiring(Multiplication, *this, term);
}

bool FreeSemiring::operator ==(const FreeSemiring& term) const
{
	switch(this->type)
	{
		case Element:
			if(term.type != Element)
				return false;
			else
				return this->elem == term.elem;
		case Addition:
		case Multiplication:
			return (this->type == term.type && this->left_ptr == term.left_ptr && this->right_ptr == term.right_ptr);
		case Star:
			return (this->type == term.type && this->left_ptr == term.left_ptr);
	}
}

FreeSemiring FreeSemiring::star() const
{
	return FreeSemiring(Star, *this);
}

std::string FreeSemiring::string() const
{
	std::stringstream ss;
	ss << "(";
	switch(this->type)
	{
	case Element:
		ss << this->elem->string();
		break;
	case Addition:
		ss << this->left_ptr->string() << " + " << this->right_ptr->string();
		break;
	case Multiplication:
		ss << this->left_ptr->string() << " * " << this->right_ptr->string();
		break;
	case Star:
		ss << "s(" << this->left_ptr->string() << ")";
		break;
	}
	ss << ")";
	return ss.str();
}

FreeSemiring FreeSemiring::null = FreeSemiring(Var("0"));
FreeSemiring FreeSemiring::one = FreeSemiring(Var("1"));

bool FreeSemiring::is_idempotent = false;
bool FreeSemiring::is_commutative = false;
