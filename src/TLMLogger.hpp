//
//  TLMLogger.hpp
//  TLM_Thesis
//
//  Created by Ian Wang on 2020/5/26.
//  Copyright © 2020 ian wang. All rights reserved.
//

#ifndef TLMLogger_hpp
#define TLMLogger_hpp

#include <iostream>
#include <string>

using namespace std;

/// TLM logging manager.
///
/// Note that you need to define macro variable DEBUG_MODE before to enable logger
/// or TLMLogger will do nothing.
class TLMLogger {
 private:
    /// Singleton shared instance.
    static TLMLogger* _shared;
    
    /// Private initializer.
    TLMLogger();
    
    /// Actual log handling.
    void _log(string tag, string text);
    
 public:
    /// Get shared instance.
    static TLMLogger* shared();
    
    /// Log output stream. Default is std::cout.
    ostream& os = cout;

    /// Log with tag.
    inline static void log(string tag, string text);

#ifdef DEBUG_MODE
    /// Debug log command.
    ///
    /// TAG: A log tag. NULL to log without tagging and do plain text exporting.
    ///      Note that for NULL tag, no auto carry return will occure.
    ///
    /// LOG_COMMAND: A c++ style output stream command.
    ///              For example, TLM("tag", "Hello " << "World!") will translate into
    ///              Log << "[tag] " << "Hello " << "World!" << endl
    #define TLMLOG(TAG, LOG_COMMAND) \
        (TAG!=NULL)?(TLMLogger::shared()->os<<"["<<TAG<<"] "<<LOG_COMMAND<<endl): \
              (TLMLogger::shared()->os<<LOG_COMMAND)
#else
    #define TLMLOG(TAG, LOG_COMMAND)
#endif
};

#endif /* TLMLogger_hpp */
