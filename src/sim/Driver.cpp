#include "Driver.h"

#include <QCoreApplication>
#include <QProcess>
#include <QDebug>

//#include <QtGlobal> // qInstallMessageHandler
#include <QFile>
// TODO: MACK

// TODO: MACK - replace this with QThread
#include <thread>

#include "Assert.h"
#include "Directory.h"
#include "Logging.h"
#include "Param.h"
#include "SimUtilities.h"
#include "State.h"
#include "Time.h"

namespace sim {

// Definition of the static variables for linking
Model* Driver::m_model;
View* Driver::m_view;
Controller* Driver::m_controller;

// TODO: MACK - put this is logging
void myMessageHandler(
        QtMsgType type,
        const QMessageLogContext& context,
        const QString& msg) {

    // TODO: MACK - benchmark this
    
    // TODO: MACK - if debug, we want line numbers
    // "[ %real_time | %sim_time | %level | %logger ] - %msg");
    static const QMap<QtMsgType, QString> mapping {
        {QtDebugMsg,    "DEBUG"   },
        {QtInfoMsg,     "INFO"    },
        {QtWarningMsg,  "WARN"    },
        {QtCriticalMsg, "CRITICAL"},
        {QtFatalMsg,    "FATAL"   },
        {QtSystemMsg,   "SYSTEM"  },
    };

    // TODO: MACK - why is this 9 sometimes?
    double seconds = Time::get()->elapsedRealTime().getSeconds();
    QString secondsString = SimUtilities::formatSeconds(seconds);
    secondsString.truncate(secondsString.indexOf(".") + 4); // Trim to 3 decimal places

    double sim_seconds = Time::get()->elapsedSimTime().getSeconds();
    QString sim_secondsString = SimUtilities::formatSeconds(sim_seconds);
    sim_secondsString.truncate(sim_secondsString.indexOf(".") + 4); // Trim to 3 decimal places

    QString formatted = QString("[ %1 | %2 | %3 ] - %4").arg(
        secondsString,
        sim_secondsString,
        mapping.value(type),
        msg
    );

    std::cout << formatted.toStdString() << std::endl;

    QFile outFile("/home/mack/Desktop/test-log.txt");
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << formatted << endl << flush;
}

void Driver::drive(int argc, char* argv[]) {

    // Make sure that this function is called just once
    SIM_ASSERT_RUNS_JUST_ONCE();

    // Before anything else, initialize the Time object
    Time::init();

    // TODO: See http://doc.qt.io/qt-5/qloggingcategory.html#setFilterRules
    // TODO: http://doc.qt.io/qt-5/qtglobal.html#qSetMessagePattern
    qInstallMessageHandler(myMessageHandler);

    // Then, initialize the Directory object
    QCoreApplication app(argc, argv);
    // TODO: MACK - app.exec()?
    QString path = app.applicationFilePath(); // .../mms/sim/bin
    path = path.left(path.lastIndexOf("/")); // Strips off /bin
    path = path.left(path.lastIndexOf("/")); // Strips off /sim
    Directory::init(path + "/");

    // TODO: MACK - Replace this with Qt functionality
    // Then, determine the runId (just datetime for now)
    QString runId = SimUtilities::timestampToDatetimeString(
        Time::get()->startTimestamp()
    );

    // Then, initiliaze logging (before initializing Param or State)
    Logging::init(runId);

    // Initialize the State object in order to:
    // 1) Set the runId
    // 2) Avoid a race condition (between threads)
    // 3) Initialize the Param object
    S()->setRunId(runId);

    // Remove any excessive archived runs
    SimUtilities::removeExcessArchivedRuns();

    // Generate the glut functions in a static context
    GlutFunctions functions = {
        []() {
            m_view->refresh();
        },
        [](int width, int height) {
            m_view->updateWindowSize(width, height);
        },
        [](unsigned char key, int x, int y) {
            m_view->keyPress(key, x, y);
        },
        [](int key, int x, int y) {
            m_view->specialKeyPress(key, x, y);
        },
        [](int key, int x, int y) {
            m_view->specialKeyRelease(key, x, y);
        }
    };

    // Initialize the model, view, and controller
    m_model = new Model();
    m_view = new View(m_model, argc, argv, functions);
    m_controller = new Controller(m_model, m_view);

    // Initialize mouse algorithm values in the model and view
    m_model->getWorld()->setOptions(
        m_controller->getOptions()
    );
    m_view->setMouseAlgorithmAndOptions(
        m_controller->getMouseAlgorithm(),
        m_controller->getOptions()
    );

    // Initialize the tile text, now that the options have been set
    m_view->initTileGraphicText();

    // Lastly, we need to populate the graphics buffers with maze information,
    // but only after we've initialized the tile graphic text
    m_view->getMazeGraphic()->draw();

    // Start the physics loop
    std::thread physicsThread([]() {
        m_model->getWorld()->simulate();
    });

    // Start the solving loop
    std::thread solvingThread([]() {

        // If the maze is invalid, don't let the algo do anything
        if (!m_model->getMaze()->isValidMaze()) {
            return;
        }

        // Wait for the window to appear
        SimUtilities::sleep(Seconds(P()->glutInitDuration()));

        // TODO: MACK
        // Begin execution of the mouse algorithm
        /*
        m_controller->getMouseAlgorithm()->solve(
            m_model->getMaze()->getWidth(),
            m_model->getMaze()->getHeight(),
            m_model->getMaze()->isOfficialMaze(),
            DIRECTION_TO_CHAR.at(m_model->getMouse()->getCurrentDiscretizedRotation()),
            m_controller->getMouseInterface());
        */
    });

    // Start the graphics loop
    glutMainLoop();
}

} // namespace sim
