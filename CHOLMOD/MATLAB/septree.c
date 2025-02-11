//------------------------------------------------------------------------------
// CHOLMOD/MATLAB/septree: MATLAB interface to CHOLMOD prune of separator tree
//------------------------------------------------------------------------------

// CHOLMOD/MATLAB Module.  Copyright (C) 2005-2023, Timothy A. Davis.
// All Rights Reserved.
// SPDX-License-Identifier: GPL-2.0+

//------------------------------------------------------------------------------

// MATLAB(tm) is a Trademark of The MathWorks, Inc.
// METIS is Copyrighted by G. Karypis

// Prune a separator tree.
//
// Usage:
//
//      [cp_new, cmember_new] = septree (cp, cmember, nd_oksep, nd_small) ;
//
// cp and cmember are outputs of the nesdis mexFunction.
//
// cmember(i)=c means that node i is in component
// c, where c is in the range of 1 to the number of components.  length(cp) is
// the number of components found.  cp is the separator tree; cp(c) is the
// parent of component c, or 0 if c is a root.  There can be anywhere from
// 1 to n components, where n is the number of rows of A, A*A', or A'*A.
//
// On output, cp_new and cmember_new are the new tree and graph-to-tree mapping.
// A subtree is collapsed into a single node if the number of nodes in the
// separator is > nd_oksep times the total size of the subtree, or if the
// subtree has fewer than nd_small nodes.
//
// Requires the CHOLMOD Partition Module.

#include "sputil2.h"

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{
#ifndef NPARTITION
    double *p ;
    int64_t *Cmember, *CParent ;
    cholmod_common Common, *cm ;
    double nd_oksep ;
    int64_t nd_small, nc, n, c, j, nc_new ;

    //--------------------------------------------------------------------------
    // start CHOLMOD and set defaults
    //--------------------------------------------------------------------------

    cm = &Common ;
    cholmod_l_start (cm) ;
    sputil2_config (SPUMONI, cm) ;

    //--------------------------------------------------------------------------
    // get inputs
    //--------------------------------------------------------------------------

    if (nargout > 2 || nargin != 4)
    {
        mexErrMsgTxt ("Usage: [cp_new, cmember_new] = "
                "septree (cp, cmember, nd_oksep, nd_small)") ;
    }

    nc = mxGetNumberOfElements (pargin [0]) ;
    n  = mxGetNumberOfElements (pargin [1]) ;
    nd_oksep = mxGetScalar (pargin [2]) ;
    nd_small = mxGetScalar (pargin [3]) ;

    if (n < nc)
    {
        mexErrMsgTxt ("invalid inputs") ;
    }

    CParent = cholmod_l_malloc (nc, sizeof (int64_t), cm) ;
    Cmember = cholmod_l_malloc (n, sizeof (int64_t), cm) ;

    p = (double *) mxGetData (pargin [0]) ;
    for (c = 0 ; c < nc ; c++)
    {
        CParent [c] = p [c] - 1 ;
        if (CParent [c] < EMPTY || CParent [c] > nc)
        {
            mexErrMsgTxt ("cp invalid") ;
        }
    }

    p = (double *) mxGetData (pargin [1]) ;
    for (j = 0 ; j < n ; j++)
    {
        Cmember [j] = p [j] - 1 ;
        if (Cmember [j] < 0 || Cmember [j] > nc)
        {
            mexErrMsgTxt ("cmember invalid") ;
        }
    }

    //--------------------------------------------------------------------------
    // collapse the tree
    //--------------------------------------------------------------------------

    nc_new = cholmod_l_collapse_septree (n, nc, nd_oksep, nd_small, CParent,
        Cmember, cm) ;
    if (nc_new < 0)
    {
        mexErrMsgTxt ("septree failed") ;
        return ;
    }

    //--------------------------------------------------------------------------
    // return CParent and Cmember
    //--------------------------------------------------------------------------

    pargout [0] = sputil2_put_int (CParent, nc_new, 1) ;
    if (nargout > 1)
    {
        pargout [1] = sputil2_put_int (Cmember, n, 1) ;
    }

    //--------------------------------------------------------------------------
    // free workspace
    //--------------------------------------------------------------------------

    cholmod_l_free (nc, sizeof (int64_t), CParent, cm) ;
    cholmod_l_free (n, sizeof (int64_t), Cmember, cm) ;
    cholmod_l_finish (cm) ;
    if (SPUMONI > 0) cholmod_l_print_common (" ", cm) ;
#else
    mexErrMsgTxt ("CHOLMOD Partition Module not installed\n") ;
#endif
}

