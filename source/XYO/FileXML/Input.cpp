// File XML
// Copyright (c) 2016-2024 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2024 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/FileXML/Input.hpp>

namespace XYO::FileXML {

	Input::Input(size_t stackSize_) {
		if (stackSize_ == 0) {
			stackSize_ = 16384;
		};

		iRead = nullptr;
		stackSize = stackSize_;
		stackIndex = 0;
		input = 0;
		eof = false;
		inputStack = new char[stackSize]();
		fileIndex = 0;
	};

	Input::~Input() {
		delete[] inputStack;
	};

	void Input::setIRead(IRead *value) {
		iRead = value;
	};

	bool Input::push() {
		if (stackIndex == stackSize) {
			return false;
		};
		inputStack[stackIndex] = input;
		++stackIndex;

		--fileIndex;
		return true;
	};

	bool Input::pop() {
		if (stackIndex == 0) {
			return false;
		};
		--stackIndex;
		input = inputStack[stackIndex];

		++fileIndex;
		return true;
	};

	bool Input::read() {
		if (stackIndex) {
			return pop();
		};
		if (eof) {
			input = 0;
			return false;
		};
		if (iRead->read(&input, 1)) {
			++fileIndex;
			return true;
		};
		input = 0;
		eof = true;
		return false;
	};

	bool Input::isEof() {
		if (stackIndex) {
			return false;
		};
		return eof;
	};

};
