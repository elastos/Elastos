01/28/2019 Tang Zhilong  <stiartsly@gmail.com>
Release-v5.2.1 Main changes:

	- Support carrier group without central administration, and group peers should be required connected to carrier network;
	- Support file transfer between to carrier peers with pull-driven mode and support to resume transfering from previous break-point as well;
	- Support to send binary message for either carrier or session;
	- Enlarge the invitation data length to almost 8K bytes when sending invitation request/reply message;
	- Refactored exception implementation;
	- Refactored API testsuites to be compatible to use Native Robot (A test stub deaemon);
	- Optimization and bugfixes to carrier/session/stream module;
	- Bugfix for low chance of succeeding to esablish session connectivity when both pees' network topology is behind symmetric NAT.
	- Integrate CI procedure.

