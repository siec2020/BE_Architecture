##### authors: Raphael BENISTANT, Laurent DEVEZ

# BE_Architecture: path finding *A\** algorithm CPU and GPU implementation

## Repository organization:

* **_Test\_CPU folder:_** contains several cpp files which corresponds to different implementation of the * *A\** algorithm on CPU. Only the *rosetta_A_star.cpp* is fully functionnal.
* **_CPU\_Finalfolder:_** contains our final version for the * *A\** algorithm implementation on CPU.
* **_GPU\_Final folder:_** contains our final version for the * *A\** algorithm implementation on CPU + GPU.


## How to compile and execute source file:

* **_CPU_Final/rosettaA_star.cpp file:_** from this folder
```
cd ./Test_CPU
g++ -o rosettaA_star rosettaA_star.cpp
./rosrosettaA_star
```
* **_GPU_Final/rosettaA_star.cu file:_** from this folder
```
cd ./Test_GPU
nvcc -o rosettaA_star rosettaA_star.cpp
./rosrosettaA_star
```

## Further information:

In each final version, execution time is measured. We can then compare the efficieny of the CPU version versus the GPU version.
