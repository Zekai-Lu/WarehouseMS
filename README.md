# 仓库库存管理系统 WarehouseMS

一个基于 C11 的控制台仓库库存管理系统

## 功能

- 商品新增、查询、修改和删除
- 商品编号唯一性校验
- 入库和出库操作
- 防止负库存和非法库存变动
- 低库存预警
- 库存数量与库存总价值统计
- CSV 文件持久化
- 自动化单元测试

## 技术

- C11
- 结构体与指针
- 模块化设计
- 文件 I/O
- 输入校验与错误处理
- assert 单元测试

## 环境

- Windows 10/11
- VS Code
- GCC（MinGW-w64）

## 编译

在项目根目录执行：

    build.bat

## 运行

    build\warehousems.exe

## 测试

    test.bat

预期输出：

    All inventory tests passed.

## 数据文件

首次启动时，程序会在 data 目录下创建或读取 inventory.csv。
退出时选择“保存并退出”，库存数据写入 CSV 文件。

## 设计说明

inventory 模块负责核心业务逻辑；
storage 模块负责数据读写；
main 模块负责用户交互；
tests 模块负责验证核心业务规则。
