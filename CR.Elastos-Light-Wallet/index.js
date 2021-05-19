"use strict";

const electron = require( 'electron' );
// Module to control application life.
const app = electron.app;
// Module to create native browser window.
const BrowserWindow = electron.BrowserWindow;
const Menu = electron.Menu;
const MenuItem = electron.MenuItem;
const remote = electron.remote;

const path = require( 'path' )

process.env['ELECTRON_DISABLE_SECURITY_WARNINGS'] = 'true';

// Report crashes to our server.
// require('crash-reporter').start();

// Linux 3d acceleration causes black screen for Electron-based apps, so turn it
// off
// see https://github.com/electron/electron/issues/4380 and
// https://github.com/electron/electron/issues/5297
if ( process.platform === 'linux' ) {
    app.disableHardwareAcceleration();
}

// Keep a global reference of the window object, if you don't, the window will
// be closed automatically when the JavaScript object is garbage collected.
var mainWindow = null;

// Quit when all windows are closed.
app.on( 'window-all-closed', function() {
    // On OS X it is common for applications and their menu bar
    // to stay active until the user quits explicitly with Cmd + Q
    // if (process.platform !== 'darwin') {
    app.quit();
    // }
} );

const createMenu = () => {
    const application = {
        label: "Application",
        submenu: [
            {
                label: "About Application",
                selector: "orderFrontStandardAboutPanel:"
            },
            {
                type: "separator"
            },
            {
                label: "Quit",
                accelerator: "Command+Q",
                click: () => {
                    app.quit()
                }
            }
        ]
    }

    const edit = {
        label: "Edit",
        submenu: [
            {
                label: "Undo",
                accelerator: "CmdOrCtrl+Z",
                selector: "undo:"
            },
            {
                label: "Redo",
                accelerator: "Shift+CmdOrCtrl+Z",
                selector: "redo:"
            },
            {
                type: "separator"
            },
            {
                label: "Cut",
                accelerator: "CmdOrCtrl+X",
                selector: "cut:"
            },
            {
                label: "Copy",
                accelerator: "CmdOrCtrl+C",
                selector: "copy:"
            },
            {
                label: "Paste",
                accelerator: "CmdOrCtrl+V",
                selector: "paste:"
            },
            {
                label: "Select All",
                accelerator: "CmdOrCtrl+A",
                selector: "selectAll:"
            }
        ]
    }

    const template = [
        application,
        edit
    ]

    Menu.setApplicationMenu( Menu.buildFromTemplate( template ) )
}

// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
app.on( 'ready', () => {
    // Create the browser window.
    mainWindow = new BrowserWindow( {
        width: 1125,
        height: 750,
        frame: false,
        // titleBarStyle: 'hidden',
        transparent: true,
        // trafficLightPosition: {x: 15, y: 15},
        maximizable: false,
        maxHeight: 750,
        // resizable: false,
        icon: path.join( __dirname, 'assets/icons/png/64x64.png' ),
        webPreferences: {
            nodeIntegration: true
        }
    } );

    require('./libraries/electron-traffic-light.js')(mainWindow);
    
    // disable navigation window.
    mainWindow.setMenu( null );

    // and load the index.html of the app.
    mainWindow.loadURL( 'file://' + __dirname + '/index.html' );

    // add right click menu to open the DevTools.
    // mainWindow.webContents.openDevTools();

    // Emitted when the window is closed.
    mainWindow.on( 'closed', function() {
        // Dereference the window object, usually you would store windows
        // in an array if your app supports multi windows, this is the time
        // when you should delete the corresponding element.
        mainWindow = null;
    } );

    createMenu();
} );
