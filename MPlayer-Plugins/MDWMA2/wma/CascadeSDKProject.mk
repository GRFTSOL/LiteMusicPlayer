# -*- Makefile -*-
# Cascade SDK project makefile
# Author: dwoodward@rokulabs.com

.EXPORT_ALL_VARIABLES:

CC     := gcc
CXX    := g++
TARGET_CFLAGS   := 
TARGET_CXXFLAGS := 

CASCADE_SDK_INCLUDE_DIR := /usr/local/include/roku

CFLAGS := $(TARGET_CFLAGS) $(EXTRA_CFLAGS)
CFLAGS += -I$(CASCADE_SDK_INCLUDE_DIR)

CXXFLAGS := $(TARGET_CXXFLAGS) $(EXTRA_CXXFLAGS)
CXXFLAGS += -I$(CASCADE_SDK_INCLUDE_DIR)

LDFLAGS  := $(EXTRA_LDFLAGS)
ifdef BUILD_SHARED_LIB
LIBRARIES := $(shell basename `pwd`).so
LDFLAGS += -shared
else
PROGRAMS := $(shell basename `pwd`)
endif
OBJECT_DIR := .

all: $(patsubst %, $(OBJECT_DIR)/%, $(PROGRAMS) $(LIBRARIES))

rebuild: clean all

CPPSOURCES := $(shell find . -name '*.cpp')
CSOURCES   := $(shell find . -name '*.c') 
EVERY_SOURCEFILE := $(CPPSOURCES) $(CSOURCES) $(shell find . -name '*.h')

TAGS: $(EVERY_SOURCEFILE)
	etags $^

.depend: $(EVERY_SOURCEFILE)
	rm -f $@
	for file in $(CPPSOURCES); do echo -n \$$\(OBJECT_DIR\)/ >> $@; $(CXX) -M $(CXXFLAGS) $$file >> $@; done
	for file in $(CSOURCES);   do echo -n \$$\(OBJECT_DIR\)/ >> $@; $(CC)  -M $(CFLAGS)   $$file >> $@; done

include .depend

$(OBJECT_DIR)/%.o: %.cpp $(OBJECT_DIR)/.made_object_dirs
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(OBJECT_DIR)/%.o: %.c $(OBJECT_DIR)/.made_object_dirs
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJECT_DIR)/.made_object_dirs:
	mkdir -p `dirname $@`
	-touch $@

ALL_OBJECTS := $(patsubst %.c,  %.o,$(CSOURCES))
ALL_OBJECTS += $(patsubst %.cpp,%.o,$(CPPSOURCES))

clean:
	rm -rf $(OBJECT_DIR)/.made_object_dirs $(ALL_OBJECTS) $(patsubst %, $(OBJECT_DIR)/%, $(PROGRAMS) $(LIBRARIES)) .depend TAGS

$(OBJECT_DIR)/$(PROGRAMS): $(ALL_OBJECTS) $(OBJECT_DIR)/.made_object_dirs
	$(CXX) $(LDFLAGS) -o $@ $(ALL_OBJECTS) 

$(OBJECT_DIR)/$(LIBRARIES): $(ALL_OBJECTS) $(OBJECT_DIR)/.made_object_dirs
	$(CXX) $(LDFLAGS) -o $@ $(ALL_OBJECTS) 
