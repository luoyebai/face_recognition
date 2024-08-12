# 基于OpenCV的简单的人脸识别

## 使用[xmake](https://xmake.io/)构建

```shell
# 构建
xmake
# 使用xmake生成CMakeLists.txt、makefile和compile_commands.json
xmake project --kind=cmake
xmake project --kind=makefile
xmake project --kind=--kind=compile_commands
# 切换编译模式
xmake f -m debug
```

## 使用[CMake](https://cmake.org/)构建

```shell
mkdir build && pushd build
cmake .. &&  make
```

## 使用[Makefile](https://www.gnu.org/software/make/)构建

```shell
make
```

## 如何使用？

向[data/targets文件夹](./data/targets)添加对象目标即可，图片文件名即是人名
如需添加新的参数配置，请修改[include/config.hpp文件](include/config.hpp)

## 运行效果

![](./data/demo.gif)
