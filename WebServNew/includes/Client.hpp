#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client {
	private:
		int _fd;
		std::string _recvBuffer;
		std::string _sendBuffer;

	public:
		Client(int fd);
		~Client();

		int getFd() const;

		void appendToRecvBuffer(const std::string& data);
		const std::string& getRecvBuffer() const;
		void clearRecvBuffer();

		void setSendBuffer(const std::string& data);
		const std::string& getSendBuffer() const;
		void clearSendBuffer();
};

#endif
