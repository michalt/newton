#ifndef POLYNOMIAL_H
#define POLYNOMIAL_H

#include <cassert>
#include <initializer_list>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>

#include "matrix.h"
#include "semiring.h"
#include "var.h"

#ifndef OLD_FREESEMIRING
#include "free-semiring.h"
#else
#include "free-semiring-old.h"
#endif  /* OLD_FREESEMIRING */


/*
 * The std::map<VarPtr, std::uint16_t> should be a separate class that contains
 * the following functionality.
 */


std::uint16_t GetDegreeOf(const std::map<VarPtr, std::uint16_t> &map,
    const VarPtr var);

void EraseAll(std::map<VarPtr, std::uint16_t> &map, const VarPtr var);

void Insert(std::map<VarPtr, std::uint16_t> &map, const VarPtr var,
    std::uint16_t deg = 1);

void Erase(std::map<VarPtr, std::uint16_t> &map, const VarPtr var,
    std::uint16_t deg = 1);


//FIXME: Polynomials are no semiring in our definition (not starable)

template <typename SR>
class Monomial {
  private:
    SR coefficient_;
    /* Maps each variable to its degree. */
    std::map<VarPtr, std::uint16_t> variables_;

    // private constructor to not leak the internal data structure
    Monomial(SR c, std::map<VarPtr, std::uint16_t> vs)
        : coefficient_(c), variables_(vs) {}

  public:
    /* Constant. */
    Monomial(SR c) : coefficient_(c) {}

    Monomial(SR c, std::initializer_list<VarPtr> vs)
        : coefficient_(c) {
      for (auto var : vs) {
        Insert(variables_, var);
      }
    }

    // Monomial(SR c, std::initializer_list< std::pair<VarPtr, std::uint16_t> > vs)
    //     : coefficient_(c) {
    //   for (auto var_degree : vs) {
    //     Insert(variables_, var_degree.first, var_degree.second);
    //   }
    // }

    /* std::vector seems to be a neutral data type and does not leak internal
     * data structure. */
    Monomial(SR c, std::vector< std::pair<VarPtr, std::uint16_t> > vs)
        : coefficient_(c) {
      for (auto var_degree : vs) {
        Insert(variables_, var_degree.first, var_degree.second);
      }
    }

    /* Add the coefficients of two monomials if their variables are equal. */
    Monomial operator+(const Monomial &monomial) const {
      assert(variables_ == monomial.variables_);
      return Monomial{coefficient_ + monomial.coefficient_, variables_};
    }

    // multiply two monomials
    Monomial operator*(const Monomial &monomial) const {
      if (is_null() || monomial.is_null()) {
        return Monomial{SR::null()};
      }
      auto tmp_vars = variables_;
      for (auto var_degree : monomial.variables_) {
        Insert(tmp_vars, var_degree.first, var_degree.second);
      }
      // FIXME: move instead of copying
      return Monomial(coefficient_ * monomial.coefficient_, tmp_vars);
    }

    // multiply a monomial with a variable
    Monomial operator*(const VarPtr &var) const {
      auto tmp_vars = variables_;
      // "add" the variables from one to the other monomial
      Insert(tmp_vars, var);
      // FIXME: move instead of copying
      return Monomial{coefficient_, tmp_vars};
    }

    // commutative version of derivative
    Monomial derivative(const VarPtr &var) const {

      auto var_iter = variables_.find(var);

      /* If the variable does not appear in the monomial, the derivative
       * must be 0. */
      if (var_iter == variables_.end()) {
        return Monomial(SR::null());
      }

      auto degree_before = var_iter->second;

      /* Remove one of these by removing the first of them and then "multiply"
       * the coefficient with degree_before. */
      auto tmp_vars = variables_;
      Erase(tmp_vars, var);

      SR tmp_coeff = coefficient_;
      for(int i = 0; i < degree_before; ++i) {
        tmp_coeff = tmp_coeff + coefficient_;
      }
      // FIXME: move instead of copying
      return Monomial{tmp_coeff, tmp_vars};
    }

    // evaluate the monomial at the position values
    SR eval(const std::map<VarPtr, SR> &values) const {
      SR result{coefficient_};

      for (auto var_degree : variables_) {
        auto value_iter = values.find(var_degree.first);
        assert(value_iter != values.end()); // all variables should be in values
        for (std::uint16_t i = 0; i < var_degree.second; ++i) {
          result = result * value_iter->second;
        }
      }

      return result;
    }

    // partially evaluate the monomial at the position values
    Monomial<SR> partial_eval(const std::map<VarPtr, SR> &values) const {
      // keep the coefficient
      Monomial<SR> result{coefficient_};

      // then loop over all variables and try to evaluate them
      for (auto var_degree : variables_) {
        auto e = values.find(var_degree.first);
        if (e == values.end()) {
          // variable not found in the mapping, so keep it
          Insert(result.variables_, var_degree.first, var_degree.second);
        } else {
          // variable was found, use it for evaluation
          result.coefficient_ = result.coefficient_ * e->second;
        }
      }

      return result;
    }

    // substitute variables with other variables
    Monomial<SR> subst(const std::map<VarPtr, VarPtr> &mapping) const {
      auto tmp_vars = variables_;

      for(auto from_to : mapping) {
        auto degree = GetDegreeOf(tmp_vars, from_to.first);
        EraseAll(tmp_vars, from_to.first);
        Insert(tmp_vars, from_to.second, degree);
      }

      // FIXME: move instead of copying
      return Monomial{coefficient_, tmp_vars};
    }

    // convert this monomial to an element of the free semiring
    FreeSemiring make_free(std::unordered_map<SR, VarPtr, SR> *valuation) const {
      FreeSemiring result = FreeSemiring::one();
      for(auto var_degree : variables_) {
        FreeSemiring tmp{var_degree.first};
        for (std::uint16_t i = 0; i < var_degree.second; ++i) {
          result = result * tmp;
        }
      }

      // change the SR element to a constant in the free semiring
      auto elem = valuation->find(coefficient_);
      if(elem == valuation->end()) { // this is a new SR element
        // map 'zero' and 'one' element to respective free semiring element
        if (coefficient_ == SR::null()) {
          // valuation->insert(valuation->begin(), std::pair<SR,FreeSemiring>(coeff,FreeSemiring::null()));
          result = FreeSemiring::null() * result;
        } else if (coefficient_ == SR::one()) {
          // valuation->insert(valuation->begin(), std::pair<SR,FreeSemiring>(coeff,FreeSemiring::one()));
          result = FreeSemiring::one() * result;
        } else {
          // use a fresh constant - the constructor of Var::getVar() will do this
          VarPtr tmp_var = Var::getVar();
          FreeSemiring tmp{tmp_var};
          valuation->emplace(coefficient_, tmp_var);
          result = tmp * result;
        }
      } else {
        // this is an already known element
        result = (elem->second) * result;
      }
      return result;
    }

    // a monomial is smaller than another monomial if the variables are smaller
    bool operator<(const Monomial& monomial) const {
      return variables_ < monomial.variables_;
    }

    // a monomial is equal to another monomial if the variables are equal
    // warning: the coefficient will not be regarded --- FIXME:this is extremly dangerous (regarding set-implementation of polynomial)!
    bool operator==(const Monomial& monomial) const {
      return variables_ == monomial.variables_;
    }

    // both monomials are equal if they have the same variables and the same coefficient
    bool equal(const Monomial& monomial) const {
      return variables_ == monomial.variables_ &&
             coefficient_ == monomial.coefficient_;
    }

    int get_degree() const {
      return variables_.size();
    }

    SR get_coeff() const {
      return coefficient_;
    }

    bool is_null() const {
      return coefficient_ == SR::null();
    }

    void set_coeff(SR coeff) {
      coefficient_ = coeff;
    }

    void add_to_coeff(const SR coeff) {
      coefficient_ = coefficient_ + coeff;
    }

    std::set<VarPtr> get_variables() const {
      std::set<VarPtr> set;
      for (auto var_degree : variables_) {
        set.insert(var_degree.first);
      }
      return set;
    }

    std::string string() const {
      std::stringstream ss;
      ss << coefficient_;
      if(!is_null()) {
        ss << "*" << variables_;
      }
      return ss.str();
    }
};

template <typename SR>
class Polynomial : public Semiring<Polynomial<SR> > {
  private:
    int degree;
    std::set<Monomial<SR> > monomials;
    std::set<VarPtr> variables;

    // private constructor to hide the internal data structure
    Polynomial(const std::set<Monomial<SR> >& monomials) {
      this->monomials = monomials;
      this->degree = 0;
      for(auto m_it = this->monomials.begin(); m_it != this->monomials.end(); ++m_it) {
        this->degree = (*m_it).get_degree() > this->degree ? (*m_it).get_degree() : this->degree;
        auto vars = m_it->get_variables();
        this->variables.insert(vars.begin(), vars.end()); // collect all used variables
      }

      // If there is a null-monomial, it should be at the front.
      // If the polynomial has more than one element, delete the null element
      if(this->monomials.size() > 1 && this->monomials.begin()->is_null()) {
        this->monomials.erase(this->monomials.begin());
      }
    }

    static std::shared_ptr<Polynomial<SR>> elem_null;
    static std::shared_ptr<Polynomial<SR>> elem_one;
  public:
    // empty polynomial
    Polynomial() {
      this->degree = 0;
    };

    Polynomial(std::initializer_list<Monomial<SR> > monomials) {
      if(monomials.size() == 0) {
        this->monomials = {Monomial<SR>(SR::null(),{})};
        this->degree = 0;
      }
      else {
        this->monomials = std::set<Monomial<SR> >();
        this->degree = 0;

        for(auto m_it = monomials.begin(); m_it != monomials.end(); ++m_it) {
          auto mon = this->monomials.find(*m_it);

          if(mon == this->monomials.end()) // not yet present as a monomial
            this->monomials.insert(*m_it); // just insert it
          else {
            //monomial already present in this polynomial---add the coefficients (note that c++ containers cannot be modified in-place!)
            Monomial<SR> tmp = *mon;
            this->monomials.erase(mon);
            this->monomials.insert( tmp + (*m_it) );
          }
          this->degree = m_it->get_degree() > this->degree ? m_it->get_degree() : this->degree;
          auto vars = m_it->get_variables();
          this->variables.insert(vars.begin(), vars.end()); // collect all used variables
        }
      }
    }

    Polynomial(const Polynomial& polynomial) {
      this->monomials = polynomial.monomials;
      this->degree = polynomial.degree;
      this->variables = polynomial.variables;
    }

    // create a 'constant' polynomial
    Polynomial(const SR& elem) {
      this->monomials = {Monomial<SR>(elem,{})};
      this->degree = 0;
    }

    // create a polynomial which consists only of one variable
    Polynomial(VarPtr var) {
      this->monomials = {Monomial<SR>(SR::one(),{var})};
      this->degree = 1;
    }

    Polynomial& operator=(const Polynomial& polynomial) {
      this->monomials = polynomial.monomials;
      this->degree = polynomial.degree;
      this->variables = polynomial.variables;
      return (*this);
    }

    Polynomial<SR> operator+=(const Polynomial<SR>& poly) {
      std::set<Monomial<SR> > monomials = this->monomials;
      for(auto m_it = poly.monomials.begin(); m_it != poly.monomials.end(); ++m_it) {
        // check if "same" monomial is already in the set
        auto mon = monomials.find(*m_it);
        if(mon == monomials.end()) // this is not the case
          monomials.insert(*m_it); // just insert it
        else { // monomial with the same variables found
          Monomial<SR> tmp = *mon;
          monomials.erase(mon);
          monomials.insert( tmp + (*m_it) ); // then add both of them and overwrite the old one
        }
      }

      *this = Polynomial(monomials);
      return *this;
    }

    Polynomial<SR> operator*=(const Polynomial<SR>& poly) {
      std::set<Monomial<SR> > monomials;
      // iterate over both this and the poly polynomial
      for(auto m_it1 = this->monomials.begin(); m_it1 != this->monomials.end(); ++m_it1) {
        for(auto m_it2 = poly.monomials.begin(); m_it2 != poly.monomials.end(); ++m_it2) {
          Monomial<SR> tmp = (*m_it1) * (*m_it2);
          auto tmp2 = monomials.find(tmp);
          if(tmp2 == monomials.end())
            monomials.insert(tmp); // multiply them and insert them to the result set
          else { // the monomial was already in the list. Add them.
            tmp = tmp + *tmp2;
            monomials.erase(*tmp2);
            monomials.insert(tmp);
          }
        }
      }

      *this = Polynomial(monomials);
      return *this;
    }

    // multiplying a polynomial with a variable
    Polynomial<SR> operator*(const VarPtr& var) const {
      std::set<Monomial<SR> > monomials;
      for(auto m_it = this->monomials.begin(); m_it != this->monomials.end(); ++m_it) {
        monomials.insert( (*m_it) * var );
      }

      return Polynomial(monomials);
    }

    friend Polynomial<SR> operator*(const SR& elem, const Polynomial<SR>& polynomial) {
      std::set<Monomial<SR> > monomials;

      for(auto m_it = polynomial.monomials.begin(); m_it != polynomial.monomials.end(); ++m_it) {
        monomials.insert(elem * (*m_it));
      }

      return Polynomial(monomials);
    }

    bool operator==(const Polynomial<SR>& polynomial) const {
      if(this->monomials.size() != polynomial.monomials.size())
        return false;

      for(auto m_it = polynomial.monomials.begin(); m_it != polynomial.monomials.end(); ++m_it) {
        auto monomial = this->monomials.find(*m_it); // search with variables
        if(monomial == this->monomials.end())
          return false; // could not find this monomial
        else {
          if(!monomial->equal(*m_it)) // check if the monomial has the same coefficient
            return false;
        }

      }
      return true;
    }

    // convert the given matrix to a matrix containing polynomials
    // TODO: needed?
    static Matrix<Polynomial<SR> > convert(const Matrix<SR>& mat) {
      std::vector<Polynomial<SR> > ret;
      for(int i=0; i<mat.getColumns() * mat.getRows(); ++i) {
        // create constant polynomials
        ret.push_back(Polynomial(mat.getElements().at(i)));
      }

      return Matrix<Polynomial<SR> >(mat.getColumns(), mat.getRows(), ret);
    }

    Polynomial<SR> derivative(const VarPtr& var) const {
      std::set<Monomial<SR> > monomials;

      for(auto m_it = this->monomials.begin(); m_it != this->monomials.end(); ++m_it) {
        //if(SR::is_commutative) // TODO: check if compiler is optimizing this out
        if(true) {
          // take the derivative of m_it and add it to the result set
          Monomial<SR> derivative = (*m_it).derivative(var);
          auto monomial = monomials.find(derivative);
          if(monomial == monomials.end()) { // TODO: think about this and remove if not needed
            monomials.insert(derivative);
          } else {
            Monomial<SR> tmp = (*monomial) + derivative;
            monomials.erase(monomial); // remove
            monomials.insert(tmp); // and insert the updated version
          }
        }
        else { // non-commutative case
          assert(false);  // TODO: not implemented yet
        }
      }

      if(monomials.empty()) // TODO: save variables explicit in this class and check if var is in vars
        return Polynomial();
      else
        return Polynomial(monomials);
    }

    Polynomial<SR> derivative(const std::vector<VarPtr>& vars) const {
      Polynomial<SR> polynomial = *this; // copy the polynomial
      for(auto var = vars.begin(); var != vars.end(); ++var) {
        polynomial = polynomial.derivative(*var);
      }
      return polynomial;
    }

    static Matrix<Polynomial<SR> > jacobian(const std::vector<Polynomial<SR> >& polynomials, const std::vector<VarPtr>& variables) {
      std::vector<Polynomial<SR> > ret;
      for(auto poly = polynomials.begin(); poly != polynomials.end(); ++poly) {
        for(auto var = variables.begin(); var != variables.end(); ++var) {
          ret.push_back(poly->derivative(*var));
        }
      }
      return Matrix<Polynomial<SR> >(variables.size(), polynomials.size(), ret);
    };

    // TODO: i need the variables in this function!
    Matrix<Polynomial<SR> > hessian() const {
      std::vector<Polynomial<SR> > ret;
      for(auto var2 = this->variables.begin(); var2 != this->variables.end(); ++var2) {
        Polynomial<SR> tmp = this->derivative(*var2);
        for(auto var1 = this->variables.begin(); var1 != this->variables.end(); ++var1) {
          ret.push_back(tmp.derivative(*var1));
        }
      }
      return Matrix<Polynomial<SR> >(this->variables.size(), this->variables.size(), ret);
    }

    SR eval(const std::map<VarPtr,SR>& values) const {
      SR result = SR::null();
      for(auto m_it = this->monomials.begin(); m_it != this->monomials.end(); ++m_it) {
        SR elem = m_it->eval(values);
        result = result + elem;
      }
      return result;
    }

    // evaluate the polynomial with the given mapping and return the new polynomial
    Polynomial<SR> partial_eval(const std::map<VarPtr,SR>& values) const {
      Polynomial<SR> result = Polynomial<SR>::null();
      for(auto m_it = this->monomials.begin(); m_it != this->monomials.end(); ++m_it) {
        Monomial<SR> elem = m_it->partial_eval(values);
        result = result + Polynomial({elem});
      }
      return result;
    }

    // substitute variables with other variables
    Polynomial<SR> subst(const std::map<VarPtr, VarPtr>& mapping) const {
      std::set<Monomial<SR> > monomials;

      for(auto m_it = this->monomials.begin(); m_it != this->monomials.end(); ++m_it)
        monomials.insert((*m_it).subst(mapping));

      return Polynomial<SR>(monomials);
    }

    static Matrix<SR> eval(const Matrix<Polynomial<SR> >& polys, const std::map<VarPtr,SR>& values) {
      std::vector<Polynomial<SR> > polynomials = polys.getElements();
      std::vector<SR> ret;
      for(int i = 0; i < polys.getRows()*polys.getColumns(); i++) {
        ret.push_back(polynomials[i].eval(values));
      }
      return Matrix<SR>(polys.getColumns(), polys.getRows(), ret);
    }

    static Matrix<Polynomial<SR> > eval(Matrix<Polynomial<SR> > polys, std::map<VarPtr,Polynomial<SR> > values) {
      std::vector<Polynomial<SR> > polynomials = polys.getElements();
      std::vector<Polynomial<SR> > ret;
      for(int i = 0; i < polys.getRows()*polys.getColumns(); i++) {
        ret.push_back(polynomials[i].eval(values));
      }
      return Matrix<Polynomial<SR> >(polys.getColumns(), polys.getRows(), ret);
    }

    // convert this polynomial to an element of the free semiring. regard the valuation map
    // which can already define a conversion from the SR element to a free SR constant
    // the valuation map is extended in this function
    FreeSemiring make_free(std::unordered_map<SR,VarPtr, SR>* valuation) {
      if(!valuation)
        valuation = new std::unordered_map<SR, VarPtr, SR>();

      FreeSemiring result = FreeSemiring::null();
      // convert this polynomial by adding all converted monomials
      for(auto m_it = this->monomials.begin(); m_it != this->monomials.end(); ++m_it) {
        result = result + m_it->make_free(valuation);
      }

      return result;
    }

    // convert this matrix of polynomials to a matrix with elements of the free semiring
    static Matrix<FreeSemiring> make_free(const Matrix<Polynomial<SR> >& polys, std::unordered_map<SR, VarPtr, SR>* valuation)
    {
      std::vector<Polynomial<SR> > polynomials = polys.getElements();
      std::vector<FreeSemiring> ret;
      if(!valuation)
        valuation = new std::unordered_map<SR, VarPtr, SR>();

      for(int i = 0; i < polys.getRows()*polys.getColumns(); i++) {
        ret.push_back(polynomials[i].make_free(valuation));
      }
      return Matrix<FreeSemiring>(polys.getColumns(), polys.getRows(), ret);
    }

    int get_degree() {
      return this->degree;
    }

    std::set<VarPtr> get_variables() const {
      return this->variables;
    }

    // some semiring functions
    Polynomial<SR> star() const {
      // TODO: we cannot star polynomials!
      assert(false);
      return (*this);
    }

    static Polynomial<SR> const null() {
      if(!Polynomial::elem_null)
        Polynomial::elem_null = std::shared_ptr<Polynomial<SR>>(new Polynomial(SR::null()));
      return *Polynomial::elem_null;
    }

    static Polynomial<SR> const one() {
      if(!Polynomial::elem_one)
        Polynomial::elem_one = std::shared_ptr<Polynomial<SR>>(new Polynomial(SR::one()));
      return *Polynomial::elem_one;
    }

    static bool is_idempotent;
    static bool is_commutative;

    std::string string() const {
      std::stringstream ss;
      for (auto m_it = this->monomials.begin(); m_it != this->monomials.end(); ++m_it) {
        if(m_it != this->monomials.begin())
          ss << " + ";
        ss << (*m_it);
      }

      return ss.str();
    }
};

template <typename SR> bool Polynomial<SR>::is_commutative = false;
template <typename SR> bool Polynomial<SR>::is_idempotent = false;
// initialize pointers
template <typename SR> std::shared_ptr<Polynomial<SR>> Polynomial<SR>::elem_null;
template <typename SR> std::shared_ptr<Polynomial<SR>> Polynomial<SR>::elem_one;

  template <typename SR>
std::ostream& operator<<(std::ostream& os, const Monomial<SR>& monomial) {
  return  os << monomial.string();
}

  template <typename SR>
std::ostream& operator<<(std::ostream& os, const std::map<VarPtr, SR>& values) {
  for(auto value = values.begin(); value != values.end(); ++value) {
    os << value->first << "→" << value->second << ";";
  }
  return os;
}

template <typename SR>
std::ostream& operator<<(std::ostream& os, const std::map<VarPtr, Polynomial<SR> >& values) {
  for(auto value = values.begin(); value != values.end(); ++value) {
    os << value->first << "→" << value->second << ";";
  }
  return os;
}

#endif
