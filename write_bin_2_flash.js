// Import the DSS packages into our namespace to save on typing
importPackage(Packages.com.ti.debug.engine.scripting)
importPackage(Packages.com.ti.ccstudio.scripting.environment)
importPackage(Packages.java.lang)

// Configurable Parameters
var deviceCCXMLFile = "C:/Users/zhang/dsp_flash_write_easy/0507_App_Load/NewTargetConfiguration.ccxml";
var programToLoad = "C:/Users/zhang/dsp_flash_write_easy/0507_App_Load/Debug/0507_App_Load.out";

// Create our scripting environment object - which is the main entry point into any script and
// the factory for creating other Scriptable ervers and Sessions
var script = ScriptingEnvironment.instance();

// Create a debug server
var debugServer = script.getServer("DebugServer.1" );

// Set the device ccxml 
debugServer.setConfig(deviceCCXMLFile);

// Open a debug session
debugSession = debugServer.openSession("Texas Instruments XDS100v3 USB Emulator_0","C66xx_0");

// Connect to the target
debugSession.target.connect();

// Load the program 
debugSession.memory.loadProgram( programToLoad );

var breakpoint1 = debugSession.breakpoint.add("main.c",125);

// Run the program
debugSession.target.run();

debugSession.memory.loadRaw(0,0x80000000,"C:/Users/zhang/dsp_flash_write_easy/helloworld.bin",32,false);

var breakpoint2 = debugSession.breakpoint.add("main.c",230);

debugSession.target.run();
// Disconnect from the target
debugSession.target.disconnect();

debugSession.terminate();
debugServer.stop();
