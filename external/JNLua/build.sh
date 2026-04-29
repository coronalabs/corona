#!/bin/sh

# Builds the "JNLua.jar" file.
ant clean
ant compile
ant jar
