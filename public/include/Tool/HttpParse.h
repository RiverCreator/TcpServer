#pragma once
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <regex>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/mman.h>    // 内存映射相关
#include <sys/file.h>
#include <unistd.h>
#include <iostream>
#include <mylog/Logger.h>

//#include<json> //POST请求还要解析json文件
class HttpParse{
public:
    static const int READ_BUFFER_SIZE=2048;
    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
    char buf[READ_BUFFER_SIZE];
    int m_read_idx; //记录buf最后的位置
    int m_check_idx;//记录当前读到的位置
    int m_start_line;//记录当前读到哪一行，这一行的开始字符位置
    char *m_url;//记录请求行中的url
    char *m_version;//记录http版本号
    char *m_host;//host字段
    int m_content_length;
    bool m_linger;
    char *m_content;
    static const int FILENAME_LEN=200;
    static const int WRITE_BUFFER_SIZE=1024;
    char m_real_file[FILENAME_LEN];
    const char *root_dir;
    struct stat m_file_stat;
    char *m_file_address;
    struct iovec m_iv[2];
    int m_iv_count;
    int m_write_idx;
    char m_write_buf[WRITE_BUFFER_SIZE];//需要再HttpServer处理好回复报文后交由conn来进行发送
    /*请求方法*/
    enum METHOD
    {
        GET=0,//暂时只支持GET和POST
        POST,
        HEAD,PUT,DELETE,TRACE,OPTIONS,CONNECT,PATCH
    };
    /*HTTP码 200 404等*/
    enum HTTP_CODE
    {
        NO_REQUEST,GET_REQUEST,BAD_REQUEST,NO_RESOURCE,FORBIDDEN_REQUEST,FILE_REQUEST,INTERNAL_ERROR,CLOSED_CONNECTION
    };
    /*行解析状态*/
    enum LINE_STATUS{LINE_OK=0,LINE_BAD,LINE_OPEN};
    /*当前主状态机状态,分别是正在分析请求航，头部，内容字段*/
    enum CHECK_STATE{CHECK_STATE_REQUESTLINE=0,CHECK_STATE_HEADER,CHECK_STATE_CONTENT};
    CHECK_STATE state_;
    METHOD m_method; //记录当前http请求的method
    HttpParse(const char* root_dir);
    void init(std::string &buf_);
    ~HttpParse();
    HTTP_CODE Parse();
    LINE_STATUS ParseLine();
    HTTP_CODE ParseRequestLine(char* text);
    HTTP_CODE ParseHeader(char* text);
    HTTP_CODE ParseContent(char* text);
    char* get_line();
    bool add_response(const char*format,...);
    bool add_status_line(int status, const char* title);
    bool add_headers(int content_len);
    bool add_content_length(int content_len);
    bool add_linger();
    bool add_blank_line();
    bool add_content_type();
    bool add_content(const char *content);
    HttpParse::HTTP_CODE do_request();
    bool process_write(HTTP_CODE ret);
    bool process(std::string buf);
    void close();
    void unmap();
};