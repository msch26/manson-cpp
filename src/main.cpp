/*
 * main.cpp
 *
 *	Copyright (C) 2020 Marco Scholtyssek <code@scholtyssek.org>
 *  Created on: Oct 12, 2020
 *
 */

#include "HCS.h"
#include <iostream>

int main(int argc, char **argv) {
	std::cout << "starting Manson HCS test\n\n";
	HCS h("/dev/ttyUSB0", static_cast<unsigned int>(9600));
	h.connect();
	h.test();


	h.disconnect();
}



