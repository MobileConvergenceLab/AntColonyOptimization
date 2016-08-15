#ifndef DBG_HPP
#define DBG_HPP

#include <iostream>

class _Logger
{
public:
    _Logger(std::string f) :m_f(f)
    {
        std::cerr << m_f << "() has been called" << std::endl;
    }
    ~_Logger()
    {
        std::cerr << m_f << "() has been ended" << std::endl << std::endl;
    }
private:
 
    std::string m_f;
};

#ifdef DBG
#	define DBG_LOGGER  _Logger __logger(__FUNCTION__)
#	define DBG_PRINT_VALUE(VALUE)	std::cerr << __FILE__ << ":" << __LINE__ << ' ' << #VALUE << ": " << (VALUE) << std::endl
#	define DBG_PRINT_MSG(STR)	std::cerr << __FILE__ << ":" << __LINE__ << ' ' << STR << std::endl
#else
#	define DBG_LOGGER
#	define DBG_PRINT_VALUE(VALUE)
#	define DBG_PRINT_MSG(STR)
#endif

#endif
