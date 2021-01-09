/*
 * Serial.h
 *
 *  Created on: Jan 9, 2021
 *      Author: scholtyssek
 */

#ifndef SERIAL_H_
#define SERIAL_H_

#include <termios.h>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <cerrno>

class Serial {
public:
	// rule of 0

	static int connect(const char *device, const int baud) {
		struct termios options;
		speed_t myBaud;
		int status, fd;

		switch (baud) {
		case 9600:
			myBaud = B9600;
			break;
		case 19200:
			myBaud = B19200;
			break;
		case 38400:
			myBaud = B38400;
			break;
		case 57600:
			myBaud = B57600;
			break;
		case 115200:
			myBaud = B115200;
			break;
		default:
			throw std::runtime_error("baud not selected");
		}

		if ((fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK))	== -1)
		{
			std::string msg = std::string(strerror(errno));
			throw std::runtime_error("open failed for usart: " + msg);
		}

		fcntl(fd, F_SETFL, O_RDWR);

		// Get and modify current options:

		tcgetattr(fd, &options);

		cfmakeraw(&options);
		cfsetispeed(&options, myBaud);
		cfsetospeed(&options, myBaud);

		options.c_cflag |= (CLOCAL | CREAD);
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		options.c_cflag &= ~CSIZE;
		options.c_cflag |= CS8;
		options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
		options.c_oflag &= ~OPOST;

		options.c_cc[VMIN] = 0;
		options.c_cc[VTIME] = 100;	// Ten seconds (100 deciseconds)

		tcsetattr(fd, TCSANOW, &options);

		ioctl(fd, TIOCMGET, &status);

		status |= TIOCM_DTR;
		status |= TIOCM_RTS;

		ioctl(fd, TIOCMSET, &status);

		usleep(10000);	// 10mS

		return fd;
	}

	static void disconnect(int fd)
	{

		if(close(fd) < 0)
		{
			throw std::runtime_error("disconnect failed for fd <" + std::to_string(fd) + ">");
		}
	}

	static void flush(int * const fd) noexcept
	{
		tcflush (*fd, TCIOFLUSH);
	}

	static int puts(int fd, const char* s) noexcept
	{
		return write (fd, s, strlen(s));
	}



	// function timeout is 10s
	static int getChar(const int fd)
	{
		uint8_t c;

		if (read (fd, &c, 1) != 1)
		{
			std::string msg = std::string(strerror(errno));
			throw std::runtime_error("open failed for usart: " + msg);
		}

		return ((uint8_t)c) & 0xFF ;
	}

};

#endif /* SERIAL_H_ */
