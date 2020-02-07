G+ = g++ 
C = gcc
#　头文件搜索路径
INC = -I ./include_u 
# 所依赖的库
OPENCV_LIB=$(shell pkg-config opencv --cflags --libs)
#user 所依赖的库
USER_LIB = -lpthread -lX11 -fopenmp -lpthread    -ldl libdvp.so libhzd.so usb2_m_all.dscam.so usb3_m_all.dscam.so $(INC) -Wl,-rpath=.

#　指定g++参数

CFLAGS = -gdwarf-2 -g3 -Wall -std=c++11
#　最终的可执行文件名
TARGET = ./bigGun

# 将源文件中所有的.c文件全部编译生成.O文件并放入./output目录中
./output/%.o:./src/%.c ./include_u 
	$(C) $(INC) $(CFLAGS) -c $< -o $@

#　将源文件中所有的.cpp文件全部编译生成.O文件并放入./output目录中
./output/%.o:./src/%.cpp ./include_u 
	$(G+) $(INC) $(CFLAGS) -c $< -o $@
 

# 展开源文件目录下的所有的.cpp文件，并将其存放于SOURCES中
SOURCES = $(wildcard ./src/*.cpp ./src/*.c) 
# 去除SOURCES中的路径存放于SOURCES_NDIR中
SOURCES_NDIR=$(notdir $(SOURCES))
# 替换,得到./output/*.o的集合　  
OBJS = $(patsubst %.c,./output/%.o,$(patsubst %.cpp,./output/%.o,$(SOURCES_NDIR)))

$(TARGET):$(OBJS)  
	$(G+) $(OBJS) $(OPENCV_LIB) $(USER_LIB) -o $(TARGET)
	chmod a+x $(TARGET) 
	@echo $(SOURCES)
  
clean:  
	rm -rf ./output/*.o $(TARGET)
