#!/usr/bin/env python3
import json
import os
import subprocess

platform = os.environ['TEMPLATE_PLATFORM']
SDKs = {"tvos":"appletvos", "iphone":"iphoneos"}
xcodeVersion = subprocess.check_output(['/usr/bin/xcrun', '--sdk', SDKs[platform],'--show-sdk-version']).decode().strip()
coronaVersion =  int(float(xcodeVersion)*10000)
xcode = subprocess.check_output(['/usr/bin/xcodebuild', '-version']).decode().split('\n')[0].strip()

def getCustomTemplateName():
    target = os.environ['TEMPLATE_TARGET']
    target = target[len('template'):]
    if target:
        return target
customTemplate = getCustomTemplateName()

def isBeta():
    return False

def getLabel():
    if coronaVersion < 140000:
        return xcodeVersion + " (Legacy)"
    if customTemplate == '-angle':
        return xcodeVersion + " Metal"
    return xcodeVersion


entry = {
    "label": getLabel(),
    "xcodeVersion": xcodeVersion,
    "failMessage": "install or xcode-select " + xcode + " to enable",
    "coronaVersion": coronaVersion,
    "beta": isBeta(),
}
if customTemplate:
    entry["customTemplate"] = customTemplate

platforms = {"tvos":"tvos", "iphone":"ios"}
output = {
    platforms[platform] : [
        entry
    ]
}

subprocess.check_call(['/bin/mkdir', '-p', 'output'])
fileName = {"tvos":"tvOS-SDKs.json", "iphone":"iOS-SDKs.json"}
ordNum = 1000000 - coronaVersion + (5 if isBeta() else 0) + (1 if customTemplate else 0)
fileName = str(ordNum).zfill(7) + '_' + fileName[platform]
with open(os.path.join('output', fileName), 'w') as f:
    json.dump(output, f, indent=4)
