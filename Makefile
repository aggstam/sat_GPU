cpu:
	gcc -o sat_CPU sat_CPU.c

gpu:
	gcc -o sat_GPU sat_GPU.c -lOpenCL

clean:
	rm -f sat_CPU sat_GPU
