#include "WSLog.h"
#include <random>

namespace WsTools
{
    using std::random_device;
    using std::unique_lock;
    using std::defer_lock;

    string WSLog::_levelName[]  = { "DEBUG", "INFO", "NOTICE", "WARN", "ERR", "CRIT", "ALERT", "EMERG" };

    map<string, string> WSLog::escapes;

    mutex WSLog::mtx;

    WSLog::WSLog(void)
    {
        _filter = level::DEBUG;
    }

    void WSLog::setFilterLevel(const level _level)
    {
        _filter = _level;
    }

    void WSLog::printLog(const string & level, const string & log) const
    {
        cout << level << log << endl;
    }

    void WSLog::_log(const level _level, const string & str) const
    {
        if(_level < _filter) return;
        //输出打印信息
        _printLog(_level, str);
    }

    bool WSLog::_format_D(stringstream & strs, const string & format, const string & source) const
    {
        static regex format_D("^[dD](\\d*)$");
        auto matchBegin = sregex_iterator(format.begin(), format.end(), format_D);
        auto matchEnd = sregex_iterator();

        if(matchBegin == matchEnd){ return false; }
        
        int minCount = atoi(matchBegin->str(1).c_str());

        static regex checkNumber("^[+-]?(\\d+(\\.\\d+)?)(e\\+\\d+)?$|^[+-]?\\d{1,3}(,\\d{3})*(\\.(\\d{3},)*\\d{1,3})?(e\\+\\d+)?$");
        matchBegin = sregex_iterator(source.begin(), source.end(), checkNumber);

        if(matchBegin == matchEnd)
        {
            error("格式化数据不匹配");
            strs << format;
            return true;
        }
        
        string sourceData = matchBegin->str();
        int convertData;
        
        for(auto index = sourceData.find(','); index != string::npos; index = sourceData.find(','))
        {
            sourceData.replace(sourceData.begin() + (int)index, sourceData.begin() + (int)index + 1, "");
        }

        if(sourceData.find('.') != string::npos)
        {
            convertData = (int)roundf((float)atof(sourceData.c_str()));
        }else
        {
            convertData = atoi(sourceData.c_str());
        }

        stringstream tempSStr;
        tempSStr << convertData;

        for(int i = (int)(tempSStr.str().length()); i < minCount; ++i){ strs << "0"; }
        strs << convertData;

        return true;
    }

    bool WSLog::_format_C(stringstream & strs, const string & format, const string & source) const
    {
        static regex format_D("^[cC](\\d*)$");
        auto matchBegin = sregex_iterator(format.begin(), format.end(), format_D);
        auto matchEnd = sregex_iterator();
        if(matchBegin == matchEnd){ return false; }

        int decimalDigits = 2;
        if(matchBegin->str(1).length() > 0){ decimalDigits = atoi(matchBegin->str(1).c_str()); }

        static regex checkNumber("^([^-+]?)([+-]?\\d+(\\.\\d+)?(e\\+\\d+)?)$|^([^-+]?)([+-]?\\d{1,3}(,\\d{3})*(\\.(\\d{3},)*\\d{1,3})?(e\\+\\d+)?)$");
        matchBegin = sregex_iterator(source.begin(), source.end(), checkNumber);

        if(matchBegin == matchEnd)
        {
            error("尝试格式化失败");
            strs << format;
            return true;
        }

       string unit = matchBegin->str(1).length() > 0 ? matchBegin->str(1) : "￥";

       string sourceData = matchBegin->str(2);

        for(auto index = sourceData.find(','); index != string::npos; index = sourceData.find(','))
        {
            sourceData.replace(sourceData.begin() + (int)index, sourceData.begin() + (int)index + 1, "");
        }

        float convertData = (float)(roundf((float)(atof(sourceData.c_str()) * pow(10, decimalDigits))) / pow(10, decimalDigits));

        stringstream tempSStr;
        tempSStr << convertData;

        //科学计数法转换
        string tempStr = scientificToDefault(tempSStr.str());
        strs << unit << tempStr;

        auto dotIndex = tempStr.find('.');
        if(dotIndex == string::npos)
        {
            strs << ".";
            dotIndex = tempStr.length() - 1;
        }

        for(int i = (int)tempStr.length() - (int)dotIndex - 1; i < decimalDigits; ++i){ strs << "0"; }
                    
        return true;
    }

    bool WSLog::_format_F(stringstream & strs, const string & format, const string & source) const
    {
        static regex format_D("^[fF](\\d*)$");
        auto matchBegin = sregex_iterator(format.begin(), format.end(), format_D);
        auto matchEnd = sregex_iterator();

        if(matchBegin == matchEnd){ return false; }

        int decimalDigits = 2;
        if(matchBegin->str(1).length() > 0)
        {
            decimalDigits = atoi(matchBegin->str(1).c_str());
        }

        static regex checkNumber("^([+-]?\\d+(\\.\\d+)?)(e[\\+-]\\d+)?$|^[+-]?\\d{1,3}(,\\d{3})*(\\.(\\d{3},)*\\d{1,3})?(e[\\+-]\\d+)?$");
        matchBegin = sregex_iterator(source.begin(), source.end(), checkNumber);
        if(matchBegin == matchEnd)
        {
            error("尝试格式化失败");
            strs << format;
            return true;
        }

        string sourceData = matchBegin->str();

        for(auto index = sourceData.find(','); index != string::npos; index = sourceData.find(','))
        {
            sourceData.replace(sourceData.begin() + (int)index, sourceData.begin() + (int)index + 1, "");
        }

        double convertData = round(atof(sourceData.c_str()) * pow(10, decimalDigits)) / pow(10, decimalDigits);

        stringstream tempSStr;
        tempSStr << convertData;

        //科学计数法转换
        string tempStr = scientificToDefault(tempSStr.str());
        strs << tempStr;

        auto dotIndex = tempStr.find('.');
        if(dotIndex == string::npos)
        {
            strs << ".";
            dotIndex = tempStr.length() - 1;
        }

        for(int i = (int)tempStr.length() - (int)dotIndex - 1; i < decimalDigits; ++i){ strs << "0"; }
        return true;
    }

    bool WSLog::_format_aline(stringstream & strs, const string & format, const string & source) const
    {
        static regex format_D("^([-]?)(\\d*)$");
        auto matchBegin = sregex_iterator(format.begin(), format.end(), format_D);
        auto matchEnd = sregex_iterator();
        if(matchBegin == matchEnd){ return false; }

        int minCount = atoi(matchBegin->str(2).c_str());

        if(matchBegin->str(1).c_str() == string("-"))
        {
            for(int i = (int)getStringLength(source); i < minCount; ++i)
            {
                strs << " ";
            }
        }

        strs << source;

        if(matchBegin->str(1).c_str() != string("-"))
        {
            for(int i = (int)getStringLength(source); i < minCount; ++i)
            {
                strs << " ";
            }
        }

        return true;
    }


    void WSLog::_printLog(const level _level, const string & log) const
    {
        stringstream sstr;
        sstr << "[" << _levelName[_level] << "] ";
        //加线程锁锁
        unique_lock<mutex> lck(mtx, defer_lock);
        lck.lock();
            printLog(sstr.str(), log);
        lck.unlock();
    }

    string WSLog::scientificToDefault(const string & source) const
    {
        string result = source;
        //处理科学计数法
        auto scientificIndex = result.find("e+");
        if(scientificIndex != string::npos)
        {
            string baseNumber = result.substr(0, scientificIndex);
            int exponential = atoi(result.substr(scientificIndex + 2).c_str());
            auto dotIndex = baseNumber.find('.');
            baseNumber.replace(dotIndex, dotIndex + 1, "");
            while(baseNumber.length() < exponential + dotIndex)
            {
                baseNumber += "0";
            }
            result = baseNumber;
        }

        scientificIndex = result.find("e-");
        if(scientificIndex != string::npos)
        {
            string baseNumber = result.substr(0, scientificIndex);
            int exponential = atoi(result.substr(scientificIndex + 2).c_str());
            auto dotIndex = baseNumber.find('.');
            baseNumber.replace(dotIndex, dotIndex + 1, "");

            string tempHead = "";
            while((int)tempHead.length() + 1 < exponential)
            {
                tempHead += "0";
            }
            result = "0." + tempHead + baseNumber;
        }
        return result;
    }

    string WSLog::createUuid(void) const
    {
        char temp[33] = {0};
        for(size_t i = 0; i < (sizeof(temp) - 1) / 2; ++i)
        {
            random_device rd;
            uniform_int_distribution<int> dis(0x00, 0xFF);
            SPRINTF(temp + 2 * i, 2, "%02X", dis(rd));
        }
        return temp;
    }

    //返回字符串实际长度（单位是一个英文字符）
    const unsigned WSLog::getStringLength(const string & str, const string & coding) const
    {
        unsigned result = 0;
        if(coding == "utf-8")
        {
            //根据utf-8编码规则判断一个字符占几个字节
            for(size_t i = 0; i < str.length();)
            {
                int length = 0;
                unsigned short temp = (unsigned short)(str.at(i) & 0x00FF);
                while((temp << length & 0x00FF) > 1 << 7)
                {
                    length ++;
                }
                //大于０x80的字符是中文并且占两个英文字符的宽度
                result += temp < 0x80 ? 1 : 2;
                length = length < 1 ? 1 : length;
                i += length;
            }
        }else
        {
            //其他编码类型还没有做
            result = str.length();
        }
        return result;
    }

    WSLog Log;
}
