# sat_GPU
This program solves the Propositional (Boolean) Satisfiability problem using Depth-First Search algorithm.
<br>
Problems are read from an input file, while solution is written to screen and an output file.
<br>
Two implementations are included, one executed only in CPU, and one which validates each vector using OpenCl.
<br>
GPU implementation requires *opencl-headers* and *clinfo* packages to be installed, along with the corresponding platform sdk.

## Usage
Both version can be invocted via the Makefile, or by directly compiling and executing.

### Make usage
#### CPU code
```shell
$ make cpu
```
To include a different input file:
```shell
$ make cpu FILE={file_path}
```

#### GPU code
```shell
$ make gpu
```
To configure GPU workers:
```shell
$ make gpu WORKERS={number}
```
To include a different input file:
```shell
$ make gpu FILE={file_path}
```

### Direct usage
#### CPU code
Compilation:
```shell
$ gcc -o sat_CPU sat_CPU.c
```
Execution:
```shell
$ ./sat_CPU {file_path}
```

#### GPU code
Compilation:
```shell
$ gcc -o sat_GPU sat_GPU.c -lOpenCL
```
Execution:
```shell
$ ./sat_GPU {workers_number} {file_path}
```

## Execution examples
### CPU code
```shell
$ make cpu
Executing CPU code...
gcc -o sat_CPU sat_CPU.c
./sat_CPU test_file.txt

This programm solves the Propositional (Boolean) Satisfiability Problem written
in file test_file.txt, using Depth First Search Algorithm.

Solution found with depth-first!

Solution vector propositions values:
P1=true  P2=true  P3=true  P4=true  P5=false  P6=false  P7=true  P8=true  P9=false  P10=true  P11=false  P12=true  P13=false  P14=true  P15=false  P16=false  P17=true  P18=false  P19=false  P20=false

Time spent: 21.848 secs
```

### GPU code
```shell
$ make gpu
Executing GPU code...
gcc -o sat_GPU sat_GPU.c -lOpenCL
./sat_GPU 100 test_file.txt

This OpenCL programm solves the Propositional (Boolean) Satisfiability Problem
written in file test_file.txt, using Depth First Search Algorithm.
Number of work items: 100

Device info:

1 platforms detected
Platform 0:
    Vendor: NVIDIA Corporation
    Name: NVIDIA CUDA

1 devices detected
Device 0:
    Device: NVIDIA Corporation
    Name: NVIDIA GeForce GTX 1070

No build errors, starting solving the problem...

Solution found with depth-first!

Solution vector propositions values:
P1=true  P2=true  P3=true  P4=true  P5=false  P6=false  P7=true  P8=true  P9=false  P10=true  P11=false  P12=true  P13=false  P14=true  P15=false  P16=false  P17=true  P18=false  P19=false  P20=false

Time spent = 9.538
GPU execution time = 0.000
Communication time = 9.420
```
