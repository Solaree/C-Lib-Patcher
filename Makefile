# Makefile - The Patch C++ Library
# Copyright (C) 2023 Solar
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

CXX = clang++
CXXFLAGS = -Wall -target armv7a-linux-androideabi19 -static-libstdc++
SRC_FILES = injector.cpp
BUILD_DIR = build
OUTPUT = $(BUILD_DIR)/injector

all: $(OUTPUT)

$(OUTPUT): $(SRC_FILES)
	mkdir -p $(BUILD_DIR)
	$(CXX) -o $@ $^ $(CXXFLAGS)

clean:
	rm -rf ./$(BUILD_DIR)

.PHONY: all clean
