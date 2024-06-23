/*
pickmessage:GET / HTTP/1.1 //这个是请求行 下面的叫请求头部
Host: 192.168.203.128:5085
Connection: keep-alive
Cache-Control: max-age=0
Upgrade-Insecure-Requests: 1
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/123.0.0.0 Safari/537.36 Edg/123.0.0.0
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*\/*;q=0.8,application/signed-exchange;v=b3;q=0.7
Accept-Encoding: gzip, deflate
Accept-Language: zh-CN,zh;q=0.9,en;q=0.8,en-GB;q=0.7,en-US;q=0.6
//下面的叫做content，GET是没有请求正文的，POST则有
*/
#include "HttpParse.h"
/*定义HTTP响应的一些状态信息*/
const char*ok_200_title="OK";
const char*error_400_title="Bad Request";
const char*error_400_form="Your request has bad syntax or is inherently impossible to satisfy.\n";
const char*error_403_title="Forbidden";
const char*error_403_form="You do not have permission to get file from this server.\n";
const char*error_404_title="Not Found";
const char*error_404_form="The requested file was not found onthis server.\n";
const char*error_500_title="Internal Error";
const char*error_500_form="There was an unusual problem serving the requested file.\n";
const std::unordered_map<std::string, std::string> HttpParse::SUFFIX_TYPE = {
    { ".html",  "text/html" },
    { ".xml",   "text/xml" },
    { ".xhtml", "application/xhtml+xml" },
    { ".txt",   "text/plain" },
    { ".rtf",   "application/rtf" },
    { ".pdf",   "application/pdf" },
    { ".word",  "application/msword" }, // 修复文件类型描述的错误
    { ".png",   "image/png" },
    { ".gif",   "image/gif" },
    { ".jpg",   "image/jpeg" },
    { ".jpeg",  "image/jpeg" },
    { ".au",    "audio/basic" },
    { ".mpeg",  "video/mpeg" },
    { ".mpg",   "video/mpeg" },
    { ".avi",   "video/x-msvideo" },
    { ".gz",    "application/x-gzip" },
    { ".tar",   "application/x-tar" },
    { ".css",   "text/css "},
    { ".js",    "text/javascript "},
};
HttpParse::HttpParse(const char* root_dir){
    m_write_idx=0;
    this->root_dir = root_dir;
    memset(buf, '\0', READ_BUFFER_SIZE);
    memset(m_write_buf,'\0',WRITE_BUFFER_SIZE);
    memset(m_real_file,'\0',FILENAME_LEN);
}
void HttpParse::init(std::string& buf_){
    memcpy(buf, buf_.c_str(),buf_.size());
    m_read_idx = buf_.size();
    m_start_line = 0;
    m_content_length = 0;
    state_ = CHECK_STATE_REQUESTLINE; // 最开始就为解析请求行状态
    m_linger = false;
    m_write_idx=0;
    m_check_idx = 0;
    m_method = GET;
    m_url=0;
    m_version=0;
    m_host=0;
}
char* HttpParse::get_line(){
    return buf + m_start_line;
}

HttpParse::LINE_STATUS HttpParse::ParseLine(){
    char tmp;
    while (m_check_idx < m_read_idx)
    {
        tmp = buf[m_check_idx];
        if (tmp == '\r')
        {
            if(m_check_idx+1==m_read_idx){
                return LINE_OPEN;
            }
            else if(buf[m_check_idx+1]=='\n'){
                buf[m_check_idx++] = '\0';
                buf[m_check_idx++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
        else if(tmp=='\n'){
            if((m_check_idx>1)&&(buf[m_check_idx-1]=='\r')){
                buf[m_check_idx - 1] = '\0';
                buf[m_check_idx++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
        ++m_check_idx;
    }
    return LINE_OPEN;
}

HttpParse::HTTP_CODE HttpParse::ParseRequestLine(char* text){
    m_url = strpbrk(text," ");//搜索到第一个\t的位置
    if(!m_url){
        //printf("no t\n");
        return BAD_REQUEST;
    }
    *m_url++ = '\0';
    char *method = text;
    //printf("method:%s\n", method);
    if (strcasecmp(method, "GET") == 0)
    {
        m_method = GET;
    }
    else if(strcasecmp(method,"POST")==0){
        m_method = POST;
    }
    else{
        return BAD_REQUEST;
    }
    m_url+=strspn(m_url," "); //跳过空格
    m_version=strpbrk(m_url," "); //然后找到下一个空格的下一个位置
    if (!m_version)
    {
        return BAD_REQUEST;
    }
    *m_version++='\0';
    //printf("m_url:%s\n", m_url);
    m_version+=strspn(m_version," ");
    //printf("m_version:%s\n", m_version);
    if (strcasecmp(m_version, "HTTP/1.1") != 0) // 这里只解析HTTP1.1 后续可以增加更多版本
    {
        //printf("state:%d", state_);
        //printf("wrong version?:%s\n", m_version);
        return BAD_REQUEST;
    }
    if(strncasecmp(m_url,"http://",7)==0)
    {
        m_url+=7;
        m_url=strchr(m_url,'/');
    }
    //std::cout << "parse:"<<m_url << std::endl;
    //url为http://后面的
    if(!m_url||m_url[0]!='/')
    {
        return BAD_REQUEST;
    }
    state_ = CHECK_STATE_HEADER;
    return NO_REQUEST;
}

HttpParse::HTTP_CODE HttpParse::ParseHeader(char* text){
    //读到空行表示头部解析完成
    if(text[0]=='\0'){
        if(m_content_length!=0)
        {
            state_=CHECK_STATE_CONTENT;
            return NO_REQUEST;
        }
        /*否则说明我们已经得到了一个完整的HTTP请求*/
        return GET_REQUEST;
    }
    else if(strncasecmp(text,"Connection:",11)==0){
        text+=11;
        text+=strspn(text," ");
        if(strcasecmp(text,"keep-alive")==0)
        {
            m_linger=true;
        }
    }
    else if(strncasecmp(text,"Content-Length:",15)){
        text += 15;
        text += strspn(text, " ");
        m_content_length=atol(text);
    }
    else if(strncasecmp(text,"Host:",5)==0){
        text+=5;
        text+=strspn(text," ");
        m_host=text;
    }
    else{
        //其他的解析
    }
    return NO_REQUEST;
}
//TODO POST请求
HttpParse::HTTP_CODE HttpParse::ParseContent(char* text){
    //如果是POST请求，则可以进行解析json文件或者xml文件
    if(m_read_idx>=(m_content_length+m_check_idx))
    {
        text[m_content_length] = '\0';
        m_content = text;
        return GET_REQUEST;
    }
    return NO_REQUEST;
}

HttpParse::HTTP_CODE HttpParse::Parse(){
    LINE_STATUS line_status=LINE_OK;
    HTTP_CODE ret=NO_REQUEST;
    char* text=0;
    while(((state_==CHECK_STATE_CONTENT)&&(line_status==LINE_OK))||((line_status=ParseLine())==LINE_OK)){
        text = get_line();
        //printf("get line:%s\n", text);
        m_start_line = m_check_idx;
        switch (state_)
        {
        case CHECK_STATE_REQUESTLINE:
        {
            ret = ParseRequestLine(text);
            if (ret == BAD_REQUEST)
            {
                return BAD_REQUEST;
            }
            break;
            }
            case CHECK_STATE_HEADER:
            {
                ret = ParseHeader(text);
                if(ret==BAD_REQUEST){
                    return BAD_REQUEST;
                }
                else if(ret==GET_REQUEST)
                {
                    return do_request();
                }
                break;
            }
            case CHECK_STATE_CONTENT:
            {
                ret = ParseContent(text);
                if(ret==BAD_REQUEST){
                    return BAD_REQUEST;
                }
                else if(ret==GET_REQUEST){
                    return do_request();
                }
                line_status = LINE_OPEN;
                break;
            }
            default:
            {
                return INTERNAL_ERROR;
            }
            }
    }
    return NO_REQUEST;
}

bool HttpParse::add_response(const char*format,...){
    if(m_write_idx>=WRITE_BUFFER_SIZE)
    {
        return false;
    }
    va_list arg_list;
    va_start(arg_list,format);
    int len=vsnprintf(m_write_buf+m_write_idx,WRITE_BUFFER_SIZE-1-m_write_idx,format,arg_list);
    if(len>=(WRITE_BUFFER_SIZE-1-m_write_idx))//如果要格式化写入的字符个数大于缓冲区剩余，则直接失败
    {
        return false;
    }
    m_write_idx+=len;//要写入缓冲区的字符个数加上len
    va_end(arg_list);
    return true;
}

bool HttpParse::add_status_line(int status,const char*title)
{
	return add_response("%s%d%s\r\n","HTTP/1.1",status,title);
}

bool HttpParse::add_content_type(){
    char tmp[FILENAME_LEN];
    int t_len = strlen(m_url);
    memcpy(tmp, m_url, t_len);
    char *suffix;
    for (int i = 0; i < t_len; i++)
    {
        if(tmp[i]=='.'){
            suffix = tmp;
        }
    }
    std::string s(suffix);
    std::string t;
    if (SUFFIX_TYPE.count(s))
    {
        t = SUFFIX_TYPE.find(s)->second;
    }
    else{
        t = "text/plain";
    }
    //printf("suffix:%s\n", t.c_str());
    return add_response("Content-type:%s\r\n", t.c_str());
}

bool HttpParse::add_headers(int content_len)
{
    add_content_length(content_len);
    add_linger();
    add_content_type();
    add_blank_line();
    return true;
}
bool HttpParse::add_content_length(int content_len)
{
	return add_response("Content-Length:%d\r\n",content_len);
}
bool HttpParse::add_linger()
{
	return add_response("Connection:%s\r\n",(m_linger==true)?"keep-alive":"close");
}
bool HttpParse::add_blank_line()
{
	return add_response("%s","\r\n");
}
bool HttpParse::add_content(const char*content)
{
	return add_response("%s",content);
}

HttpParse::HTTP_CODE HttpParse::do_request(){
    strcpy(m_real_file,root_dir);
    int len=strlen(root_dir);
    strncpy(m_real_file+len,m_url,FILENAME_LEN-len-1);
    //printf("request path:%s\n", m_real_file);
    LOG_INFO<<"request path:"<<m_real_file;
    if (stat(m_real_file, &m_file_stat) < 0)
    {
        return HttpParse::NO_RESOURCE;
    }
    if(!(m_file_stat.st_mode&S_IROTH)) //其他用户权限，则禁止访问
    {
        return HttpParse::FORBIDDEN_REQUEST;
    }
    if(S_ISDIR(m_file_stat.st_mode)) //如果是一个目录
    {
        return HttpParse::BAD_REQUEST;
    }
    int fd=open(m_real_file,O_RDONLY);
    m_file_address=(char*)mmap(0,m_file_stat.st_size,PROT_READ,MAP_PRIVATE,fd,0);//mmap映射，PROT_READ表示映射区域可读，MAP_PRIVATE表示对映射区域的写操作不会影响到源文件
    ::close(fd);
    return HttpParse::FILE_REQUEST;
}

bool HttpParse::process_write(HTTP_CODE ret){
    switch(ret)
    {
        case INTERNAL_ERROR:
        {
            add_status_line(500,error_500_title);
            add_headers(strlen(error_500_form));
            if(!add_content(error_500_form))
            {
                return false;
            }
            break;
        }
        case BAD_REQUEST:
        {
            add_status_line(400,error_400_title);
            add_headers(strlen(error_400_form));
            if(!add_content(error_400_form))
            {
                return false;
            }
            break;
        }
        case NO_RESOURCE:
        {
            add_status_line(404,error_404_title);
            add_headers(strlen(error_404_form));
            if(!add_content(error_404_form))
            {
                return false;
            }
            break;
        }
        case FORBIDDEN_REQUEST:
        {
            add_status_line(403,error_403_title);
            add_headers(strlen(error_403_form));
            if(!add_content(error_403_form))
            {
                return false;
            }
            break;
        }
        case FILE_REQUEST:
        {
            add_status_line(200,ok_200_title);
            if(m_file_stat.st_size!=0)
            {
                add_headers(m_file_stat.st_size);
                m_iv[0].iov_base=m_write_buf;
                m_iv[0].iov_len=m_write_idx;
                m_iv[1].iov_base=m_file_address;//文件内容，映射到文件映射区的
                m_iv[1].iov_len=m_file_stat.st_size;
                m_iv_count=2;
                return true;
            }
            else
            {
                const char*ok_string="<html><body></body></html>";
                add_headers(strlen(ok_string));
                if(!add_content(ok_string))
                {
                    return false;
                }
            }
        }
        default:
        {
            return false;
        }
    }
    m_iv[0].iov_base=m_write_buf;
    m_iv[0].iov_len=m_write_idx;
    m_iv_count=1;
    return true;
}

bool HttpParse::process(std::string buf){
    this->init(buf);
    HTTP_CODE ret = this->Parse();
    return this->process_write(ret);
}

void HttpParse::unmap(){
    if(m_file_address)
    {
        munmap(m_file_address,m_file_stat.st_size);
        m_file_address=0;
    }
}

void HttpParse::close(){
    // std::cout << "m_url:" <<m_url<< std::endl;
    // std::cout << "m_version"<<m_version << std::endl;
    // std::cout << "m_host" <<m_host<< std::endl;
    // std::cout << "m_content" <<m_content<< std::endl;
    // std::cout << "root_dir" <<root_dir<< std::endl;
    //delete root_dir;

    unmap();
}

HttpParse::~HttpParse(){
    close();
}