# define the C compiler to use
CC = g++

# The recommended compiler flags for the Raspberry Pi
CFLAGS=-Ofast -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s -ggdb -O0

# define any directories containing header files other than /usr/include
INCLUDES = -I../

# define any libraries to link into executable:
LIBS = -lm -lrf24-bcm -lrf24network -lncurses

# define the C source files
SRCS = Rf24CentralNode.cpp 
#T6963/main.cpp T6963/lcd_register.cpp T6963/gpio.cpp T6963/lcd_command.cpp T6963/lcd_controller.cpp T6963/lcd_graphics.cpp T6963/lcd_screen.cpp 

# define the C object files 
#
# This uses Suffix Replacement within a macro:
#   $(name:string1=string2)
#         For each word in 'name' replace 'string1' with 'string2'
# Below we are replacing the suffix .cpp of all words in the macro SRCS
# with the .o suffix
#
OBJS = $(SRCS:.cpp=.o) 

# define the executable file 
MAIN = prog

#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

.PHONY: depend clean

all:    $(MAIN)

$(MAIN): $(OBJS) 
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)
	
Rf24CentralNode.o: Rf24CentralNode.cpp Rf24PacketDefine.h

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file) 
# (see the gnu make manual section about automatic variables)
.cpp.o: 
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) *.o *~ $(MAIN) T6963/*.o

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it

