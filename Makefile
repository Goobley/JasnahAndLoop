CXX=clang++

HERE:=$(shell pwd)

TARGET=loop
INTLIBTARGET=libLoop.so
SRCEXT=cpp
SRCDIR=src
INTLIBSRCDIR=libSrc
OBJDIR=obj
BINDIR=bin
INC=

CXXFLAGS=-std=c++11 -fPIC

OPTFLAGS1=-O0 -g
OBJECT=-c
ASM=-S
OPTFLAGS=$(OPTFLAGS1)
# OBJECT=$(ASM)

FEATUREFLAGS1=-DDEBUG
FFLAGS=$(FEATUREFLAGS1)

WARNINGFLAGS=-Wall -Wextra

LDFLAGS=
LIBS=
LIBDIR=

## Platform specific library requirements
UNAME_S:=$(shell uname -s)
ifeq ($(UNAME_S),Linux)
LIBS+= dl
LDFLAGS+= -pthread
endif

SDLFLAGS=$(shell sdl2-config --cflags)
SDLLIBS=$(shell sdl2-config --libs)

## Shouldn't need to modify much below this line other than SDL ##
INC:=$(addprefix -I,$(INC))
LIBS:=$(addprefix -l,$(LIBS))
LIBDIR:=$(addprefix -L,$(LIBDIR))
CXXFLAGS+=$(OBJECT) $(OPTFLAGS) $(WARNINGFLAGS) $(FFLAGS) $(INC)
INTLIBSOURCES:=$(shell find $(INTLIBSRCDIR) -name '*.$(SRCEXT)')
SOURCES:=$(shell find $(SRCDIR) -name '*.$(SRCEXT)')
SRCDIRS:=$(shell find . -name '*.$(SRCEXT)' -exec dirname {} \; | uniq)
OBJECTS:=$(patsubst %.$(SRCEXT),$(OBJDIR)/%.o,$(SOURCES))
INTLIBOBJECTS:=$(patsubst %.$(SRCEXT),$(OBJDIR)/%.o,$(INTLIBSOURCES))
CXXFLAGS+=$(SDLFLAGS)
LIBS+=$(SDLLIBS)
LDFLAGS+=$(LIBDIR) $(LIBS)
DEPS:=$(OBJECTS:.o=.d)

## Don't touch below this line ##
.phony: all clean distclean

all: $(BINDIR)/$(INTLIBTARGET) $(BINDIR)/$(TARGET)

$(BINDIR)/$(TARGET): $(OBJECTS)
	@mkdir -p `dirname $@`
	@echo "Linking $@..."
	@$(CXX) $(OBJECTS) $(OPTFLAGS) $(LDFLAGS) -o $@

$(BINDIR)/$(INTLIBTARGET): buildrepo $(INTLIBOBJECTS)
	@echo "Building Shared: $@..."
	@$(CXX) -shared $(INTLIBOBJECTS) $(OPTFLAGS) $(LDFLAGS) -o $@

-include $(DEPS)

$(OBJDIR)/%.o: %.$(SRCEXT)
	@echo "Generating dependencies for $<..."
	@$(call make-depend,$<,$@,$(subst .o,.d,$@))
	@echo "Compiling $<..."
	@$(CXX) $(CXXFLAGS) $< -o $@

clean:
	$(RM) -r $(OBJDIR)

distclean: clean
	$(RM) -r $(BINDIR)/$(TARGET)
	$(RM) -r $(BINDIR)/$(INTLIBTARGET)

buildrepo:
	@$(call make-repo)

define make-repo
   for dir in $(SRCDIRS); \
   do \
	mkdir -p $(OBJDIR)/$$dir; \
   done
endef


# usage: $(call make-depend,source-file,object-file,depend-file)
define make-depend
  $(CXX) -MM       \
         -MF $3    \
         -MP       \
         -MT $2    \
         $(CXXFLAGS) \
         $1
endef
