// Created by Grigore Stefan <g_stefan@yahoo.com>
// Public domain (Unlicense) <http://unlicense.org>
// SPDX-FileCopyrightText: 2016-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: Unlicense

#include <XYO/FileXML.hpp>

using namespace XYO::FileXML;

void test() {
	TPointer<Document> xml;	
	if(!load("../../input/simple.xml",xml)){
		throw std::runtime_error("File load");
	};
	if(!save("simple.normal.xml",xml,Mode::Normal)){
		throw std::runtime_error("File save");
	};
	if(!save("simple.minified.xml",xml,Mode::Minified)){
		throw std::runtime_error("File save");
	};
	if(!save("simple.indentation-tab.xml",xml,Mode::IndentationTab)){
		throw std::runtime_error("File save");
	};
	if(!save("simple.indentation-4-spaces.xml",xml,Mode::Indentation4Spaces)){
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
