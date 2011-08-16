/*
 * Logger.cpp
 *
 * Do not instantiate or inherit from this class.
 *
 * An asynchronous logger. Call by using 'Logger::info("msg");' or equivalent method.
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

#include "Logger.h"



/* declare static variables */

boost::mutex Logger::log_writer_mtx;
boost::mutex Logger::formatter_mtx;
boost::mutex Logger::shutdown_mtx;
boost::condition_variable Logger::shutdown_cvar;

std::queue<pair<string, string> > Logger::write_requests;
std::queue<pair<pair<string, int> , pair<string, string> > > Logger::raw_requests;

bool Logger::initialized = false;
bool Logger::enabled = true;

string Logger::pid(boost::lexical_cast<string>(PID));

boost::thread Logger::frm_t;
boost::thread Logger::lw_t;

boost::condition_variable Logger::formatter_cvar;
boost::condition_variable Logger::log_writer_cvar;

/* end of static variables */



/* Initialize the background logging thread. Should only be called once. */
void Logger::initialize()
{
	initialized = true;

    frm_t = boost::thread(&Logger::formatter_run);
    lw_t = boost::thread(&Logger::log_writer_run);
}

/* Helper for building the directory path. Gets the current date (local time) in YYYY-MM-DD format. */
string Logger::get_date()
{
	boost::gregorian::date d(boost::gregorian::day_clock::local_day());
	return boost::gregorian::to_iso_extended_string(d);
}

/* Helper for building the directory path. Gets the current timestamp (local time) in HH:MM:SS format */
string Logger::get_timestamp()
{
	boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
	return boost::posix_time::to_simple_string(now);
}

/* Helper for building the directory path. Gets the base log path. */
string Logger::get_log_base_path()
{
#if defined (__UNIX__) || defined (__OSX__)

	string path(getenv("HOME"));

#else

#define PROFILE_BUF_LEN 250
	char profilepath[PROFILE_BUF_LEN];
	ExpandEnvironmentStringsA("%USERPROFILE%", profilepath, PROFILE_BUF_LEN);
	string path(profilepath);

#endif
	path.append("/sockit");
	return path;
}

/* Called on plugin shutdown, not when just one object is shutting down. Should not be called at any other time. */
void Logger::shutdown()
{
    // disable any more log requests and disable the background threads from sleeping
    enabled = false;

    if(initialized)
    {
        // wait until the log writing queue is done
        boost::unique_lock<boost::mutex> lock(shutdown_mtx);
        shutdown_cvar.wait(lock);
    }
}

/* Add a message to the log writing queue */
void Logger::queue(string host, int port, string msg, string cat)
{
	pair<string, int> item1(host, port);
	pair<string, string> item2(msg, cat);
	pair<pair<string, int> , pair<string, string> > item(item1, item2);

	formatter_mtx.lock();

	if (!Logger::initialized)
		Logger::initialize();

	raw_requests.push(item);
    formatter_cvar.notify_one();

	formatter_mtx.unlock();
}

/* Only the formatter thread should be here. Run until logging is disabled and handle formatting requests. */
void Logger::formatter_run()
{
    // needed for sleeping on a boost cvar
    boost::unique_lock<boost::mutex> fmt_lock(formatter_mtx);

    // continue as long as logging is enabled and there are still raw requests to process
	while(true)
	{
		while (!raw_requests.empty())
		{
            // handle one request
			pair<pair<string, int> , pair<string, string> > p = raw_requests.front();
			raw_requests.pop();
            fmt_lock.unlock();
			handle_format_request(p.first.first, p.first.second, p.second.first, p.second.second);
            fmt_lock.lock();
		}
       
	    if(enabled || !raw_requests.empty()) 
        {
            // Sigh. There's a bug with firebreath + boost::this_thread::sleep/boost::condition_variable.timed_wait
            // where the sleeping thread hangs forever. So this is a compromise/work around.

            // don't hold the lock across the gettime system call
            fmt_lock.unlock();
            boost::system_time sleep_time = boost::get_system_time() + boost::posix_time::milliseconds(SLEEP_TIME_MS);
            fmt_lock.lock();

            // wake up periodically to check for work.
            formatter_cvar.timed_wait(fmt_lock, sleep_time);
        }
        else
        {
            fmt_lock.unlock();
            break;
        }
	}
}

/* Only the formatter thread should be here. Handle a format request. */
void Logger::handle_format_request(string host, int port, string msg, string cat)
{
	// used to build the log path
	string host_str = (host == "local" ? "localhost" : host);
	string port_str = (port == NO_PORT ? "" : boost::lexical_cast<string>(port));

	string day = get_date();
	string time = get_timestamp();

	// figure out the path for this message
	string log_path(get_log_base_path() + "/" + day + "/" + host_str + "/" + pid);
	string entry(cat + "[" + time + "] " + "[" + host_str + (port == NO_PORT ? "] " : "] [" + port_str + "] ") + msg);

	// log request item
	pair<string, string> item(log_path, entry);

	// add item to log write queue
	log_writer_mtx.lock();
	write_requests.push(item);

    // awaken log writing thread - it has work
	log_writer_mtx.unlock();
    log_writer_cvar.notify_one();
}

/* Only the log writing thread should be in this function. Run until logging is disabled and handle log writing
 * requests. */
void Logger::log_writer_run()
{
    // Needed to sleep on a boost cvar
    boost::unique_lock<boost::mutex> lw_lock(log_writer_mtx);

    // continue as long as logging is enabled and there are either
    // raw requests to process or log writing requests to process
	while(true)
	{
		while (!write_requests.empty())
		{
            // handle one request
			pair<string, string> p = write_requests.front();
			write_requests.pop();
			log_writer_mtx.unlock();
			handle_write_request(p.first, p.second);
            log_writer_mtx.lock();
		}

        if(enabled || !raw_requests.empty() || !write_requests.empty())
        {
            // don't hold the lock across a system call
            lw_lock.unlock();
            boost::system_time sleep_time = boost::get_system_time() + boost::posix_time::milliseconds(SLEEP_TIME_MS);
            lw_lock.lock();

            // wait until there's work
            log_writer_cvar.timed_wait(lw_lock, sleep_time);
        }
        else
        {
            lw_lock.unlock();
            break;
        }
	}

    // notify the main thread to wake up
    shutdown_cvar.notify_all();
}

/* Only the log writing thread should be in this function (or the main thread, on shutdown). This handles a log write request. */
void Logger::handle_write_request(string dir, string line)
{
    namespace fs = boost::filesystem;

	if (!fs::exists(dir))
	{
		try
		{
			fs::create_directories(dir);
		} catch (fs::filesystem_error &err)
		{
			// hope someone is listening..
			std::cout << "Error: could not create directory " << dir << std::endl;
			return;
		}
	}

	string fileName(dir + "/sockit-traffic.log");
    line.append("\n");

    FILE *fp = fopen(fileName.c_str(), "a");

    if(fp)
    {
        fwrite(line.c_str(), line.size(), 1,  fp);
        fclose(fp);
    }
    else
    {
        std::cout << "Error: could not open file " << fileName << std::endl;
    }
}
