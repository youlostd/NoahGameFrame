#ifndef __INC_SCRIPT_PARSER_H__
#define __INC_SCRIPT_PARSER_H__

#include <list>
#include <string>

class ScriptGroup
{
public:
    struct SArgument
    {
        SArgument(std::string name,
                  std::string value)
            : strName(std::move(name))
              , strValue(std::move(value))
        {
            // ctor
        }

        std::string strName;
        std::string strValue;
    };

    typedef std::list<SArgument> TArgList;

    struct SCmd
    {
        std::string name;
        TArgList argList;
    };

    /** 스트링으로 부터 스크립트 그룹을 만든다.
    *
    * 실패하면 GetError 메소드로 확인할 수 있다.
    *
    * @param source 이 스트링으로 부터 그룹이 만들어 진다.
    * @return 성공시 true, 실패하면 false
    */
    uint32_t Create(const std::string &source);

    /** 명령어를 받는 메소드
    *
    * @param cmd 성공시에 이 구조체로 명령어가 복사 된다.
    * @return 명령어가 남아 있다면 true, 없다면 false
    */
    bool GetCmd(SCmd &cmd);

    /*
        명령어를 가져오되 꺼내지는 않는다.
    */
    bool ReadCmd(SCmd &cmd);

private:
    bool ParseArguments(const std::string &source, TArgList &argList);

    std::list<SCmd> m_cmdList;
};

#endif
