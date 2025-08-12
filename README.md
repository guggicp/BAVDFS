# BAVDFS

## How to build

需要安装依赖如：
1. cmake
2. openssl
3. libssl-dev
4. libz-dev
5. ...

```shell
echo "export VECTORDB_CODE_BASE=_______" >> ~/.bashrc  #下载后的代码根路径 例如/home/pzp/code/BAVDFS
source ~/.bashrc
```

### build the third-party

```shell
cd third_party
bash ./build.sh
```

### build vdb_server

```shell
mkdir build
cd build
cmake ..
make
```

## How to run

```shell
./build/bin/vdb_server
```

```shell
# send some commands
bash ./test/test.0.0.1.sh
```

## reference

1. 《从零构建向量数据库》：参考代码实现
2. 参考 [TinyVecDB](https://github.com/Xiaoccer/TinyVecDB) 的 项目结构/CMake