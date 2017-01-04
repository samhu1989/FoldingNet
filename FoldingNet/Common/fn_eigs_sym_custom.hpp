#ifndef FN_EIGS_SYM_CUSTOM_HPP
#define FN_EIGS_SYM_CUSTOM_HPP
#include <armadillo>
using namespace arma;
namespace arma_custom{
namespace sp_auxlib_custom {
template<typename eT, typename T, typename T1>
inline
void
run_aupd
(
        const uword n_eigvals, char* which, const SpProxy<T1>& p, const bool sym,
        blas_int& n, eT& tol,
        podarray<T>& resid, blas_int& ncv, podarray<T>& v, blas_int& ldv,
        podarray<blas_int>& iparam, podarray<blas_int>& ipntr,
        podarray<T>& workd, podarray<T>& workl, blas_int& lworkl, podarray<eT>& rwork,
        blas_int& info
        )
{
#if defined(ARMA_USE_ARPACK)
    {
        // ARPACK provides a "reverse communication interface" which is an
        // entertainingly archaic FORTRAN software engineering technique that
        // basically means that we call saupd()/naupd() and it tells us with some
        // return code what we need to do next (usually a matrix-vector product) and
        // then call it again.  So this results in some type of iterative process
        // where we call saupd()/naupd() many times.
        blas_int ido = 0; // This must be 0 for the first call.
        char bmat = 'I'; // We are considering the standard eigenvalue problem.
        n = p.get_n_rows(); // The size of the matrix.
        blas_int nev = n_eigvals;

//        info = 0;   // Set to 0 initially to use random initial vector.
        if(resid.n_elem!=n)resid.set_size(n);

        // Two contraints on NCV: (NCV > NEV + 2) and (NCV <= N)
        //
        // We're calling either arpack::saupd() or arpack::naupd(),
        // which have slighly different minimum constraint and recommended value for NCV:
        // http://www.caam.rice.edu/software/ARPACK/UG/node136.html
        // http://www.caam.rice.edu/software/ARPACK/UG/node138.html

        ncv = nev + 2 + 1;

        if (ncv < (2 * nev + 1)) { ncv = 2 * nev + 1; }
        if (ncv > n            ) { ncv = n;           }

        if(v.n_elem!=n * ncv)v.set_size(n * ncv); // Array N by NCV (output).
        if(rwork.n_elem!=ncv)rwork.set_size(ncv); // Work array of size NCV for complex calls.
        ldv = n; // "Leading dimension of V exactly as declared in the calling program."

        // IPARAM: integer array of length 11.
        iparam.zeros(11);
        iparam(0) = 1; // Exact shifts (not provided by us).
        iparam(2) = 1000; // Maximum iterations; all the examples use 300, but they were written in the ancient times.
        iparam(6) = 1; // Mode 1: A * x = lambda * x.

        // IPNTR: integer array of length 14 (output).
        if(ipntr.n_elem!=14)ipntr.set_size(14);

        // Real work array used in the basic Arnoldi iteration for reverse communication.
        if(workd.n_elem!=3*n)workd.set_size(3 * n);

        // lworkl must be at least 3 * NCV^2 + 6 * NCV.
        lworkl = 3 * (ncv * ncv) + 6 * ncv;

        // Real work array of length lworkl.
        if(workl.n_elem!=14)workl.set_size(lworkl);


        // All the parameters have been set or created.  Time to loop a lot.
        while (ido != 99)
        {
            // Call saupd() or naupd() with the current parameters.
            if(sym)
                arpack::saupd(&ido, &bmat, &n, which, &nev, &tol, resid.memptr(), &ncv, v.memptr(), &ldv, iparam.memptr(), ipntr.memptr(), workd.memptr(), workl.memptr(), &lworkl, &info);
            else
                arpack::naupd(&ido, &bmat, &n, which, &nev, &tol, resid.memptr(), &ncv, v.memptr(), &ldv, iparam.memptr(), ipntr.memptr(), workd.memptr(), workl.memptr(), &lworkl, rwork.memptr(), &info);

            // What do we do now?
            switch (ido)
            {
            case -1:
            case 1:
            {
                // We need to calculate the matrix-vector multiplication y = OP * x
                // where x is of length n and starts at workd(ipntr(0)), and y is of
                // length n and starts at workd(ipntr(1)).

                // operator*(sp_mat, vec) doesn't properly put the result into the
                // right place so we'll just reimplement it here for now...

                // Set the output to point at the right memory.  We have to subtract
                // one from FORTRAN pointers...
                Col<T> out(workd.memptr() + ipntr(1) - 1, n, false /* don't copy */);
                // Set the input to point at the right memory.
                Col<T> in(workd.memptr() + ipntr(0) - 1, n, false /* don't copy */);

                out.zeros();
                typename SpProxy<T1>::const_iterator_type x_it     = p.begin();
                typename SpProxy<T1>::const_iterator_type x_it_end = p.end();

                while(x_it != x_it_end)
                {
                    out[x_it.row()] += (*x_it) * in[x_it.col()];
                    ++x_it;
                }

                // No need to modify memory further since it was all done in-place.

                break;
            }
            case 99:
                // Nothing to do here, things have converged.
                break;
            default:
            {
                return; // Parent frame can look at the value of info.
            }
            }
        }

        // The process has ended; check the return code.
        if( (info != 0) )
        {
            // Print warnings if there was a failure.
            if(sym)
            {
                arma_debug_warn("sym run_aupd(): ARPACK error ", info, " in saupd()");
                if(info==1)
                {
                    arma_debug_warn("sym run_aupd(): ARPACK error maximum number of iterations taken.");
                }
            }
            else
            {
                arma_debug_warn("gen run_aupd(): ARPACK error ", info, " in naupd()");
            }

            return; // Parent frame can look at the value of info.
        }
    }
#else
    arma_ignore(n_eigvals);
    arma_ignore(which);
    arma_ignore(p);
    arma_ignore(sym);
    arma_ignore(n);
    arma_ignore(tol);
    arma_ignore(resid);
    arma_ignore(ncv);
    arma_ignore(v);
    arma_ignore(ldv);
    arma_ignore(iparam);
    arma_ignore(ipntr);
    arma_ignore(workd);
    arma_ignore(workl);
    arma_ignore(lworkl);
    arma_ignore(rwork);
    arma_ignore(info);
#endif
}
template<typename eT, typename T1>
inline
bool
eigs_sym(
        Col<eT>& eigval,
        Mat<eT>& eigvec,
        const SpBase<eT, T1>& X,
        const uword n_eigvals,
        const char* form_str,
        const eT default_s_tol,
        const eT default_e_tol
        )
{
  arma_extra_debug_sigprint();
  #if defined(ARMA_USE_ARPACK)
    {
    const sp_auxlib::form_type form_val = sp_auxlib::interpret_form_str(form_str);
//    std::cerr<<"default_s_tol:"<<default_s_tol<<std::endl;
//    std::cerr<<"default_e_tol:"<<default_e_tol<<std::endl;
    arma_debug_check( (form_val != sp_auxlib::form_lm) && (form_val != sp_auxlib::form_sm) && (form_val != sp_auxlib::form_la) && (form_val != sp_auxlib::form_sa), "eigs_sym(): unknown form specified" );
    arma_debug_check( (default_s_tol < default_e_tol) || (default_e_tol < 0) , "eigs_sym():Invalid range of tolerance");

    char  which_sm[3] = "SM";
    char  which_lm[3] = "LM";
    char  which_sa[3] = "SA";
    char  which_la[3] = "LA";
    char* which;
    switch (form_val)
      {
      case sp_auxlib::form_sm:  which = which_sm;  break;
      case sp_auxlib::form_lm:  which = which_lm;  break;
      case sp_auxlib::form_sa:  which = which_sa;  break;
      case sp_auxlib::form_la:  which = which_la;  break;

      default:       which = which_lm;  break;
      }

    // Make a sparse proxy object.
    SpProxy<T1> p(X.get_ref());

    // Make sure it's square.
    arma_debug_check( (p.get_n_rows() != p.get_n_cols()), "eigs_sym(): given matrix must be square sized" );

    // Make sure we aren't asking for every eigenvalue.
    // The _saupd() functions allow asking for one more eigenvalue than the _naupd() functions.
    arma_debug_check( (n_eigvals >= p.get_n_rows()), "eigs_sym(): n_eigvals must be less than the number of rows in the matrix" );

    // If the matrix is empty, the case is trivial.
    if(p.get_n_cols() == 0) // We already know n_cols == n_rows.
      {
      eigval.reset();
      eigvec.reset();
      return true;
      }

    // Set up variables that get used for neupd().
    blas_int n, ncv, ldv, lworkl, info;
    eT tol = default_s_tol;
    eT last_tol;
    podarray<eT> resid, v, workd, workl;
    podarray<blas_int> iparam, ipntr;
    podarray<eT> rwork; // Not used in this case.
    info = 0;
    run_aupd(n_eigvals, which, p, true /* sym, not gen */, n, tol, resid, ncv, v, ldv, iparam, ipntr, workd, workl, lworkl, rwork, info);
//    std::cerr<<"tried tol:"<<tol<<std::endl;
    if(info != 0)
    {
      return false;
    }
    last_tol = tol;
    tol*=0.5;
    blas_int n_, ncv_, ldv_, lworkl_, info_;
    podarray<eT> resid_, v_, workd_, workl_;
    podarray<blas_int> iparam_, ipntr_;
    podarray<eT> rwork_;
    int32_t restart = 3;
    while( ( tol > default_e_tol ) && ( restart > 0 ))
    {
        run_aupd(n_eigvals, which, p, true /* sym, not gen */, n_, tol, resid_, ncv_, v_, ldv_, iparam_, ipntr_, workd_, workl_, lworkl_, rwork_, info_);
        if( info_ == 0 )
        {
            info = info_;
            ncv = ncv_;
            ldv = ldv_;
            lworkl = lworkl_;
            n = n_;
            resid = resid_;
            v = v_;
            workd = workd_;
            workl = workl_;
            iparam = iparam_;
            ipntr = ipntr_;
            rwork = rwork_;
            last_tol = tol;
            tol*=0.1;
            info_ = 1;
        }else{
            info_ = 0;
            --restart;
            tol=10.0*( tol + last_tol );
        }
    }
    std::cerr<<"Done saupd()"<<info<<std::endl;
    // The process has converged, and now we need to recover the actual eigenvectors using seupd()
    do
    {
        blas_int rvec = 1; // .TRUE
        blas_int nev  = n_eigvals;

        char howmny = 'A';
        char bmat   = 'I'; // We are considering the standard eigenvalue problem.

        podarray<blas_int> select(ncv); // Logical array of dimension NCV.
        blas_int ldz = n;
        // seupd() will output directly into the eigval and eigvec objects.
        eigval.set_size(n_eigvals);
        eigvec.set_size(n, n_eigvals);
        arpack::seupd(&rvec, &howmny, select.memptr(), eigval.memptr(), eigvec.memptr(), &ldz, (eT*) NULL, &bmat, &n, which, &nev, &tol, resid.memptr(), &ncv, v.memptr(), &ldv, iparam.memptr(), ipntr.memptr(), workd.memptr(), workl.memptr(), &lworkl, &info);
        std::cerr<<"seupd() info:"<<info<<std::endl;
        tol*=2.0;
    }while(info !=0);

    // Check for errors.
    if(info != 0)
      {
      arma_debug_warn("eigs_sym(): ARPACK error ", info, " in seupd()");
      return false;
      }
    return (info == 0);
    }
  #else
    {
    arma_ignore(eigval);
    arma_ignore(eigvec);
    arma_ignore(X);
    arma_ignore(n_eigvals);
    arma_ignore(form_str);
    arma_ignore(default_tol);
    arma_stop("eigs_sym(): use of ARPACK must be enabled");
    return false;
    }
  #endif
  }

}
template<typename T1>
inline
bool
eigs_sym
(
        Col<typename T1::pod_type >&    eigval,
        Mat<typename T1::elem_type>&    eigvec,
        const SpBase<typename T1::elem_type,T1>& X,
        const uword                              n_eigvals,
        const char*                              form = "lm",
        const typename T1::elem_type             stol  = 1.0,
        const typename T1::elem_type             etol  = 0.0,
        const typename arma_real_only<typename T1::elem_type>::result* junk = 0
        )
{
    arma_ignore(junk);
    arma_debug_check( arma::void_ptr(&eigval) == arma::void_ptr(&eigvec), "eigs_sym(): paramater 'eigval' is an alias of parameter 'eigvec'" );
    const bool status = sp_auxlib_custom::eigs_sym(eigval, eigvec, X, n_eigvals, form, stol,etol);
    if(status == false)
    {
        eigval.reset();
        arma_debug_warn("eigs_sym(): decomposition failed");
    }
    return status;
}
}
#endif // FN_EIGS_SYM_CUSTOM_HPP
