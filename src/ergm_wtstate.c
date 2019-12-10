#include "ergm_wtstate.h"

WtErgmState *WtErgmStateInit(// Network settings
			     SEXP stateR, Rboolean empty,
                             // Model settings
			     SEXP mR, Rboolean noinit_s,
                             // Proposal settings
			     SEXP pR,
                             // Network state
                             Rboolean timings, int time, int *lasttoggle){
  WtErgmState *s = Calloc(1, WtErgmState);

  /* Extract stats vector */
  SEXP tmp = getListElement(stateR, "stats");
  s->stats = length(tmp) ? REAL(tmp) : NULL;

  /* Form the network */
  s->nwp=Redgelist2WtNetwork(getListElement(stateR,"el"), empty, timings, time, lasttoggle);

  /* Initialize the model */
  s->m=NULL;
  if(s->nwp && mR) // Model also requires network.
    s->m = WtModelInitialize(mR, s->nwp, noinit_s);

  /* Initialize the M-H proposal */
  s->MHp=NULL;
  if(s->m && pR) // Proposal also requires model's auxiliaries.
    s->MHp = WtMHProposalInitialize(pR, s->nwp, s->m->termarray->aux_storage);
  
  return s;
}

SEXP WtErgmStateRSave(SEXP startR, WtErgmState *s){
  // Duplicate state
  SEXP outl = PROTECT(allocVector(VECSXP, length(startR)));
  setAttrib(outl, R_NamesSymbol, getAttrib(startR, R_NamesSymbol));
  for(unsigned int i=0; i<length(startR); i++)
    SET_VECTOR_ELT(outl, i, VECTOR_ELT(startR, i));

  // Network state
  if(s->nwp) setListElement(outl, "el", WtNetwork2Redgelist(s->nwp));

  // Statistics
  if(s->stats){
    SEXP statsR = PROTECT(allocVector(REALSXP, length(getListElement(startR, "stats"))));
    memcpy(REAL(statsR), s->stats, length(statsR)*sizeof(double));
    setListElement(outl, "stats", statsR);
    UNPROTECT(1); // statsR
  }

  SEXP class = PROTECT(mkRStrVec((const char*[]){"ergm_state", NULL}));
  classgets(outl, class);
  UNPROTECT(1); // class

  UNPROTECT(1); // outl
  return outl;
}

void WtErgmStateDestroy(WtErgmState *s){
  if(s->MHp) WtMHProposalDestroy(s->MHp, s->nwp);
  if(s->m) WtModelDestroy(s->nwp, s->m);
  if(s->nwp) WtNetworkDestroy(s->nwp);
  Free(s);
}
