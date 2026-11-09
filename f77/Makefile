LAPACK_LIB_DIR ?= 

CFITSIO_LIB_DIR ?= 
CFITSIO_LIB ?= $(CFITSIO_LIB_DIR)/libcfitsio.so

LDFLAGS := -L$(LAPACK_LIB_DIR)
LDLIBS := -llapack -lblas $(CFITSIO_LIB)
FFLAGS := -mcmodel=medium -w

FC := mpif77
TARGET := Fourier_Quad_Pipe
SOURCES := $(sort $(wildcard *.f))

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCES) para.inc cust_para.inc sig_para.inc path_layout.inc
	$(FC) $(SOURCES) -o $@ $(FFLAGS) $(LDFLAGS) $(LDLIBS)

clean:
	$(RM) $(TARGET) *.o *.mod
