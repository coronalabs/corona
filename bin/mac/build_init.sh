#!/bin/bash

./lua -epackage.path=[[loop/?.lua]] loop/precompiler.constant.lua -o Rtt_LuaInit -l ../../platform/resources/?.lua -n Rtt_LuaInit
