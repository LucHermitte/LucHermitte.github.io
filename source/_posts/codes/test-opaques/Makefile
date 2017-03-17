.PHONY: default depend all clean install

SRCDIR   = .
OBJDIR   = .
BINDIR   = .
DEPDIR   = .depends
TARGET   = domain
CXX      = clang++
CXXFLAGS = -std=c++14 -fno-elide-type -stdlib=libc++
LDFLAGS  = -stdlib=libc++ -gdwarf-2 -lcxxrt $(shell llvm-config --libs core --ldflags)

SED    = sed
SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))
DEPS = $(patsubst %.c,$(DEPDIR)/%.d,$(SRCS))
DEPS = $(OBJ:%.o=%.d)

$(BIN) : $(BINDIR)/$(BIN)

# Lorsque la prise en compte de dépendances n'est pas nécessaire à la génération
# de certaines cibles comme clean
DEPEND=no

ifeq ($(findstring all,$(MAKECMDGOALS)),all)
    DEPEND = yes
    -include $(DEPS)
endif
ifeq ($(findstring lib,$(MAKECMDGOALS)),lib)
    DEPEND = yes
    -include $(DEPS)
endif

default:
	@echo "Available Make commands are:"
	@echo ""
	@echo " - all:         Compiles and builds library"
	@echo " - install:     Installs lib and include into $(PREFIX)"
	@echo " - clean:       Cleans products."
	@echo ""

all: $(TARGET)

# Link
$(LIBRARY): depend $(OBJS) $(BINDIR)
	@echo "Archiving $(LIBRARY) ..."
	@$(CXX) $(LDFLAGS) $(LIBRARY) $(OBJS)
	@echo "done"

# Compilation
$(OBJDIR)/%.o:$(SRCDIR)/%.cpp $(OBJDIR)
	$(CXX) $(CXXFLAGS) -I$(SRCDIR) -c "$<" -o "$@"

$(DEPDIR)/%.d:$(SRCDIR)/%.cpp $(DEPDIR)
	$(CXX) $(DEPFLAGS) $(CXXLAGS) $(CPPFLAGS) -I$(SRCDIR) $< \
	| $(SED) 's,\($*\)\.o[ :]*,\1.o $@: ,' > $@

# Dependances façon gcc
depend: $(DEPDIR) $(DEPS)

# Directories
$(LIBDIR):
	mkdir $(LIBDIR)

$(OBJDIR):
	mkdir $(OBJDIR)

$(DEPDIR):
	mkdir $(DEPDIR)

$(PREFIX)/include:
	mkdir -p $(PREFIX)/include

$(PREFIX)/bin:
	mkdir -p $(PREFIX)/bin

$(PREFIX)/lib:
	mkdir -p $(PREFIX)/lib

clean:
	@rm -rf $(DEPDIR) $(OBJDIR) $(LIBDIR)

