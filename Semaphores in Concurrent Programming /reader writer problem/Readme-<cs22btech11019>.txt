							                              README

					                     	Implement solutions to Readers-Writers (writerpreference) 
						                        and Fair Readers-Writers problems using Semaphores
							
																	
																	
															        DUDEKULA DHEERAJ
																 CS22BTECH11019


1. Readers-Writers using Semaphores

    File: rw-<cs22btech11019>.cpp
    Description: This implementation utilizes semaphores to control access to the shared resource. It allows multiple readers to access the resource simultaneously but ensures exclusive access for writers.

2. Readers-Writers for Fair Resource Allocation

    File: frw-<cs22btech11019>.cpp
    Description: This implementation ensures fairness in resource allocation by employing a turnstile mechanism. It prevents starvation by fairly allowing readers and writers access to the resource.
					


Compilation: Compile the codes using a C++ compiler. For example:

	   g++ rw-\<cs22btech11019\>.cpp 
	   g++ frw-\<cs22btech11019\>.cpp 


Execution: Run the compiled executable:

    	./a.out

*NOTE* Here for better output for the average times I outputed with two different Average waiting time files as follows
       for :
       Fair Reader Writer : Average_time_FRW.txt
       for :
       Reader Writer      : Average_time_RW.txt
							
