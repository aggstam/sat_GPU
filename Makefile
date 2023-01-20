FILE = test_file.txt
WORKERS = 100

cpu:
	$(info Executing CPU code...)
	gcc -o sat_CPU sat_CPU.c
	./sat_CPU $(FILE)

gpu:
	$(info Executing GPU code...)
	gcc -o sat_GPU sat_GPU.c -lOpenCL
	./sat_GPU $(WORKERS) $(FILE)

clean:
	rm -f sat_CPU sat_GPU
