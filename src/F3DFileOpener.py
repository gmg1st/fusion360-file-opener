# F3DFileOpener - Final Version
# Uses run()/stop() (correct Fusion 360 Python Add-In entry points)
# Background thread polls for request file when Fusion is already running

import adsk.core
import adsk.fusion
import os
import time
import traceback
import threading
import ctypes

# Hardcoded paths
_logFile = r'C:\Users\G11\AppData\Local\Temp\fusion360_f3d_opener.log'
_logFile2 = r'C:\Users\G11\AppData\Roaming\Autodesk\Autodesk Fusion 360\API\AddIns\F3DFileOpener\addin_debug.log'
_readyFile = r'C:\Users\G11\AppData\Local\Temp\fusion360_f3d_opener_ready.txt'
_requestFile = r'C:\Users\G11\AppData\Local\Temp\fusion360_f3d_opener_request.txt'
_customEventId = 'F3DFileOpenerImportEvent'

# Module-level log
try:
    with open(_logFile, 'a') as f:
        f.write('[%s] >>> MODULE IMPORTED <<<\n' % time.strftime('%Y-%m-%d %H:%M:%S'))
except:
    pass


def _log(message):
    try:
        with open(_logFile, 'a') as f:
            f.write('[%s] %s\n' % (time.strftime('%Y-%m-%d %H:%M:%S'), message))
    except:
        pass


# Globals
_app = None
_ui = None
_customEvent = None
_eventHandler = None
_pollThread = None
_pollRunning = False
_started = False
_mutexHandle = None
_isRunning = False  # True only for the instance that acquired the mutex and is actively running


def _do_import(file_path):
    """Import a file into a new document. Must be called from main thread.
    Supports multiple formats: .f3d, .f3z, .step, .stp, .iges, .igs, .sat, .smt, .svg"""
    try:
        if not file_path or not os.path.exists(file_path):
            _log('File not found: ' + str(file_path))
            return False

        _log('Importing: ' + file_path)
        import_mgr = _app.importManager
        if not import_mgr:
            _log('No ImportManager')
            return False

        # Select import options based on file extension
        ext = os.path.splitext(file_path)[1].lower()
        options = None

        if ext in ('.f3d', '.f3z'):
            options = import_mgr.createFusionArchiveImportOptions(file_path)
        elif ext in ('.step', '.stp'):
            options = import_mgr.createSTEPImportOptions(file_path)
        elif ext in ('.iges', '.igs'):
            options = import_mgr.createIGESImportOptions(file_path)
        elif ext == '.sat':
            options = import_mgr.createSATImportOptions(file_path)
        elif ext == '.smt':
            options = import_mgr.createSMTImportOptions(file_path)
        elif ext == '.svg':
            options = import_mgr.createSVGImportOptions(file_path)
        else:
            _log('Unsupported file extension: ' + ext)
            return False

        if not options:
            _log('Cannot create import options for: ' + ext)
            return False

        doc = import_mgr.importToNewDocument(options)
        if doc:
            # Set document name to the original file name (without extension)
            try:
                file_name = os.path.splitext(os.path.basename(file_path))[0]
                doc.name = file_name
            except:
                pass
            _log('SUCCESS: ' + doc.name)
            return True
        else:
            _log('importToNewDocument returned null')
            return False
    except:
        _log('Import error: ' + traceback.format_exc())
        return False


def _check_and_import():
    """Check for pending request file and import it."""
    try:
        if not os.path.exists(_requestFile):
            return False

        # Skip stale request files (older than 5 minutes) from previous crashed sessions
        file_age = time.time() - os.path.getmtime(_requestFile)
        if file_age > 300:
            _log('Stale request file detected (age: %.0f seconds), deleting without importing' % file_age)
            try:
                os.remove(_requestFile)
            except:
                pass
            return False

        _log('Found request file')
        with open(_requestFile, 'r', encoding='utf-8-sig') as f:
            file_path = f.read().strip()

        try:
            os.remove(_requestFile)
        except:
            pass

        return _do_import(file_path)
    except:
        _log('Error in _check_and_import: ' + traceback.format_exc())
        return False


class F3DImportEventHandler(adsk.core.CustomEventHandler):
    """Handler for custom event - called when polling thread detects a request."""
    def notify(self, eventArgs):
        _log('Custom event received')
        _check_and_import()


def _poll_loop():
    """Background thread: checks for request file every 2 seconds.
    Also sends heartbeat by updating ready file timestamp so VBS launcher can detect stale state."""
    _log('Poll thread started')
    while _pollRunning:
        try:
            # Heartbeat: update ready file modification time so VBS can detect if Add-In is alive
            try:
                with open(_readyFile, 'w') as f:
                    f.write('ready')
            except:
                pass

            if os.path.exists(_requestFile):
                _log('Poll: request file detected, firing event')
                if _app:
                    _app.fireCustomEvent(_customEventId)
        except:
            pass
        time.sleep(2)
    _log('Poll thread stopped')


def run(context):
    """Add-In entry point."""
    global _app, _ui, _customEvent, _eventHandler, _pollThread, _pollRunning, _started, _mutexHandle, _isRunning

    if _started:
        _log('run() duplicate call - skipping')
        return
    _started = True

    # 跨实例互斥：Fusion 360 会因 C盘<->D盘符号链接把同一插件从两条路径各加载一次，
    # 导致两个实例都 registerCustomEvent 同名事件并启动 poll 线程，偶发崩溃。
    # 用 Windows 命名互斥体保证全局只有一个实例真正运行，第二个实例静默退出。
    try:
        kernel32 = ctypes.windll.kernel32
        _mutexHandle = kernel32.CreateMutexW(None, False, 'Global\\F3DFileOpener_SingleInstance_v1')
        if kernel32.GetLastError() == 183:  # ERROR_ALREADY_EXISTS
            _log('Another F3DFileOpener instance already running (named mutex exists). Exiting run() silently to avoid duplicate-instance crash.')
            return  # _isRunning remains False - stop() will skip cleanup
        _log('Acquired single-instance named mutex')
    except:
        _log('Named mutex creation failed (non-fatal, continuing): ' + traceback.format_exc())

    _isRunning = True  # This instance is the active one

    _log('========================================')
    _log('F3DFileOpener run() CALLED')
    _log('========================================')

    try:
        _app = adsk.core.Application.get()
        if not _app:
            _log('ERROR: Application.get() returned None')
            return
        _log('Got Application')

        _ui = _app.userInterface
        _log('Got UserInterface')

        # Register custom event - registerCustomEvent RETURNS the event object
        try:
            _customEvent = _app.registerCustomEvent(_customEventId)
            if _customEvent:
                _eventHandler = F3DImportEventHandler()
                _customEvent.add(_eventHandler)
                _log('Custom event registered: ' + _customEventId)
            else:
                _log('registerCustomEvent returned null')
        except:
            _log('Event registration error: ' + traceback.format_exc())

        # Create ready file
        try:
            with open(_readyFile, 'w') as f:
                f.write('ready')
            _log('Ready file created')
        except:
            _log('Failed to create ready file: ' + traceback.format_exc())

        # Check for pending request (handles startup case)
        _log('Checking for pending request...')
        result = _check_and_import()
        _log('Pending check result: ' + str(result))

        # Start background polling thread (handles "Fusion already running" case)
        _pollRunning = True
        _pollThread = threading.Thread(target=_poll_loop, daemon=True)
        _pollThread.start()
        _log('Background poll thread started')

        _log('run() COMPLETED SUCCESSFULLY')

    except:
        _log('FATAL ERROR in run(): ' + traceback.format_exc())


def stop(context):
    """Add-In cleanup."""
    global _pollRunning, _mutexHandle, _isRunning

    _log('F3DFileOpener stop() CALLED')

    # If this instance lost the mutex race (never actually ran), skip all cleanup
    # to avoid deleting the ready file or unregistering events owned by the active instance.
    if not _isRunning:
        _log('stop() - this instance was not the active one (mutex loser), skipping cleanup')
        return

    _pollRunning = False
    if _pollThread:
        try:
            _pollThread.join(timeout=5)
        except:
            pass

    try:
        if _customEvent and _eventHandler:
            _customEvent.remove(_eventHandler)
    except:
        pass

    try:
        if _app:
            _app.unregisterCustomEvent(_customEventId)
    except:
        pass

    try:
        if os.path.exists(_readyFile):
            os.remove(_readyFile)
    except:
        pass

    # 释放命名互斥体，允许下次启动时获取
    try:
        if _mutexHandle:
            ctypes.windll.kernel32.CloseHandle(_mutexHandle)
            _mutexHandle = None
    except:
        pass

    _log('stop() COMPLETED')
