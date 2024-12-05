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
- `test/socket_server/`：存放验证后端核心请求接口能否正常工作的单元测试样例。
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

# 单元测试

## 简介

为了验证后端核心在被构建后，其暴露给前端的各个请求接口是否能够正常工作，我们在`test/socket_server/test_cases`目录中提供了一系列的单元测试样例，并且实现了一个批量执行这些单元测试的脚本程序`test/socket_server/run_test_cases.js`

这些测试样例由Node.js代码编写，因此在进行测试前需要首先安装Node.js环境。Node.js版本号22.11.0，与前端开发环境一致。

假设当前命令行定位在本仓库源码的根目录下，则批量进行单元测试的命令如下：

```shell
node test/socket_server/run_test_cases.js
```

执行该命令后，若所有单元测试全部执行完毕，且没有收到任何错误信息，则说明当前构建出来的后端核心提供的请求接口通过单元测试，其行为基本正确。

## 测试样例说明

下面简单说明各个单元测试样例所验证的内容。

- `test_case_1.js`：验证`/get_all_tables`接口
- `test_case_2.js`：验证`/get_buffer_pool_info`接口
- `test_case_3.js`：验证调用`/submit_sql_command`接口，能否成功执行`create table`、`insert into`、`select ... from ...`等基本的SQL命令。
- `test_case_4.js`：验证调用`/submit_sql_command`接口能否成功执行`create index`命令为数据表建索引。以及调用`/query_b_plus_tree`能否成功拉取数据表索引内部的B+树数据结构信息。
- `test_case_5.js`：验证`/query_table_by_name`, `/get_table_heap_info`, `/get_table_page_info` 和 `/get_tuple_info`接口。
- `test_case_6.js`：验证调用`/submit_sql_command`接口为数据表建立索引后，针对`select ... from ... order by ...`SQL命令的优化加速能否被成功实施。
