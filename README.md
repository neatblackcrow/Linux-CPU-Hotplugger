# Linux-CPU-Hotplugger
This application simply monitor a system load from */proc/stat* then decides a number CPU core(s) online.

**Note:** The minimum core powered on is always >= 1

It will not going to be a full featured app. But means to help some users to reduce the power consumption of their laptops.

Mainly because, Linux isn't targeting a desktop users at the first place - High performance computing / server aren't suitable for battery powered devices.

**Screenshot:**
Inline-style: 
![alt text](https://github.com/fieldfirst/Linux-CPU-Hotplugger/blob/master/screen-shot.png "Main UI")

**P.S.:** For further power management or system administration tools, plese check out its sister project - [AutoCGroup] (https://github.com/fieldfirst/AutoCGroup)


For a Linux user out there I also recommend this options:

1. Reduce **a timer clock frequency to 300Hz** - By compiling your own custom kernel

    *Note: You can just grab a stock config from /proc/config.gz (by using zcat)*

2. Adjust your p_state values (for Sandy bridge or newer with kernel version >= 3.18) otherwise adjust your CPU's governor instead.

3. Using this simple application.

    *Note: You must enable a __cgroup subsystem__ - By compiling your own custom kernel*
