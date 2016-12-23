#include <WSLog.h>

using WsTools::Log;

int main(int argc, char ** argv)
{
    WSLog ws_log;

    //只打印字符串
    ws_log.info("只打印字符串");

    //占位符
    ws_log.info("占位符: {0}", "字符串");

    //占位符复用
    ws_log.info("占位符复用: {0}, {0}", "字符串");

    //字符串长度
    ws_log.info("字符串长度: {0, 10}{0}", "字符串");
    ws_log.info("字符串长度: {0, -10}", "字符串");

    //数字
    ws_log.info("数字: {0}", 12);

    //数字格式化
    ws_log.info("数字格式化: {0, D}", 12);
    ws_log.info("数字格式化: {0, D3}", 12);
    ws_log.info("数字格式化: {0, D3}", 12.56);

    //小数
    ws_log.info("小数: {0, F}", "12.345");
    ws_log.info("小数: {0, F}", 12.345);
    ws_log.info("小数: {0, F1}", 12.345);

    //金钱
    ws_log.info("t金钱est: {0, C4}", "$12.34567");

    //设置日志级别
    
    ws_log.info("设置日志级别：DEBUG");
    ws_log.setFilterLevel(Log::level::DEBUG);
    ws_log.debug(".");
    ws_log.info(".");
    ws_log.notice(".");
    ws_log.warning(".");
    ws_log.error(".");
    ws_log.critical(".");
    ws_log.alert(".");
    ws_log.emerg(".");

    ws_log.info("设置日志级别：ERR");
    ws_log.setFilterLevel(Log::level::ERR);
    ws_log.debug(".");
    ws_log.info(".");
    ws_log.notice(".");
    ws_log.warning(".");
    ws_log.error(".");
    ws_log.critical(".");
    ws_log.alert(".");
    ws_log.emerg(".");
    
    return 0;
}