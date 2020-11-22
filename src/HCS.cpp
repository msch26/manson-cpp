/*
 * HCS.cpp
 *
 *	Copyright (C) 2020 Marco Scholtyssek <code@scholtyssek.org>
 *  Created on: Oct 12, 2020
 *
 */

#include "HCS.h"
#include "wiringSerial.h"
#include <iostream>
#include <exception>

#include <cstdlib>

#include <iomanip>
#include <sstream>

#include <chrono>
#include <thread>


#ifdef TEST
#include <vector>
#include <algorithm>
#endif

int HCS::fd = -1;
bool HCS::initialized = false;

const std::string HCS::UART_COMMAND_GMAX = "GMAX";
const std::string HCS::UART_COMMAND_VOLT = "VOLT";
const std::string HCS::UART_COMMAND_CURRENT = "CURR";
const std::string HCS::UART_COMMAND_GETS = "GETS";
const std::string HCS::UART_COMMAND_GETD = "GETD";
const std::string HCS::UART_COMMAND_GOVP = "GOVP";
const std::string HCS::UART_COMMAND_SOVP = "SOVP";
const std::string HCS::UART_COMMAND_GOCP = "GOCP";
const std::string HCS::UART_COMMAND_SOCP = "SOCP";
const std::string HCS::UART_COMMAND_GETM = "GETM";
const std::string HCS::UART_COMMAND_RUNM = "RUNM";
const std::string HCS::UART_COMMAND_PROM = "PROM";

const std::string HCS::UART_RESPONSE_OK = "OK";


class LimitExceededError : public std::runtime_error{
public:
	LimitExceededError(std::string msg):runtime_error(msg.c_str()){}
};

class ExprectedReceiveError : public std::runtime_error{
public:
	ExprectedReceiveError(std::string msg):runtime_error(msg.c_str()){}
};

void HCS::init() {
	if(!connected){
		throw std::runtime_error("failed to read max values. HSC is not connected via uart");
	}
	maxValues = this->getMaxValues();
	getPresentUpperLimitVoltage();
	getPresentUpperLimitCurrent();
	initialized = true;
}

void HCS::setDisconnected()
{
	connected = false;
}

bool HCS::isConnected(void) {
#ifndef SIMULATION
	if(!connected)
	{
		throw std::runtime_error("not connected to HSC");
	}
	if (fd < 0) {
		throw std::runtime_error("serial stream is not open\n");
	}
#endif
	return true;
}


void HCS::setConnected()
{
	connected = true;
}

void HCS::connect(){

#ifdef SIMULATION
	throw std::runtime_error("SIMULATION mode is not implemented completely. Do not use -DSIMULATION in this version.");
	std::cerr << "To use the simulation mode, create a socat stream:\n\tsocat PTY,link=/tmp/virtual-tty,raw,echo=0 -\n";
	std::cout << "-DSIMULATION is set\nusing /tmp/virtual-tty as device, not usb.\n\n";
	this->uart = "/tmp/virtual-tty";
#endif

	if((fd = (serialOpen(uart.data(), baud))) < 0){
		throw std::runtime_error("failed to open serial device");
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(20));
	std::cout << "connecting to device <" << this->uart << "> \nbaud <" << this->baud << ">\n";
	flush();	// clear pending uart buffer
	setConnected();
}

void HCS::disconnect(){
	if(connected){
		serialClose(fd);
		setDisconnected();
	}
}


std::string HCS::receiveViaUart(uint8_t byteCount) {
	isConnected();

	uint8_t byteCounter = 0x0;
	char c;
	char debug[byteCount];
	int i=0;
	std::string received = "";

	std::this_thread::sleep_for(std::chrono::milliseconds(30));

	while(serialDataAvail(fd) == -1)
	{
		std::cout << "waiting for data\n";
	}

	while ((c = serialGetchar(fd)) != -1 && byteCounter < byteCount) {
		if (c < 0x20)  { // || c > 0x7E) {
			continue;
		}
		debug[i] = c;
		++i;
		++byteCounter;


#ifdef DEBUG
//		std::cout << "received byte [" << std::to_string(byteCounter) << "] <"
//				<< c << ">\n";
		std::cout << c;
		std::cout.flush();
#endif
//		std::printf("%d :<%c>\n", byteCount, c);
		received += c;
	}

#ifdef DEBUG
		std::cout << std::endl;
#endif
//	std::printf("%d : %d\n", byteCounter, byteCount);

	if(byteCounter != byteCount){
		std::cerr << "WARN: received " << std::to_string(byteCounter) << " bytes, but exprected " << std::to_string(byteCount) << std::endl;
	}
	return received;
}

bool HCS::receiveOk() {
	std::string s = receiveViaUart(2);

	return (s == "OK") ? true : false;
}

void HCS::verifyReceived(const std::string& receivedData, const std::string& errMsg)
{
	if(receivedData.length() == 0){
		throw std::runtime_error(errMsg);
	}
}

std::string HCS::sendCommand(const std::string& cmd, const uint8_t receiveBytesCount, const bool expectOk)
{
	isConnected();
	std::string response = "";

	send(cmd);

	if(receiveBytesCount > 0)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(150));
		response = receiveViaUart(receiveBytesCount);
	}

//	try{
		if(expectOk)
		{
			// wait to receive "OK" from device
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			if(!HCS::receiveOk()){
				throw ExprectedReceiveError(cmd + " was not acknowledged by HCS. OK is missing");
			}
		}
//	}catch (ExprectedReceiveError& e) {
//		std::cerr << "Exprected receive data missing: " << e.what() << '\n';
//		flush(); // flush triggers receive buffer to be cleared
//	}

	return response;
}

void HCS::send(const std::string& msg)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(20));
	serialPuts(fd, msg.c_str());
	serialPuts(fd, "\r\n");
	flush();
}

/**
 * Sends data, if it still is in buffer.
 * discards all data from received, if there is something in buffer
 */
void HCS::flush(void)
{
	serialFlush(fd);
}

void HCS::uartDebug(const std::string& data)
{
	std::cout << "sending via uart: " << data << "\n";
}

HCS::MansonData HCS::toMansonData(std::string& voltageCurrentString) {
	float voltage = std::atoi(voltageCurrentString.substr(0, 3).c_str());
	float current = std::atoi(voltageCurrentString.substr(3, 3).c_str());

	voltage = voltage / 10.0f;
	current = current / 10.0f;

	return std::move(std::make_pair(voltage, current));
}

void HCS::setCurrent(const float current) {
	try{
		isConnected();

		if(current < 0.0f || current >= static_cast<int>(getMaxCurrent()))
		{
			throw std::runtime_error("current has to be between 0 and 32,5V");
		}
		else if(current > upperLimits.second){
			throw LimitExceededError("current is limited by upper current limit to <" + std::to_string(upperLimits.second) + "> A");
		}
		int curr = current * 10;
		std::stringstream ss;

		ss << std::setfill('0') << std::setw(3) << curr;

		std::cout << std::fixed << std::setprecision(1) << "setting current to: <" << current << "A>\n";

		std::string msg = UART_COMMAND_CURRENT + ss.str();
		sendCommand(msg, 0, true);
	}catch (LimitExceededError& e) {
				std::cerr << "could not set current to <" << current << ">: " << e.what() << std::endl;
	}
}

void HCS::setVoltage(const float voltage)
{
	try{
		isConnected();

		if(voltage < 0.0f || voltage > static_cast<int>(getMaxVoltage()))
		{
			throw std::runtime_error("voltage has to be between 0 and " + std::to_string(getMaxVoltage()));
		}else if(voltage > upperLimits.first){
			throw LimitExceededError("voltage is limited by upper voltage limit to <" + std::to_string(upperLimits.first) + "> V");
		}

		int volt = voltage * 10;
		std::stringstream ss;

		std::cout << std::fixed << std::setprecision(1) << "setting voltage to: <" << voltage << "V>\n";
		ss << std::setfill('0') << std::setw(3) << volt;

		std::string msg = UART_COMMAND_VOLT + ss.str();
		sendCommand(msg, 0, true);
	}catch (LimitExceededError& e) {
		std::cerr << "could not set voltage  to <" << voltage << ">: " << e.what() << std::endl;
	}
}

std::string HCS::readStatus() {

	std::string status = sendCommand(UART_COMMAND_GETD, 9, true);
	std::string s = status.substr(0, 5);
//	displayValue = toMansonData(s);		// FIXME: the returned value is 4 byte, not 3. so the conversion toMansionData won't work here

	statusCC = std::atoi(status.substr(8,1).c_str());

	if(statusCC){
		return "CC activated";
	}
	return "CV activated";
}

std::string HCS::getPresentVoltageAndCurrent(void) {
	std::string voltCurr = sendCommand(UART_COMMAND_GETS, 6, true);

	verifyReceived(voltCurr, "no present voltage and current received via uart");
	MansonData d = toMansonData(voltCurr);

	std::cout << "received present voltage: <" << std::fixed  << std::setprecision( 2 )  << d.first << "> " << "current: <" << d.second << ">\n";

	return voltCurr;
}

float HCS::getPresentUpperLimitVoltage(void){

	std::this_thread::sleep_for(std::chrono::milliseconds(20));
	std::string presentUpperLimit = sendCommand(UART_COMMAND_GOVP, 3, true);
	std::this_thread::sleep_for(std::chrono::milliseconds(20));
	verifyReceived(presentUpperLimit, "no present upper voltage limit received via uart");

	upperLimits = toMansonData(presentUpperLimit);

#ifdef DEBUG
	std::cout << "received upper limit voltage: <" << upperLimits.first << ">\n";
#endif
	return upperLimits.second;
}

void HCS::setUpperVoltageLimit(const float voltage)
{
	isConnected();

	if (voltage < 0.0f || voltage > static_cast<int>(getMaxVoltage())) {
		throw std::runtime_error(
				"voltage has to be between 0 and "
						+ std::to_string(getMaxVoltage()));
	}
	int volt = voltage * 10;
	std::stringstream ss;

	std::cout << std::fixed << std::setprecision(1) << "setting setUpperVoltageLimit to: <" << voltage << "V>\n";

	ss << std::setfill('0') << std::setw(3) << volt;

	std::string msg = UART_COMMAND_SOVP + ss.str();
	sendCommand(msg, 0, true);

	// update present upper limit
	getPresentUpperLimitVoltage();
}

float HCS::getPresentUpperLimitCurrent(void){
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	std::string presentUpperLimit = sendCommand(UART_COMMAND_GOCP, 3, true);
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	verifyReceived(presentUpperLimit, "no present upper current limit received via uart");

	MansonData d = toMansonData(presentUpperLimit);
	upperLimits.second = d.first;
#ifdef DEBUG
	std::cout << "received upper limit current: <" << upperLimits.second << ">\n";
#endif

	return upperLimits.second;
}

void HCS::setUpperCurrentLimit(const float current)
{
	isConnected();

	if (current < 0.0f || current > static_cast<int>(getMaxCurrent())) {
		throw std::runtime_error(
				"current has to be between 0 and "
						+ std::to_string(getMaxCurrent()));
	}
	int curr = current * 10;
	std::stringstream ss;

	std::cout << std::fixed << std::setprecision(1) << "setting setUpperCurrentLimit to: <" << current << "V>\n";

	ss << std::setfill('0') << std::setw(3) << curr;

	std::string msg = UART_COMMAND_SOCP + ss.str();
	sendCommand(msg, 0, true);

	// update present upper limit
	getPresentUpperLimitCurrent();
}


void HCS::readMemoryValues()
{
	isConnected();
	std::string voltCurr = sendCommand(UART_COMMAND_GETM, 18, true);
	std::string s = "";

	verifyReceived(voltCurr, "no memory voltage and current values received via uart");

	s = voltCurr.substr(0, 5);
	memory1 = toMansonData(s);

	s = voltCurr.substr(6, 12);
	memory2 = toMansonData(s);

	s = voltCurr.substr(12, 17);
	memory3 = toMansonData(s);

	std::cout << "Memory Voltage: \n";
	std::cout << "M1: voltage: <" << std::fixed  << std::setprecision( 2 )  << memory1.first << ">  current: <" << memory1.second << ">\n";
	std::cout << "M2: voltage: <" << std::fixed  << std::setprecision( 2 )  << memory2.first << ">  current: <" << memory2.second << ">\n";
	std::cout << "M3: voltage: <" << std::fixed  << std::setprecision( 2 )  << memory3.first << ">  current: <" << memory3.second << ">\n";

	// GETM
	// 050165138165250165
	// 050010138165250165
	// 111111022122033133
}

void HCS::runMemory(MEMORY m)
{
	if(m > 2 || m < 0){
		throw std::runtime_error("bad memory position selected: " + static_cast<int>(m));
	}

	std::cout << "run voltage and current from memory <" << m << ">\n";

	isConnected();

	std::string msg = UART_COMMAND_RUNM + std::to_string(m);
	sendCommand(msg, 0, true);
}

//void HCS::setMemory(MansonData& m0, MansonData& m1, MansonData& m2)
void HCS::setMemory(float v0, float c0, float v1, float c1, float v2, float c2)
{
	isConnected();


	if(v0 > upperLimits.first || v1 > upperLimits.first || v2 > upperLimits.first)
	{
		throw std::runtime_error("can not set memory. Voltage to set <" + std::to_string(v0) + ", " + std::to_string(v1) + ", " + std::to_string(v2) + "> is bigger than upper voltage limit <" + std::to_string(upperLimits.first) + ">");
	}

	if(c0 > upperLimits.second || c1 > upperLimits.second || c2 > upperLimits.second)
	{
		throw std::runtime_error("can not set memory. Current to set <" + std::to_string(c0) + ", " + std::to_string(c1) + ", " + std::to_string(c2) + "> is bigger than upper current limit <" + std::to_string(upperLimits.second) + ">");
	}


	if(v0 < 0 || v0 > maxValues.first)
	{
		throw std::runtime_error("can not set memory. Voltage for m0 is <" + std::to_string(v0) + ">");
	}
	if(c0 < 0 || c0 > maxValues.second)
	{
		throw std::runtime_error("can not set memory. Current for m0 is <" + std::to_string(c0) + ">");
	}

	if(v1 < 0 || v1 > maxValues.first)
	{
		throw std::runtime_error("can not set memory. Voltage for m1 is <" + std::to_string(v1) + ">");
	}
	if(c1 < 0 || c1 > maxValues.second)
	{
		throw std::runtime_error("can not set memory. Current for m1 is <" + std::to_string(c1) + ">");
	}

	if(v2 < 0 || v2 > maxValues.first)
	{
		throw std::runtime_error("can not set memory. Voltage for m2 is <" + std::to_string(v2) + ">");
	}
	if(c2 < 0 || c2 > maxValues.second)
	{
		throw std::runtime_error("can not set memory. Current for m2 is <" + std::to_string(c2) + ">");
	}


	std::cout << "saving m0 <" << v0 << ", " << c0 << ">\n";
	std::cout << "saving m1 <" << v1 << ", " << c1 << ">\n";
	std::cout << "saving m2 <" << v2 << ", " << c2 << ">\n";

	std::stringstream ss;
	int v = 0x0;
	int c = 0x0;

	v =	v0 * 10;
	c = c0 * 10;
	ss << std::setfill('0') << std::setw(3) << v;
	ss << std::setfill('0') << std::setw(3) << c;

	v =	v1 * 10;
	c = c1 * 10;
	ss << std::setfill('0') << std::setw(3) << v;
	ss << std::setfill('0') << std::setw(3) << c;

	v =	v2 * 10;
	c = c2 * 10;
	ss << std::setfill('0') << std::setw(3) << v;
	ss << std::setfill('0') << std::setw(3) << c;

	std::string msg = UART_COMMAND_PROM + ss.str();

	std::cout << "MEMORY CMD: "<< msg << std::endl;

	std::string mv = sendCommand(msg, 0, true);

}

HCS::MansonData HCS::getMaxValues() {

	isConnected();
	std::string mv = sendCommand(UART_COMMAND_GMAX, 6, true);
	verifyReceived(mv, "no max values received via uart");

	MansonData d = toMansonData(mv);

	// cut decimal places by cast to integer
	d.first = static_cast<int>(d.first);
	d.second = static_cast<int>(d.second);

	return d;
}

float HCS::getMaxCurrent(void) {
	if(!isInitialized()){
			init();
	}
	return maxValues.second;
}

float HCS::getMaxVoltage(void) {
	if(!isInitialized()){
		init();
	}
	return maxValues.first;
}

#ifdef TEST
void HCS::test(){
		std::cout << "max voltage: " << getMaxVoltage() << "V\n";
		std::cout << "max current: " << getMaxCurrent() << "A\n";
		std::cout.flush();

		setVoltage(3.0f);
		setCurrent(0.1f);

		setUpperVoltageLimit(32);

//		std::vector<float> voltages = {1.0f, 1.2f, 1.5f, 1.8f, 2.0f, 4.0f, 12.5f, 21.5f, 28.0f, 32.0f};
//
//		std::for_each(begin(voltages), end(voltages), [this](float f){
//			std::cout << std::endl;
//			std::cout << '\t';
//			setVoltage(f);
//			std::this_thread::sleep_for(std::chrono::milliseconds(150));
//			std::cout << '\t';
//			getPresentVoltageAndCurrent();
//
//			std::this_thread::sleep_for(std::chrono::seconds(5));
//		});



		setUpperCurrentLimit(5);
	////	std::vector<float> currents = {1.0f, 1.2f, 1.5f, 1.8f, 2.0f, 3.0f, 3.5f, 4.0f, 4.5f, 5.0f};
	////	std::vector<float> currents = {0.0f, 1.0f, 1.1f, 3.2f, 4.5f, 5.0f};
	//	std::vector<float> currents = {4.5f, 5.0f};
	//	std::for_each(begin(currents), end(currents), [this](float f){
	//		std::cout << std::endl;
	//		std::cout << '\t';
	//		setCurrent(f);
	//		std::this_thread::sleep_for(std::chrono::milliseconds(150));
	//		std::cout << '\t';
	//		getPresentVoltageAndCurrent();
	//
	//		std::this_thread::sleep_for(std::chrono::milliseconds(300));
	//		std::cout << '\t';
	//		std::cout << "status is: " << readStatus() << '\n';
	//
	//
	//		std::this_thread::sleep_for(std::chrono::seconds(2));
	//	});

		std::cout << std::endl;
		readMemoryValues();

		std::cout << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(5));
		runMemory(HCS::M0);
		std::this_thread::sleep_for(std::chrono::seconds(5));
		runMemory(HCS::M1);
		std::this_thread::sleep_for(std::chrono::seconds(5));
		runMemory(HCS::M2);


//		setMemory(1.5, 1.0, 2.5, 4.2, 3.5, 3.3);
//		std::cout << std::endl;
//		readMemoryValues();

		std::cout << "\nfinished Manson HCS test\n";
}
#else
	void HCS::test(){
		throw std::runtime_error("compile with -DTEST to run tests");
	}
#endif // #ifdef TEST_DEVICE
