namespace nen {
enum class event_type {
  FIRSTEVENT = 0, /**< Unused (do not remove) */

  /* Application events */
  QUIT = 0x100, /**< User-requested quit */

  /* These application events have special meaning on iOS, see README-ios.md for
     details */
  APP_TERMINATING,         /**< The application is being terminated by the OS
                                    Called on iOS in applicationWillTerminate()
                                    Called on Android in onDestroy()
                               */
  APP_LOWMEMORY,           /**< The application is low on memory, free memory if
                                  possible.   Called on iOS in
                                  applicationDidReceiveMemoryWarning()   Called on Android
                                  in onLowMemory()
                               */
  APP_WILLENTERBACKGROUND, /**< The application is about to enter the
                                  background Called on iOS in
                                  applicationWillResignActive() Called on
                                  Android in onPause()
                              */
  APP_DIDENTERBACKGROUND,  /**< The application did enter the background and
                                  may not get CPU for some time  Called on iOS in
                                  applicationDidEnterBackground()  Called on
                                  Android in onPause()
                               */
  APP_WILLENTERFOREGROUND, /**< The application is about to enter the
                                  foreground Called on iOS in
                                  applicationWillEnterForeground() Called on
                                  Android in onResume()
                              */
  APP_DIDENTERFOREGROUND,  /**< The application is now interactive
                                    Called on iOS in applicationDidBecomeActive()
                                    Called on Android in onResume()
                               */

  LOCALECHANGED, /**< The user's locale preferences have changed. */

  /* Display events */
  DISPLAYEVENT = 0x150, /**< Display state change */

  /* Window events */
  WINDOWEVENT = 0x200, /**< Window state change */
  SYSWMEVENT,          /**< System specific event */

  /* Keyboard events */
  KEYDOWN = 0x300, /**< Key pressed */
  KEYUP,           /**< Key released */
  TEXTEDITING,     /**< Keyboard text editing (composition) */
  TEXTINPUT,       /**< Keyboard text input */
  KEYMAPCHANGED,   /**< Keymap changed due to a system event such as an
                            input language or keyboard layout change.
                       */

  /* Mouse events */
  MOUSEMOTION = 0x400, /**< Mouse moved */
  MOUSEBUTTONDOWN,     /**< Mouse button pressed */
  MOUSEBUTTONUP,       /**< Mouse button released */
  MOUSEWHEEL,          /**< Mouse wheel motion */

  /* Joystick events */
  JOYAXISMOTION = 0x600, /**< Joystick axis motion */
  JOYBALLMOTION,         /**< Joystick trackball motion */
  JOYHATMOTION,          /**< Joystick hat position change */
  JOYBUTTONDOWN,         /**< Joystick button pressed */
  JOYBUTTONUP,           /**< Joystick button released */
  JOYDEVICEADDED,   /**< A new joystick has been inserted into the system */
  JOYDEVICEREMOVED, /**< An opened joystick has been removed */

  /* Game controller events */
  CONTROLLERAXISMOTION = 0x650, /**< Game controller axis motion */
  CONTROLLERBUTTONDOWN,         /**< Game controller button pressed */
  CONTROLLERBUTTONUP,           /**< Game controller button released */
  CONTROLLERDEVICEADDED,    /**< A new Game controller has been inserted into
                                   the system */
  CONTROLLERDEVICEREMOVED,  /**< An opened Game controller has been removed
                             */
  CONTROLLERDEVICEREMAPPED, /**< The controller mapping was updated */
  CONTROLLERTOUCHPADDOWN,   /**< Game controller touchpad was touched */
  CONTROLLERTOUCHPADMOTION, /**< Game controller touchpad finger was moved
                             */
  CONTROLLERTOUCHPADUP,     /**< Game controller touchpad finger was lifted */
  CONTROLLERSENSORUPDATE,   /**< Game controller sensor was updated */

  /* Touch events */
  FINGERDOWN = 0x700,
  FINGERUP,
  FINGERMOTION,

  /* Gesture events */
  DOLLARGESTURE = 0x800,
  DOLLARRECORD,
  MULTIGESTURE,

  /* Clipboard events */
  CLIPBOARDUPDATE = 0x900, /**< The clipboard changed */

  /* Drag and drop events */
  DROPFILE = 0x1000, /**< The system requests a file open */
  DROPTEXT,          /**< text/plain drag-and-drop event */
  DROPBEGIN,         /**< A new set of drops is beginning (NULL filename) */
  DROPCOMPLETE, /**< Current set of drops is now complete (NULL filename) */

  /* Audio hotplug events */
  AUDIODEVICEADDED = 0x1100, /**< A new audio device is available */
  AUDIODEVICEREMOVED,        /**< An audio device has been removed. */

  /* Sensor events */
  SENSORUPDATE = 0x1200, /**< A sensor was updated */

  /* Render events */
  RENDER_TARGETS_RESET = 0x2000, /**< The render targets have been reset and
                                        their contents need to be updated */
  RENDER_DEVICE_RESET, /**< The device has been reset and all textures need
                              to be recreated */

  /* Internal events */
  POLLSENTINEL = 0x7F00, /**< Signals the end of an event poll cycle */

  /** Events ::USEREVENT through ::LASTEVENT are for your use,
   *  and should be allocated with RegisterEvents()
   */
  USEREVENT = 0x8000,

  /**
   *  This last event is only for bounding internal arrays
   */
  LASTEVENT = 0xFFFF

};
}