#ifndef STDLIB_CONVERT_H
    #define STDLIB_CONVERT_H
    #include <string>
    #include <type_traits>
    template<typename Arg> constexpr std::string _obn_toString(Arg arg)
    {
        return std::to_string(arg);
    }
    template<typename Arg> constexpr int _obn_toInt(Arg arg)
    {
        if constexpr (std::is_same_v<T, std::string>)
        {
            return std::stoi(arg);
        }
        if constexpr (std::is_same_v<T, const char*> || std::is_same_v<T, char*>)
        {
            return std::atoi(arg);
        }
        return static_cast<int>(arg);
    }
    template<typename Arg> constexpr double _obn_toFloat(Arg arg)
    {
        if constexpr (std::is_same_v<T, std::string>)
        {
            return std::stod(arg);
        }
        if constexpr (std::is_same_v<T, const char*> || std::is_same_v<T, char*>)
        {
            return std::atod(arg);
        }
        return static_cast<double>(arg);
    }
#endif