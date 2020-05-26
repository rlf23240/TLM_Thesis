//
//  TLMLogger.cpp
//  TLM_Thesis
//
//  Created by Ian Wang on 2020/5/26.
//  Copyright © 2020 ian wang. All rights reserved.
//

#include "TLMLogger.hpp"

TLMLogger* TLMLogger::_shared = NULL;

TLMLogger* TLMLogger::shared() {
    if (_shared == NULL) {
        _shared = new TLMLogger();
    }
    return _shared;
}

TLMLogger::TLMLogger() {}

void TLMLogger::_log(string tag, string text) {
    os << "[" << tag << "] " << text << endl;
}

void TLMLogger::log(string tag, string text) {
#ifdef DEBUG_MODE
    TLMLogger::shared()->_log(tag, text);
#endif
}
