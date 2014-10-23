/*

   BLIS    
   An object-based framework for developing high-performance BLAS-like
   libraries.

   Copyright (C) 2014, The University of Texas at Austin

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    - Neither the name of The University of Texas at Austin nor the names
      of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "blis.h"

dim_t bli_trmm_determine_kc_f( dim_t    i,
                               dim_t    dim,
                               obj_t*   a,
                               obj_t*   b,
                               blksz_t* bsize )
{
	num_t dt;
	dim_t mnr;
	dim_t b_alg, b_max, b_now;
	dim_t dim_left_now;

	// We assume that this function is being called from an algorithm that
	// is moving "forward" (ie: top to bottom, left to right, top-left
	// to bottom-right).

	// Extract the execution datatype and use it to query the corresponding
	// blocksize and blocksize maximum values from the blksz_t object.
	dt    = bli_obj_execution_datatype( *a );
	b_alg = bli_blksz_for_type( dt, bsize );
	b_max = bli_blksz_max_for_type( dt, bsize );

	// Nudge the default and maximum kc blocksizes up to the nearest
	// multiple of MR if the triangular matrix is on the left, or NR
	// if the triangular matrix is one the right.
	if ( bli_obj_root_is_triangular( *a ) ) mnr = bli_info_get_default_mr( dt );
	else                                    mnr = bli_info_get_default_nr( dt );
	b_alg = bli_align_dim_to_mult( b_alg, mnr );
	b_max = bli_align_dim_to_mult( b_max, mnr );

	// Compute how much of the matrix dimension is left, including the
	// chunk that will correspond to the blocksize we are computing now.
	dim_left_now = dim - i;

	// If the dimension currently remaining is less than the maximum
	// blocksize, use it instead of the default blocksize b_alg.
	// Otherwise, use b_alg.
	if ( dim_left_now <= b_max )
	{
		b_now = dim_left_now;
	}
	else
	{
		b_now = b_alg;
	}

	return b_now;
}


dim_t bli_trmm_determine_kc_b( dim_t    i,
                               dim_t    dim,
                               obj_t*   a,
                               obj_t*   b,
                               blksz_t* bsize )
{
	num_t dt;
	dim_t mnr;
	dim_t b_alg, b_max, b_now;
	dim_t dim_at_edge;
	dim_t dim_left_now;

	// We assume that this function is being called from an algorithm that
	// is moving "backward" (ie: bottom to top, right to left, bottom-right
	// to top-left).

	// Extract the execution datatype and use it to query the corresponding
	// blocksize and blocksize maximum values from the blksz_t object.
	dt    = bli_obj_execution_datatype( *a );
	b_alg = bli_blksz_for_type( dt, bsize );
	b_max = bli_blksz_max_for_type( dt, bsize );

	// Nudge the default and maximum kc blocksizes up to the nearest
	// multiple of MR if the triangular matrix is on the left, or NR
	// if the triangular matrix is one the right.
	if ( bli_obj_root_is_triangular( *a ) ) mnr = bli_info_get_default_mr( dt );
	else                                    mnr = bli_info_get_default_nr( dt );
	b_alg = bli_align_dim_to_mult( b_alg, mnr );
	b_max = bli_align_dim_to_mult( b_max, mnr );

	// Compute how much of the matrix dimension is left, including the
	// chunk that will correspond to the blocksize we are computing now.
	dim_left_now = dim - i;

	dim_at_edge = dim_left_now % b_alg;

	// If dim_left_now is a multiple of b_alg, we can safely return b_alg
	// without going any further.
	if ( dim_at_edge == 0 )
		return b_alg;

	// If the dimension currently remaining is less than the maximum
	// blocksize, use it as the chosen blocksize. If this is not the case,
	// then we know dim_left_now is greater than the maximum blocksize.
	// To determine how much of it we should use for the current blocksize,
	// we inspect dim_at_edge; if it is smaller than (or equal to) b_max -
	// b_alg, then we use b_alg + dim_at_edge. Otherwise, dim_at_edge is
	// greater than b_max - b_alg, in which case we use dim_at_edge.
	if ( dim_left_now <= b_max )
	{
		b_now = dim_left_now;
	}
	else // if ( dim_left_now > b_max )
	{
		if ( dim_at_edge <= b_max - b_alg )
		{
			b_now = b_alg + dim_at_edge;
		}
		else // if ( dim_at_edge > b_max - b_alg )
		{
			b_now = dim_at_edge;
		}
	}

	return b_now;
}

