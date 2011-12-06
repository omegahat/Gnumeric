ifdef ADMIN
createRPackage::
endif

-include local.config

include $(OMEGA_HOME)/Config/GNUmakefile.config
include $(OMEGA_HOME)/Config/GNUmakefile.CConfig

RGnumericPlugin.so:
RGnumeric.o:

# local.config should define 
# GNUMERIC_DIR - where we can find the func.h, etc.
# R_HOME - location of the R distribution
# EXTRA_INCLUDES - any other directories needed for the gnome, ... compilation.
-include local.config

GTK_CFLAGS=$(shell gtk-config --cflags)
CFLAGS+=-Wall $(DEFINES) -I$(GNUMERIC_DIR) -I$(GNUMERIC_DIR)/.. -I/usr/local/lib/gnome-libs/include -I/usr/lib/glib/include $(EXTRA_INCLUDES:%=-I%)  -I$(OMEGA_HOME)/include/Corba -D_R_=1 -DUSE_R=1 $(GTK_CFLAGS) -I$(R_HOME)/include -I$(R_HOME)/src/include -I/usr/local/include/gnome-xml

#CFLAGS+=-DNEED_GNOME_SUPPORT_H -DHAVE_CONFIG_H

include $(OMEGA_HOME)/Config/GNUmakefile.CRules

SRC=RGnumeric.c RSheet.c RConvert.c RGnumericPlugin.c RWorkbook.c RFormat.c \
    RUtils.c RCell.c RReadWritePlugin.c
OBJS=$(SRC:%.c=%.o)

RGnumericPlugin.so: $(OBJS)
	$(CC) -shared -o $@ $(OBJS) -L$(R_HOME)/bin  -lR


editor: FunctionEditor.o
	$(CC) -o $@ $^ `gtk-config --libs`

ifdef ADMIN
include Install/GNUmakefile.admin
endif


clean:
	- rm $(OBJS)