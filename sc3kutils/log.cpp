
#include "log.h"
#include <iomanip>
#ifdef WIN32
#include <Windows.h>
#endif
#include <cstdlib>
#include <iostream>

using namespace std;

Log::LevelBuffer::LevelBuffer()
    : std::streambuf()
    , m_stream(this)
    , m_pmanager(0L)
{
}

int Log::LevelBuffer::overflow(int c)
{
#ifdef USE_THREAD
    m_bufferMapMutex.lock();
    // create or get threadData for a given thread id:
    const thread::id ttid = this_thread::get_id();
    ThreadData& threadData = m_strstreammap[ttid];
    int nbthreads = (int)m_strstreammap.size();
    m_bufferMapMutex.unlock();
#else
     ThreadData& threadData = m_tdata;
#endif
    if (threadData.m_linestart) {
        threadData.m_linestart = false;
        threadData.m_hasColorCode = false;
        // feed start of line -> set in prefs ?

        if (m_pmanager->m_bAutoColor && m_autocolor.length() > 0) {
            threadData.m_strstream << m_autocolor; // don't color start of line
            threadData.m_hasColorCode = true;
        }
    }

    threadData.m_strstream.put((char)c);
    if (c == '\033') {
        threadData.m_hasColorCode = true;
    } else if (c == '\n') {
        if (m_pmanager->m_bAutoColor && m_autocolor.length() > 0) {
            threadData.m_strstream << ANSICOL_DEF; // if automatic color, must return to normal color at end.
        }

        string strline = threadData.m_strstream.str();
        threadData.m_strstream.str(""); // some way to restart string stream
        threadData.m_strstream.clear(); // clear state
        threadData.m_linestart = true;
#ifdef USE_THREAD
        m_dispatchMutex.lock();
#endif
        if (threadData.m_hasColorCode) {
            // special treatment if line has color code: split color change along line
            // treat std::cout as ansi case if possible.
            size_t ilastStart = 0;
            size_t isf = strline.find("\033[");
            while (isf != string::npos) {
                dispatchString(strline.substr(ilastStart, isf - ilastStart));
                size_t iendesc = strline.find("m", isf + 1);
                if (iendesc != string::npos) {
                    string scodes = strline.substr(isf, (iendesc + 1) - (isf));
                    dispatchColorStringParts(scodes);
                    ilastStart = iendesc + 1;
                } else { // mlformated ? shouldnt happen.
                    break;
                }
                isf = strline.find("\033[", ilastStart);
            }
            if (ilastStart != string::npos) {
                // end of line:
                dispatchString(strline.substr(ilastStart, strline.length() - ilastStart)); // bad string case, end line.
            }
        } else { // simple "no color in line" case.
            dispatchString(strline);
        }
#ifdef USE_THREAD
        m_dispatchMutex.unlock();
#endif
        // - flush incremental buffer per thread , because some os increments id and so m_bufferMapMutex may leak.
#ifdef USE_THREAD
        m_bufferMapMutex.lock();
        m_strstreammap.erase(ttid);
        m_bufferMapMutex.unlock();
#endif
    }
    return c;
}
void Log::LevelBuffer::dispatchString(std::string strline)
{
#ifdef USE_THREAD
    lock_guard<mutex> lg(m_bufferMapMutex);
#endif
    map<ostream*, unsigned int>::iterator it = m_listeners.begin();
    while (it != m_listeners.end()) {
        pair<ostream* const, unsigned int>& p = *it++;
        ostream* pstream = p.first;
        (*pstream) << strline;
        pstream->flush();
        pstream->clear(pstream->rdstate() & ~ios::failbit); // fail bit story.
    }
}
void Log::LevelBuffer::dispatchColorStringParts(std::string strline)
{
#ifdef USE_THREAD
    lock_guard<mutex> lg(m_bufferMapMutex);
#endif
    map<ostream*, unsigned int>::iterator it = m_listeners.begin();
    while (it != m_listeners.end()) {
        pair<ostream* const, unsigned int>& p = *it++;
        ostream* pstream = p.first;
        if (pstream == (&std::cout) /*&& ((p.second & listenerFlags_AllowColors) != 0)*/ ) {
#ifdef WIN32
            static CONSOLE_SCREEN_BUFFER_INFOEX cbie; //hold info
            cbie.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);
            if (!m_pmanager->m_checkedEnv) {
                m_pmanager->m_checkedEnv = true;
                const char* pe = getenv("TERM"); // if is used from unix like terminal.

                m_pmanager->m_outputSupportAnsi = (pe && pe[0] != 0) /*&& ((p.second & listenerFlags_AllowColors) != 0)*/;
                if (!m_pmanager->m_outputSupportAnsi) {
                    // keep DOS attribs at start of app...
                    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
                    GetConsoleScreenBufferInfoEx(hStdout, &cbie); //get info
                }
            }
            if (m_pmanager->m_outputSupportAnsi) {
                // on cygwin outputs or deepROMGui, can use Ansi color codes !
                (*pstream) << strline; // "\x033[x;ym"
            } else {
                // use windows things
                // color code should look like x;y
                int c1 = 0, c2 = 0;
                int done = sscanf_s(strline.c_str(), "\033[%d;%dm", &c1, &c2);
                if (done == 2) {
                    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
                    WORD flags = cbie.wAttributes & ~(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);

                    switch (c2) {
                    case 92:
                    case 32:
                        flags |= FOREGROUND_GREEN;
                        break;
                    case 31:
                        flags |= FOREGROUND_RED;
                        break;
                    case 33:
                        flags |= FOREGROUND_RED | FOREGROUND_GREEN;
                        break;
                    case 34:
                        flags |= FOREGROUND_BLUE;
                        break;
                    case 35:
                        flags |= FOREGROUND_RED | FOREGROUND_BLUE;
                        break;
                    case 36:
                        flags |= FOREGROUND_GREEN | FOREGROUND_BLUE;
                        break;
                    default:
                        // back to normal :
                        flags |= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
                        break;
                        break;
                    }
                    if (c1 == 1)
                        flags |= FOREGROUND_INTENSITY;

                    // FOREGROUND_INTENSITY
                    // FOREGROUND_RED
                    //sets the color
                   // if (p.second & listenerFlags_AllowColors) {
                        ::SetConsoleTextAttribute(hStdout, flags);
                   // }

                }
            }
#else
            (*pstream) << strline;
#endif
        } else {
            // do not display colors on log text file !!
        }
    }
}


void Log::LevelBuffer::addLogListener(std::ostream& listener, unsigned int flags)
{
#ifdef USE_THREAD
    lock_guard<mutex> lg(m_bufferMapMutex);
#endif
    m_listeners[&listener] = flags; // just to make it unique
}
void Log::LevelBuffer::removeLogListener(std::ostream& listener)
{
#ifdef USE_THREAD
    lock_guard<mutex> lg(m_bufferMapMutex);
#endif
    map<ostream*, unsigned int>::iterator fit = m_listeners.find(&listener); // map::[] operator creates if not found.
    if (fit != m_listeners.end()) {
        m_listeners.erase(fit);
    }
}

/** use a streamlog object per thread */
#ifdef USE_THREAD
map<thread::id, unique_ptr<Log>> m_StreamLogThreadMap;
mutex m_StreamLogThreadMap_mutex;
#endif
/**
* get a StreamLog instance per thread.
*/
Log& Log::global()
{
#ifdef USE_THREAD
    thread::id threadid = this_thread::get_id();
    lock_guard<mutex> lg(m_StreamLogThreadMap_mutex);
    map<thread::id, unique_ptr<Log>>::iterator fit = m_StreamLogThreadMap.find(threadid);
    if (fit == m_StreamLogThreadMap.end()) {
        // create if not done
        m_StreamLogThreadMap[threadid] = make_unique<Log>();
        return *(m_StreamLogThreadMap[threadid].get());
    } else {
        // re-use
        return *(fit->second.get());
    }
#else
    static Log log;
    return log;
#endif
}

Log::Log()
{

    vector<string> colorsperlevel = { ANSICOL_RED, ANSICOL_YELLOW, "", "", "", "" };
    vector<string> levelLetters = AllLogLevelLetters;
    if ((int)eNumberOfLevels > (int)colorsperlevel.size())
        colorsperlevel.resize((int)eNumberOfLevels);
    if ((int)eNumberOfLevels > (int)levelLetters.size())
        levelLetters.resize((int)eNumberOfLevels);

    for (int i = 0; i < eNumberOfLevels; i++) { // note: the definition
        m_levelBuffers[i].m_levelLetter = levelLetters[i];
        m_levelBuffers[i].m_autocolor = colorsperlevel[i];
        m_levelBuffers[i].m_pmanager = this;
    }
}

Log::~Log()
{
    clear();
}

/**
* to be implemented by any log or interface
* log like:  mylog(el_info) << "woot" <<endl;
* @return log stream to flow in.
*/
std::ostream& Log::operator()(eLevels level)
{
    return m_levelBuffers[level].m_stream;
}

/**
* Register to logs
* @param listener stream that will receive notifications through notify()
* @param levels vector of enum levels to listen. like {lev1,lev2}
* @param sayhello write init message on this stream.
*/
void Log::addLogListener(std::ostream& listener,
    const vector<eLevels>& levels, unsigned int flags)
{
    // good idea to set log header here... ->prefs ?
  /*  if ((flags & listenerFlags_SayHello) != 0) {
        vector<string> levelLetters = AllLogLevelLetters;
        listener << "Log initiated with levels: ";
        auto it = levels.begin();
        while (it != levels.end()) {
            eLevels el = *it++;
            listener << levelLetters[el] << " ";
        }
        listener << endl;
        listener << "\n\n";
        listener.flush();
    }*/

    // remove everywhere if already set:
    for (int i = 0; i < eNumberOfLevels; i++) {
        LevelBuffer& lb = m_levelBuffers[i];
        lb.removeLogListener(listener);
    }

    auto it = levels.begin();
    while (it != levels.end()) {
        eLevels el = *it++;
        m_levelBuffers[el].addLogListener(listener, flags);
    }
}


/**
* Unregister object that was previously registered with addLogListener().
* @param pListener object to remove
*/
void Log::removeLogListener(std::ostream& listener)
{ // from everywhere

    for (int i = 0; i < eNumberOfLevels; i++) {
        m_levelBuffers[i].removeLogListener(listener);
    }
}

/**
* Unregister all
*/
void Log::clear()
{
    for (int i = 0; i < eNumberOfLevels; i++) {
        m_levelBuffers[i].clear();
    }
}
void Log::LevelBuffer::clear()
{
#ifdef USE_THREAD
    m_bufferMapMutex.lock();
    m_strstreammap.clear();
#endif
    m_listeners.clear();
#ifdef USE_THREAD
    m_bufferMapMutex.unlock();
#endif
}

