###########################################

# File Name : Makefile

# Purpose : Makefile for

# Creation Date : 20-05-2013

# Last Modified : Mon 20 May 2013 09:18:43 PM CST

# Created By : Philip Zhang 

############################################

CC=g++
CFLAGS= -g -w
LDFLAGS= `pkg-config --cflags --libs opencv`
PROGRAM= robust

.cpp.o:
	$(CC) -c $< $(CFLAGS)
$(PROGRAM): robust.cpp
	$(CC) -o $@ $< $(LDFLAGS)

pyrlk: pyrlk.cpp
	$(CC) -o $@ $< $(LDFLAGS)

add:
	git add *
