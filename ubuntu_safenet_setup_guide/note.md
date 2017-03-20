# Setup
- extract the `protecttoolkit5_x32_ubuntu.tar.gz` or `protecttoolkit5_x64_ubuntu.tar.gz` to /opt
```bash
tar -xf protecttoolkit5_x32_ubuntu.tar.gz -C /opt
# or
tar -xf protecttoolkit5_x64_ubuntu.tar.gz -C /opt
```

# To build the `session_build` or `casign`
- cd to <package-folder>/src/
- make clean
- make
(no more action should be taken, the environment is already specified in the Makefile)

# To run the application
```bash
export ET_HSM_NETCLIENT_SERVERLIST=192.168.9.76
export LD_LIBRARY_PATH=/opt/safenet/protecttoolkit5/ptk/lib/linux-i386/
```
