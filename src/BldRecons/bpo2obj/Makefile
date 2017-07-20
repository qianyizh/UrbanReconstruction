# File : Makefile -- makefile for bpo2obj.

include ../../../Makefile.inc

CPPFLAGS = -I. -I../Common -I../../../core
CPPFLAGS += `$(PKG_CONFIG) --cflags opencv`

LDFLAGS = -L../Common -L../../../core
LDFLAGS += `$(PKG_CONFIG) --libs opencv` 

LDLIBS = -lmodeling -lboost_program_options -lcore -lboost_filesystem

ifeq ($(UNAME),Darwin)
	LDLIBS += -lboost_system
else
	LDLIBS += -lml
endif

EXECUTABLE = bpo2obj

SRC = bpo2obj.cpp

OBJ = $(SRC:.cpp=.o)

all: $(SRC) $(EXECUTABLE)

docs: $(EXECUTABLE).1 $(EXECUTABLE).pdf

$(EXECUTABLE): $(OBJ)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(OBJ) $(LDLIBS) -o $@

clean:
	rm -f $(OBJ) $(EXECUTABLE) $(EXECUTABLE).1 $(EXECUTABLE).pdf

install: all docs
	cp $(EXECUTABLE) $(INSTALL_DIR)/bin
	cp $(EXECUTABLE).1 $(INSTALL_DIR)/man/man1
	cp $(EXECUTABLE).pdf $(INSTALL_DIR)/docs


