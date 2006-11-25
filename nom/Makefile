# Makefile

include ./make.inc
	

GCCINC		=	$(INC) 
INC		=	./include -I ./class_h 
 
WPSLIBS		=	$(OBJDIR)/cwsomtk.lib

# Define search directories
CDIR		=	./src
CLASSCDIR	=	./class_c
CLASSTEMPLATEDIR	=	./../../../class_templates
WPSCDIR		=	./wpsclass_c
CLASSINC	=	./class_h
WPSINC		=	./wps_h
IDLDIR		=	./idl
WPSIDLDIR	=	./wps_idl
INCDIR		=	./include
OBJDIR		=	./../../../o
BINDIR		=	./../../../bin
TOOLDIR		=	./../tools

# Define objects to build

TKOBJECTS	= 	$(OBJDIR)/nomtkinit.o \
			$(OBJDIR)/nommemory.o \
			$(OBJDIR)/nombuildclass.o \
			$(OBJDIR)/nomdebug.o \
			$(OBJDIR)/nombuildnomobj.o \
			$(OBJDIR)/nombuildnomcls.o \
			$(OBJDIR)/nomid.o

#			$(OBJDIR)/nomlistmgr.o \



NOMOBJECTS	=	$(OBJDIR)/nomobj.o \
			$(OBJDIR)/nomclassmanager.o \
			$(OBJDIR)/nomcls.o

# $(OBJDIR)/nomtst.o $(OBJDIR)/nomtst2.o


NOMINCLUDES	=	$(INCDIR)/nomtk.h \
			$(INCDIR)/nomapi.h \
			$(INCDIR)/nom.h

TSTHFILES	=	$(CLASSINC)/nomobj.h $(CLASSINC)/nomcls.h

IHFILES		=	$(CLASSINC)/nomobj.ih $(CLASSINC)/nomcls.ih \
			$(CLASSINC)/nomclassmanager.ih



OBJECTS		=	 $(NOMOBJECTS) $(TKOBJECTS)

old_IHFILES		=	$(CLASSINC)/cwsomobj.ih \
			$(CLASSINC)/cwsomcls.ih \
			$(CLASSINC)/somclassmanager.ih

WPSIHFILES	=	$(CLASSINC)/cwwpobject.ih

.PRECIOUS:	$(CLASSINC)/%.ih $(CLASSCDIR)/%.c $(CDIR)/%.c $(WPSCDIR)/%.c

all:	$(BINDIR)/nobjtk.dll
#all:	$(CLASSINC)/nomobj.h


$(BINDIR)/nobjtk.dll:	 $(OBJDIR)/o.dep $(NOMOBJECTS) $(OBJECTS) ./exports.def
	@echo "[33;1;mLinking "$@"...[0;m"
	cmd.exe /C create_vobjtk_def $(OBJDIR)/nobjtk.def
	$(CC) $(GCCLDFLAGS)  -o $@ $(OBJECTS) $(LIBS) $(OBJDIR)/nobjtk.def
	emximp -o $(OBJDIR)/$(basename $(notdir $@)).a $(OBJDIR)/nobjtk.def
	@echo "[32;1;mDone linking "$@"...[0;m"

$(OBJDIR)/cwwp.dll:	 $(WPSIHFILES) $(WPSOBJECTS) cwwp.def
	@echo "[33;1;mLinking "$@"...[0;m"
	gcc -o $@ $(GCCLDFLAGS) $(WPSOBJECTS) $(WPSLIBS) cwwp.def
	implib $(basename $@).lib cwwp.def
	@echo "[32;1;mDone linking "$@"...[0;m"

###################################
#
#    Rules for compiling
#
###################################

# FIXME: includes should be created in the object dir
BASE_SCCOMPILEC		=	sc -I $(IDLDIR) -p -r -sc -d$(CLASSCDIR) -maddstar -mnoint -S128000 -C128000  $< && $(TOUCHFILE)
BASE_SCCOMPILEIH	=	sc -I $(IDLDIR) -p -r -sih -d$(CLASSINC) -maddstar -mnoint -S128000 -C128000  $< && $(FIXLINKAGEIH)
BASE_SCCOMPILEH		=	sc -I $(IDLDIR) -p -r -sh -d$(CLASSINC) -maddstar -mnoint -S128000 -C128000  $< && $(FIXLINKAGEH)

BASE_NOMCOMPILEH	=	$(IDLCOMP) --showcpperrors --header --output-dir=$(CLASSINC) $<
BASE_NOMCOMPILEIH	=	$(IDLCOMP) --showcpperrors --ihfile --output-dir=$(CLASSINC) $<
BASE_NOMCOMPILEC	=	$(IDLCOMP) --showcpperrors --c-template --output-dir=$(CLASSTEMPLATEDIR) $< 
 

$(OBJDIR)/%.o:	$(CLASSCDIR)/%.c $(CLASSINC)/%.ih $(NOMINCLUDES)
	$(CC) -I $(INC) $(GCCFLAGS)  -o$@ $<

$(OBJDIR)/%.o:	$(CDIR)/%.c $(NOMINCLUDES)
	$(CC) -I $(INC) $(GCCFLAGS)  -o$@ $<

# For WPS classes
$(OBJDIR)/%.obj:	$(WPSCDIR)/%.c $(INCDIR)/nomtk.h
	gcc -I $(GCCINC) $(GCCFLAGS) -o $@ $<


$(CLASSINC)/%.h:	$(IDLDIR)/%.idl
	$(BASE_NOMCOMPILEH)

$(CLASSINC)/%.ih:	$(IDLDIR)/%.idl
	$(BASE_NOMCOMPILEIH)
	$(BASE_NOMCOMPILEH)
	$(BASE_NOMCOMPILEC)

$(CLASSCDIR)/%.c:	$(IDLDIR)/%.idl
	$(BASE_NOMCOMPILEIH)
	$(BASE_NOMCOMPILEH)
	$(BASE_NOMCOMPILEC)
	@echo '!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'
	@echo '!!! C template file created. Copy template source now !!!'
	@echo '!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'

# Create object dir for compilation results
$(OBJDIR)/o.dep:
	mkdir -p $(OBJDIR)
	echo > $(OBJDIR)/o.dep
	mkdir -p $(BINDIR)
	mkdir -p $(CLASSTEMPLATEDIR)

clean:
	@cd $(OBJDIR) && rm *
