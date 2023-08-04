# Spike

[![build](https://github.com/PredatorCZ/Spike/actions/workflows/cmake.yml/badge.svg)](https://github.com/PredatorCZ/Spike/actions/workflows/cmake.yml)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/50479b1d15d848bcb389dc2166c294e2)](https://www.codacy.com/manual/PredatorCZ/Spike?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=PredatorCZ/Spike&amp;utm_campaign=Badge_Grade)
[![Coverage Status](https://coveralls.io/repos/github/PredatorCZ/PreCore/badge.svg?branch=master)](https://coveralls.io/github/PredatorCZ/PreCore?branch=master)

Spike is a powerful foundation for batch processing of files.\
Spike can:

* Handle large amount of files and make something from them.
* Extract archives as well as make them from large amount of files.
* Also convert lasrge amount of files into something else.
* Respect your HDD heath by creating single ZIP archive for extracted files, erasing directory and file fragmentation.
* Squeze every possbile watt from your CPU in order to process everything as fast as possible.

Spike also provides library for:

* Class and Enum static reflection and their runtime serialization
* SIMD math classes for Vectors and Matrix
* UNI module that acts as high level abstraction over complicated data structures
* Simple multithreaded logger
* Complete stream serialization library
* And a lots of helper classes and methods

Spike can built for C++23 standard and it's compilable under Clang 13+ and GCC 12+.

Spike library can built for C++20 standard and it's compilable under Clang 12+ and GCC 10+.

## Visit [Wiki](https://github.com/PredatorCZ/Spike/wiki) for more info

## Uses additional libraries

* pugixml (Copyright (C) 2006-2020, by Arseny Kapoulkine)
* glm (Copyright (c) 2005 - G-Truc Creation)
* nlohmann::json (Copyright (c) 2013-2022 Niels Lohmann)
* gltf is a derivative work of fx-gltf (Copyright(c) 2018-2021 Jesse Yurkovich)
