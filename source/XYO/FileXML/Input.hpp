// File XML
// Copyright (c) 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_FILEXML_INPUT_HPP
#define XYO_FILEXML_INPUT_HPP

#ifndef XYO_FILEXML_DEPENDENCY_HPP
#	include <XYO/FileXML/Dependency.hpp>
#endif

namespace XYO::FileXML {

	// Character input with block buffering and unlimited push back.
	// 'input' is the current character, valid while isEof() is false.
	class Input {
			XYO_PLATFORM_DISALLOW_COPY_ASSIGN_MOVE(Input);

		public:
			TPointer<IRead> iRead;

			char *inputStack;
			size_t stackSize;
			size_t stackIndex;

			char *readBuffer;
			size_t readBufferSize;
			const char *buffer;
			size_t bufferIndex;
			size_t bufferLength;
			bool streamEnd;

			char input;
			bool eof;

			Input(size_t readBufferSize_ = 65536);
			~Input();

			inline operator char() {
				return input;
			};

			inline char value() {
				return input;
			};

			inline Input &operator=(char value) {
				input = value;
				return *this;
			};

			void setIRead(IRead *value);
			// Read directly from memory, value must stay valid while reading
			void setMemory(const char *value, size_t length);

			// Make value the current character, the current one (if any) will be read next
			void pushBack(char value);

			inline bool read() {
				if (stackIndex) {
					--stackIndex;
					input = inputStack[stackIndex];
					eof = false;
					return true;
				};
				if (bufferIndex < bufferLength) {
					input = buffer[bufferIndex];
					++bufferIndex;
					eof = false;
					return true;
				};
				return readFromStream();
			};

			inline bool isEof() const {
				return eof;
			};

		protected:
			bool readFromStream();
	};

};

#endif
