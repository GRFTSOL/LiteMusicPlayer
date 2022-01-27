# -*- Makefile -*- 
# standard makefile for cascade-sdk-projects
# Author: dwoodward@rokulabs.com
#
SDK_MAKEFILE_DIR := .

EXTRA_CFLAGS = -I./include -I /mnt/flash1/myogg/include -Wall -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE -msoft-float  -O2 -DHAVE_CONFIG_H 

EXTRA_CXXFLAGS = -I./include -I /mnt/flash1/myogg/include -Wall -O3  -DHAVE_CONFIG_H

EXTRA_LDFLAGS = -lCascade -lm

include $(SDK_MAKEFILE_DIR)/CascadeSDKProject.mk

