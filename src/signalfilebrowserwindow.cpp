#include "signalfilebrowserwindow.h"

#include "DataModel/opendatafile.h"
#include "options.h"
#include "error.h"
#include "signalviewer.h"
#include "spikedetanalysis.h"
#include "myapplication.h"
#include "spikedetsettingsdialog.h"
#include "canvas.h"
#include "Manager/trackmanager.h"
#include "Manager/eventmanager.h"
#include "Manager/eventtypemanager.h"
#include "Manager/montagemanager.h"
#include "Manager/eventtypetablemodel.h"
#include "Manager/montagetablemodel.h"
#include "Manager/eventtablemodel.h"
#include "Manager/tracktablemodel.h"
#include "Sync/syncserver.h"
#include "Sync/syncclient.h"
#include "Sync/syncdialog.h"
#include "DataModel/vitnessdatamodel.h"
#include <AlenkaSignal/openclcontext.h>
#include <AlenkaSignal/montage.h>
#include <AlenkaFile/gdf2.h>
#include <AlenkaFile/edf.h>

#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QDockWidget>
#include <QStatusBar>
#include <QLayout>
#include <QInputDialog>
#include <QLocale>
#include <QProgressDialog>
#include <QDateTime>

#include <locale>

using namespace std;
using namespace AlenkaFile;

namespace
{

const double horizontalZoomFactor = 1.3;
const double verticalZoomFactor = 1.3;

const char* title = "Signal File Browser";

double byteArray2Double(const char* data)
{
	return *reinterpret_cast<const double*>(data);
}

void unpackMessage(const QByteArray& message, double* timePossition)
{
	assert(message.size() == (int)sizeof(double));
	*timePossition = byteArray2Double(message.data());
}

QByteArray packMessage(double timePossition)
{
	return QByteArray(reinterpret_cast<char*>(&timePossition), sizeof(double));
}

void loadSpikedetOptions(AlenkaSignal::DETECTOR_SETTINGS* settings, double* eventDuration)
{
	if (PROGRAM_OPTIONS.isSet("fl"))
		settings->m_band_low = PROGRAM_OPTIONS["fl"].as<int>();

	if (PROGRAM_OPTIONS.isSet("fh"))
		settings->m_band_high = PROGRAM_OPTIONS["fh"].as<int>();

	if (PROGRAM_OPTIONS.isSet("k1"))
		settings->m_k1 = PROGRAM_OPTIONS["k1"].as<double>();

	if (PROGRAM_OPTIONS.isSet("k2"))
		settings->m_k2 = PROGRAM_OPTIONS["k2"].as<double>();

	if (PROGRAM_OPTIONS.isSet("k3"))
		settings->m_k3 = PROGRAM_OPTIONS["k3"].as<double>();

	if (PROGRAM_OPTIONS.isSet("w"))
		settings->m_winsize = PROGRAM_OPTIONS["w"].as<int>();

	if (PROGRAM_OPTIONS.isSet("n"))
		settings->m_noverlap = PROGRAM_OPTIONS["n"].as<double>();

	if (PROGRAM_OPTIONS.isSet("buf"))
		settings->m_buffering = PROGRAM_OPTIONS["buf"].as<int>();

	if (PROGRAM_OPTIONS.isSet("h"))
		settings->m_main_hum_freq = PROGRAM_OPTIONS["h"].as<int>();

	if (PROGRAM_OPTIONS.isSet("dt"))
		settings->m_discharge_tol = PROGRAM_OPTIONS["dt"].as<double>();

	if (PROGRAM_OPTIONS.isSet("pt"))
		settings->m_polyspike_union_time = PROGRAM_OPTIONS["pt"].as<double>();

	if (PROGRAM_OPTIONS.isSet("dec"))
		settings->m_decimation = PROGRAM_OPTIONS["dec"].as<int>();

	if (PROGRAM_OPTIONS.isSet("sed"))
		*eventDuration = PROGRAM_OPTIONS["sed"].as<double>();
}

void executeWithCLocale(function<void ()> code)
{
	std::locale localeCopy;
	std::locale::global(std::locale("C"));

	code();

	std::locale::global(localeCopy);
}

} // namespace

SignalFileBrowserWindow::SignalFileBrowserWindow(QWidget* parent) : QMainWindow(parent)
{
	setWindowTitle(title);

	signalViewer = new SignalViewer(this);
	setCentralWidget(signalViewer);

	// Construct dock widgets.
	setDockNestingEnabled(true);

	QDockWidget* trackManagerDockWidget = new QDockWidget("Track Manager", this);
	trackManagerDockWidget->setObjectName("Track Manager QDockWidget");
	trackManager = new TrackManager(this);
	trackManagerDockWidget->setWidget(trackManager);

	QDockWidget* eventManagerDockWidget = new QDockWidget("Event Manager", this);
	eventManagerDockWidget->setObjectName("Event Manager QDockWidget");
	eventManager = new EventManager(this);
	eventManager->setReferences(signalViewer->getCanvas());
	eventManagerDockWidget->setWidget(eventManager);

	QDockWidget* eventTypeDockWidget = new QDockWidget("EventType Manager", this);
	eventTypeDockWidget->setObjectName("EventType Manager QDockWidget");
	eventTypeManager = new EventTypeManager(this);
	eventTypeDockWidget->setWidget(eventTypeManager);

	QDockWidget* montageManagerDockWidget = new QDockWidget("Montage Manager", this);
	montageManagerDockWidget->setObjectName("Montage Manager QDockWidget");
	montageManager = new MontageManager(this);
	montageManagerDockWidget->setWidget(montageManager);

	addDockWidget(Qt::RightDockWidgetArea, trackManagerDockWidget);
	tabifyDockWidget(trackManagerDockWidget, eventManagerDockWidget);
	tabifyDockWidget(eventManagerDockWidget, eventTypeDockWidget);
	tabifyDockWidget(eventTypeDockWidget, montageManagerDockWidget);

	// Construct File actions.
	QAction* openFileAction = new QAction("&Open File", this);
	openFileAction->setShortcut(QKeySequence::Open);
	openFileAction->setToolTip("Open an existing file.");
	openFileAction->setStatusTip(openFileAction->toolTip());
	connect(openFileAction, SIGNAL(triggered()), this, SLOT(openFile()));

	QAction* closeFileAction = new QAction("Close File", this);
	closeFileAction->setShortcut(QKeySequence::Close);
	closeFileAction->setToolTip("Close the currently opened file.");
	closeFileAction->setStatusTip(closeFileAction->toolTip());
	connect(closeFileAction, SIGNAL(triggered()), this, SLOT(closeFile()));

	QAction* saveFileAction = new QAction("Save File", this);
	saveFileAction->setShortcut(QKeySequence::Save);
	saveFileAction->setToolTip("Save the currently opened file.");
	saveFileAction->setStatusTip(saveFileAction->toolTip());
	connect(saveFileAction, SIGNAL(triggered()), this, SLOT(saveFile()));

	// Construct Zoom actions.
	QAction* horizontalZoomInAction = new QAction("Horizontal Zoom In", this);
	horizontalZoomInAction->setShortcut(QKeySequence("Alt++"));
	horizontalZoomInAction->setToolTip("Zoom in time line.");
	horizontalZoomInAction->setStatusTip(horizontalZoomInAction->toolTip());
	connect(horizontalZoomInAction, SIGNAL(triggered()), this, SLOT(horizontalZoomIn()));

	QAction* horizontalZoomOutAction = new QAction("Horizontal Zoom Out", this);
	horizontalZoomOutAction->setShortcut(QKeySequence("Alt+-"));
	horizontalZoomOutAction->setToolTip("Zoom out time line.");
	horizontalZoomOutAction->setStatusTip(horizontalZoomOutAction->toolTip());
	connect(horizontalZoomOutAction, SIGNAL(triggered()), this, SLOT(horizontalZoomOut()));

	QAction* verticalZoomInAction = new QAction("Vertical Zoom In", this);
	verticalZoomInAction->setShortcut(QKeySequence("Shift++"));
	verticalZoomInAction->setToolTip("Zoom in amplitudes of signals.");
	verticalZoomInAction->setStatusTip(verticalZoomInAction->toolTip());
	connect(verticalZoomInAction, SIGNAL(triggered()), this, SLOT(verticalZoomIn()));

	QAction* verticalZoomOutAction = new QAction("Vertical Zoom Out", this);
	verticalZoomOutAction->setShortcut(QKeySequence("Shift+-"));
	verticalZoomOutAction->setToolTip("Zoom out amplitudes of signals.");
	verticalZoomOutAction->setStatusTip(verticalZoomOutAction->toolTip());
	connect(verticalZoomOutAction, SIGNAL(triggered()), this, SLOT(verticalZoomOut()));

	// Construct Spikedet actions.
	QAction* runSpikedetAction = new QAction(QIcon(":/play-icon.png"), "Run Spikedet Analysis", this);
	runSpikedetAction->setToolTip("Run Spikedet analysis on the current montage.");
	runSpikedetAction->setStatusTip(runSpikedetAction->toolTip());
	connect(runSpikedetAction, &QAction::triggered, [this] () { runSpikedet(); } );

	QAction* spikedetSettingsAction = new QAction(QIcon(":/settings-icon.png"), "Spikedet Settings", this);
	spikedetSettingsAction->setToolTip("Change Spikedet settings.");
	spikedetSettingsAction->setStatusTip(spikedetSettingsAction->toolTip());
	connect(spikedetSettingsAction, &QAction::triggered, [this] ()
	{
		AlenkaSignal::DETECTOR_SETTINGS settings = spikedetAnalysis->getSettings();
		SpikedetSettingsDialog dialog(&settings, &eventDuration, this);

		if (dialog.exec() == QDialog::Accepted)
			spikedetAnalysis->setSettings(settings);
	});

	// Construct Time Mode action group.
	timeModeActionGroup = new QActionGroup(this);

	QAction* timeModeAction0 = new QAction("Sample", this);
	timeModeAction0->setToolTip("Samples from the start.");
	timeModeAction0->setStatusTip(timeModeAction0->toolTip());
	connect(timeModeAction0, SIGNAL(triggered()), this, SLOT(timeMode0()));
	timeModeAction0->setActionGroup(timeModeActionGroup);
	timeModeAction0->setCheckable(true);

	QAction* timeModeAction1 = new QAction("Offset", this);
	timeModeAction1->setToolTip("Time offset from the start.");
	timeModeAction1->setStatusTip(timeModeAction1->toolTip());
	connect(timeModeAction1, SIGNAL(triggered()), this, SLOT(timeMode1()));
	timeModeAction1->setActionGroup(timeModeActionGroup);
	timeModeAction1->setCheckable(true);

	QAction* timeModeAction2 = new QAction("Real", this);
	timeModeAction2->setToolTip("Real time.");
	timeModeAction2->setStatusTip(timeModeAction2->toolTip());
	connect(timeModeAction2, SIGNAL(triggered()), this, SLOT(timeMode2()));
	timeModeAction2->setActionGroup(timeModeActionGroup);
	timeModeAction2->setCheckable(true);

	// Construct Time Line Interval action group.
	timeLineIntervalActionGroup = new QActionGroup(this);

	QAction* timeLineOffAction = new QAction("Off", this);
	timeLineOffAction->setToolTip("Turn off the time lines.");
	timeLineOffAction->setStatusTip(timeLineOffAction->toolTip());
	connect(timeLineOffAction, &QAction::triggered, [this] ()
	{
		if (file)
		{
			OpenDataFile::infoTable.setTimeLineInterval(0);
		}
	});

	setTimeLineIntervalAction = new QAction("Set", this);
	setTimeLineIntervalAction->setActionGroup(timeLineIntervalActionGroup);
	connect(setTimeLineIntervalAction, &QAction::triggered, [this] ()
	{
		if (file)
		{
			double value = OpenDataFile::infoTable.getTimeLineInterval();
			if (value == 0)
			{
				value = 1;
			}

			bool ok;
			value = QInputDialog::getDouble(this, "Set the interval", "Please, enter the value for the time line interval here:", value, 0, 1000*1000*1000, 2, &ok);

			if (ok)
			{
				OpenDataFile::infoTable.setTimeLineInterval(value);
			}
		}
	});

	// Construct SyncDialog.
	syncServer = new SyncServer();
	syncClient = new SyncClient();
	syncDialog = new SyncDialog(syncServer, syncClient, this);

	QAction* showSyncDialog = new QAction("Show Sync Dialog", this);
	connect(showSyncDialog, SIGNAL(triggered(bool)), syncDialog, SLOT(show()));

	synchronize = new QAction("Synchronize", this);
	synchronize->setCheckable(true);
	synchronize->setChecked(true);

	connect(syncServer, SIGNAL(messageReceived(QByteArray)), this, SLOT(receiveSyncMessage(QByteArray)));
	connect(syncClient, SIGNAL(messageReceived(QByteArray)), this, SLOT(receiveSyncMessage(QByteArray)));

	// TODO: filter the stupid ssl warnings from debug output.

	// Tool bars.
	const int spacing = 3;

	// Construct File tool bar.
	QToolBar* fileToolBar = addToolBar("File Tool Bar");
	fileToolBar->setObjectName("File QToolBar");
	fileToolBar->layout()->setSpacing(spacing);

	fileToolBar->addAction(openFileAction);
	fileToolBar->addAction(closeFileAction);
	fileToolBar->addAction(saveFileAction);

	// Construct Filter tool bar.
	QToolBar* filterToolBar = addToolBar("Filter Tool Bar");
	filterToolBar->setObjectName("Filter QToolBar");
	filterToolBar->layout()->setSpacing(spacing);

	QLabel* label = new QLabel("LF:", this);
	label->setToolTip("Low-pass Filter frequency");
	filterToolBar->addWidget(label);
	lowpassComboBox = new QComboBox(this);
	lowpassComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	lowpassComboBox->setMaximumWidth(150);
	lowpassComboBox->setEditable(true);
	filterToolBar->addWidget(lowpassComboBox);

	label = new QLabel("HF:", this);
	label->setToolTip("High-pass Filter frequency");
	filterToolBar->addWidget(label);
	highpassComboBox = new QComboBox(this);
	highpassComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	highpassComboBox->setMaximumWidth(150);
	highpassComboBox->setEditable(true);
	filterToolBar->addWidget(highpassComboBox);

	notchCheckBox = new QCheckBox("Notch:", this);
	notchCheckBox->setToolTip("Notch Filter on/off");
	notchCheckBox->setLayoutDirection(Qt::RightToLeft);
	filterToolBar->addWidget(notchCheckBox);

	// Construct Select tool bar.
	QToolBar* selectToolBar = addToolBar("Select Tool bar");
	selectToolBar->setObjectName("Select QToolBar");
	selectToolBar->layout()->setSpacing(spacing);

	selectToolBar->addWidget(new QLabel("Montage:", this));
	montageComboBox = new QComboBox(this);
	montageComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	montageComboBox->setMaximumWidth(200);
	selectToolBar->addWidget(montageComboBox);

	selectToolBar->addWidget(new QLabel("Event Type:", this));
	eventTypeComboBox = new QComboBox(this);
	eventTypeComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	eventTypeComboBox->setMaximumWidth(200);
	selectToolBar->addWidget(eventTypeComboBox);

	// Construct Zoom tool bar.
	QToolBar* zoomToolBar = addToolBar("Zoom Tool Bar");
	zoomToolBar->setObjectName("Zoom QToolBar");
	zoomToolBar->layout()->setSpacing(spacing);

	zoomToolBar->addAction(horizontalZoomInAction);
	zoomToolBar->addAction(horizontalZoomOutAction);
	zoomToolBar->addAction(verticalZoomInAction);
	zoomToolBar->addAction(verticalZoomOutAction);

	// Construct Spikedet tool bar.
	QToolBar* spikedetToolBar = addToolBar("Spikedet Tool Bar");
	spikedetToolBar->setObjectName("Spikedet QToolBar");
	spikedetToolBar->layout()->setSpacing(spacing);

	spikedetToolBar->addAction(runSpikedetAction);
	spikedetToolBar->addAction(spikedetSettingsAction);

	// Construct File menu.
	QMenu* fileMenu = menuBar()->addMenu("&File");

	fileMenu->addAction(openFileAction);
	fileMenu->addAction(closeFileAction);
	fileMenu->addAction(saveFileAction);

	// Construct View menu.
	QMenu* viewMenu = menuBar()->addMenu("&View");

	viewMenu->addAction(horizontalZoomInAction);
	viewMenu->addAction(horizontalZoomOutAction);
	viewMenu->addAction(verticalZoomInAction);
	viewMenu->addAction(verticalZoomOutAction);

	QMenu* timeModeMenu = new QMenu("Time Mode", this);
	timeModeMenu->addAction(timeModeAction0);
	timeModeMenu->addAction(timeModeAction1);
	timeModeMenu->addAction(timeModeAction2);
	viewMenu->addMenu(timeModeMenu);

	QMenu* timeLineIntervalMenu = new QMenu("Time Line Interval", this);
	timeLineIntervalMenu->addAction(timeLineOffAction);
	timeLineIntervalMenu->addAction(setTimeLineIntervalAction);
	viewMenu->addMenu(timeLineIntervalMenu);

	// Construct Window menu.
	QMenu* windowMenu = menuBar()->addMenu("&Window");

	windowMenu->addAction(trackManagerDockWidget->toggleViewAction());
	windowMenu->addAction(eventManagerDockWidget->toggleViewAction());
	windowMenu->addAction(eventTypeDockWidget->toggleViewAction());
	windowMenu->addAction(montageManagerDockWidget->toggleViewAction());

	windowMenu->addSeparator();
	windowMenu->addAction(fileToolBar->toggleViewAction());
	windowMenu->addAction(filterToolBar->toggleViewAction());
	windowMenu->addAction(selectToolBar->toggleViewAction());
	windowMenu->addAction(zoomToolBar->toggleViewAction());
	windowMenu->addAction(spikedetToolBar->toggleViewAction());

	// Construct Tools menu.
	QMenu* toolsMenu = menuBar()->addMenu("&Tools");

	toolsMenu->addAction(runSpikedetAction);
	toolsMenu->addAction(spikedetSettingsAction);
	toolsMenu->addSeparator();

	toolsMenu->addAction(showSyncDialog);
	toolsMenu->addAction(synchronize);

	// Construct status bar.
	timeModeStatusLabel = new QLabel(this);
	timeModeStatusLabel->setContextMenuPolicy(Qt::ActionsContextMenu);
	timeModeStatusLabel->addAction(timeModeAction0);
	timeModeStatusLabel->addAction(timeModeAction1);
	timeModeStatusLabel->addAction(timeModeAction2);
	statusBar()->addPermanentWidget(timeModeStatusLabel);

	timeStatusLabel = new QLabel(this);
	timeStatusLabel->setContextMenuPolicy(Qt::ActionsContextMenu);
	timeStatusLabel->addAction(timeModeAction0);
	timeStatusLabel->addAction(timeModeAction1);
	timeStatusLabel->addAction(timeModeAction2);
	statusBar()->addPermanentWidget(timeStatusLabel);

	positionStatusLabel = new QLabel(this);
	positionStatusLabel->setContextMenuPolicy(Qt::ActionsContextMenu);
	positionStatusLabel->addAction(timeModeAction0);
	positionStatusLabel->addAction(timeModeAction1);
	positionStatusLabel->addAction(timeModeAction2);
	statusBar()->addPermanentWidget(positionStatusLabel);

	cursorStatusLabel = new QLabel(this);
	cursorStatusLabel->setContextMenuPolicy(Qt::ActionsContextMenu);
	cursorStatusLabel->addAction(timeModeAction0);
	cursorStatusLabel->addAction(timeModeAction1);
	cursorStatusLabel->addAction(timeModeAction2);
	statusBar()->addPermanentWidget(cursorStatusLabel);

	// Restore settings.
	restoreGeometry(PROGRAM_OPTIONS.settings("SignalFileBrowserWindow geometry").toByteArray());
	restoreState(PROGRAM_OPTIONS.settings("SignalFileBrowserWindow state").toByteArray());

	// Set up Spikedet.
	spikedetAnalysis = new SpikedetAnalysis(globalContext.get());

	auto settings = spikedetAnalysis->getSettings();
	loadSpikedetOptions(&settings, &eventDuration);
	spikedetAnalysis->setSettings(settings);
}

SignalFileBrowserWindow::~SignalFileBrowserWindow()
{
	closeFile();

	delete spikedetAnalysis;
}

QDateTime SignalFileBrowserWindow::sampleToDate(DataFile* file, int sample)
{
	int timeOffset = round(sample/file->getSamplingFrequency()*1000);

	QDateTime date = QDateTime::fromTime_t(file->getStartDate());
	date = date.addMSecs(timeOffset);

	return date;
}

QDateTime SignalFileBrowserWindow::sampleToOffset(DataFile* file, int sample)
{
	int timeOffset = round(sample/file->getSamplingFrequency()*1000);

	QDateTime date(QDate(1970, 1, 1));
	date = date.addMSecs(timeOffset);

	return date;
}

QString SignalFileBrowserWindow::sampleToDateTimeString(DataFile* file, int sample, InfoTable::TimeMode mode)
{
	QLocale locale;

	if (mode == InfoTable::TimeMode::size)
	{
		mode = OpenDataFile::infoTable.getTimeMode();
	}

	if (mode == InfoTable::TimeMode::samples)
	{
		return QString::number(sample);
	}
	else if (mode == InfoTable::TimeMode::offset)
	{
		QDateTime date = sampleToOffset(file, sample);
		return QString::number(date.date().day() - 1) + "d " + date.toString("hh:mm:ss" + QString(locale.decimalPoint()) + "zzz");
	}
	else if (mode == InfoTable::TimeMode::real)
	{
		return sampleToDate(file, sample).toString("d.M.yyyy hh:mm:ss" + QString(locale.decimalPoint()) + "zzz");
	}

	return QString();
}

void SignalFileBrowserWindow::closeEvent(QCloseEvent* event)
{
	// Store settings.
	PROGRAM_OPTIONS.settings("SignalFileBrowserWindow geometry", saveGeometry());
	PROGRAM_OPTIONS.settings("SignalFileBrowserWindow state", saveState());

	QMainWindow::closeEvent(event);
}

void SignalFileBrowserWindow::connectVitness(DataModelVitness* vitness, std::function<void ()> f)
{
	auto c = connect(vitness, &DataModelVitness::valueChanged, f);
	openFileConnections.push_back(c);
	c = connect(vitness, &DataModelVitness::rowsInserted, f);
	openFileConnections.push_back(c);
	c = connect(vitness, &DataModelVitness::rowsRemoved, f);
	openFileConnections.push_back(c);
}

void SignalFileBrowserWindow::horizontalZoom(double factor)
{
	if (file)
		OpenDataFile::infoTable.setVirtualWidth(OpenDataFile::infoTable.getVirtualWidth()*factor);
}

void SignalFileBrowserWindow::verticalZoom(double factor)
{
	if (file)
	{
		AbstractTrackTable* tt = file->getDataModel()->montageTable()->trackTable(OpenDataFile::infoTable.getSelectedMontage());

		for (int i = 0; i < tt->rowCount(); ++i)
		{
			Track t = tt->row(i);

			double value = t.amplitude*factor;
			value = value != 0 ? value : -0.000001;
			t.amplitude = value;

			tt->row(i, t);
		}
	}
}

void SignalFileBrowserWindow::mode(int m)
{
	if (file)
	{
		OpenDataFile::infoTable.setTimeMode(static_cast<InfoTable::TimeMode>(m));

		updatePositionStatusLabel();
		updateCursorStatusLabel();
	}
}

bool SignalFileBrowserWindow::shouldSynchronizeView()
{
	return synchronize->isChecked();
}

void SignalFileBrowserWindow::openFile()
{
	QString fileName = QFileDialog::getOpenFileName(this, "Open File", "", "GDF file (*.gdf);;EDF file (*.edf)");

	if (fileName.isNull())
		return; // No file was selected.

	closeFile();

	QFileInfo fileInfo(fileName);

	if (fileInfo.exists() == false)
	{
		logToFileAndConsole("File '" + fileName.toStdString() + "' not found.");
		return;
	}
	else if (fileInfo.isReadable() == false)
	{
		logToFileAndConsole("File '" + fileName.toStdString() + "' cannot be read.");
		return;
	}
	else if (fileInfo.isWritable() == false)
	{
		logToFileAndConsole("File '" + fileName.toStdString() + "' cannot be written to.");
		return;
	}

	logToFile("Opening file '" << fileName.toStdString() << "'.");

	string stdFileName = fileName.toStdString();
	if (fileInfo.suffix().toLower() == "gdf")
	{
		file = new GDF2(stdFileName);
	}
	else
	{
		file = new EDF(stdFileName);
	}

	dataModel = new DataModel(new VitnessEventTypeTable(), new VitnessMontageTable());
	file->setDataModel(dataModel);

	executeWithCLocale([this] () {
		file->load();
		OpenDataFile::infoTable.readXML(file->getFilePath() + ".info");
	});

	setWindowTitle(fileInfo.fileName() + " - " + title);

	// Check for any values in InfoTable that could make trouble.
	if (OpenDataFile::infoTable.getSelectedMontage() < 0 || OpenDataFile::infoTable.getSelectedMontage() >= file->getDataModel()->montageTable()->rowCount())
		OpenDataFile::infoTable.setSelectedMontage(0);

	// Pass the file to the child widgets.
	trackManager->changeFile(file);
	eventManager->changeFile(file);
	eventTypeManager->changeFile(file);
	montageManager->changeFile(file);

	signalViewer->changeFile(file);

	// Update Filter tool bar.
	QStringList comboOptions;

	comboOptions << "---" << "0" << "5" << "10";

	for (int i = 50; i <= file->getSamplingFrequency()/2; i *= 2)
		comboOptions << QString::number(i);

	QMetaObject::Connection c;

	lowpassComboBox->clear();
	lowpassComboBox->addItems(comboOptions);
	c = connect(lowpassComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(lowpassComboBoxUpdate(QString)));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(lowpassFrequencyChanged(double)), this, SLOT(lowpassComboBoxUpdate(double)));
	openFileConnections.push_back(c);

	highpassComboBox->clear();
	highpassComboBox->addItems(comboOptions);
	c = connect(highpassComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(highpassComboBoxUpdate(QString)));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(highpassFrequencyChanged(double)), this, SLOT(highpassComboBoxUpdate(double)));
	openFileConnections.push_back(c);

	c = connect(notchCheckBox, SIGNAL(toggled(bool)), &OpenDataFile::infoTable, SLOT(setNotch(bool)));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(notchChanged(bool)), notchCheckBox, SLOT(setChecked(bool)));
	openFileConnections.push_back(c);

	// Set up table models and managers.
	eventTypeTable = new EventTypeTableModel(file);
	eventTypeManager->setModel(eventTypeTable);

	montageTable = new MontageTableModel(file);
	montageManager->setModel(montageTable);

	eventTable = new EventTableModel(file);
	eventManager->setModel(eventTable);

	trackTable = new TrackTableModel(file);
	trackManager->setModel(trackTable);

	// Update the Select tool bar.
	connectVitness(VitnessMontageTable::vitness(dataModel->montageTable()), [this] () { updateMontageComboBox(); });
	openFileConnections.push_back(c);
	updateMontageComboBox();

	c = connect(montageComboBox, SIGNAL(currentIndexChanged(int)), &OpenDataFile::infoTable, SLOT(setSelectedMontage(int)));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(selectedMontageChanged(int)), montageComboBox, SLOT(setCurrentIndex(int)));
	openFileConnections.push_back(c);

	connectVitness(VitnessEventTypeTable::vitness(dataModel->eventTypeTable()), [this] () { updateEventTypeComboBox(); });
	updateEventTypeComboBox();

	c = connect(eventTypeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [] (int index) {
		OpenDataFile::infoTable.setSelectedType(index - 1);
	});
	openFileConnections.push_back(c);

	c = connect(&OpenDataFile::infoTable, &InfoTable::selectedTypeChanged, [this] (int value){
		eventTypeComboBox->setCurrentIndex(value + 1);
	});
	openFileConnections.push_back(c);

	// Update the status bar.
	QString str = "Start: " + sampleToDateTimeString(file, 0, InfoTable::TimeMode::real);
	str += " Total time: " + sampleToDateTimeString(file, file->getSamplesRecorded(), InfoTable::TimeMode::offset);
	timeStatusLabel->setText(str);

	c = connect(&OpenDataFile::infoTable, SIGNAL(positionChanged(int)), this, SLOT(updatePositionStatusLabel()));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(positionIndicatorChanged(double)), this, SLOT(updatePositionStatusLabel()));
	openFileConnections.push_back(c);
	c = connect(signalViewer->getCanvas(), SIGNAL(cursorPositionSampleChanged(int)), this, SLOT(updateCursorStatusLabel()));
	openFileConnections.push_back(c);

	// Connect slot SignalViewer::updateSignalViewer() to make sure that the SignalViewer gets updated when needed.
	c = connect(&OpenDataFile::infoTable, SIGNAL(virtualWidthChanged(int)), signalViewer, SLOT(updateSignalViewer()));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(positionChanged(int)), signalViewer, SLOT(updateSignalViewer()));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(lowpassFrequencyChanged(double)), signalViewer, SLOT(updateSignalViewer()));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(highpassFrequencyChanged(double)), signalViewer, SLOT(updateSignalViewer()));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(notchChanged(bool)), signalViewer, SLOT(updateSignalViewer()));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(selectedMontageChanged(int)), signalViewer, SLOT(updateSignalViewer()));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(timeLineIntervalChanged(double)), signalViewer, SLOT(updateSignalViewer()));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(positionIndicatorChanged(double)), signalViewer, SLOT(updateSignalViewer()));
	openFileConnections.push_back(c);

	connectVitness(VitnessMontageTable::vitness(dataModel->montageTable()), [this] () { signalViewer->updateSignalViewer(); });
	connectVitness(VitnessEventTypeTable::vitness(dataModel->eventTypeTable()), [this] () { signalViewer->updateSignalViewer(); });
	c = connect(&OpenDataFile::infoTable, SIGNAL(selectedMontageChanged(int)), this, SLOT(updateManagers(int)));
	openFileConnections.push_back(c);

	// Update the View submenus.
	c = connect(&OpenDataFile::infoTable, SIGNAL(timeModeChanged(int)), this, SLOT(updateTimeMode(int)));
	c = connect(&OpenDataFile::infoTable, &InfoTable::timeLineIntervalChanged, [this] (double value)
	{
		setTimeLineIntervalAction->setToolTip("The time line interval is " + locale().toString(value) + " s.");
		setTimeLineIntervalAction->setStatusTip(setTimeLineIntervalAction->toolTip());
	});
	openFileConnections.push_back(c);

	// Connect Sync.
	c = connect(&OpenDataFile::infoTable, SIGNAL(positionChanged(int)), this, SLOT(sendSyncMessage()));
	openFileConnections.push_back(c);
	c = connect(&OpenDataFile::infoTable, SIGNAL(positionIndicatorChanged(double)), this, SLOT(sendSyncMessage()));
	openFileConnections.push_back(c);

	// Emit all signals to ensure there are no uninitialized controls.
	OpenDataFile::infoTable.emitAllSignals();
}

void SignalFileBrowserWindow::closeFile()
{
	logToFile("Closing file.");

	setWindowTitle(title);

	for (auto e : openFileConnections)
		disconnect(e);
	openFileConnections.clear();

	montageComboBox->clear();
	eventTypeComboBox->clear();

	if (file)
	{
		executeWithCLocale([this] () {
			OpenDataFile::infoTable.writeXML(file->getFilePath() + ".info");
		});
	}

	signalViewer->changeFile(nullptr);

	trackManager->changeFile(nullptr);
	eventManager->changeFile(nullptr);
	eventTypeManager->changeFile(nullptr);
	montageManager->changeFile(nullptr);

	delete eventTypeTable; eventTypeTable = nullptr;
	delete montageTable; montageTable = nullptr;
	delete eventTable; eventTable = nullptr;
	delete trackTable; trackTable= nullptr;

	delete dataModel; dataModel = nullptr;

	delete file; file = nullptr;

	signalViewer->updateSignalViewer();
}

void SignalFileBrowserWindow::saveFile()
{
	logToFile("Saving file.");

	if (file)
	{
		executeWithCLocale([this] () {
			file->save();
		});
	}
}

void SignalFileBrowserWindow::lowpassComboBoxUpdate(const QString& text)
{
	if (file)
	{
		bool ok;
		double tmp = locale().toDouble(text, &ok);

		if (ok)
			OpenDataFile::infoTable.setLowpassFrequency(tmp);
		else
			OpenDataFile::infoTable.setLowpassFrequency(1000000); // TODO: properly turn of/on
	}
}

void SignalFileBrowserWindow::lowpassComboBoxUpdate(double value)
{
	if (file)
	{
		if (value < 0 || value > file->getSamplingFrequency()/2)
			lowpassComboBox->setCurrentIndex(0);
		else
			lowpassComboBox->setCurrentText(locale().toString(value, 'f', 2));
	}
}

void SignalFileBrowserWindow::highpassComboBoxUpdate(const QString& text)
{
	if (file)
	{
		bool ok;
		double tmp = locale().toDouble(text, &ok);

		if (ok)
			OpenDataFile::infoTable.setHighpassFrequency(tmp);
		else
			OpenDataFile::infoTable.setHighpassFrequency(-1000000); // TODO: properly turn of/on
	}
}

void SignalFileBrowserWindow::highpassComboBoxUpdate(double value)
{
	if (file)
	{
		if (value < 0 || value > file->getSamplingFrequency()/2)
			highpassComboBox->setCurrentIndex(0);
		else
			highpassComboBox->setCurrentText(locale().toString(value, 'f', 2));
	}
}

void SignalFileBrowserWindow::updateManagers(int value)
{
	connectVitness(VitnessTrackTable::vitness(dataModel->montageTable()->trackTable(value)), [this] () { signalViewer->updateSignalViewer(); });
	connectVitness(VitnessEventTable::vitness(dataModel->montageTable()->eventTable(value)), [this] () { signalViewer->updateSignalViewer(); });
}

void SignalFileBrowserWindow::horizontalZoomIn()
{
	horizontalZoom(horizontalZoomFactor);
}

void SignalFileBrowserWindow::horizontalZoomOut()
{
	horizontalZoom(1/horizontalZoomFactor);
}

void SignalFileBrowserWindow::verticalZoomIn()
{
	verticalZoom(verticalZoomFactor);
}

void SignalFileBrowserWindow::verticalZoomOut()
{
	verticalZoom(1/verticalZoomFactor);
}

void SignalFileBrowserWindow::updateTimeMode(int mode)
{
	QAction* a = timeModeActionGroup->actions().at(mode);
	a->setChecked(true);

	timeModeStatusLabel->setText("Time Mode: " + a->text());
}

void SignalFileBrowserWindow::updatePositionStatusLabel()
{
	double ratio = static_cast<double>(file->getSamplesRecorded())/OpenDataFile::infoTable.getVirtualWidth();
	double position = (OpenDataFile::infoTable.getPosition() + OpenDataFile::infoTable.getPositionIndicator()*signalViewer->getCanvas()->width())*ratio;

	positionStatusLabel->setText("Position: " + sampleToDateTimeString(file, round(position)));
}

void SignalFileBrowserWindow::updateCursorStatusLabel()
{
	cursorStatusLabel->setText("Cursor at: " + sampleToDateTimeString(file, signalViewer->getCanvas()->getCursorPositionSample()));
}

void SignalFileBrowserWindow::updateMontageComboBox()
{
	if (file)
	{
		AbstractMontageTable* montageTable = file->getDataModel()->montageTable();
		int itemCount = montageComboBox->count();
		int selectedMontage = OpenDataFile::infoTable.getSelectedMontage();

		for (int i = 0; i < montageTable->rowCount(); ++i)
			montageComboBox->addItem(QString::fromStdString(montageTable->row(i).name));

		for (int i = 0; i < itemCount; ++i)
			montageComboBox->removeItem(0);

		OpenDataFile::infoTable.setSelectedMontage(min(selectedMontage, montageTable->rowCount() - 1));
	}
}

void SignalFileBrowserWindow::updateEventTypeComboBox()
{
	if (file)
	{
		AbstractEventTypeTable* eventTypeTable = file->getDataModel()->eventTypeTable();
		int itemCount = eventTypeComboBox->count();
		int selectedType = OpenDataFile::infoTable.getSelectedType();

		eventTypeComboBox->addItem("<No Type>");
		for (int i = 0; i < eventTypeTable->rowCount(); ++i)
			eventTypeComboBox->addItem(QString::fromStdString(eventTypeTable->row(i).name));

		for (int i = 0; i < itemCount; ++i)
			eventTypeComboBox->removeItem(0);

		OpenDataFile::infoTable.setSelectedType(min(selectedType, eventTypeTable->rowCount() - 1));
	}
}

void SignalFileBrowserWindow::runSpikedet()
{
	if (file)
	{
		// Build montage from code.
		vector<AlenkaSignal::Montage<float>*> montage;

		QFile headerFile(":/montageHeader.cl");
		headerFile.open(QIODevice::ReadOnly);
		string header = headerFile.readAll().toStdString();

		AbstractTrackTable* tt = file->getDataModel()->montageTable()->trackTable(OpenDataFile::infoTable.getSelectedMontage());
		for (int i = 0; i < tt->rowCount(); ++i)
			montage.push_back(new AlenkaSignal::Montage<float>(tt->row(i).code, globalContext.get(), header));

		// Run Spikedet.
		QProgressDialog progress("Running Spikedet analysis", "Abort", 0, 100, this);
		progress.setWindowModality(Qt::WindowModal);

		progress.setMinimumDuration(0); // This is to show the dialog immediately.
		progress.setValue(1);

		spikedetAnalysis->runAnalysis(file, montage, &progress);

		// Add three new event types for the different levels of spike events.
		AbstractEventTypeTable* ett = file->getDataModel()->eventTypeTable();
		int index = ett->rowCount();
		ett->insertRows(index, 3);

		QColor colors[3] = {QColor(0, 0, 255), QColor(0, 255, 0), QColor(0, 255, 255)};
		for (int i = 0; i < 3; ++i)
		{
			EventType et = ett->row(index + i);
			et.name = "Spikedet K" + to_string(i + 1);
			DataModel::color2array(colors[i], et.color);
			ett->row(index + i, et);
		}

		// Process the output structure.
		AbstractEventTable* et = file->getDataModel()->montageTable()->eventTable(OpenDataFile::infoTable.getSelectedMontage());

		AlenkaSignal::CDetectorOutput* out = spikedetAnalysis->getOutput();
		assert(out);
		unsigned int count = out->m_pos.size();

		if (count > 0)
		{
			assert(out->m_chan.size() == count);

			int etIndex = et->rowCount();
			et->insertRows(etIndex, count);

			for (unsigned int i = 0; i < count; i++)
			{
				Event e = et->row(etIndex + i);

				e.label = "Spike " + to_string(i);
				e.type = index + (out->m_con[i] == 0.5 ? 1 : 0); // TODO: Figure out what should be used as the third type.
				e.position = out->m_pos[i]*file->getSamplingFrequency();
				e.duration = file->getSamplingFrequency()*eventDuration;
				//e.duration = out->m_dur[i]*file->getSamplingFrequency();
				e.channel = out->m_chan[i] - 1;

				et->row(etIndex + i, e);
			}
		}
	}
}

void SignalFileBrowserWindow::receiveSyncMessage(const QByteArray& message)
{
	if (file && shouldSynchronizeView())
	{
		double timePosition;
		unpackMessage(message, &timePosition);

		const double ratio = static_cast<double>(file->getSamplesRecorded())/OpenDataFile::infoTable.getVirtualWidth();
		int position = timePosition*file->getSamplingFrequency()/ratio;

#ifndef NDEBUG
		cerr << "Received position: " << position << " " << timePosition << endl;
#endif

		lastPositionReceived = position;
		position -= signalViewer->getCanvas()->width()*OpenDataFile::infoTable.getPositionIndicator();
		OpenDataFile::infoTable.setPosition(position);
	}
}

void SignalFileBrowserWindow::sendSyncMessage()
{
	if (file && shouldSynchronizeView())
	{
		const int position = OpenDataFile::infoTable.getPosition() + signalViewer->getCanvas()->width()*OpenDataFile::infoTable.getPositionIndicator();

		const double ratio = static_cast<double>(file->getSamplesRecorded())/OpenDataFile::infoTable.getVirtualWidth();
		const int epsilon = max<int>(3, file->getSamplingFrequency()/50/ratio);

		// This condition is to break the message deadlock. This comes about because the source of the signal that trigers
		// this method cannot be distinguished between user input and received sync message.

		// When the current position is very close (within a small fraction of a second) to the last received position,
		// you are most likely just repeating what the server already has. So there is no need to send this information.
		if (position < (lastPositionReceived - epsilon) || position > (lastPositionReceived + epsilon))
		{
			const double timePosition = position*ratio/file->getSamplingFrequency();
#ifndef NDEBUG
			if (syncServer->connectionCount() > 0 || syncClient->isValid())
				cerr << "Sending position: " << position << " " << timePosition << endl;
#endif
			QByteArray message = packMessage(timePosition);

			syncServer->sendMessage(message);
			syncClient->sendMessage(message);
		}
#ifndef NDEBUG
		else
		{
			cerr << "Message skipped: " << position << endl;
		}
#endif

		// Reset to the default value, so that the message is skipped at most once.
		lastPositionReceived = lastPositionReceivedDefault;
	}
}
