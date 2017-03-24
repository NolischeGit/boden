#ifndef BDN_AsyncStdioReader_H_
#define BDN_AsyncStdioReader_H_

#include <bdn/AsyncOpRunnable.h>

namespace bdn
{
	

/** Implements asynchronous reading from a std::basic_istream.   
*/
template<typename CharType>
class AsyncStdioReader : public Base
{
public:
    
    /** Constructor. The implementation does NOT take ownership of the specified streams, 
        i.e. it will not delete it. So it is ok to use std::cin here.*/
    AsyncStdioReader( basic_istream<CharType>* pStream );


    /** Asynchronously reads a line of text from the stream.
        The function does not wait until the line is read - it returns
        immediately.
        
        Use IAsyncOp::onDone() to register a callback that is executed when the
        operation has finished.
        */
    IAsyncOp<String> readLine()
    {
        {
            MutexLock lock(_mutex);

            if(_pThread==nullptr)
            {
                _pRunnable = newObj<Runnable>( _pInStream );
                _pThread = newObj<Thread>( _pRunnable );
            }
        }
            
        return _pRunnable->readLine();
    }


private:

    class ReadLineOp : public AsyncOpRunnable<String>
    {
    public:
        ReadLineOp()
        {
        }

    protected:        
        String doOp() override
        {           
            std::basic_string<CharType> l;
            std::getline(*_pInStream, l);
            
            _promise.set_value( decodeString(l, _pInStream->getloc() ) );
        }

    private:       
        
        template<class EncodedStringType>
        static String decodeString( const EncodedStringType& s, std::locale loc )
        {
            // the locale is not needed for most string types. See below for char specialization.
            return String(s);
        }

        template<>
        static String decodeString< std::basic_string<char> >( const std::basic_string<char>& s, std::locale loc )
        {
            return String::fromLocaleEncoding( s.c_str(), loc );
        }
    };
};


}


#endif
