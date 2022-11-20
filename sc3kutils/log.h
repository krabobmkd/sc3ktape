#ifndef SC3KTAPELOG_H
#define SC3KTAPELOG_H

#include <string>
#include <ostream>
#include <streambuf>
#include <sstream>
#include <map>
#include <vector>

#ifdef USE_THREAD
#include <thread>
#include <mutex>
#endif

#define LOGE()  Log::global()(eError)
#define LOGW()  Log::global()(eWarning)
#define LOGI()  Log::global()(eInfo)
#define LOGT()  Log::global()(eTrace)
#define LOGD()  Log::global()(eDebug)


#define LOG(a)  StreamLog::global()(a)

#define ANSICOL_RED	"\033[1;31m"
#define ANSICOL_ORA	"\033[1;33m"
#define ANSICOL_LGREEN	"\033[1;92m"
#define ANSICOL_GREEN	"\033[1;32m"
#define ANSICOL_YELLOW	"\033[1;33m"

#define ANSICOL_BLUE	"\033[1;34m"
#define ANSICOL_MAGENTA	"\033[1;35m"
#define ANSICOL_CYAN	"\033[1;36m"
#define ANSICOL_BOLD	"\x1b[1;0m"
// return to default color:
#define ANSICOL_DEF	"\033[0;49m"


/**
* levels of log to be filtered
*  for ILogListener and ILogEmiter
*/
typedef enum {
    eError = 0,
    eWarning,
    eInfo,
    eTrace,
    eDebug,
    eTestSuite,
    eNumberOfLevels
} eLevels;
// keep these lists synchronized with enum:
#define AllLogLevels { eError,eWarning,eInfo,eTrace,eDebug }
#define AllLogLevelLetters { "error","warning","info","trace","debug" }

/**
* Allows external objects to be notified during build().
* instances to be passed to addListener() / removeListener()
*/
class ILogListener
{
public:
    /** force virtual destructor for everybody */
    virtual ~ILogListener() {};
    /**
    * to be implemented by any log or interface
    * log like:  mylog(el_info) << "woot" <<endl;
    * @return log stream to flow in.
    */
    virtual std::ostream &operator()(eLevels level=eInfo) = 0;
};

/**
* \class Log
* \brief Simple Log system
*/
class Log : public ILogListener
{
public:
    Log();
    virtual ~Log();

    /**
    * get unique static instance, for the current thread.
    * @return a stream you can use << into.
    */
    static Log &global();
    /**
    * log like:  mylog(eInfo) << "woot" <<endl;
    * @param level enum of level
    * @return log stream to flow in.
    */
    std::ostream &operator()(eLevels level=eInfo) override;

    static const unsigned int listenerFlags_SayHello = 1;
    static const unsigned int listenerFlags_AllowColors = 2;
    /**
    * Register to logs
    * Version that takes a list of enum.
    * @param listener stream that will receive notifications through notify()
    * @param levels vector of enum levels to listen. like {lev1,lev2}
    * @param sayhello write init message on this stream.
    */
    void addLogListener(std::ostream &listener,
        const std::vector<eLevels> &levels = AllLogLevels, unsigned int flags = listenerFlags_SayHello) ;

    /**
    * Unregister object that was previously registered with addLogListener().
    * @param listener object to remove
    */
    void removeLogListener(std::ostream &listener);

    /**
    * clear all streams. (not unregister)
    */
    void clear();

    inline void setWarningErrorAutoColor(bool doAutoColor) { m_bAutoColor = doAutoColor; }

protected:
    // internal use
    class ThreadData
    {
    public:
        std::stringstream m_strstream;
        bool m_linestart = true;
        bool m_hasColorCode = false;
    };
    // internal use
    class LevelBuffer : public std::streambuf
    {
    public:
        LevelBuffer();
        int overflow(int c);
        void addLogListener(std::ostream &listener, unsigned int flags);
        void removeLogListener(std::ostream &listener);
        void clear();

        /** objects to be notified with messages when changes, for this level */
        std::map<std::ostream *,unsigned int> m_listeners;
        std::ostream m_stream;
        Log *m_pmanager;
        // just have to know this:
        std::string m_levelLetter;
        std::string m_autocolor;
    protected:

#ifdef USE_THREAD
        std::map< std::thread::id, ThreadData>	m_strstreammap;
        std::mutex		m_bufferMapMutex;
        std::mutex		m_dispatchMutex;
#else
        ThreadData  m_tdata;
#endif
        void dispatchString(std::string);
        void dispatchColorStringParts(std::string);
    };
    LevelBuffer m_levelBuffers[eNumberOfLevels];

    bool	m_bAutoColor = false;

    #ifdef WIN32
        bool m_checkedEnv = false;
        bool m_outputSupportAnsi = false;
    #endif
};


#endif


