#include "RGnumeric.h"

#ifdef HAVE_CONFIG_H
#include "Rconfig.h"
#endif

gboolean loadGnumericLibrary();

#ifndef GNUMERIC_VERSION_STRING
# define GNUMERIC_VERSION_STRING GNUMERIC_VERSION
#endif

Rboolean RGnumeric_libraryAvailable = FALSE;

/**
 Initialize the plugin when it is loaded by Gnumeric.
 */
void
plugin_init_general(ErrorInfo **ret_error)
{
 extern int Rf_initEmbeddedR(int argc, char *argv[]);
 char *argv[] = {"Rgnumeric", "--silent"}; 
 const char *profile;
 char buf[1000];

  *ret_error = NULL;

  Rf_initEmbeddedR(sizeof(argv)/sizeof(argv[0]), argv);

  /* Now, ensure the Gnumeric library is loaded. */
  if(loadGnumericLibrary() == FALSE) {
     *ret_error = error_info_new_printf("Failed to load RGnumeric library into R. Something's wrong");
     return;
  }


    /* Read the different R gnumeric startup scripts
         ~/.gnumeric/plugins/R/Rprofile
         ~/.gnumeric/<version-number>/plugins/R/Rprofile
         value of R_GNUMERIC_PROFILE
     */
  sprintf(buf, "%s/.gnumeric/Rprofile", getenv("HOME"));
  RGnumeric_loadProfile(buf);

  sprintf(buf, "%s/.gnumeric/%s/plugins/R/Rprofile",
                 getenv("HOME"), GNUMERIC_VERSION_STRING);
  RGnumeric_loadProfile(buf);


  if((profile=getenv("R_GNUMERIC_PROFILE")) != NULL) {
      RGnumeric_loadProfile(profile);
  }
}


USER_OBJECT_
RGnumeric_getVersion()
{
 USER_OBJECT_ ans;
 PROTECT(ans = NEW_CHARACTER(2));
 SET_STRING_ELT(ans, 0, COPY_TO_USER_STRING(GNUMERIC_VERSION_STRING));
 /* Want to put in version of running Gnumeric. */
 /* SET_STRING_ELT(ans, 1, COPY_TO_USER_STRING(GNUMERIC_VERSION_STRING)); */
 UNPROTECT(1);

 return(ans);
}


/**
 Load the R library RGnumeric into the R session,
 making its functions available to the sesion.
 */
gboolean
loadGnumericLibrary()
{
  USER_OBJECT_ e, fun, tmp;
  int error;

  PROTECT(fun = Rf_findFun(Rf_install("library"), R_GlobalEnv));
  PROTECT(e = allocVector(LANGSXP, 2));

  SETCAR(e, fun);
  SETCAR(CDR(e), tmp = NEW_CHARACTER(1));
  SET_VECTOR_ELT(tmp, 0, COPY_TO_USER_STRING("RGnumeric"));
  R_tryEval(e, R_GlobalEnv, &error);
  if(error) {
      fprintf(stderr, "Couldn't load RGnumeric package. Check the setting of R_LIBS\n");
      fflush(stderr);
      RGnumeric_libraryAvailable = FALSE;
  } else
      RGnumeric_libraryAvailable = TRUE;
   
  UNPROTECT(2);
  return(TRUE);
}

/**
 Whether the user can deactivate the plugin.
 */
gboolean
plugin_can_deactivate_general (void)
{
	return FALSE;
}

/**

 */
void
plugin_cleanup_general (ErrorInfo **ret_error)
{
	*ret_error = NULL;
}

