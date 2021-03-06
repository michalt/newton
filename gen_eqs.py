import numpy.random
import itertools
import random
import sys

# run with: python gen_eqs.py [#vars] [sparseness-parameter in (0,1] ], e.g. "python gen_eqs.py 14 0.7"

N = int(sys.argv[1])
tol = float(sys.argv[2])


# generate a (sparse) probabilistic system of n quadratic equations in n variables with real coefficients in (0,1)
# return the system (as a vector of symbolic expressions) as well as its variables
# eps is the "density", i.e. eps*(binom(n,2)) of the coefficients will be non-zero.
# However, at least one coefficient will be non-zero (so the minimum number of terms on each rhs is 2)!

def gen_random_quadratic_eqns(n, eps) :
    poly_vars = ['x%d' %i for i in range(n)]
    
    # n variables ==> n choose 2 monomials, select eps*n of those which will be non-zero
    monomials = [m for m in itertools.combinations(poly_vars,2)]

    k = int(eps*(n*n/2 - n/2))

    if(k<2) :
        k=2

    #for the non-zero monomials choose random coefficients from (0,1) that add up to 1
    # this can be done by sampling from a (/the) k-dimensional (symmetric) Dirichlet-distribution
    
    for i in range(n) :
        non_zero = random.sample(monomials, k-1)
        coeff = list(numpy.random.dirichlet([1]*k))
        const_coeff = coeff.pop()
        mon_coeffs = map(lambda x : str(x[0]) + " " + "<"+x[1][0]+">"+"<"+x[1][1]+">", zip(coeff,non_zero) )
        f = reduce(lambda x, y : x + " | " + y, mon_coeffs)
        f = f + " | " + str(const_coeff)
        print "<" + poly_vars[i] + ">" + " ::= " + f + ";"



gen_random_quadratic_eqns(N, tol)


#TODO: make coefficients symbolic constants -> obtain generator for sl-sets or regexp-SR
