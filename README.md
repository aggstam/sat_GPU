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
```
% make cpu
```
To include a different input file:
```
% make cpu FILE={file_path}
```

#### GPU code
```
% make gpu
```
To configure GPU workers:
```
% make gpu WORKERS={number}
```
To include a different input file:
```
% make gpu FILE={file_path}
```

### Direct usage
#### CPU code
Compilation:
```
% gcc -o sat_CPU sat_CPU.c
```
Execution:
```
% ./sat_CPU {file_path}
```

#### GPU code
Compilation:
```
% gcc -o sat_GPU sat_GPU.c -lOpenCL
```
Execution:
```
% ./sat_GPU {workers_number} {file_path}
```
