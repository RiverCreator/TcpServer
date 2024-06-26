#pragma once
#include <google/protobuf/descriptor.h>
#include "google/protobuf/service.h"
#include <mytcp/TcpServer.h>
#include <mytcp/EventLoop.h>
#include <mytcp/Connection.h>
#include <string>
#include <unordered_map>
#include <functional>
#include <mylog/Logger.h>
#include "RpcApp.h"
#include "RpcHeader.pb.h"
#include "RpcController.h"
#include "zkClient.h"
class RpcProvider
{
public:
    RpcProvider();
    // 这里是框架提供给外部使用的，可以发布rpc方法的函数接口
    void NotifyService(google::protobuf::Service *service);

    // 启动rpc服务节点，开始提供rpc远程网络调用服务
    void Run();

private:
    // service服务类型信息
    struct ServiceInfo
    {
        google::protobuf::Service *m_service; // 保存服务对象
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> m_methodMap; // 保存服务方法
    };
    // 存储注册成功的服务对象和其服务方法的所有信息
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;

    // 新的socket连接回调
    void OnConnection(const spConnection);
    // 已建立连接用户的读写事件回调
    void OnMessage(const spConnection, std::string&);
    // Closure的回调操作，用于序列化rpc的响应和网络发送
    void SendRpcResponse(const spConnection, google::protobuf::Message*);
};