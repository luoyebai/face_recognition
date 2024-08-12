# 基于OpenCV的简单的人脸识别

## 使用[xmake](https://xmake.io/)构建

```shell
# 编译运行
xmake r
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
make
```

## 使用[Makefile](https://www.gnu.org/software/make/)构建

```shell
make
```

## 如何使用？

向data/targets文件夹添加对象目标即可，图片文件名即是人名

## 运行效果

![视频](./data/demo.mp4)
