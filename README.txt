Linux watchdog device API for Python
====================================

    This distribution contains implementation of Linux watchdog device API for
    Python (see Documentation/watchdog/watchdog-api.txt in Linux kernel source
    tree).

    The built-in type included in extension module provides wrapper methods for
    ioctl commands as well as simple write access to the device.


Installation
============

    The module can be installed using standard setuptools script:

        python setup.py install

Usage
=====

    Make sure your watchdog driver is loaded and /dev/watchdog exists and is
    writable. If you don't have a watchdog card, you can load the softdog
    driver which implements software watchdog timer:

        # modprobe softdog

    After that you should be able to import the module and open the watchdog
    device:

    >>> from watchdogdev import *
    >>> wdt = watchdog('/dev/watchdog')
    >>> wdt.get_support()
    >>> wdt.identity
    'Software Watchdog'
    >>> if wdt.options & WDIOF_MAGICCLOSE == WDIOF_MAGICCLOSE:
    ...     wdt.magic_close()
    ... 
    >>> 

    See help(watchdogdev) for details

