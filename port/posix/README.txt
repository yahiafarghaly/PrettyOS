POSIX Port
-----------

This port is developed to work in a OS with a Linux Kernel which supports
POSIX APIs for threads and realtime extensions.

---> The requirement for this port to work:
===========================================

1- A GNU toolchain.
2- Support at least POSIX.1b standard (IEEE Standard 1003.1b-1993) or higher.


---> Configuring the Build Environment:
======================================

On Linux, the maximum realtime priority for processes must be increased.
	1- Open a terminal and acquire root access.
	2- Modify /etc/security/limits.conf by adding the following line in the end of the file:
		"username - rtprio unlimited"
	3- Replace \username with your login name. And save the file.
	4- Log out of your original session and then log back in.

---> Checking the Build Environment:
======================================	
To check that it has been affected the system,
	1- Open the terminal and enter the command "limit | grep "rt_priority""
		If the output is "unlimited". then you're good to build the port files.
		else, revise the steps of "Configuring the Build Environment".
		
		
END