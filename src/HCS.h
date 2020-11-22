/*
 * HCS.h
 *
 *	Copyright (C) 2020 Marco Scholtyssek <code@scholtyssek.org>
 *  Created on: Oct 12, 2020
 *
 */

#ifndef HCS_H_
#define HCS_H_

#include <cstdint>
#include <string>
#include <utility>	// std::pair


//#define SIMULATION
//#define DEBUG
#define TEST

class HCS {
private:
	unsigned int baud;
	std::string uart;
	static int fd;
	std::pair<int,int> hcsData;	// <voltage, current>

	static bool initialized;
	bool connected;
	int statusCC = 0x00;
	int statusCV = 0x00;

	// UART commands
	static const std::string UART_COMMAND_GMAX;
	static const std::string UART_COMMAND_VOLT;
	static const std::string UART_COMMAND_CURRENT;

	static const std::string UART_COMMAND_GETS;
	static const std::string UART_COMMAND_GETD;
	static const std::string UART_COMMAND_GOVP;
	static const std::string UART_COMMAND_SOVP;
	static const std::string UART_COMMAND_GOCP;
	static const std::string UART_COMMAND_SOCP;
	static const std::string UART_COMMAND_GETM;
	static const std::string UART_COMMAND_RUNM;
	static const std::string UART_COMMAND_PROM;

	static const std::string UART_RESPONSE_OK;

	using MansonData = std::pair<float, float>;
	MansonData maxValues;
	MansonData upperLimits;
	MansonData displayValue;
	MansonData memory1, memory2, memory3;


	HCS(const HCS &other) = delete;
	HCS(HCS &&other) = delete;
	HCS& operator=(const HCS &other) = delete;
	HCS& operator=(HCS &&other) = delete;

	void uartDebug(const std::string& data);
	bool receiveOk();
	void verifyReceived(const std::string& receivedData, const std::string& errMsg);

	bool isConnected(void);
	std::string receiveViaUart(uint8_t byteCount);
	void send(const std::string& msg);
	std::string sendCommand(const std::string& msg, const uint8_t receiveBytesCount = 0x0, const bool expectOk = true);

	MansonData getMaxValues();
	MansonData toMansonData(std::string& voltageCurrentString);

public:
	enum MEMORY {M0 = 0, M1, M2};

	explicit HCS(std::string _uart, unsigned int _baud) : baud(_baud), uart(_uart), connected(false)
	{
	}
	virtual ~HCS() = default;
	void init();
	void connect();
	void disconnect(void);
	void setDisconnected(void);
	void setConnected(void);

	void setVoltage(const float voltage);
	void setUpperVoltageLimit(const float voltage);

	void setCurrent(const float current);
	void setUpperCurrentLimit(const float current);

	float getMaxCurrent(void);
	float getMaxVoltage(void);
	float getPresentUpperLimitVoltage(void);
	float getPresentUpperLimitCurrent(void);
	std::string getPresentVoltageAndCurrent(void);

	void readMemoryValues();
	void runMemory(MEMORY m);
	void setMemory(float v0, float c0, float v1, float c1, float v2, float c2);

	std::string readStatus();

	void flush(void);

	static bool isInitialized() {
		return initialized;
	}

	void test();

//	const MansonData& getDisplayValue() const {
//		return displayValue;
//	}
//
//	const MansonData& getMemory1() const {
//		return memory1;
//	}
//
//	const MansonData& getMemory2() const {
//		return memory2;
//	}
//
//	const MansonData& getMemory3() const {
//		return memory3;
//	}
//
//	const MansonData& getUpperLimits() const {
//		return upperLimits;
//	}
};

#endif /* HCS_H_ */
