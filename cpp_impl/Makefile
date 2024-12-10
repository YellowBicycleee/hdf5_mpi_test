TARGETS = read4d write4d read7d write7d

read4d: h5cpp_read_4d.cpp
	mpicxx h5cpp_read_4d.cpp  -o $@ \
    -I/usr/include/hdf5/openmpi \
    -L/usr/lib/x86_64-linux-gnu/hdf5/openmpi \
    -lhdf5_cpp -lhdf5_openmpi

write4d: h5cpp_write_4d.cpp
	mpicxx h5cpp_write_4d.cpp -o $@ \
    -I/usr/include/hdf5/openmpi \
    -L/usr/lib/x86_64-linux-gnu/hdf5/openmpi \
    -lhdf5_cpp -lhdf5_openmpi

run4d: 
	mpirun -np 1 ./write4d && mpirun -np 1 ./read4d

read7d: h5_read_7d.cpp
	mpicxx h5_read_7d.cpp -o $@ \
    -I/usr/include/hdf5/openmpi \
    -L/usr/lib/x86_64-linux-gnu/hdf5/openmpi \
    -lhdf5_cpp -lhdf5_openmpi

write7d: h5_write_7d.cpp
	mpicxx h5_write_7d.cpp -o $@ \
    -I/usr/include/hdf5/openmpi \
    -L/usr/lib/x86_64-linux-gnu/hdf5/openmpi \
    -lhdf5_cpp -lhdf5_openmpi

run7d: 
	mpirun -np 1 ./write7d && mpirun -np 1 ./read7d

all: $(TARGETS)

.PHONY: clean
clean:
	rm -f $(TARGETS)
