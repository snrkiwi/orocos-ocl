#ifndef NO_GPL
/***************************************************************************
  tag: Peter Soetens  Thu Jul 3 15:31:34 CEST 2008  TaskBrowser.cpp

                        TaskBrowser.cpp -  description
                           -------------------
    begin                : Thu July 03 2008
    copyright            : (C) 2008 Peter Soetens
    email                : peter.soetens@fmtc.be

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   General Public License for more details.                              *
 *                                                                         *
 *   You should have received a copy of the GNU General Public             *
 *   License along with this program; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 ***************************************************************************/
#else
/***************************************************************************
  tag: Peter Soetens  Tue Dec 21 22:43:07 CET 2004  TaskBrowser.cxx

                        TaskBrowser.cxx -  description
                           -------------------
    begin                : Tue December 21 2004
    copyright            : (C) 2004 Peter Soetens
    email                : peter.soetens@mech.kuleuven.ac.be

 ***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/
#endif


#include <rtt/Logger.hpp>
#include <rtt/MultiVector.hpp>
#include <rtt/TypeStream.hpp>
#include "TaskBrowser.hpp"

#include "rtt/TryCommand.hpp"
#include <rtt/TaskContext.hpp>
#include <rtt/scripting/parser-debug.hpp>
#include <rtt/scripting/Parser.hpp>
#include <rtt/scripting/ProgramLoader.hpp>
#include <rtt/scripting/parse_exception.hpp>
#include <rtt/scripting/PeerParser.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <deque>
#include <stdio.h>

#if defined(HAS_READLINE) && !defined(NO_GPL)
#define USE_READLINE
#endif

#ifdef USE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif
#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>

#include <signal.h>

namespace OCL
{
    using namespace RTT;
    using namespace RTT::detail;
#ifdef USE_READLINE
    std::vector<std::string> TaskBrowser::candidates;
    std::vector<std::string> TaskBrowser::completes;
    std::vector<std::string>::iterator TaskBrowser::complete_iter;
    std::string TaskBrowser::component;
    std::string TaskBrowser::peerpath;
    std::string TaskBrowser::text;
#endif
    TaskContext* TaskBrowser::taskcontext = 0;
    OperationInterface* TaskBrowser::taskobject = 0;
    TaskContext* TaskBrowser::peer = 0;
    TaskContext* TaskBrowser::tb = 0;
    TaskContext* TaskBrowser::context = 0;

    using boost::bind;
    using namespace RTT;
    using namespace std;

    string TaskBrowser::red;
    string TaskBrowser::green;
    string TaskBrowser::blue;
    std::deque<TaskContext*> taskHistory;
    std::string TaskBrowser::prompt(" (type 'ls' for context info) :");
    std::string TaskBrowser::coloron;
    std::string TaskBrowser::underline;
    std::string TaskBrowser::coloroff;


    /**
     * Our own defined "\n"
     */
    static std::ostream&
    nl(std::ostream& __os)
    { return __os.put(__os.widen('\n')); }

#ifdef _POSIX_VERSION
    // catch ctrl+c signal
    void ctrl_c_catcher(int sig)
    {
        ::signal(sig, SIG_IGN);
#ifdef USE_READLINE
//        cerr <<nl<<"TaskBrowser intercepted Ctrl-C. Type 'quit' to exit."<<endl;
//         rl_delete_text(0, rl_end);
//         //cerr << "deleted " <<deleted <<endl;
        rl_free_line_state();
//         rl_cleanup_after_signal();
#endif
        ::signal(SIGINT, ctrl_c_catcher);
    }
#endif

#ifdef USE_READLINE
    char *TaskBrowser::rl_gets ()
    {
        /* If the buffer has already been allocated,
           return the memory to the free pool. */
        if (line_read)
            {
#ifdef _WIN32
	      /**
	       * If you don't have this function, download the readline5.2 port
	       * from :
	       http://gpsim.sourceforge.net/gpsimWin32/packages/readline-5.2-20061112-src.zip
	       http://gpsim.sourceforge.net/gpsimWin32/packages/readline-5.2-20061112-lib.zip
	       http://gpsim.sourceforge.net/gpsimWin32/packages/readline-5.2-20061112-bin.zip
	       *
	       * free(line_read) will cause crashes since the malloc has been
	       * done in another DLL. You can emulate rl_free by adding this function
	       * yourself to your readline library and let it free its argument.
	       */
	      free(line_read);
#else
                free (line_read);
#endif
                line_read = 0;
            }

        /* Get a line from the user. */
        std::string p;
        if ( !macrorecording ) {
            p = prompt;
        } else {
            p = "> ";
        }
#ifndef _WIN32  // does not work on win32
        if (rl_set_signals() != 0)
            cerr << "Error setting signals !" <<endl;
#endif
        line_read = readline ( p.c_str() );

        /* If the line has any text in it,
           save it on the history. */
        if (line_read && *line_read) {
            // do not store "quit"
            string s = line_read;
            if (s != "quit")
                add_history (line_read);
        }
        return (line_read);
    }

    char* TaskBrowser::dupstr( const char *s )
    {
        char * rv;
        // must be C-style :
        rv = (char*) malloc( strlen( s ) + 1 );
        strncpy( rv, s, strlen(s) + 1 );
        return rv;
    }

    char *TaskBrowser::command_generator( const char *_text, int state )
    {
        // first time called.
        if ( !state )
            {
                // make a copy :
                text = _text;
                // rebuild a completion list
                completes.clear();
                find_completes();
                complete_iter = completes.begin();
            }
        else
            ++complete_iter;

        // return zero if nothing more is found
        if ( complete_iter == completes.end() )
            return 0;
        // return the next completion option
        return  dupstr( complete_iter->c_str() ); // RL requires malloc !
    }

    /**
     * This is the entry function to look up all possible completions at once.
     *
     */
    void TaskBrowser::find_completes() {
        std::string::size_type pos;
        std::string::size_type startpos;
        std::string line( rl_line_buffer, rl_point );

        // complete on 'cd' or 'ls' :
        if ( line.find(std::string("cd ")) == 0 || line.find(std::string("ls ")) == 0) {
            //cerr <<endl<< "switch to :" << text<<endl;
//             pos = text.rfind(".");
            pos = line.find(" ");      // pos+1 is first peername
            startpos = line.find_last_of(". "); // find last peer
            //cerr <<"startpos :"<<startpos<<endl;
            // start searching from component.
            peer = taskcontext;
            if ( pos+1 != line.length() ) // bounds check
                peer = findPeer( line.substr(pos+1) );

            if (!peer)
                return;
            //std::string peername = text.substring( pos+1, std::string::npos );
            TaskContext::PeerList v = peer->getPeerList();
            for (TaskContext::PeerList::iterator i = v.begin(); i != v.end(); ++i) {
                std::string path;
                if ( !( pos+1 > startpos) )
                    path = line.substr(pos+1, startpos - pos);
                //cerr << "path :"<<path<<endl;
                if ( *i == line.substr(startpos+1) )
                     completes.push_back( path + *i + ".");
                else
                    if ( startpos == std::string::npos || startpos+1 == line.length() || i->find( line.substr(startpos+1)) == 0 )
                        completes.push_back( path + *i );
            }
            // Stop here if 'cd'
            if (line.find(std::string("cd ")) == 0)
                return;
            // Add objects for 'ls'.
            v = peer->getObjectList();
            for (TaskContext::PeerList::iterator i = v.begin(); i != v.end(); ++i) {
                std::string path;
                if ( !( pos+1 > startpos) )
                    path = line.substr(pos+1, startpos - pos);
                //cerr << "path :"<<path<<endl;
                if ( *i == line.substr(startpos+1) )
                     completes.push_back( path + *i + ".");
                else
                    if ( startpos == std::string::npos || startpos+1 == line.length() || i->find( line.substr(startpos+1)) == 0 )
                        completes.push_back( path + *i );
            }
            return; // do not add component names.
        }

        // TaskBrowser commands :
        if ( line.find(std::string(".")) == 0 ) {
            // first make a list of all sensible completions.
            std::vector<std::string> tbcoms;
            tbcoms.push_back(".loadProgram ");
            tbcoms.push_back(".unloadProgram ");
            tbcoms.push_back(".loadStateMachine ");
            tbcoms.push_back(".unloadStateMachine ");
            tbcoms.push_back(".light");
            tbcoms.push_back(".dark");
            tbcoms.push_back(".nocolors");
            tbcoms.push_back(".connect");
            tbcoms.push_back(".record");
            tbcoms.push_back(".end");
            tbcoms.push_back(".cancel");

            // then see which one matches the already typed line :
            for( std::vector<std::string>::iterator it = tbcoms.begin();
                 it != tbcoms.end();
                 ++it)
                if ( it->find(line) == 0 )
                    completes.push_back( *it ); // if partial match, add.
            return;
        }

        if ( line.find(std::string("list ")) == 0
             || line.find(std::string("trace ")) == 0
             || line.find(std::string("untrace ")) == 0) {
            stringstream ss( line.c_str() ); // copy line into ss.
            string lcommand;
            ss >> lcommand;
            lcommand += ' ';
            std::vector<std::string> progs;

            // THIS:
            progs = context->scripting()->getPrograms();
            // then see which one matches the already typed line :
            for( std::vector<std::string>::iterator it = progs.begin();
                 it != progs.end();
                 ++it) {
                string res = lcommand + *it;
                if ( res.find(line) == 0 )
                    completes.push_back( *it ); // if partial match, add.
            }
            progs = context->scripting()->getStateMachines();
            for( std::vector<std::string>::iterator it = progs.begin();
                 it != progs.end();
                 ++it) {
                string res = lcommand + *it;
                if ( res.find(line) == 0 )
                    completes.push_back( *it ); // if partial match, add.
            }

            return;
        }

        startpos = text.find_last_of(",( ");
        if ( startpos == std::string::npos )
            startpos = 0;      // if none is found, start at beginning

        // complete on peers and objects, and find the peer the user wants completion for
        find_peers(startpos);
        // now proceed with 'this->peer' as TC,
        // this->taskobject as TO and
        // this->component as object and
        // this->peerpath as the text leading up to 'this->component'.

        // store the partial compname or peername
        std::string comp = component;


        // NEXT: use taskobject to complete commands, events, methods, attrs
        // based on 'component' (find trick).
        // if taskobject == peer, also complete properties
        find_ops();

        // TODO: concat two cases below as text.find("cd")...
        // check if the user is tabbing on an empty command, then add the console commands :
        if (  line.empty() ) {
            completes.push_back("cd ");
            completes.push_back("cd ..");
            completes.push_back("ls");
            completes.push_back("help");
            completes.push_back("quit");
            completes.push_back("list");
            completes.push_back("trace");
            completes.push_back("untrace");
            if (taskcontext == context)
                completes.push_back("leave");
            else
                completes.push_back("enter");
            // go on below to add components and tasks as well.
        }

        // only try this if text is not empty.
        if ( !text.empty() ) {
            if ( std::string( "cd " ).find(text) == 0 )
                completes.push_back("cd ");
            if ( std::string( "ls" ).find(text) == 0 )
                completes.push_back("ls");
            if ( std::string( "cd .." ).find(text) == 0 )
                completes.push_back("cd ..");
            if ( std::string( "help" ).find(text) == 0 )
                completes.push_back("help");
            if ( std::string( "quit" ).find(text) == 0 )
                completes.push_back("quit");
            if ( std::string( "list " ).find(text) == 0 )
                completes.push_back("list ");
            if ( std::string( "trace " ).find(text) == 0 )
                completes.push_back("trace ");
            if ( std::string( "untrace " ).find(text) == 0 )
                completes.push_back("untrace ");

            if (taskcontext == context && string("leave").find(text) == 0)
                completes.push_back("leave");

            if (context == tb && string("enter").find(text) == 0)
                completes.push_back("enter");
        }
    }

    void TaskBrowser::find_ops()
    {
        // the last (incomplete) text is stored in 'component'.
        // all attributes :
        std::vector<std::string> attrs;
        attrs = taskobject->attributes()->names();
        for (std::vector<std::string>::iterator i = attrs.begin(); i!= attrs.end(); ++i ) {
            if ( i->find( component ) == 0 && !component.empty() )
                completes.push_back( peerpath + *i );
        }
        // all properties if TaskContext:
        if (taskobject == peer && peer->properties() != 0 ) {
            std::vector<std::string> props;
            peer->properties()->list(props);
            for (std::vector<std::string>::iterator i = props.begin(); i!= props.end(); ++i ) {
                if ( i->find( component ) == 0 && !component.empty() ) {
                    completes.push_back( peerpath + *i );
                }
            }
        }

        // commands:
        std::vector<std::string> comps;
        comps = taskobject->commands()->getNames();
        for (std::vector<std::string>::iterator i = comps.begin(); i!= comps.end(); ++i ) {
            if ( i->find( component ) == 0  )
                completes.push_back( peerpath + *i );
        }
        // methods:
        comps = taskobject->methods()->getNames();
        for (std::vector<std::string>::iterator i = comps.begin(); i!= comps.end(); ++i ) {
            if ( i->find( component ) == 0  )
                completes.push_back( peerpath + *i );
        }
        // events:
        comps = taskobject->events()->getNames();
        for (std::vector<std::string>::iterator i = comps.begin(); i!= comps.end(); ++i ) {
            if ( i->find( component ) == 0  )
                completes.push_back( peerpath + *i );
        }
    }

    void TaskBrowser::find_peers( std::string::size_type startpos )
    {
        peerpath.clear();
        peer = context;
        taskobject = context;

        std::string to_parse = text.substr(startpos);
        startpos = 0;
        std::string::size_type endpos = 0;
        // Traverse the entered peer-list
        component.clear();
        peerpath.clear();
        while (endpos != std::string::npos )
            {
                bool itemfound = false;
                endpos = to_parse.find(".");
                if ( endpos == startpos ) {
                    component.clear();
                    break;
                }
                std::string item = to_parse.substr(startpos, endpos);

                if ( taskobject->getObject( item ) ) {
                    taskobject = peer->getObject(item);
                    itemfound = true;
                } else
                    if ( peer->hasPeer( item ) ) {
                        peer = peer->getPeer( item );
                        taskobject = peer;
                        itemfound = true;
                    }
                if ( itemfound ) { // if "." found and correct path
                    peerpath += to_parse.substr(startpos, endpos) + ".";
                    if ( endpos != std::string::npos )
                        to_parse = to_parse.substr(endpos + 1);
                    else
                        to_parse.clear();
                    startpos = 0;
                }
                else {
                    // no match: typo or member name
                    // store the (incompletely) typed text
                    component = item;
                    break;
                }
            }

        // now we got the peer and taskobject pointers,
        // the completed path in peerpath
        // the last partial path in component
//         cout << "text: '" << text <<"'"<<endl;
//         cout << "to_parse: '" << text <<"'"<<endl;
//         cout << "Peerpath: '" << peerpath <<"'"<<endl;
//         cout << "Component: '" << component <<"'"<<endl;

        TaskContext::PeerList v;
        if ( taskobject == peer ) {
            // add peer's completes:
            v = peer->getPeerList();
            for (TaskContext::PeerList::iterator i = v.begin(); i != v.end(); ++i) {
                if ( i->find( component ) == 0 ) { // only add if match
                    completes.push_back( peerpath + *i );
                    completes.push_back( peerpath + *i + "." );
                    //cerr << "added " << peerpath+*i+"."<<endl;
                }
            }
        }
        // add taskobject's completes:
        v = taskobject->getObjectList();
        for (TaskContext::PeerList::iterator i = v.begin(); i != v.end(); ++i) {
            if ( i->find( component ) == 0 ) { // only add if match
                completes.push_back( peerpath + *i );
                if ( *i != "this" ) // "this." confuses our parsing lateron
                    completes.push_back( peerpath + *i + "." );
                //cerr << "added " << peerpath+*i+"."<<endl;
            }
        }
        return;
    }

    char ** TaskBrowser::orocos_hmi_completion ( const char *text, int start, int end )
    {
        char **matches;
        matches = ( char ** ) 0;

        matches = rl_completion_matches ( text, &TaskBrowser::command_generator );

        return ( matches );
    }
#endif // USE_READLINE

    TaskBrowser::TaskBrowser( TaskContext* _c )
        : TaskContext("TaskBrowser"),
          command(0),
          debug(0),
          line_read(0),
          lastc(0), storedname(""), storedline(-1),
          macrorecording(false)
    {
        tb = this;
        context = tb;
        this->switchTaskContext(_c);
#ifdef USE_READLINE
        rl_completion_append_character = '\0'; // avoid adding spaces
        rl_attempted_completion_function = &TaskBrowser::orocos_hmi_completion;

        if ( read_history(".tb_history") != 0 ) {
            read_history("~/.tb_history");
        }
#endif

#ifdef _WIN32
        this->setColorTheme( nocolors );
#else
        this->setColorTheme( darkbg );
#endif
        this->enterTask();
    }

    TaskBrowser::~TaskBrowser() {
#ifdef USE_READLINE
        if (line_read)
            {
                free (line_read);
            }
        if ( write_history(".tb_history") != 0 ) {
            write_history("~/.tb_history");
        }
#endif
    }

    /**
     * Helper functions to display task and script states.
     */
    char getTaskStatusChar(TaskContext* t)
    {
        if (t->isRunning() )
            return 'R'; // Running
        if (t->isConfigured() )
            return 'S'; // Stopped
        return 'U';     // Unconfigured/Preoperational
    }

    char getStateMachineStatusChar(TaskContext* t, string progname)
    {
        string ps = t->scripting()->getStateMachineStatus(progname);
        return toupper(ps[0]);
    }

    char getProgramStatusChar(TaskContext* t, string progname)
    {
        string ps = t->scripting()->getProgramStatus(progname);
        return toupper(ps[0]);
    }

    void str_trim(string& str, char to_trim)
    {
        string::size_type pos1 = str.find_first_not_of(to_trim);
        string::size_type pos2 = str.find_last_not_of(to_trim);
        str = str.substr(pos1 == string::npos ? 0 : pos1,
                         pos2 == string::npos ? str.length() - 1 : pos2 - pos1 + 1);
    }


    /**
     * @brief Call this method from ORO_main() to
     * process keyboard input.
     */
    void TaskBrowser::loop()
    {
#ifdef _POSIX_VERSION
        // Intercept Ctrl-C
        ::signal( SIGINT, ctrl_c_catcher );
#ifdef USE_READLINE
        // Let readline intercept relevant signals
        if(rl_catch_signals == 0)
            cerr << "Error: not catching signals !"<<endl;
        if (rl_set_signals() != 0)
            cerr << "Error setting signals !" <<endl;
#endif
#endif
        cout << nl<<
            coloron <<
            "  This console reader allows you to browse and manipulate TaskContexts."<<nl<<
            "  You can type in a command, event, method, expression or change variables."<<nl;
        cout <<"  (type '"<<underline<<"help"<<coloroff<<coloron<<"' for instructions)"<<nl;
#ifdef USE_READLINE
        cout << "    TAB completion and HISTORY is available ('bash' like)" <<coloroff<<nl<<nl;
#else
        cout << "    TAB completion and history is NOT available (LGPL-version)" <<coloroff<<nl<<nl;
#endif
        while (1)
            {
                if (!macrorecording) {
                    if ( context == tb )
                        cout << green << " Watching " <<coloroff;
                    else
                        cout << red << " In " << coloroff;

                    char state = getTaskStatusChar(taskcontext);

                    cout << "Task "<<green<< taskcontext->getName() <<coloroff<< "["<< state <<"]. (Status of last Command : ";
                    if ( command == 0 )
                        cout << "none";
                    else if ( command->done() ) // disposed or done
                        cout <<green + "done";
                    else if ( command->valid() )
                        cout << blue+"busy";
                    else if ( command->executed() ) // if not valid, but executed: fail
                        cout << red+"fail";
                    else if ( command->accepted() )
                        cout << blue+"queued";
                    cout << coloroff << " )";
                    // This 'endl' is important because it flushes the whole output to screen of all
                    // processing that previously happened, which was using 'nl'.
                    cout << endl;

                    // print traces.
                    for (PTrace::iterator it = ptraces.begin(); it != ptraces.end(); ++it) {
                        TaskContext* progpeer = it->first.first;
                        int line = progpeer->scripting()->getProgramLine(it->first.second);
                        if ( line != it->second ) {
                            it->second = line;
                            printProgram( it->first.second, -1, progpeer );
                        }
                    }

                    for (PTrace::iterator it = straces.begin(); it != straces.end(); ++it) {
                        TaskContext* progpeer = it->first.first;
                        int line = progpeer->scripting()->getStateMachineLine(it->first.second);
                        if ( line != it->second ) {
                            it->second = line;
                            printProgram( it->first.second, -1, progpeer );
                        }
                    }
                }
                // Check port status:
                checkPorts();
#ifdef _POSIX_VERSION
                // Call readline wrapper :
                ::signal( SIGINT, ctrl_c_catcher ); // catch ctrl_c only when editting a line.
#endif
#ifdef USE_READLINE
                const char* const commandStr = rl_gets();
                // quit on EOF (Ctrl-D)
                std::string command( commandStr ? commandStr : "quit" ); // copy over to string
#else
                std::string command;
                cout << prompt;
                getline(cin,command);
                if (!cin) // Ctrl-D
                    command = "quit";
#endif
                str_trim( command, ' ');
#ifdef _POSIX_VERSION
                ::signal( SIGINT, SIG_DFL );        // do not catch ctrl_c
#endif
                cout << coloroff;
                if ( command == "quit" ) {
                    // Intercept no Ctrl-C
                    cout << endl;
                    return;
                } else if ( command == "help") {
                    printHelp();
                } else if ( command == "#debug") {
                    debug = !debug;
                } else if ( command.find("list ") == 0 || command == "list" ) {
                    browserAction(command);
                } else if ( command.find("trace ") == 0 || command == "trace" ) {
                    browserAction(command);
                } else if ( command.find("untrace ") == 0 || command == "untrace" ) {
                    browserAction(command);
                } else if ( command.find("ls") == 0 ) {
                    std::string::size_type pos = command.find("ls")+2;
                    command = std::string(command, pos, command.length());
                    printInfo( command );
                } else if ( command == "" ) { // nop
                } else if ( command.find("cd ..") == 0  ) {
                    this->switchBack( );
                } else if ( command.find("enter") == 0  ) {
                    this->enterTask();
                } else if ( command.find("leave") == 0  ) {
                    this->leaveTask();
                } else if ( command.find("cd ") == 0  ) {
                    std::string::size_type pos = command.find("cd")+2;
                    command = std::string(command, pos, command.length());
                    this->switchTaskContext( command );
                } else if ( command.find(".") == 0  ) {
                    command = std::string(command, 1, command.length());
                    this->browserAction( command );
                } else if ( macrorecording) {
                    macrotext += command +'\n';
                } else {
                    try {
                        this->evalCommand( command );
                    } catch(...){
                        cerr << "The command '"<<command<<"' caused an unknown exception and could not be completed."<<endl;
                    }
                    // a command was typed... clear storedline such that a next 'list'
                    // shows the 'IP' again.
                    storedline = -1;
                }
                cout <<endl;
            }
    }

    void TaskBrowser::enterTask()
    {
        if ( context == taskcontext ) {
            log(Info) <<"Already in Task "<< taskcontext->getName()<<endlog();
            return;
        }
        context = taskcontext;
        log(Info) <<"Entering Task "<< taskcontext->getName()<<endlog();
    }

    void TaskBrowser::leaveTask()
    {
        if ( context == tb ) {
            log(Info) <<"Already watching Task "<< taskcontext->getName()<<endlog();
            return;
        }
        context = tb;
        log(Info) <<"Watching Task "<< taskcontext->getName()<<endlog();
    }

    void TaskBrowser::recordMacro(std::string name)
    {
        if ( name.empty() ) {
            cerr << "Please specify a macro name." <<endl;
            return;
        } else {
            cout << "Recording macro "<< name <<endl;
            cout << "Use program scripting syntax (do, set,...) !" << endl <<endl;
            cout << "export function "<< name<<" {"<<endl;
        }
        macrorecording = true;
        macroname = name;
    }

    void TaskBrowser::cancelMacro() {
        cout << "Canceling macro "<< macroname <<endl;
        macrorecording = false;
    }

    void TaskBrowser::endMacro() {
        string fname = macroname + ".ops";
        macrorecording = false;
        cout << "}" <<endl;
        cout << "Saving file "<< fname <<endl;
        ofstream macrofile( fname.c_str() );
        macrofile << "/* TaskBrowser macro '"<<macroname<<"' */" <<endl<<endl;
        macrofile << "export function "<<macroname<<" {"<<endl;
        macrofile << macrotext.c_str();
        macrofile << "}"<<endl;

        cout << "Loading file "<< fname <<endl;
        ProgramLoader loader;
        if ( loader.loadProgram( fname, context ) ) {
            cout << "Done."<<endl;
        } else
            cout << "Failed."<<endl;
    }

    void TaskBrowser::switchBack()
    {
        if ( taskHistory.size() == 0)
            return;
        if ( !taskcontext->engine()->commands()->isProcessed( lastc ) ) {
            Logger::log()<<Logger::Warning
                         << "Previous command was not yet processed by previous Processor." <<Logger::nl
                         << " Can not track command status across tasks."<< endlog();
            // memleak command...
            command = 0;
        } else {
            delete command;
            command = 0;
        }

        this->switchTaskContext( taskHistory.front(), false ); // store==false
        lastc = 0;
        taskHistory.pop_front();
    }

    void TaskBrowser::checkPorts()
    {
        // check periodically if the taskcontext did not change its ports.

        DataFlowInterface::Ports ports;
        ports = this->ports()->getPorts();
        for( DataFlowInterface::Ports::iterator i=ports.begin(); i != ports.end(); ++i) {
            // If our port is no longer connected, try to reconnect.
            PortInterface* p = *i;
            PortInterface* tcp = taskcontext->ports()->getPort( p->getName() );
            if ( p->ready() == false || tcp == 0 || tcp->ready() == false) {
                this->ports()->removePort( p->getName() );
                delete p;
            }
        }
    }

    void TaskBrowser::setColorTheme(ColorTheme t)
    {
        // background color palettes:
        const char* dbg = "\033[01;";
        const char* wbg = "\033[02;";
        // colors in palettes:
        const char* r = "31m";
        const char* g = "32m";
        const char* b = "34m";
        const char* con = "31m";
        const char* coff = "\33[0m";
        const char* und  = "\33[4m";

        switch (t)
            {
            case nocolors:
                green.clear();
                red.clear();
                blue.clear();
                coloron.clear();
                coloroff.clear();
                underline.clear();
                return;
                break;
            case darkbg:
                green = dbg;
                red = dbg;
                blue = dbg;
                coloron = dbg;
				coloroff = wbg;
                break;
            case whitebg:
                green = wbg;
                red = wbg;
                blue = wbg;
                coloron = wbg;
				coloroff = wbg;
                break;
            }
        green += g;
        red += r;
        blue += b;
        coloron += con;
        coloroff = coff;
        underline = und;
    }

    void TaskBrowser::switchTaskContext(std::string& c) {
        // if nothing new found, return.
        peer = taskcontext;
        if ( this->findPeer( c + "." ) == 0 ) {
            cerr << "No such peer: "<< c <<nl;
            return;
        }

        if ( peer == taskcontext ) {
            cerr << "Already in "<< c <<nl;
            return;
        }

        if ( peer == tb ) {
            cerr << "Can not switch to TaskBrowser." <<nl;
            return;
        }

        // findPeer has set 'peer' :
        this->switchTaskContext( peer );
    }

    void TaskBrowser::switchTaskContext(TaskContext* tc, bool store) {
        // put current on the stack :
        if (taskHistory.size() == 20 )
            taskHistory.pop_back();
        if ( taskcontext && store)
            taskHistory.push_front( taskcontext );

        // We need to release the comms, since taskcontext is changing,
        // and we do not keep track of in which processor the comm was dropped.
        if ( taskcontext && !taskcontext->engine()->commands()->isProcessed( lastc ) ) {
            Logger::log()<<Logger::Warning
                         << "Previous command was not yet processed by previous Processor." <<Logger::nl
                         << " Can not track command status across tasks."<< endlog();
            // memleak it...
            command = 0;
        } else {
            delete command;
            command = 0;
        }

        // disconnect from current peers.
        this->disconnect();

        // cleanup port left-overs.
        DataFlowInterface::Ports ports = this->ports()->getPorts();
        for( DataFlowInterface::Ports::iterator i=ports.begin(); i != ports.end(); ++i) {
            this->ports()->removePort( (*i)->getName() );
            delete *i;
        }

        // now switch to new one :
        if ( context == taskcontext )
            context = tc;
        taskcontext = tc; // peer is the new taskcontext.
        lastc = 0;

        // connect peer.
        this->addPeer( taskcontext );

        cerr << "   Switched to : " << taskcontext->getName() <<endl;

    }

    TaskContext* TaskBrowser::findPeer(std::string c) {
        // returns the one but last peer, which is the one we want.
        std::string s( c );

        our_pos_iter_t parsebegin( s.begin(), s.end(), "teststring" );
        our_pos_iter_t parseend;

        CommonParser cp;
        PeerParser pp( peer, cp, true );
        bool &skipref = cp.skipeol;
        try {
            parse( parsebegin, parseend, pp.parser(), SKIP_PARSER );
        }
        catch( ... )
            {
                log(Debug) <<"No such peer : "<< c <<endlog();
                return 0;
            }
        taskobject = pp.taskObject();
        assert(taskobject);
        peer = pp.peer();
        return pp.peer();
    }

    void TaskBrowser::browserAction(std::string& act)
    {
        std::stringstream ss(act);
        std::string instr;
        ss >> instr;

        if ( instr == "list" ) {
            int line;
            ss >> line;
            if (ss) {
                this->printProgram(line);
                return;
            }
            ss.clear();
            string arg;
            ss >> arg;
            if (ss) {
                ss.clear();
                ss >> line;
                if (ss) {
                    // progname and line given
                    this->printProgram(arg, line);
                    return;
                }
                // only progname given.
                this->printProgram( arg );
                return;
            }
            // just 'list' :
            this->printProgram();
            return;
        }

        //
        // TRACING
        //
        if ( instr == "trace") {
            string arg;
            ss >> arg;
            if (ss) {
                bool pi = context->scripting()->hasProgram(arg);
                if (pi) {
                    ptraces[make_pair(context, arg)] = context->scripting()->getProgramLine(arg); // store current line number.
                    this->printProgram( arg );
                    return;
                }
                pi = context->scripting()->hasStateMachine(arg);
                if (pi) {
                    straces[make_pair(context, arg)] = context->scripting()->getStateMachineLine(arg); // store current line number.
                    this->printProgram( arg );
                    return;
                }
                cerr <<"No such program or state machine: "<< arg <<endl;
                return;
            }

            // just 'trace' :
            std::vector<std::string> names;
            names = context->scripting()->getPrograms();
            for (std::vector<std::string>::iterator it = names.begin(); it != names.end(); ++it) {
                bool pi = context->scripting()->hasProgram(arg);
                if (pi)
                    ptraces[make_pair(context, arg)] = context->scripting()->getProgramLine(arg); // store current line number.
            }

            names = context->scripting()->getStateMachines();
            for (std::vector<std::string>::iterator it = names.begin(); it != names.end(); ++it) {
                bool pi = context->scripting()->hasStateMachine(arg);
                if (pi)
                    straces[make_pair(context, arg)] = context->scripting()->getStateMachineLine(arg); // store current line number.
            }

            cerr << "Tracing all programs and state machines in "<< context->getName() << endl;
            return;
        }

        if ( instr == "untrace") {
            string arg;
            ss >> arg;
            if (ss) {
                ptraces.erase( make_pair(context, arg) );
                straces.erase( make_pair(context, arg) );
                cerr <<"Untracing "<< arg <<" of "<< context->getName()<<endl;
                return;
            }
            // just 'untrace' :
            std::vector<std::string> names;
            names = context->scripting()->getPrograms();
            for (std::vector<std::string>::iterator it = names.begin(); it != names.end(); ++it) {
                bool pi = context->scripting()->hasProgram(arg);
                if (pi)
                    ptraces.erase(make_pair(context, arg));
            }

            names = context->scripting()->getStateMachines();
            for (std::vector<std::string>::iterator it = names.begin(); it != names.end(); ++it) {
                bool pi = context->scripting()->hasStateMachine(arg);
                if (pi)
                    straces.erase(make_pair(context, arg));
            }

            cerr << "Untracing all programs and state machines of "<< context->getName() << endl;
            return;
        }

        std::string arg;
        ss >> arg;
        ProgramLoader loader;
        if ( instr == "loadProgram") {
            if ( loader.loadProgram( arg, context ) )
                cout << "Done."<<endl;
            else
                cout << "Failed."<<endl;
            return;
        }
        if ( instr == "unloadProgram") {
            if ( loader.unloadProgram( arg, context ) )
                cout << "Done."<<endl;
            else
                cout << "Failed."<<endl;
            return;
        }

        if ( instr == "loadStateMachine") {
            if ( loader.loadStateMachine( arg, context ) )
                cout << "Done."<<endl;
            else
                cout << "Failed."<<endl;
            return;
        }
        if ( instr == "unloadStateMachine") {
            if ( loader.unloadStateMachine( arg, context ) )
                cout << "Done."<<endl;
            else
                cout << "Failed."<<endl;
            return;
        }
        if ( instr == "dark") {
            this->setColorTheme(darkbg);
            cout << nl << "Setting Color Theme for "+green+"dark"+coloroff+" backgrounds."<<endl;
            return;
        }
        if ( instr == "light") {
            this->setColorTheme(whitebg);
            cout << nl << "Setting Color Theme for "+green+"light"+coloroff+" backgrounds."<<endl;
            return;
        }
        if ( instr == "nocolors") {
            this->setColorTheme(nocolors);
            cout <<nl << "Disabling all colors"<<endl;
            return;
        }
        if ( instr == "connect") {
            if (arg.empty() ) {
                cout <<nl << "TaskBrowser connects to all ports of "<<taskcontext->getName()<<endl;
                // create 'anti-ports' to allow port-level interaction with the peer.
                DataFlowInterface::Ports ports = taskcontext->ports()->getPorts();
                for( DataFlowInterface::Ports::iterator i=ports.begin(); i != ports.end(); ++i) {
                    if (this->ports()->getPort( (*i)->getName() ) == 0 )
                        this->ports()->addPort( (*i)->antiClone() );
                }
                RTT::connectPorts(this,taskcontext);
            }
            else {
                cout <<nl << "TaskBrowser connects to port '"<<arg <<"' of "<<taskcontext->getName()<<endl;
                // create 'anti-port' and connect.
                DataFlowInterface::Ports ports = taskcontext->ports()->getPorts();
                for( DataFlowInterface::Ports::iterator i=ports.begin(); i != ports.end(); ++i) {
                    if ( (*i)->getName() == arg && this->ports()->getPort( (*i)->getName() ) == 0 ) {
                        this->ports()->addPort( (*i)->antiClone() );
                        this->ports()->getPort( arg )->connectTo( *i ); // this should always succeed
                        assert( this->ports()->getPort( arg )->connected() );
                        return;
                    }
                }
            }
            return;
        }
        if ( instr == "record") {
            recordMacro( arg );
            return;
        }
        if ( instr == "cancel") {
            cancelMacro();
            return;
        }
        if ( instr == "end") {
            endMacro();
            return;
        }
        cerr << "Unknown Browser Action : "<< act <<endl;
        cerr << "See 'help' for valid syntax."<<endl;
    }

    void TaskBrowser::evaluate(std::string& comm) {
        this->evalCommand(comm);
    }

    void TaskBrowser::evalCommand(std::string& comm )
    {
        cout << "      Got :"<< comm <<nl;

        OperationInterface* ops = context->getObject( comm );
        if ( ops ) // only object name was typed
            {
                sresult << nl << "Printing Interface of '"<< coloron << ops->getName() <<coloroff <<"' :"<<nl<<nl;
                std::vector<std::string> methods = ops->commands()->getNames();
                std::for_each( methods.begin(), methods.end(), boost::bind(&TaskBrowser::printCommand, this, _1, ops) );
                methods = ops->methods()->getNames();
                std::for_each( methods.begin(), methods.end(), boost::bind(&TaskBrowser::printMethod, this, _1, ops) );
                if (comm == "this") {
                    methods = taskcontext->events()->getNames();
                    std::for_each( methods.begin(), methods.end(), boost::bind(&TaskBrowser::printEvent, this, _1, taskcontext->events()) );
                }
                cout << sresult.str();
                sresult.str("");

            }
        // Minor hack : also check if it was an attribute of current TC, for example,
        // if both the object and attribute with that name exist. the if
        // statement after this one would return and not give the expr parser
        // time to evaluate 'comm'.
        if ( context->attributes()->getValue( comm ) ) {
                this->printResult( context->attributes()->getValue( comm )->getDataSource().get(), true );
                cout << sresult.str();
                sresult.str("");
                return;
        }

        if ( ops ) {
            return;
        }

        Parser _parser;

        if (debug)
            cerr << "Trying ValueChange..."<<nl;
        try {
            // Check if it was a method or datasource :
            DataSourceBase::shared_ptr ds = _parser.parseValueChange( comm, context );
            // methods and DS'es are processed immediately.
            if ( ds.get() != 0 ) {
                this->printResult( ds.get(), false );
                cout << sresult.str() << nl;
                sresult.str("");
                return; // done here
            } else if (debug)
                cerr << "returned zero !"<<nl;
        } catch ( fatal_semantic_parse_exception& pe ) { // incorr args, ...
            // way to fatal,  must be reported immediately
            if (debug)
                cerr << "fatal_semantic_parse_exception: ";
            cerr << pe.what() <<nl;
            return;
        } catch ( syntactic_parse_exception& pe ) { // wrong content after = sign etc..
            // syntactic errors must be reported immediately
            if (debug)
                cerr << "syntactic_parse_exception: ";
            cerr << pe.what() <<nl;
            return;
        } catch ( parse_exception_parser_fail &pe )
            {
                // ignore, try next parser
                if (debug) {
                    cerr << "Ignoring ValueChange exception :"<<nl;
                    cerr << pe.what() <<nl;
                }
        } catch ( parse_exception& pe ) {
            // syntactic errors must be reported immediately
            if (debug)
                cerr << "parse_exception :";
            cerr << pe.what() <<nl;
            return;
        }
        if (debug)
            cerr << "Trying Expression..."<<nl;
        try {
            // Check if it was a method or datasource :
            DataSourceBase::shared_ptr ds = _parser.parseExpression( comm, context );
            // methods and DS'es are processed immediately.
            if ( ds.get() != 0 ) {
                this->printResult( ds.get(), true );
                cout << sresult.str() << nl;
                sresult.str("");
                return; // done here
            } else if (debug)
                cerr << "returned zero !"<<nl;
        } catch ( syntactic_parse_exception& pe ) { // missing brace etc
            // syntactic errors must be reported immediately
            if (debug)
                cerr << "syntactic_parse_exception :";
            cerr << pe.what() <<nl;
            return;
        } catch ( fatal_semantic_parse_exception& pe ) { // incorr args, ...
            // way to fatal,  must be reported immediately
            if (debug)
                cerr << "fatal_semantic_parse_exception :";
            cerr << pe.what() <<nl;
            return;
        } catch ( parse_exception_parser_fail &pe )
            {
                // ignore, try next parser
                if (debug) {
                    cerr << "Ignoring Expression exception :"<<nl;
                    cerr << pe.what() <<nl;
                }
        } catch ( parse_exception& pe ) {
                // ignore, try next parser
                if (debug) {
                    cerr << "Ignoring Expression parse_exception :"<<nl;
                    cerr << pe.what() <<nl;
                }
        }
        if (debug)
            cerr << "Trying Command..."<<nl;
        try {
            CommandInterface* com = _parser.parseCommand( comm, context, true ).first; // create a dispatch command.
            // check if it is a plain Action:
            if ( com && dynamic_cast<DispatchInterface*>(com) == 0 ) {
                string prompt =" = ";
                sresult <<prompt<< setw(20)<<left;
                sresult << com->execute() << right;
                cout << sresult.str();
                sresult.str("");
                delete com;
                return;
            }
            if ( command && !command->executed() ) {
                cerr << "Warning : previous command is not yet processed by Processor." <<nl;
            } else {
                delete command;
            }
            command = dynamic_cast<DispatchInterface*>(com);
        } catch ( parse_exception& pe ) {
            if (debug)
                cerr << "CommandParser parse_exception :"<<nl;
            cerr << pe.what() <<nl;
            return;
        } catch (...) {
            cerr << "Illegal Input."<<nl;
            return;
        }

        if ( command == 0 ) { // this should not be reached
            cerr << "Uncaught : Illegal command."<<nl;
            return;
        }

        command->readArguments();
        if ( command->dispatch() == false ) {
            cerr << "Command not accepted by "<<context->getName()<<"'s Processor !" << nl;
            delete command;
            command = 0;
            accepted = 0;
        }
    }

    void TaskBrowser::printResult( DataSourceBase* ds, bool recurse) {
        std::string prompt(" = ");
        // setup prompt :
        sresult <<prompt<< setw(20)<<left;
        if ( ds )
            doPrint( ds, recurse );
        else
            sresult << "(null)";
        sresult << right;
    }

    void TaskBrowser::doPrint( DataSourceBase* ds, bool recurse) {
        // this is needed for ds's that rely on initialision.
        // e.g. eval true once or time measurements.
        // becomes only really handy for 'watches' (todo).
        ds->reset();

        DataSource<PropertyBag>* dspbag = DataSource<PropertyBag>::narrow(ds);
        if (dspbag) {
            PropertyBag bag( dspbag->get() );
            if (!recurse) {
                int siz = bag.getProperties().size();
                int wdth = siz ? (20 - (siz / 10 + 1)) : 20;
                sresult <<setw(0)<< siz <<setw( wdth )<< " Properties";
            } else {
            if ( ! bag.empty() ) {
                sresult <<setw(0)<<nl;
                for( PropertyBag::iterator it= bag.getProperties().begin(); it!=bag.getProperties().end(); ++it) {
                    sresult <<setw(14)<<right<<(*it)->getType()<<" "<<coloron<<setw(14)<< (*it)->getName()<<coloroff;
                    DataSourceBase::shared_ptr propds = (*it)->getDataSource();
                    this->printResult( propds.get(), false );
                    sresult <<" ("<<(*it)->getDescription()<<')' << nl;
                }
            } else {
                sresult <<prompt<<"(empty PropertyBag)";
            }
            }
            return;
        }

        DataSourceBase::shared_ptr dsb(ds);
        dsb->evaluate();
        sresult << dsb;
    }

    struct comcol
    {
        const char* command;
        comcol(const char* c) :command(c) {}
        std::ostream& operator()( std::ostream& os ) const {
            os<<"'"<< TaskBrowser::coloron<< TaskBrowser::underline << command << TaskBrowser::coloroff<<"'";
            return os;
        }
    };

    struct keycol
    {
        const char* command;
        keycol(const char* c) :command(c) {}
        std::ostream& operator()( std::ostream& os )const {
            os<<"<"<< TaskBrowser::coloron<< TaskBrowser::underline << command << TaskBrowser::coloroff<<">";
            return os;
        }
    };

    struct titlecol
    {
        const char* command;
        titlecol(const char* c) :command(c) {}
        std::ostream& operator()( std::ostream& os ) const {
            os<<endl<<"["<< TaskBrowser::coloron<< TaskBrowser::underline << command << TaskBrowser::coloroff<<"]";
            return os;
        }
    };

    std::ostream& operator<<(std::ostream& os, comcol f ){
        return f(os);
    }

    std::ostream& operator<<(std::ostream& os, keycol f ){
        return f(os);
    }

    std::ostream& operator<<(std::ostream& os, titlecol f ){
        return f(os);
    }

    void TaskBrowser::printHelp()
    {
        cout << coloroff;
        cout <<titlecol("Task Browsing")<<nl;
        cout << "  To switch to another task, type "<<comcol("cd <path-to-taskname>")<<nl;
        cout << "  and type "<<comcol("cd ..")<<" to go back to the previous task (History size is 20)."<<nl;
        cout << "  Pressing "<<keycol("tab")<<" multiple times helps you to complete your command."<<nl;
        cout << "  It is not mandatory to switch to a task to interact with it, you can type the"<<nl;
        cout << "  peer-path to the task (dot-separated) and then type command or expression :"<<nl;
        cout << "     PeerTask.OtherTask.FinalTask.countTo(3) [enter] "<<nl;
        cout << "  Where 'countTo' is a method of 'FinalTask'."<<nl;
        cout << "  The TaskBrowser starts by default 'In' the current component. In order to watch"<<nl;
        cout << "  the TaskBrowser itself, type "<<comcol("leave")<<" You will notice that it"<<nl;
        cout << "  has connected to the data ports of the visited component. Use "<<comcol("enter")<<" to enter"<<nl;
        cout << "  the visited component again. The "<<comcol("cd")<<" command works transparantly in both"<<nl;
        cout << "  modi."<<nl;

        cout << "  "<<titlecol("Task Context Info")<<nl;
        cout << "  To see the contents of a task, type "<<comcol("ls")<<nl;
        cout << "  For a detailed argument list (and helpful info) of the object's methods, "<<nl;
        cout <<"   type the name of one of the listed task objects : " <<nl;
        cout <<"      this [enter]" <<nl<<nl;
        cout <<"  Command    : bool factor( int number )" <<nl;
        cout <<"   Factor a value into its primes." <<nl;
        cout <<"   number : The number to factor in primes." <<nl;
        cout <<"  Method     : bool isRunning( )" <<nl;
        cout <<"   Is this TaskContext started ?" <<nl;
        cout <<"  Method     : bool loadProgram( const& std::string Filename )" <<nl;
        cout <<"   Load an Orocos Program Script from a file." <<nl;
        cout <<"   Filename : An ops file." <<nl;
        cout <<"   ..."<<nl;

        cout <<titlecol("Expressions")<<nl;
        cout << "  You can evaluate any script expression by merely typing it :"<<nl;
        cout << "     1+1 [enter]" <<nl;
        cout << "   = 2" <<nl;
        cout << "  or inspect the status of a program :"<<nl;
        cout << "     programs.myProgram.isRunning [enter]" <<nl;
        cout << "   = false" <<nl;
        cout << "  and display the contents of complex data types (vector, array,...) :"<<nl;
        cout << "     array(6)" <<nl;
        cout << "   = {0, 0, 0, 0, 0, 0}" <<nl;

        cout <<titlecol("Changing Attributes and Properties")<<nl;
        cout << "  To change the value of a Task's attribute, type "<<comcol("varname = <newvalue>")<<nl;
        cout << "  If you provided a correct assignment, the browser will inform you of the success"<<nl;
        cout <<"   with '= true'." <<nl;

        cout <<titlecol("Commands")<<nl;
        cout << "  A Command is 'sent' to a task, which will process it in its own context (thread)."<<nl;
        cout << "  A command consists of an object, followed by a dot ('.'), the command "<<nl;
        cout << "  name, followed by the parameters. An example could be :"<<nl;
        cout << "     otherTask.bar.orderBeers(\"Palm\", 5) [enter] "<<nl;
        cout << "  The prompt will inform you about the status of the last command you entered. "<<nl;
        cout << "  It is allowed to enter a new command while the previous is still busy. "<<nl;

        cout <<titlecol("Methods")<<nl;
        cout << "  Methods 'look' the same as commands, but they are evaluated"<<nl;
        cout << "  immediately and print the result. An example could be :"<<nl;
        cout << "     someTask.bar.getNumberOfBeers(\"Palm\") [enter] "<<nl;
        cout << "   = 99" <<nl;

        cout <<titlecol("Events")<<nl;
        cout << "  Events behave as methods, they are emitted immediately."<<nl;
        cout << "  An example emitting an event :"<<nl;
        cout << "     someTask.notifyUserState(\"Drunk\") [enter] "<<nl;
        cout << "   = (void)" <<nl;

        cout <<titlecol("Program and StateMachine Scripts")<<nl;
        cout << "  To load a program script from local disc, type "<<comcol(".loadProgram <filename>")<<nl;
        cout << "  To load a state machine script from local disc, type "<<comcol(".loadStateMachine <filename>")<<nl;
        cout << "   ( notice the starting dot '.' )"<<nl;
        cout << "  Likewise, "<<comcol(".loadProgram <ProgramName>")<<" and "<<comcol(".unloadStateMachine <StateMachineName>")<<nl;
        cout << "   are available (notice it is the program's name, not the filename)."<<nl;
        cout << "  You can use "<<comcol("ls progname")<<nl;
        cout << "   to see the programs commands, methods and variables. You can manipulate each one of these,."<<nl;
        cout << "   as if the program is a Task itself (see all items above)."<<nl;

        cout << "  To print a program or state machine listing, use "<<comcol("list progname [linenumber]")<<nl;
        cout << "   to list the contents of the current program lines being executed,"<<nl;
        cout << "   or 10 lines before or after <linenumber>. When only "<<comcol("list [n]")<<nl;
        cout << "   is typed, 20 lines of the last listed program are printed from line <n> on "<<nl;
        cout << "   ( default : list next 20 lines after previous list )."<<nl;

        cout << "  To trace a program or state machine listing, use "<<comcol("trace [progname]")<<" this will"<<nl;
        cout << "   cause the TaskBrowser to list the contents of a traced program,"<<nl;
        cout << "   each time the line number of the traced program changes."<<nl;
        cout << "   Disable tracing with "<<comcol("untrace [progname]")<<""<<nl;
        cout << "   If no arguments are given to "<<comcol("trace")<<" and "<<comcol("untrace")<<", it applies to all programs."<<nl;

        cout << "   A status character shows which line is being executed."<<nl;
        cout << "   For programs : 'E':Error, 'S':Stopped, 'R':Running, 'P':Paused"<<nl;
        cout << "   For state machines : <the same as programs> + 'A':Active, 'I':Inactive"<<nl;

        cout <<titlecol("Changing Colors")<<nl;
        cout << "  You can inform the TaskBrowser of your background color by typing "<<comcol(".dark")<<nl;
        cout << "  "<<comcol(".light")<<", or "<<comcol(".nocolors")<<" to increase readability."<<nl;

        cout <<titlecol("Macro Recording / Command line history")<<nl;
        cout << "  You can browse the commandline history by using the up-arrow key or press "<<comcol("Ctrl r")<<nl;
        cout << "  and a search term. Hit enter to execute the current searched command."<<nl;
        cout << "  Macros can be recorded using the "<<comcol(".record 'macro-name'")<<" command."<<nl;
        cout << "  You can cancel the recording by typing "<<comcol(".cancel")<<" ."<<nl;
        cout << "  You can save and load the macro by typing "<<comcol(".end")<<" . The macro becomes"<<nl;
        cout << "  available as a command with name 'macro-name' in the current TaskContext." << nl;
        cout << "  While you enter the macro, it is not executed, as you must use scripting syntax which"<<nl;
        cout << "  may use loop or conditional statements, variables etc."<<nl;

        cout <<titlecol("Connecting Ports")<<nl;
        cout << "  You can instruct the TaskBrowser to connect to the ports of the current Peer by"<<nl;
        cout << "  typing "<<comcol(".connect [port-name]")<<", which will temporarily create connections"<<nl;
        cout << "  to all ports if [port-name] is omitted or to the specified port otherwise."<<nl;
        cout << "  The TaskBrowser disconnects these ports when it visits another component, but the"<<nl;
        cout << "  created connection objects remain in place (this is more or less a bug)!"<<nl;
    }

    void TaskBrowser::printProgram(const std::string& progname, int cl /*= -1*/, TaskContext* progpeer /* = 0 */) {
        string ps;
        char s;
        stringstream txtss;
        int ln;
        int start;
        int end;
        bool found(false);

        if (progpeer == 0 )
            progpeer = context;

        // if program exists, display.
        if ( progpeer->scripting()->hasProgram( progname ) ) {
            s = getProgramStatusChar(progpeer, progname);
            txtss.str( progpeer->scripting()->getProgramText(progname) );
            ln = progpeer->scripting()->getProgramLine(progname);
            if ( cl < 0 ) cl = ln;
            start = cl < 10 ? 1 : cl - 10;
            end   = cl + 10;
            this->listText( txtss, start, end, ln, s);
            found = true;
        }

        // If statemachine exists, display.
        if ( progpeer->scripting()->hasStateMachine( progname ) ) {
            s = getStateMachineStatusChar(progpeer, progname);
            txtss.str( progpeer->scripting()->getStateMachineText(progname) );
            ln = progpeer->scripting()->getStateMachineLine(progname);
            if ( cl < 0 ) cl = ln;
            start = cl <= 10 ? 1 : cl - 10;
            end   = cl + 10;
            this->listText( txtss, start, end, ln, s);
            found = true;
        }
        if ( !found ) {
            cerr << "Error : No such program or state machine found : "<<progname;
            cerr << " in "<< progpeer->getName() <<"."<<endl;
            return;
        }
        storedname = progname;
    }

    void TaskBrowser::printProgram(int cl /* = -1 */) {
        string ps;
        char s;
        stringstream txtss;
        int ln;
        int start;
        int end;
        bool found(false);
        if ( context->scripting()->hasProgram( storedname ) ) {
            s = getProgramStatusChar(context, storedname);
            txtss.str( context->scripting()->getProgramText(storedname) );
            ln = context->scripting()->getProgramLine(storedname);
            if ( cl < 0 ) cl = storedline;
            if (storedline < 0 ) cl = ln -10;
            start = cl;
            end   = cl + 20;
            this->listText( txtss, start, end, ln, s);
            found = true;
        }
        if ( context->scripting()->hasStateMachine(storedname) ) {
            s = getStateMachineStatusChar(context, storedname);
            txtss.str( context->scripting()->getStateMachineText(storedname) );
            ln = context->scripting()->getStateMachineLine(storedname);
            if ( cl < 0 ) cl = storedline;
            if (storedline < 0 ) cl = ln -10;
            start = cl;
            end   = cl+20;
            this->listText( txtss, start, end, ln, s);
            found = true;
        }
        if ( !found )
            cerr << "Error : No such program or state machine found : "<<storedname<<endl;
    }

    void TaskBrowser::listText(stringstream& txtss,int start, int end, int ln, char s) {
        int curln = 1;
        string line;
        while ( start > 1 && curln != start ) { // consume lines
            getline( txtss, line, '\n' );
            if ( ! txtss )
                break; // no more lines, break.
            ++curln;
        }
        while ( end > start && curln != end ) { // print lines
            getline( txtss, line, '\n' );
            if ( ! txtss )
                break; // no more lines, break.
            if ( curln == ln ) {
                cout << s<<'>';
            }
            else
                cout << "  ";
            cout<< setw(int(log(double(end)))) <<right << curln<< left;
            cout << ' ' << line <<endl;
            ++curln;
        }
        storedline = curln;
        // done !
    }

    void TaskBrowser::printInfo(const std::string& peerp)
    {
        // this sets this->peer to the peer given
        peer = context;
        if ( this->findPeer( peerp+"." ) == 0 ) {
            cerr << "No such peer or object: " << peerp << endl;
            return;
        }

        if ( !peer || !peer->ready()) {
            cout << nl << " Connection to peer "+peerp+" lost (peer->ready() == false)." <<endlog();
            return;
        }

        if ( peer == taskobject )
            sresult <<nl<<" Listing TaskContext "<< green << peer->getName()<<coloroff<< " :"<<nl;
        else
            sresult <<nl<<" Listing TaskObject "<< green << taskobject->getName()<<coloroff<< " :"<<nl;

        // Only print Properties for TaskContexts
        if ( peer == taskobject ) {
            sresult <<nl<<" Configuration Properties: ";
            PropertyBag* bag = peer->properties();
            if ( bag && bag->size() != 0 ) {
                // Print Properties:
                for( PropertyBag::iterator it = bag->begin(); it != bag->end(); ++it) {
                    DataSourceBase::shared_ptr pds = (*it)->getDataSource();
                    sresult << nl << setw(11)<< right << (*it)->getType()<< " "
                         << coloron <<setw(14)<<left<< (*it)->getName() << coloroff;
                    this->printResult( pds.get(), false ); // do not recurse
                    sresult<<" ("<< (*it)->getDescription() <<')';
                }
            } else {
                sresult << "(none)";
            }
            sresult <<nl;
        }

        // Print "this" interface (without detail) and then list objects...
        sresult <<nl<< " Execution Interface:";

        sresult <<nl<< "  Attributes   : ";
        std::vector<std::string> objlist = taskobject->attributes()->names();
        if ( !objlist.empty() ) {
            sresult << nl;
            // Print Attributes:
            for( std::vector<std::string>::iterator it = objlist.begin(); it != objlist.end(); ++it) {
                DataSourceBase::shared_ptr pds = taskobject->attributes()->getValue(*it)->getDataSource();
                sresult << setw(11)<< right << pds->getType()<< " "
                     << coloron <<setw( 14 )<<left<< *it << coloroff;
                this->printResult( pds.get(), false ); // do not recurse
                sresult <<nl;
            }
        } else {
            sresult << coloron << "(none)";
        }

        sresult <<coloroff<<nl<< "  Methods      : "<<coloron;
        objlist = taskobject->methods()->getNames();
        if ( !objlist.empty() ) {
            copy(objlist.begin(), objlist.end(), std::ostream_iterator<std::string>(sresult, " "));
        } else {
            sresult << "(none)";
        }
        sresult <<coloroff<<nl<< "  Commands     : "<<coloron;
        objlist = taskobject->commands()->getNames();
        if ( !objlist.empty() ) {
            copy(objlist.begin(), objlist.end(), std::ostream_iterator<std::string>(sresult, " "));
        } else {
            sresult << "(none)";
        }
        sresult <<coloroff<<nl<< "  Events       : "<<coloron;
        objlist = taskobject->events()->getEvents();
        if ( !objlist.empty() ) {
            copy(objlist.begin(), objlist.end(), std::ostream_iterator<std::string>(sresult, " "));
        } else {
            sresult << "(none)";
        }
        sresult << coloroff << nl;

        if ( peer == taskobject ) {
            sresult <<nl<< " Data Flow Ports: ";
            objlist = peer->ports()->getPortNames();
            if ( !objlist.empty() ) {
                for(vector<string>::iterator it = objlist.begin(); it != objlist.end(); ++it) {
                    PortInterface* port = peer->ports()->getPort(*it);
                    PortInterface::PortType pt = port->getPortType();
                    // Port type R/W
                    sresult << nl << " " << (pt == PortInterface::ReadPort ?
                        " R" : pt == PortInterface::WritePort ? " W" : "RW");
                    // Port data type + name
                    if ( !port->ready() || !port->connection() )
                        sresult << "(U) " << setw(11)<<right<< port->getTypeInfo()->getTypeName();
                    else
                        sresult << "(C) " << setw(11)<<right<< port->getTypeInfo()->getTypeName();
                    sresult << " "
                         << coloron <<setw( 14 )<<left<< *it << coloroff;
                    if ( port->ready() )
                        sresult << " = " <<port->connection()->getDataSource();
                    else {
                        ConnectionInterface::shared_ptr c = port->createConnection();
                        if ( c )
                            sresult << " = " << c->getDataSource();
                    }
                    // Port description
//                     if ( peer->getObject(*it) )
//                         sresult << " ( "<< taskobject->getObject(*it)->getDescription() << " ) ";
                }
            } else {
                sresult << "(none)";
            }
            sresult << coloroff << nl;
        }

        objlist = taskobject->getObjectList();
        sresult <<nl<< " Task Objects: "<<nl;
        if ( !objlist.empty() ) {
            for(vector<string>::iterator it = objlist.begin(); it != objlist.end(); ++it)
                sresult <<coloron<< "  " << setw(14) << *it <<coloroff<< " ( "<< taskobject->getObject(*it)->getDescription() << " ) "<<nl;
        } else {
            sresult <<coloron<< "(none)" <<coloroff <<nl;
        }

        // TaskContext specific:
        if ( peer == taskobject ) {

            objlist = peer->scripting()->getPrograms();
            if ( !objlist.empty() ) {
                sresult << " Programs     : "<<coloron;
                for(vector<string>::iterator it = objlist.begin(); it != objlist.end(); ++it)
                    sresult << *it << "["<<getProgramStatusChar(peer,*it)<<"] ";
                sresult << coloroff << nl;
            }

            objlist = peer->scripting()->getStateMachines();
            if ( !objlist.empty() ) {
                sresult << " StateMachines: "<<coloron;
                for(vector<string>::iterator it = objlist.begin(); it != objlist.end(); ++it)
                    sresult << *it << "["<<getStateMachineStatusChar(peer,*it)<<"] ";
                sresult << coloroff << nl;
            }

            // if we are in the TB, display the peers of our connected task:
            if ( context == tb )
                sresult <<nl<< " "<<peer->getName()<<" Peers : "<<coloron;
            else
                sresult << nl <<" Peers        : "<<coloron;

            objlist = peer->getPeerList();
            if ( !objlist.empty() )
                for(vector<string>::iterator it = objlist.begin(); it != objlist.end(); ++it) {
                    assert( peer->getPeer(*it) );
                    sresult << *it << "["<<getTaskStatusChar(peer->getPeer(*it))<<"] ";
	      }
            else
                sresult << "(none)";
        }
        sresult <<coloroff<<nl;
        cout << sresult.str();
        sresult.str("");
    }

    void TaskBrowser::printCommand( const std::string m, OperationInterface* ops )
    {
        std::vector<ArgumentDescription> args;
        args = ops->commands()->getArgumentList( m );
        sresult << "  Command    : bool " << coloron << m << coloroff<< "( ";
        for (std::vector<ArgumentDescription>::iterator it = args.begin(); it != args.end(); ++it) {
            sresult << it->type <<" ";
            sresult << coloron << it->name << coloroff;
            if ( it+1 != args.end() )
                sresult << ", ";
            else
                sresult << " ";
        }
        sresult << ")"<<nl;
        sresult << "   " << ops->commands()->getDescription( m )<<nl;
        for (std::vector<ArgumentDescription>::iterator it = args.begin(); it != args.end(); ++it)
            sresult <<"   "<< it->name <<" : " << it->description << nl;
    }

    void TaskBrowser::printMethod( const std::string m, OperationInterface* ops )
    {
        std::vector<ArgumentDescription> args;
        args = ops->methods()->getArgumentList( m );
        sresult << "  Method     : "<< ops->methods()->getResultType(m)<<" " << coloron << m << coloroff<< "( ";
        for (std::vector<ArgumentDescription>::iterator it = args.begin(); it != args.end(); ++it) {
            sresult << it->type <<" ";
            sresult << coloron << it->name << coloroff;
            if ( it+1 != args.end() )
                sresult << ", ";
            else
                sresult << " ";
        }
        sresult << ")"<<nl;
        sresult << "   " << ops->methods()->getDescription( m )<<nl;
        for (std::vector<ArgumentDescription>::iterator it = args.begin(); it != args.end(); ++it)
            sresult <<"   "<< it->name <<" : " << it->description << nl;
    }

    void TaskBrowser::printEvent( const std::string m, EventService* ops )
    {
        std::vector<ArgumentDescription> args;
        args = ops->getArgumentList( m );
        sresult << "  Event     : "<< ops->getResultType(m)<<" " << coloron << m << coloroff<< "( ";
        for (std::vector<ArgumentDescription>::iterator it = args.begin(); it != args.end(); ++it) {
            sresult << it->type <<" ";
            sresult << coloron << it->name << coloroff;
            if ( it+1 != args.end() )
                sresult << ", ";
            else
                sresult << " ";
        }
        sresult << ")"<<nl;
        sresult << "   " << ops->getDescription( m )<<nl;
        for (std::vector<ArgumentDescription>::iterator it = args.begin(); it != args.end(); ++it)
            sresult <<"   "<< it->name <<" : " << it->description << nl;
    }

}
