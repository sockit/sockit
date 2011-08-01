#ifndef LOGGER_H_
#define LOGGER_H_

#if defined (__UNIX__) || defined (__OSX__)

#include <unistd.h>
#include <sys/types.h>
#define PID getpid()

#else
#define WIN32_LEAN_AND_MEAN
#include "process.h"
#include <winsock2.h>
#include <windows.h>
#include <conio.h>
#define PID _getpid()

#endif

#include <stdio.h>

#include <queue>
#include <string>

#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>

// apparently std::queue doesn't quite work.
// don't try it.
using std::pair;
using std::string;

/**
 * Logger.h
 *
 * Do not instantiate or inherit from this class.
 *
 * An asynchronous logger. Call by using <code>Logger::info("msg");</code> or equivalent method.
 * Designed to have the main thread do as little work as possible.
 *
 * Internally, there are two background threads. One formats a log request into a file directory
 * and a log entry. The other actually writes this into the log. Hopefully, one thread can be in a
 * system call (to get the current date or time) and the other can be doing I/O.
 *
 * Sadly, a bug in firebreath+boost prevents both background threads from sleeping
 * and just periodically checking that there's work. The bug makes the sleeping thread
 * hang indefinitely (until interrupted). Since the bug affects waiting/timed waiting 
 * on a condition variable, (which is how boost implements sleep), the main thread has 
 * to signal the formatter thread just in case the bug happened. The formatter thread
 * then signals the log writing thread when there's work.
 *
 * Logs are placed in /sockit/YYYY-MM-DD/REMOTEHOST/PROCESSID/sockit-traffic.log.
 *
 * Note: Logger::shutdown() should only be called once, when the plugin is shutting down.
 * After this is called, no more logging will take place.
 */
class Logger
{
	public:

		/** 
         * Log that there is an error, along with a message.
		 *
		 * @param host      Either a client's remote host or the localhost for servers
		 * @param port      Either the remote host's port (for a client) or the server's local port
		 * @param msg       The message to log
		 */
		static inline void error(string msg, int port = NO_PORT, string host = "local")
		{
			if (Logger::is_enabled())
				Logger::queue(host, port, msg, "ERROR :  ");
		}

		/**
         * Write low-priority information to the log.
		 *
		 * @param host      Either a client's remote host or the localhost for servers
		 * @param port      Either the remote host's port (for a client) or the server's local port
		 * @param msg       The message to log
		 */
		static inline void info(string msg, int port = NO_PORT, string host = "local")
		{
			if (Logger::is_enabled())
				Logger::queue(host, port, msg, "INFO  :  ");
		}

		/**
         * Log that there is an warning, along with a message.
		 *
		 * @param host      Either a client's remote host or the localhost for servers
		 * @param port      Either the remote host's port (for a client) or the server's local port
		 * @param msg       The message to log
		 */
		static inline void warn(string msg, int port = NO_PORT, string host = "local")
		{
			if (Logger::is_enabled())
				Logger::queue(host, port, msg, "WARN  :  ");
		}

        /**
         * Returns true if logging is enabled (default). Returns false if the shutdown
         * method has been called (and logging has been disabled).
         */
        static const bool is_enabled()
        {
            return Logger::enabled;
        }

        /** Dangerous. Disables logging and forces the remaining logging requests to finish. Should only be called
         * when the plugin is shutting down. */
		static void shutdown();

		/** no port is given */
		static const int NO_PORT = 0;

	private:

        /* Disallow instantiating. */
		Logger()
		{
		}

        /** The formatter thread polls for work; this sets its sleep time. */
		static const int SLEEP_TIME_MS = 50;

		// add a log write request to the queue
		static void queue(string host, int port, string msg, string cat);

        /** Helper for building the directory path. Gets the current date (local time) in YYYY-MM-DD format. */
		static string get_date();

        /** Helper for building the directory path. Gets the current timestamp (local time) in HH:MM:SS format */
		static string get_timestamp();

        /** Helper for building the directory path. Gets the base log path. */
		static string get_log_base_path();

        /** Initialize the background logging thread. Should only be called once. */
		static void initialize();

        /** Only the log writing thread should be in this function. Run until logging is disabled and handle log writing requests. */
		static void log_writer_run();

        /** Only the log writing thread should be in this function. Handles a log write request. */
		static void handle_write_request(string dir, string line);

        /** Only the formatter thread should be here. Run until logging is disabled and handle formatting requests. */
		static void formatter_run();

        /** Only the formatter thread should be here. Handles a format request. */
		static void handle_format_request(string host, int port, string msg, string cat);

        /** Initialized? */
		static bool initialized;

        /** Enabled? */
		static bool enabled;

        /** Mutex for the log writer queue */
		static boost::mutex log_writer_mtx;

        /** Mutex for the formatter queue */
		static boost::mutex formatter_mtx;

        /** Mutex for the shutdown cvar */
		static boost::mutex shutdown_mtx;

        /** Cvar for shutting down (main thread uses this) */
		static boost::condition_variable shutdown_cvar;

        /** The formatter thread sleeps on this. */
        static boost::condition_variable formatter_cvar;

        /** The log writer thread sleeps on this. */
        static boost::condition_variable log_writer_cvar;

        /** Queue for the raw requests */
		static std::queue<pair<pair<string, int> , pair<string, string> > > raw_requests;

        /** Queue for the write requests */
		static std::queue<pair<string, string> > write_requests;

        /** The logger's pid */
		static string pid;

        /** The formatter thread */
        static boost::thread frm_t;

        /** The log-writer thread */
        static boost::thread lw_t;
};

#endif /* LOGGER_H_ */
