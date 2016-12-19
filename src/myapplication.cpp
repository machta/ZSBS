#include "myapplication.h"

#include "options.h"
#include "error.h"
#include <AlenkaSignal/openclcontext.h>

#include <QSurfaceFormat>
#include <QMessageBox>
#include <clFFT.h>

#include <stdexcept>
#include <string>

using namespace std;

unique_ptr<AlenkaSignal::OpenCLContext> globalContext(nullptr);

MyApplication::MyApplication(int& argc, char** argv) : QApplication(argc, argv)
{
	try
	{
		// Set up the global options object.
		options = new Options(argc, argv);
		PROGRAM_OPTIONS_POINTER = options;

		// Set up the log.
		const int maxLogFileNameLength = 1000;
		char logFileName[maxLogFileNameLength + 1];
		time_t now = time(nullptr);
		size_t len = strftime(logFileName, maxLogFileNameLength, PROGRAM_OPTIONS["logFileName"].as<string>().c_str(), localtime(&now));
		logFileName[len] = 0;

		LOG_FILE.open(logFileName);
		checkErrorCode(LOG_FILE.good(), true, "Could not open log file '" + string(logFileName) + "' for writing.");
	}
	catch (std::exception& e)
	{
		QMessageBox::critical(nullptr, "Exception caught", QString::fromStdString(e.what()));
		abort();
	}
	catch (...)
	{
		QMessageBox::critical(nullptr, "Exception caught", "Unknown exception caught.");
		abort();
	}

	// Log the command line parameters and config file.
	{
		stringstream ss;
		ss << "Starting with command: ";

		for (int i = 0; i < argc; ++i)
		{
			if (i != 0)
			{
				ss << " ";
			}
			ss << argv[i];
		}

		logToFile(ss.str());
	}

	PROGRAM_OPTIONS.logConfigFile();

	// Initialize the global OpenCL context.
	globalContext.reset(new AlenkaSignal::OpenCLContext(PROGRAM_OPTIONS["clPlatform"].as<int>(), PROGRAM_OPTIONS["clDevice"].as<int>()));

	// Set up the clFFT library.
	clfftStatus errFFT;
	clfftSetupData setupData;

	errFFT = clfftInitSetupData(&setupData);
	checkClfftErrorCode(errFFT, "clfftInitSetupData()");

	errFFT = clfftSetup(&setupData);
	checkClfftErrorCode(errFFT, "clfftSetup()");

	// Set some OpenGL context details.
	QSurfaceFormat format = QSurfaceFormat::defaultFormat();

#if defined GL_2_0
	format.setVersion(2, 0);
	format.setProfile(QSurfaceFormat::NoProfile);
#elif defined GL_3_0
	format.setVersion(3, 0);
	format.setProfile(QSurfaceFormat::NoProfile);
#elif defined GL_3_2
	format.setVersion(3, 2);
	format.setProfile(QSurfaceFormat::CoreProfile);
#endif

#ifndef NDEBUG
	format.setOption(QSurfaceFormat::DebugContext);
#endif

	QSurfaceFormat::setDefaultFormat(format);

	// Process some of the command-line only options.
	stringstream ss;

	ss << globalContext->getPlatformInfo() << endl << endl;
	ss << globalContext->getDeviceInfo() << endl;

	logToFile(ss.str());

	if (PROGRAM_OPTIONS.isSet("help"))
	{
		cout << PROGRAM_OPTIONS.getDescription() << endl;

		std::exit(EXIT_SUCCESS);
	}
	else if (PROGRAM_OPTIONS.isSet("clInfo"))
	{
		cout << ss.str();
		std::exit(EXIT_SUCCESS);
	}

	// Set locale.
	QLocale locale(PROGRAM_OPTIONS["locale"].as<string>().c_str());
	QLocale::setDefault(locale);
}

MyApplication::~MyApplication()
{
	clfftStatus errFFT = clfftTeardown();
	checkClfftErrorCode(errFFT, "clfftTeardown()");

	delete options;
}

bool MyApplication::notify(QObject* receiver, QEvent* event)
{
	try
	{
		return QApplication::notify(receiver, event);
	}
	catch (std::exception& e)
	{
		logToFileAndConsole("Exception caught: " << e.what());
	}
	catch (...)
	{
		logToFileAndConsole("Unknown exception caught.");
	}

	return false;
}