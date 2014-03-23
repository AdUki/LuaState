//
//  LuaException.h
//  LuaState
//
//  Created by Simon Mikuda on 17/03/14.
//
//

#pragma once

#include <string>
#include <exception>

namespace lua {

    //////////////////////////////////////////////////////////////////////////////////////////////////
    class LoadError: public std::exception
    {
        std::string _message;
        
    public:
        LoadError(const std::string& message) : _message(message) {}
        virtual ~LoadError() throw() {}
        
        virtual const char* what() const throw()
        {
            return _message.c_str();
        }
    };
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    /// Don't forget to call execute manualy while using protected call! Exceptions cannot be catched while throwed in destructor...
    class RuntimeError: public std::exception
    {
        std::string _message;
        
    public:
        RuntimeError(const std::string& message) : _message(message) {}
        virtual ~RuntimeError() throw() {}
        
        virtual const char* what() const throw()
        {
            return _message.c_str();
        }
    };

    //////////////////////////////////////////////////////////////////////////////////////////////////
    class StackError: public std::exception
    {
        int _stackTop;
        int _valueTop;
        
    public:
        StackError(int stackTop, int valueTop) : _stackTop(stackTop), _valueTop(valueTop) {}
        virtual ~StackError() throw() {}
        
        virtual const char* what() const throw()
        {
            static const char format[] =
                "Invalid stack manipulation! "
                "Stack top is %d lua::Value top is %d. "
                "Clean lua::Values that are not in use!";
            
            static char message[sizeof(format) + 10];
            sprintf(message, format, _stackTop, _valueTop);
            return  message;
        }
    };
}