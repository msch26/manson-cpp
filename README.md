# Manson HCS-3XXX C++ library

This repository contains a C++ library to control [Manson HCS](https://www.manson.com.hk/product/hcs-3202/) USB programmable power supplys.
It was tested with a HCS 3302 device. The library implements all UART commands, that are available for that device.

## Contact

To keep up with the latest announcements for this project, or to ask questions:

Questions, comments and bug reports can be send to:

 * Marco Scholtyssek <code@scholtyssek.org>
 * **Twitter** [@_marbo_](https://twitter.com/_marbo_)

## Licence

This library is licenced under *MIT* licence. So you can use it, but there is no support.
It would be nice to give a feedback if you are using this library.

## Quick start

- Clone the repo: `git clone https://github.com/msch26/manson-cpp`
- Build with make

## Simulation mode:

**The simulation mode is not implemented yet**

To use the simulation mode, create a socat stream:

```bash
$ socat PTY,link=/tmp/virtual-tty,raw,echo=0 -
```

## Library
A static library will be created in lib/ dir. It can be used to link your own implementation.


## Examples
The example will be created in build/ directory. It can be run by ./build/manson-example

### setting voltage and current

```C++
#include <iostream>
#include "HCS.h"

const std::string SERIAL_DEVICE = "/dev/ttyUSB0";

int main(int argc, char **argv) {

    HCS h(SERIAL_DEVICE, static_cast<unsigned int>(9600));
    h.connect();
    std::cout << "max voltage: " << h.getMaxVoltage() << "V\n";
    std::cout << "max current: " << h.getMaxCurrent() << "A\n";
    std::cout.flush();

    h.setVoltage(3.0f);
    h.setCurrent(0.1f);
		
    h.disconnect();
	return 0;
}
```

### setting limits

```C++
#include "HCS.h"

const std::string SERIAL_DEVICE = "/dev/ttyUSB0";

int main(int argc, char **argv) {
    HCS h(SERIAL_DEVICE, static_cast<unsigned int>(9600));
    h.connect();
    
	// sets the upper voltage limit
	// caution, setVoltage() will fail, if the voltage to set is higher that the upperVoltageLimitValue
	h.setUpperVoltageLimit(32);
	
	// same behaviour for current
	h.setUpperCurrentLimit(5);
    
    h.disconnect();
    return 0;
}
```

### Memory functions

```C++
#include <chrono>
#include <thread>
#include "HCS.h"

const std::string SERIAL_DEVICE = "/dev/ttyUSB0";

int main(int argc, char **argv) {
    HCS h(SERIAL_DEVICE, static_cast<unsigned int>(9600));
    h.connect();
    
	// store memory values
	setMemory(1.5f, 1.0f, 2.5f, 4.2f, 3.5f, 3.3f);
	
	std::this_thread::sleep_for(std::chrono::seconds(5));
	// set values stored in memory on location 0, 1 or 2
	runMemory(HCS::M0);
	std::this_thread::sleep_for(std::chrono::seconds(5));
	runMemory(HCS::M1);
	std::this_thread::sleep_for(std::chrono::seconds(5));
	runMemory(HCS::M2);
    
    h.disconnect();
	return 0;
}
```

