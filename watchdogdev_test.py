#!/usr/bin/env python

from watchdogdev import *

def test():
    wd = watchdog('/dev/watchdog')
    print 'fileno:', wd.fileno()
    wd.close()

    wd = watchdog('/dev/watchdog')

    print 'options:', wd.options
    assert(wd.options == None)
    print 'firmware_version:', wd.firmware_version
    assert(wd.firmware_version == None)
    print 'identity:', wd.identity
    assert(wd.identity == None)
    print

    wd.close()

    wd = watchdog('/dev/watchdog')

    wd.get_support()

    print 'options:', wd.options
    for name, value in globals().iteritems():
        if not name.startswith('WDIOF_'): 
            continue
        print '%s:' % name, wd.options & value == value

    print
    print 'firmware_version:', wd.firmware_version
    print 'identity:', wd.identity
    print

    wd.close()

    wd = watchdog('/dev/watchdog')

    print 'keep_alive:',
    try:
        print wd.keep_alive()
    except IOError, e:
        print e

    print 'get_status:',
    try:
        print wd.get_status()
    except IOError, e:
        print e

    print 'get_boot_status:',
    try:
        print wd.get_boot_status()
    except IOError, e:
        print e

    print 'get_temp:',
    try:
        print wd.get_temp()
    except IOError, e:
        print e

    timeout = 60
    print 'get_timeout:',
    try:
        timeout = wd.get_timeout()
        print timeout
    except IOError, e:
        print e

    print 'get_pretimeout:',
    try:
        print wd.get_pretimeout()
    except IOError, e:
        print e

    print 'get_time_left:',
    try:
        print wd.get_time_left()
    except IOError, e:
        print e

    print 'set_options:',
    try:
        for i in range(0, 65536):
            wd.set_options(i)
            assert(wd.get_options == i)
        print 'OK'
    except IOError, e:
        print e

    print 'set_timeout:',
    try:
        for i in range(1, 65536):
            wd.set_timeout(i)
            assert(wd.get_timeout() == i)
        wd.set_timeout(timeout)
        print 'OK'
    except IOError, e:
        print e

    print 'set_pretimeout:',
    try:
        for i in range(1, 65536):
            wd.set_pretimeout(i)
            assert(wd.get_pretimeout == i)
        wd.set_pretimeout(0)
        print 'OK'
    except IOError, e:
        print e

    print 'magic_close:',
    try:
        wd.magic_close()
        print 'OK'
    except IOError, e:
        print e


if __name__ == '__main__':
    test()
