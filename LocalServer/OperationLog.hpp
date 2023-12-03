//
//  OperationLog.hpp
//  Mp3Player
//
//  Created by henry_xiao on 2023/1/24.
//

#ifndef OperationLog_hpp
#define OperationLog_hpp

#include "../third-parties/sqlite/Sqlite3.hpp"


class OperationLog {
public:
    CSqlite3                    m_db;

    CSqlite3Stmt                m_stmtAdd, m_stmtQueryRecent;

};


#endif /* OperationLog_hpp */
