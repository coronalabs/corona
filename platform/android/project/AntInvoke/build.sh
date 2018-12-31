#!/bin/sh

# This command will rebuild AntInvoke.class on a machine that has other JDKs besides 1.6:
# /usr/libexec/java_home -v 1.6 -a x86_64 -V -exec javac -d bin/classes -sourcepath src -g:none -classpath /Users/perry/src/corona/core/main-tachyon-home/platform/android/project/AntInvoke/bin/classes:/usr/local/Cellar/ant/1.9.4/libexec/lib/ant-launcher.jar:/usr/local/Cellar/ant/1.9.4/libexec/lib/ant-antlr.jar:/usr/local/Cellar/ant/1.9.4/libexec/lib/ant-apache-bcel.jar:/usr/local/Cellar/ant/1.9.4/libexec/lib/ant-apache-bsf.jar:/usr/local/Cellar/ant/1.9.4/libexec/lib/ant-apache-log4j.jar:/usr/local/Cellar/ant/1.9.4/libexec/lib/ant-apache-oro.jar:/usr/local/Cellar/ant/1.9.4/libexec/lib/ant-apache-regexp.jar:/usr/local/Cellar/ant/1.9.4/libexec/lib/ant-apache-resolver.jar:/usr/local/Cellar/ant/1.9.4/libexec/lib/ant-apache-xalan2.jar:/usr/local/Cellar/ant/1.9.4/libexec/lib/ant-commons-logging.jar:/usr/local/Cellar/ant/1.9.4/libexec/lib/ant-commons-net.jar:/usr/local/Cellar/ant/1.9.4/libexec/lib/ant-jai.jar:/usr/local/Cellar/ant/1.9.4/libexec/lib/ant-javamail.jar:/usr/local/Cellar/ant/1.9.4/libexec/lib/ant-jdepend.jar:/usr/local/Cellar/ant/1.9.4/libexec/lib/ant-jmf.jar:/usr/local/Cellar/ant/1.9.4/libexec/lib/ant-jsch.jar:/usr/local/Cellar/ant/1.9.4/libexec/lib/ant-junit.jar:/usr/local/Cellar/ant/1.9.4/libexec/lib/ant-junit4.jar:/usr/local/Cellar/ant/1.9.4/libexec/lib/ant-netrexx.jar:/usr/local/Cellar/ant/1.9.4/libexec/lib/ant-swing.jar:/usr/local/Cellar/ant/1.9.4/libexec/lib/ant-testutil.jar:/usr/local/Cellar/ant/1.9.4/libexec/lib/ant.jar:/Library/Java/JavaVirtualMachines/jdk1.8.0_20.jdk/Contents/Home/lib/tools.jar src/com/ansca/util/AntInvoke.java


# Builds the "AntInvoke.jar" file.
ant $* clean
ant $* compile
ant $* jar
