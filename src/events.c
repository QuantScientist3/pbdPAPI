/* This Source Code Form is subject to the terms of the BSD 2-Clause
 * License. If a copy of the this license was not distributed with this
 * file, you can obtain one from http://opensource.org/licenses/BSD-2-Clause. */

// Copyright 2014, Heckendorf, Schmidt.  All rights reserved.


#include "pbdPAPI.h"

#define NUM_EVENTS LENGTH(which)

#define CHARPT(x,i)	((char*)CHAR(STRING_ELT(x,i)))
#define SETLISTVAR(RVAR,i,RNAME) PROTECT(RVAR=allocVector(REALSXP,1));REAL(RVAR)[0]=(double) values[i];SET_VECTOR_ELT(RET, i, RVAR);SET_STRING_ELT(RET_NAMES, i, mkChar(RNAME))

SEXP papi_event_counter_init(SEXP which)
{
  SEXP vec;
  int i;
  const int num = NUM_EVENTS;
  int ret;
  int id;

  vec = PROTECT(allocVector(INTSXP,num));

  for (i=0; i<num; i++)
  {
    ret=PAPI_event_name_to_code(CHARPT(which, i),&id);
    if(ret!=PAPI_OK)
    {
      UNPROTECT(1);
      return R_papi_error(PAPI_ENOEVNT); // Should we make a custom error?
    }
    INTEGER(vec)[i]=id;
  }

  UNPROTECT(1);

  return vec;
}



SEXP papi_event_counter_on(SEXP which)
{
  SEXP ret;
  int retval;
  int *events;
  const int num = NUM_EVENTS;

  events=INTEGER(which);
  retval = PAPI_start_counters(events, num);

  ret = R_papi_error(retval);

  return ret;
}



SEXP papi_event_counter_off(SEXP which)
{
  int retval, unpt;
  int i;
  const int num = NUM_EVENTS;
  long_long *values;//[NUM_EVENTS];
  PAPI_event_info_t ev;
  int ret;

  SEXP RET, RET_NAMES;
  SEXP *svals;

  values=malloc(sizeof(*values)*num);
  svals=malloc(sizeof(*svals)*num);

  retval = PAPI_stop_counters(values, num);

  if (retval != PAPI_OK)
  {
    PROTECT(RET = allocVector(INTSXP, 1));
    INTEGER(RET)[0] = PBD_ERROR;

    unpt = 1;
  }
  else
  {
    PROTECT(RET = allocVector(VECSXP, num));
    PROTECT(RET_NAMES = allocVector(STRSXP, num));

    for (i=0; i<num; i++)
    {
      ret=PAPI_get_event_info(INTEGER(which)[i],&ev);
      if(ret != PAPI_OK)
      {
        PROTECT(RET = allocVector(INTSXP, 1));
        INTEGER(RET)[0] = PBD_ERROR;

        unpt = 2;
        goto cleanup;
      }
      SETLISTVAR(svals[i],i,ev.long_descr);
    }

    setAttrib(RET, R_NamesSymbol, RET_NAMES);

    unpt = num + 2;
  }

cleanup:
  UNPROTECT(unpt);

  free(svals);
  free(values);


  return RET;
}


