#pragma once

#include "RpcConfig.h"

// mprpc框架的基础类，负责框架的一些初始化操作
class RpcApp
{
public:
    static void Init(int argc, char **argv);
    static RpcApp& GetInstance();
    static RpcConfig& GetConfig();
private:
    static RpcConfig m_config;

    RpcApp(){}
    RpcApp(const RpcApp&) = delete;
    RpcApp(RpcApp&&) = delete;
};