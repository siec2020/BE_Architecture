##### authors: Raphael BENISTANT, Laurent DEVEZ

# BE_Architecture: path finding *A\** algorithm CPU and GPU implementation

## Repository organization:

* **_Test\_CPU_** *folder:* contains several cpp files which corresponds to different implementation of the * *A\** algorithm on CPU. Only the *rosetta_A_star.cpp* is fully functionnal.
* **_CPU\_Final_** *folder:* contains our final version for the * *A\** algorithm implementation on CPU.
* **_GPU\_Final_** *folder:* contains our final version for the * *A\** algorithm implementation on CPU + GPU.


## How to compile and execute source file:

* **_CPU_Final/rosettaA_star.cpp_** *file:* from this folder
```
cd ./Test_CPU
g++ -o rosettaA_star rosettaA_star.cpp
./rosrosettaA_star
```
* **_GPU_Final/rosettaA_star.cu_** *file:* from this folder
```
cd ./Test_GPU
nvcc -o rosettaA_star rosettaA_star.cpp
./rosrosettaA_star
```

## Further information:

In each final version, execution time is measured. We can then compare the efficieny of the CPU version versus the GPU version.
