#ifndef NEWTON_H
#define NEWTON_H

#include <cstdio>
#include "matrix.h"
#include "polynomial.h"

template <typename SR>
class Newton
{
private:
	void xp(const std::vector<std::vector<int>*>& vecs,
			std::vector<std::vector<int>*> *result) {
		std::vector<std::vector<int>*>* rslts;
		for (int ii = 0; ii < vecs.size(); ++ii) {
			const std::vector<int>& vec = *vecs[ii];
			if (ii == 0) {
				// vecs=[[1,2],...] ==> rslts=[[1],[2]]
				rslts = new std::vector<std::vector<int>*>;
				for (int jj = 0; jj < vec.size(); ++jj) {
					std::vector<int>* v = new std::vector<int>;
					v->push_back(vec[jj]);
					rslts->push_back(v);
				}
			} else {
				// vecs=[[1,2],[3,4],...] ==> rslts=[[1,3],[1,4],[2,3],[2,4]]
				std::vector<std::vector<int>*>* tmp = new std::vector<
						std::vector<int>*>;
				for (int jj = 0; jj < vec.size(); ++jj) { // vec[jj]=3 (first iter jj=0)
					for (std::vector<std::vector<int>*>::const_iterator it =
							rslts->begin(); it != rslts->end(); ++it) {
						std::vector<int>* v = new std::vector<int>(**it); // v=[1]
						v->push_back(vec[jj]);                        // v=[1,3]
						tmp->push_back(v);                        // tmp=[[1,3]]
					}
				}
				for (int kk = 0; kk < rslts->size(); ++kk) {
					delete (*rslts)[kk];
				}
				delete rslts;
				rslts = tmp;
			}
		}
		result->insert(result->end(), rslts->begin(), rslts->end());
		delete rslts;
	}

	Matrix<Polynomial<SR> > compute_symbolic_delta(const std::vector<Polynomial<SR> >& v,
							const std::vector<Polynomial<SR> >& v_upd,
							const std::vector<Polynomial<SR> >& F,
							const std::multiset<Var>& poly_vars)
	{
		std::vector<Polynomial<SR> > delta;
		int n = v.size();

		for(int i=0; i<n; ++i)
		{
			Polynomial<SR> delta_i = Polynomial<SR>::null();
			Polynomial<SR> f = F.at(i);
			int deg = f.get_degree();

			std::vector<int> list; // holds [0,1,...,deg]
			for(int j=0; j<deg+1; ++j)
			{
				list.push_back(j);
			}
			std::vector<std::vector<int>* > tmplist; // holds n-times $list
			for(int j=0; j<n; ++j)
			{
				tmplist.push_back(&list);
			}
			std::vector<std::vector<int>* > p; // n-ary cartesian product
			xp(tmplist,&p); // generate n-ary cartesian product

			//iterate over (x,y,...,z) with x,y,...,z \in [0,deg+1]
			for(std::vector<std::vector<int>* >::const_iterator it = p.begin(); it != p.end(); ++it)
			{
				int sum_p=0;
				for(std::vector<int>::iterator it2=(*it)->begin();it2!=(*it)->end();++it2)
				    sum_p += *it2;
				// sum over p must be 2 <= p <= deg
				if(sum_p < 2 || sum_p > deg)
				{
					continue;
				}

				std::multiset<Var> dx;
				Polynomial<SR> prod = Polynomial<SR>::one();

				std::multiset<Var>::const_iterator var = poly_vars.begin();
				typename std::vector<Polynomial<SR> >::const_iterator elem = v_upd.begin();
				for(std::vector<int>::const_iterator z = (*it)->begin(); z != (*it)->end(); ++z)
				{
					for(int j=0; j<(*z); ++j)
					{
						dx.insert(*var); // generate a multiset of variables like "xxxyyzzzz";
						prod = prod * (*elem);	// prod = prod * elem^z ...
					}
					++var; // next variable
					++elem; // next element of vector v_upd
				}

				// eval f.derivative(dx) at v
				std::map<Var,Polynomial<SR> > values;
				int i = 0;
				for(std::multiset<Var>::const_iterator poly_var = poly_vars.begin(); poly_var != poly_vars.end(); ++poly_var)
				{
					values.insert(values.begin(), std::pair<Var,Polynomial<SR> >((*poly_var),v.at(i++)));
				}
				Polynomial<SR> f_eval = f.derivative(dx).eval(values);

				delta_i = delta_i + f_eval * prod;
				//std::cout << delta_i << std::endl;
			}

			delta.push_back(delta_i);
		}

		return Matrix<Polynomial<SR> >(1, delta.size(), delta);
	}

	std::vector<Polynomial<SR> > get_symbolic_vector(int size, std::string prefix)
	{
		// define new symbolic vector [u1,u2,...,un] TODO: this is ugly...
		std::vector<Polynomial<SR> > ret;
		std::map<std::multiset<Var>, FloatSemiring> coeff;
		std::multiset<Var> variables;
		char var[] = "0"; // initialize char*
		for(int i=0; i<size; i++)
		{
			std::stringstream ss;
			coeff.clear();
			variables.clear();
			ss << prefix << "_" << i;
			Var var = Var(ss.str());
			std::multiset<Var> vars = {var};
			coeff[vars] = SR::one(); // variable "u_1", "u_2",...
			variables.insert(var);
			Polynomial<SR> f = Polynomial<SR>(variables, coeff);
			ret.push_back(f);
		}
		return ret;
	}

public:
	// calculate the next newton iterand
	Matrix<Polynomial<SR> > step(const std::multiset<Var>& poly_vars, const Matrix<Polynomial<SR> >& J_s,
			const Matrix<Polynomial<SR> >& v, const Matrix<Polynomial<SR> >& delta)
	{
		assert(poly_vars.size() == v.getRows());
		std::map<Var,Polynomial<SR> > values;
		int i=0;
		for(std::multiset<Var>::const_iterator poly_var = poly_vars.begin(); poly_var != poly_vars.end(); ++poly_var)
		{
			values.insert(values.begin(), std::pair<Var,Polynomial<SR> >((*poly_var),v.getElements().at(i++)));
		}
		Matrix<Polynomial<SR> > J_s_new = Polynomial<SR>::eval(J_s, values);
		Matrix<Polynomial<SR> > result = J_s_new * delta;
		return result;
	}

	// iterate until convergence
	Matrix<Polynomial<SR> > solve_fixpoint(const std::vector<Polynomial<SR> >& F, const std::multiset<Var>& poly_vars, int max_iter)
	{
		Matrix<Polynomial<SR> > F_mat = Matrix<Polynomial<SR> >(1,F.size(),F);
		Matrix<Polynomial<SR> > J = Polynomial<SR>::jacobian(F, poly_vars);
		Matrix<Polynomial<SR> > J_s = J.star();

		std::cout << J_s;

		// define new symbolic vectors [u1,u2,...,un] TODO: this is ugly...
		std::vector<Polynomial<SR> > u = this->get_symbolic_vector(poly_vars.size(), "u");
		std::vector<Polynomial<SR> > u_upd = this->get_symbolic_vector(poly_vars.size(), "u_upd");

		Matrix<Polynomial<SR> > delta = compute_symbolic_delta(u, u_upd, F, poly_vars);
		Matrix<Polynomial<SR> > v = Matrix<Polynomial<SR> >(1,(int)F.size()); // v^0 = 0

		// d^0 = F(0)
		std::map<Var,Polynomial<SR> > values;
		Polynomial<SR> null = Polynomial<SR>::null();
		for(std::multiset<Var>::const_iterator poly_var = poly_vars.begin(); poly_var != poly_vars.end(); ++poly_var)
		{
			values.insert(values.begin(), std::pair<Var,Polynomial<SR> >(*poly_var, null));
		}
		Matrix<Polynomial<SR> > delta_new = Polynomial<SR>::eval(F_mat, values);

		Matrix<Polynomial<SR> > v_upd = step(poly_vars, J_s, v, delta);

		for(int i=0; i<max_iter; ++i)
		{
			/*
			for(int i = 0; i<u.size(); i++)
			{
				values.insert(values.begin(), std::pair<char,Polynomial<SR> >(u_upd.at(i), v_upd.getElements().at(i)));
				values.insert(values.begin(), std::pair<char,Polynomial<SR> >(u.at(i), v.getElements().at(i)));
			}
			delta_new = Polynomial<SR>::eval(delta,values);
			*/
			v = v + v_upd;
			v_upd = step(poly_vars, J_s, v, delta_new);
		}

		return v_upd;
	}
};

#endif
