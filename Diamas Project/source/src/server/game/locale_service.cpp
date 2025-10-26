#include "locale_service.h"
#include "GBanWord.h"
#include "mob_manager.h"
#include <locale>

using namespace std;

bool has_x_consecutive_characters(std::string const &str, int32_t x)
{
    // Keep the end of the string, and point i to the first run's beginning
    auto e = end(str), i = std::adjacent_find(begin(str), e);

    bool requirementsMet = false;

    if (i == e)
        return requirementsMet;

    do
    {
        // Locate the end of the run
        auto next = std::find_if(i, e, [&i](auto const &c) { return c != *i; });

        auto count = std::distance(i, next);
        if (count >= x)
        {
            requirementsMet = true;
            break;
        }

        // Skip to the next run's beginning
        i = std::adjacent_find(next, e);

        // Do so until we reached the end of the string
    }
    while (i != e);

    return requirementsMet;
}

int check_name_independent(const char *str)
{
    std::string stTemp(str);
    std::transform(stTemp.begin(), stTemp.end(), stTemp.begin(), ::tolower);

    if (CBanwordManager::instance().CheckString(stTemp))
        return 0;

    if (has_x_consecutive_characters(str, 3))
        return 0;

    if (CMobManager::instance().Get(stTemp, false))
        return 0;

    return 1;
}

int check_name(const char *str)
{
    if (!str || !*str)
        return 0;

    if (strlen(str) < 2)
        return 0;

    for (const char *tmp = str; *tmp; ++tmp)
    {
        // 알파벳과 수자만 허용
        if (isdigit(*tmp) || isalpha(*tmp))
            continue;
        else
            return 0;
    }

    return check_name_independent(str);
}
