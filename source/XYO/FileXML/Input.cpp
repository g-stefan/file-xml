// File XML
// Copyright (c) 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/FileXML/Input.hpp>

namespace XYO::FileXML {

	Input::Input(size_t readBufferSize_) {
		if (readBufferSize_ == 0) {
			readBufferSize_ = 65536;
		};

		iRead = nullptr;
		stackSize = 64;
		stackIndex = 0;
		inputStack = new char[stackSize];
		readBuffer = nullptr;
		readBufferSize = readBufferSize_;
		buffer = nullptr;
		bufferIndex = 0;
		bufferLength = 0;
		streamEnd = false;
		input = 0;
		eof = true;
	};

	Input::~Input() {
		delete[] inputStack;
		delete[] readBuffer;
	};

	void Input::setIRead(IRead *value) {
		iRead = value;
		if (!readBuffer) {
			readBuffer = new char[readBufferSize];
		};
		buffer = readBuffer;
		bufferIndex = 0;
		bufferLength = 0;
		streamEnd = (value == nullptr);
	};

	void Input::setMemory(const char *value, size_t length) {
		iRead = nullptr;
		buffer = value;
		bufferIndex = 0;
		bufferLength = length;
		streamEnd = true;
	};

	void Input::pushBack(char value) {
		if (!eof) {
			if (stackIndex == stackSize) {
				size_t newStackSize = stackSize * 2;
				char *newInputStack = new char[newStackSize];
				memcpy(newInputStack, inputStack, stackSize);
				delete[] inputStack;
				inputStack = newInputStack;
				stackSize = newStackSize;
			};
			inputStack[stackIndex] = input;
			++stackIndex;
		};
		input = value;
		eof = false;
	};

	bool Input::readFromStream() {
		if (!streamEnd) {
			bufferLength = iRead->read(readBuffer, readBufferSize);
			bufferIndex = 0;
			if (bufferLength > 0) {
				input = buffer[bufferIndex];
				++bufferIndex;
				eof = false;
				return true;
			};
			streamEnd = true;
		};
		input = 0;
		eof = true;
		return false;
	};

};
