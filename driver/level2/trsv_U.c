/*********************************************************************/
/* Copyright 2009, 2010 The University of Texas at Austin.           */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/*   1. Redistributions of source code must retain the above         */
/*      copyright notice, this list of conditions and the following  */
/*      disclaimer.                                                  */
/*                                                                   */
/*   2. Redistributions in binary form must reproduce the above      */
/*      copyright notice, this list of conditions and the following  */
/*      disclaimer in the documentation and/or other materials       */
/*      provided with the distribution.                              */
/*                                                                   */
/*    THIS  SOFTWARE IS PROVIDED  BY THE  UNIVERSITY OF  TEXAS AT    */
/*    AUSTIN  ``AS IS''  AND ANY  EXPRESS OR  IMPLIED WARRANTIES,    */
/*    INCLUDING, BUT  NOT LIMITED  TO, THE IMPLIED  WARRANTIES OF    */
/*    MERCHANTABILITY  AND FITNESS FOR  A PARTICULAR  PURPOSE ARE    */
/*    DISCLAIMED.  IN  NO EVENT SHALL THE UNIVERSITY  OF TEXAS AT    */
/*    AUSTIN OR CONTRIBUTORS BE  LIABLE FOR ANY DIRECT, INDIRECT,    */
/*    INCIDENTAL,  SPECIAL, EXEMPLARY,  OR  CONSEQUENTIAL DAMAGES    */
/*    (INCLUDING, BUT  NOT LIMITED TO,  PROCUREMENT OF SUBSTITUTE    */
/*    GOODS  OR  SERVICES; LOSS  OF  USE,  DATA,  OR PROFITS;  OR    */
/*    BUSINESS INTERRUPTION) HOWEVER CAUSED  AND ON ANY THEORY OF    */
/*    LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR TORT    */
/*    (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY WAY OUT    */
/*    OF  THE  USE OF  THIS  SOFTWARE,  EVEN  IF ADVISED  OF  THE    */
/*    POSSIBILITY OF SUCH DAMAGE.                                    */
/*                                                                   */
/* The views and conclusions contained in the software and           */
/* documentation are those of the authors and should not be          */
/* interpreted as representing official policies, either expressed   */
/* or implied, of The University of Texas at Austin.                 */
/*********************************************************************/

#include <stdio.h>
#include <ctype.h>
#include "common.h"

const static FLOAT dm1 = -1.;

int CNAME(BLASLONG m, FLOAT *a, BLASLONG lda, FLOAT *b, BLASLONG incb, void *buffer){

  BLASLONG i, is, min_i;
  FLOAT *gemvbuffer = (FLOAT *)buffer;
  FLOAT *B = b;

  if (incb != 1) {
    B = buffer;
    gemvbuffer = (FLOAT *)(((BLASLONG)buffer + m * sizeof(FLOAT) + 4095) & ~4095);
    COPY_K(m, b, incb, buffer, 1);
  }

  for (is = m; is > 0; is -= DTB_ENTRIES){

    min_i = MIN(is, DTB_ENTRIES);

#ifdef TRANSA
    if (m - is > 0){
      GEMV_T(m - is, min_i, 0, dm1,
	     a + is + (is - min_i) * lda, lda,
	     B + is, 1,
	     B + is - min_i, 1, gemvbuffer);
    }
#endif

    for (i = 0; i < min_i; i++) {
      FLOAT *AA = a + (is - i - 1) + (is - i - 1) * lda;
      FLOAT *BB = B + (is - i - 1);

#ifdef TRANSA
      if (i > 0) BB[0] -= DOTU_K(i, AA + 1, 1, BB + 1, 1);
#endif

#ifndef UNIT
      BB[0] /= AA[0];
#endif

#ifndef TRANSA
      if (i < min_i - 1) AXPYU_K(min_i - i - 1, 0, 0, -BB[0], AA - (min_i - i - 1), 1, BB - (min_i - i - 1), 1, NULL, 0);
#endif
    }

#ifndef TRANSA
    if (is - min_i > 0){
      GEMV_N(is - min_i, min_i, 0, dm1,
	     a +  (is - min_i) * lda, lda,
	     B + is - min_i, 1,
	     B,              1, gemvbuffer);
    }
#endif

  }

  if (incb != 1) {
    COPY_K(m, buffer, 1, b, incb);
  }

  return 0;
}

