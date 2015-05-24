basem
=====

Brings Sys-V semaphore efficiently to the bash

# Usage

      [usage] <progname>	[-c | --create ] //create semaphor 
 	[ -d | --destroy ] //destroy semaphor 
 	[ -p | --pop ] //Do the p-operation on the given semaphor 
 	[ -v | --vop] // Do the v-operation on the given semaphor 
 	[ -r | --rights ] // Define the rights to use for newly created semaphors 
 	[ -k | --key ] // Define the Key to use for the newly created semaphor
 	[ -i | --init-value] //Set the default value while creation 
 		

 	Examples 
	$ <prog> --create --rights 0600 --value 100 --key 12345 
	# creates a semaphor. If it is created, this will be a nop. 

	$ <prog> --destroy --key 12345 
	# destroys the previously created semaphor 
	# It exits with 0 if successful and 1 if otherwise 

	$ <prog> --key 12345 --pop 
	# Does the p-Op on the Semaphor (key: 12345) 

	$ <prog> --key 12345 --vop 
	# Does the v-Op on the Semaphor (key: 12345)
	The standard values for permission and value are 0600 and 0
