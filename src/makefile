
# compiler
CXX :=g++ -Wall -w -O3 -msse2 -fPIC -ggdb -pthread
CC :=gcc -Wall -w -O3 -msse2 -fPIC -ggdb -pthread

#CXX :=g++ -fPIC 
#CC :=gcc -fPIC 

#shared library
SFLAG := -fPIC -shared

###### target and sources
# sources

COMMEN_DIR = ../src/API_commen
COMMEN_INC = -I$(COMMEN_DIR)
COMMEN_SRC = $(COMMEN_DIR)/API_commen.cpp

LINEARSVM_DIR = ../src/API_linearsvm
LINEARSVM_INC = -I$(LINEARSVM_DIR)
LINEARSVM_SRC = $(LINEARSVM_DIR)/tron.cpp $(LINEARSVM_DIR)/linear.cpp $(LINEARSVM_DIR)/API_linearsvm.cpp

PCA_DIR = ../src/API_pca
PCA_INC = -I$(PCA_DIR)
PCA_SRC = $(PCA_DIR)/API_pca.cpp

KDTREE_DIR = ../src/kdtree++
KDTREE_INC = -I$(KDTREE_DIR)

FLANN_DIR = ../src/API_flann
FLANN_INC = -I$(FLANN_DIR) -I$(CUTWORD_DIR)/flann
#FLANN_SRC = $(FLANN_DIR)/flann/flann.cpp

CUTWORD_DIR = ../src/API_cutword_jieba
CUTWORD_INC = -I$(CUTWORD_DIR) -I$(CUTWORD_DIR)/limonp

DOC2VEC_DIR = ../src/API_doc2vec/v0.0.3
DOC2VEC_INC = -I$(DOC2VEC_DIR)
DOC2VEC_SRC = $(DOC2VEC_DIR)/API_doc2vec.cpp

# include && lib
INCLUDE = -I../include -I../input/include -I/usr/local/include/opencv -I/usr/local/include/opencv2 -I/usr/local/include -I/usr/local/cuda-6.5/include
LIBARAY = -L../lib -L../input/lib -L/usr/local/lib -L/usr/local/cuda/lib64 -L/usr/lib

# flags
CFLAGS =  -fopenmp  -DHAVE_JPEG -DHAVE_PNG
CXXFLAGS = -fopenmp  -DHAVE_JPEG -DHAVE_PNG \
	 $(COMMEN_INC) $(LINEARSVM_INC) $(PCA_INC) $(KDTREE_INC) $(FLANN_INC) $(REGEX_INC) $(CUTWORD_INC) $(DOC2VEC_INC) $(INCLUDE)

LDFLAGS  = -lm -lpthread -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_ml
LDFLAGS  += -ldl -lrt -ljpeg -lpng  -lz -lcurl -lstdc++ -lglog -lopenblas -lboost_regex -lhdf5

SRCS = $(COMMEN_SRC) $(LINEARSVM_SRC) $(PCA_SRC) $(DOC2VEC_SRC)
OBJS = $(SRCS:%.cpp=%.o)
OBJECTS := $(OBJS)

# BIN && LIB
BIN = ../bin
LIB = ../lib

###### generation rules
#.SUFFIXES: .cpp .c .o .so .a .d
.c.o:
	@echo ----------------------------------------------
	$(CC) $(CFLAGS) -c $< -o $@
	@echo ----------------------------------------------

.cpp.o:
	@echo ----------------------------------------------
	$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo ----------------------------------------------


###### main
#.PHONY : all 
all: lib Demo_word2vec Demo_w2v Demo_emotion Demo_kdtree flann_example
#all: Demo_word2vec Demo_w2v

#deps: $(addprefix $(OBJSPATH),$(DEPS))
Demo_word2vec:$(BIN) $(LIB) $(OBJECTS) Demo_word2vec.cpp
	@echo ----------------------------------------------
	$(CXX) $(CXXFLAGS) Demo_word2vec.cpp  -o $(BIN)/Demo_word2vec -lword2vec_1.0.0 $(LIBARAY) $(LDFLAGS) 
	#$(CXX) $(CXXFLAGS) Demo_word2vec.cpp  -o $(BIN)/Demo_word2vec $(LIBARAY) $(LDFLAGS)
	@echo ----------------------------------------------

Demo_w2v:$(BIN) $(LIB) $(OBJECTS) Demo_w2v.cpp
	@echo ----------------------------------------------
	$(CXX) $(CXXFLAGS) Demo_w2v.cpp  -o $(BIN)/Demo_w2v -lword2vec_1.0.0 $(LIBARAY) $(LDFLAGS)
	@echo ----------------------------------------------

Demo_emotion:$(BIN) $(LIB) $(OBJECTS) Demo_emotion.cpp
	@echo ----------------------------------------------
	$(CXX) $(CXXFLAGS) Demo_emotion.cpp  -o $(BIN)/Demo_emotion -lword2vec_1.0.0 $(LIBARAY) $(LDFLAGS)
	@echo ----------------------------------------------

Demo_kdtree:$(BIN) $(LIB) $(OBJECTS) Demo_kdtree.cpp
	@echo ----------------------------------------------
	$(CXX) $(CXXFLAGS) Demo_kdtree.cpp  -o $(BIN)/Demo_kdtree -lword2vec_1.0.0 $(LIBARAY) $(LDFLAGS)
	@echo ----------------------------------------------

flann_example:$(BIN) $(LIB) $(OBJECTS) flann_example.cpp
	@echo ----------------------------------------------
	$(CXX) $(CXXFLAGS) flann_example.cpp  -o $(BIN)/flann_example -lword2vec_1.0.0 $(LIBARAY) $(LDFLAGS)
	@echo ----------------------------------------------

lib: $(LIB) $(OBJECTS)
	@echo ----------------------------------------------
	$(CXX) $(CXXFLAGS) -shared -o -fPIC -o $(LIB)/libword2vec_1.0.0.so $(OBJECTS) $(LIBARAY) $(LDFLAGS)
	@echo ----------------------------------------------

clean:
	rm -f $(OBJECTS)
	rm -f $(TARGET)
	rm $(LIB)/libword2vec_1.0.0.so
	rm -f *.o 
	rm $(BIN)/Demo_word2vec
	rm $(BIN)/Demo_w2v 
	rm $(BIN)/Demo_emotion
	rm $(BIN)/Demo_kdtree
	rm $(BIN)/flann_example

rebuild: clean all
install:
	install -T $(TARGET) $(INSTALL_PATH)/bin/$(TARGET)
