<img src="https://raw.githubusercontent.com/cmu-db/bustub/master/logo/bustub-whiteborder.svg" alt="BusTub Logo" height="200">

-----------------

# 什么是BusTubTracer

BusTubTracer是一款基于BusTub开发的关系型数据库管理系统（RDBMS）底层工作原理和内部数据结构的可视化工具，旨在帮助初学RDBMS底层原理（尤其是[CMU-15445/645课程](https://15445.courses.cs.cmu.edu/)）的计算机类专业学生更好地掌握RDBMS系统设计与实现中的关键概念。

[BusTub](https://github.com/cmu-db/bustub)是由[Carnegie Mellon University](https://www.cs.cmu.edu/)推出的一款开源教学型RDBMS系统。该系统仿照实际的商用RDBMS系统，实现了简单SQL查询、索引等基本功能，是计算机专业学生学习RDBMS底层设计实现和工作原理的宝贵资源。

BusTubTracer基于原版BusTub进行二次开发，在原版的基础上增加了可视化图形界面，用于直观展示BusTub内部的一些重要数据结构，以及BusTub针对SQL命令的处理和执行过程。

BusTubTracer在架构上由两部分组成，分别为前端用户界面`BusTubTracerFront`（基于Electron+Vue3独立开发）和后端核心`BusTubTracerCore`（基于原版BusTub修改）。两者按如下图所示的形式组织在一起，形成完整的BusTubTracer应用。

<img width="917" alt="微信图片_20241205195534" src="https://github.com/user-attachments/assets/bb742330-80ab-4100-a3ec-b9805f145acb">


# 关于本仓库

本仓库存储和管理BusTubTracer项目的后端核心`BusTubTracerCore`源码。后端核心中主要实现了实际的RDBMS系统逻辑功能。

前端用户界面`BusTubTracerFront`的源码仓库请移步：https://github.com/WU-SUNFLOWER/BusTubTracerFront

# 开发环境

- 操作系统：Ubuntu 22.04
- 代码编辑器：Visual Studio Code

在开发前请首先执行如下命令，这将自动完成后端核心的Linux C/C++开发环境配置：

```shell
sudo build_support/packages.sh
```

# 关键目录

这里仅列出`BusTubCore`相较于原版BusTub，新增加的目录项。

- `src/myapi/`: 存放后端核心暴露给前端的各个请求接口的具体实现代码。
- `tools/socket_server/`: 存放后端核心与前端用户界面之间进程间通信的实现代码。后端核心在接收到前端请求后，会转发给`src/myapi`中的相应接口处理函数，以作进一步处理。
- `third_party/rapidjson`：存放后端核心所使用的第三方JSON库。在本项目中，我们采用[RapidJSON](https://github.com/Tencent/rapidjson)来实现后端核心解析JSON数据和输出JSON字符串的功能。

此外，在`BusTubCore`的开发过程中，还对原版BusTub中的大量源码文件进行了修改。这里不再一一列举！

# 构建

在原版BusTub中，已经实现了较为完善的基于CMAKE的构建系统。我们在原版的基础上进行拓展，以支持对`BusTubCore`后端核心二进制文件的构建。

在本仓库源码根目录下执行如下命令，即可完成对后端核心的构建：

```shell
rm -rf build
mkdir -p build && cd build
cmake ..
make socket_server -j`nproc`
```

注意，在默认情况下这将构建Debug版本的后端核心二进制文件。如想得到更加优化的Release版本的二进制文件，请将其中的`cmake ..`命令替换为：
```shell
cmake -DCMAKE_BUILD_TYPE=Release
```

当构建结束后，在目录`build/bin`中新生成的`socket_server`即为后端核心的二进制文件。

请将该文件复制至[前端用户界面源码](https://github.com/WU-SUNFLOWER/BusTubTracerFront)中的`server`目录。之后运行前端Electron程序时，方会自动拉起该二进制文件创建进程，并与之建立连接！
