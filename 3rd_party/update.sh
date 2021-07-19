#!/bin/bash

if [ -d pugixml ]; then
    mkdir pugixml
fi

pushd pugixml

if [ ! -d include ]; then
    mkdir include
fi

if [ ! -d main ]; then
    git clone git@github.com:zeux/pugixml.git main
fi

cp -fl ./main/src/pugiconfig.hpp include/pugiconfig.hpp
cp -fl ./main/src/pugixml.hpp include/pugixml.hpp
cp -fl ./main/src/pugixml.cpp pugixml.cpp
cp -fl ./main/scripts/pugixml_dll.rc pugixml_dll.rc
popd
