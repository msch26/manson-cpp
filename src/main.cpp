/*
 * main.cpp
 *
 *	Copyright (C) 2020 Marco Scholtyssek <code@scholtyssek.org>
 *  Created on: Oct 12, 2020
 *
 */

#include "HCS.h"
#include <iostream>

const std::string SERIAL_DEVICE = "/dev/ttyUSB0";
constexpr unsigned int delay_ms = 1500;

#include <thread>
int main(int argc, char **argv) {


	std::cout << "starting Manson HCS test\n\n";
	HCS h(SERIAL_DEVICE, static_cast<unsigned int>(9600));
	h.connect();
//	h.test();


	h.setVoltage(15.2f);
	h.getPresentVoltageAndCurrent();
	std::cout << "\n\n";
	std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));

	h.setVoltage(12.2f);
	h.getPresentVoltageAndCurrent();
	std::cout << "\n\n";
	std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));

	h.setVoltage(5.4f);
	h.getPresentVoltageAndCurrent();
	std::cout << "\n\n";
	std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));

	h.setVoltage(7.3f);
	h.getPresentVoltageAndCurrent();
	std::cout << "\n\n";
	std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));

	h.setVoltage(6.9f);
	h.getPresentVoltageAndCurrent();
	std::cout << "\n\n";

	h.disconnect();

	/***** reconnect *****/

	h.connect();


	h.setVoltage(18.4f);
	auto voltageAndCurrent = h.getPresentVoltageAndCurrent();
	std::cout << voltageAndCurrent << "\n\n";		/* work with the return value */
	std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));

	h.disconnect();
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	/***** reconnect with a new instance *****/

	HCS j(SERIAL_DEVICE, static_cast<unsigned int>(9600));

	j.connect();
	std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
	j.setVoltage(19.0f);
	std::cout << j.getPresentVoltageAndCurrent() << "\n\n";
	std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));

	int k = 0;
	while((k+=2) <= 32){
		j.setVoltage(k);
		std::cout << j.getPresentVoltageAndCurrent() << "\n\n";
		std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
	}

	j.disconnect();
}



