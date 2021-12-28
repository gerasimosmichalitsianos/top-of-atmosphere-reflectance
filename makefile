#
# define name of output executable 
#
PROG = bin/toa

#
# specify name of C++ compiler. 
#
CC = g++

#
# include directories for header files. 
#
pugixml=libs/pugixml-1.11/src
CPPFLAGS = -g -Wall -I$(pugixml) -I/usr/include/gdal -std=c++17
LDFLAGS = -L/usr/lib -L/usr/local/lib -lgdal -lm

# 
# clean-up option to remove executable. 
# 
all:
	@$(CC) -O2 -std=c++11 src/Main.cpp src/TOAUtil.cpp src/Misc.cpp src/ImageUtil.cpp $(CPPFLAGS) $(LDFLAGS) -o $(PROG)

clean:
	@rm $(PROG)
