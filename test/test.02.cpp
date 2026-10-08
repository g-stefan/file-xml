// Created by Grigore Stefan <g_stefan@yahoo.com>
// Public domain (Unlicense) <http://unlicense.org>
// SPDX-FileCopyrightText: 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: Unlicense

#include <XYO/FileXML.hpp>

using namespace XYO::FileXML;

void test() {
	TPointer<Document> xml;	
	if(!load("../../input/test.02.xml",xml)){
		throw std::runtime_error("File load");
	};
	if(!save("test.02.normal.xml",xml,Mode::Normal)){
		throw std::runtime_error("File save");
	};
	printf("Done.\r\n");
};

int main(int cmdN, char *cmdS[]) {

	try {
		test();
		return 0;
	} catch (const std::exception &e) {
		printf("* Error: %s\n", e.what());
	} catch (...) {
		printf("* Error: Unknown\n");
	};

	return 1;
};
