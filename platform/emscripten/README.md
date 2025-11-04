# Build test app & quick setup

## Setting up
1. download [emscripten](http://emscripten.org) & unpack it
2. run `emsdk update`, `emsdk install latest` and `emsdk activate latest`

## Building test app

- from `platform/emscripten/gmake` run:
  * `./build_test.sh` to build `platform/test/assets2` with reasonable debug output
  * `./build_test.sh Release` to build reasonably sized `platform/test/assets2` 
  * use `build_app.sh` to build other apps

## Building Template
```
cd gmake
./build_template.sh
```
See script for details

## Troubleshooting

- most issues can be fixed with adding/removing files in `rtt.gmake`
- `lua/*.cpp` are actually have to be updated sometimes, for not just copy them from generated files on mac or something
- if getting "duplicated `vprintf`, remove `Rtt_VLogException` implementation because it's defined as latter

